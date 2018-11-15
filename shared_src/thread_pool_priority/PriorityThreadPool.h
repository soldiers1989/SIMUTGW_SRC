#ifndef __PRIORITY_THREADPOOL_H__
#define __PRIORITY_THREADPOOL_H__

#include <vector>

#include "thread_pool_base/ThreadPoolBase.h"
#include "PriorityThread.h"
#include "TaskPriorityBase.h"

#include "util/EzLog.h"
#include "util/TimeDuration.h"

class PriorityThreadPool :
	public ThreadPoolBase<PriorityThread>
{
	//
	// member
	//
private:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	//
	// function
	//
public:
	explicit PriorityThreadPool(const unsigned int uiNum)
		: ThreadPoolBase(uiNum),
		m_scl(keywords::channel = "PriorityThreadPool")
	{
	}

	virtual ~PriorityThreadPool(void)
	{
	}

	/*
	创建线程池对象

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	virtual int InitPool(void);

	/*
	分配任务,根据key来插入

	Param :

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int AssignTaskWithKey(T_PTR_TaskPriorityBase& ptr_task)
	{
		static const string ftag("Priority_ThreadPool::AssignTaskWithKey() ");

		int iRes = 0;

		// 按task的key，对当前线程数量取余，得到分配到哪个线程
		int iMod = ptr_task->GetKey() % m_uiCurrThreadNums;

		// 按下标插入
		iRes = m_hdThreads[iMod]->AddTask(ptr_task);
		if (0 == iRes)
		{
			// 0 -- 插入成功
			return 0;
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "AddTask() failed, res=-1";
			return -1;
		}
	}

	/*
	分配任务 取最小的队列

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int AssignTaskInMini(T_PTR_TaskPriorityBase& ptr_task)
	{
		static const string ftag("PipeLine_ThreadPool::AssignTaskInMini() ");

		int iRes = 0;
		int i = 0;

		// 最少任务的线程句柄
		std::vector<std::shared_ptr<PriorityThread>>::iterator itFewerTask;
		// 最少任务的线程任务量
		uint64_t uiFewerTaskNum = UINT_MAX;

		// 先查找最小的任务队列插入
		std::vector<std::shared_ptr<PriorityThread>>::iterator it = m_hdThreads.begin();
		for (i = 0, it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it, ++i)
		{
			uint64_t uiTaskNum = 0;
			uiTaskNum = (*it)->GetQueueSize();

			if (0 == uiTaskNum)
			{
				itFewerTask = it;
				break;
			}

			if (uiTaskNum < uiFewerTaskNum)
			{
				uiFewerTaskNum = uiTaskNum;
				itFewerTask = it;
			}
		}

		// 插入至任务最少队列中
		iRes = (*itFewerTask)->AddTask(ptr_task);
		if (0 == iRes)
		{
			return 0;
		}
		else
		{
			string strTran;
			string strDebug("AddTask() failed, res=");
			strDebug += sof_string::itostr(iRes, strTran);
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
			return -1;
		}

	}

	/*
	获取当前任务情况

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int GetTaskAssignInfo(std::string& out_strTaskInfo)
	{
		static const std::string ftag("ThreadPool::AssignTask() ");

		int iRes = 0;

		std::vector<std::shared_ptr<PriorityThread>>::const_iterator it = m_hdThreads.begin();
		for (it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it)
		{
			std::string strThreadTasks;
			iRes = (*it)->GetTaskAssignInfo(strThreadTasks);
			if (0 == iRes)
			{
				//
				out_strTaskInfo += strThreadTasks;
				out_strTaskInfo += " ";
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetTaskAssignInfo() failed, res=-1";
				return -1;
			}
		}

		return 0;
	}

private:
	// 阻止使用默认构造函数
	PriorityThreadPool(void);
};

#endif