#ifndef __PIPELINE_THREADPOOL_H__
#define __PIPELINE_THREADPOOL_H__

#include <vector>

#include "thread_pool_base/TaskBase.h"
#include "thread_pool_base/ThreadPoolBase.h"
#include "PipeLine_Thread.h"

#include "util/EzLog.h"

class PipeLine_ThreadPool :
	public ThreadPoolBase<PipeLine_Thread>
{
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

public:
	PipeLine_ThreadPool(const unsigned int uiNum)
		: ThreadPoolBase(uiNum),
		m_scl(keywords::channel = "PipeLine_ThreadPool")
	{
	}

	virtual ~PipeLine_ThreadPool(void)
	{
	}

	/*
	创建线程池对象

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	virtual int InitPool(void)
	{
		static const string ftag("PipeLine_ThreadPool::InitPool() ");

		m_pool_state = ThreadPool_Conf::STOPPED;

		unsigned int i = 0;
		for (i = 0; i < m_uiThreadMaxNums; ++i)
		{
			std::shared_ptr<PipeLine_Thread> ptrThread = std::shared_ptr<PipeLine_Thread>(new PipeLine_Thread());
			if (NULL == ptrThread)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new ThreadBase() failed, NULL";

				return -1;
			}

			m_hdThreads.push_back(ptrThread);

			++m_uiCurrThreadNums;
		}

		return 0;
	}

	/*
	分配任务

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int AssignTask(std::shared_ptr<TaskBase>& ptr_task)
	{
		static const string ftag("PipeLine_ThreadPool::AssignTask() ");

		int iRes = 0;
		int i = 0;

		// 最少任务的线程句柄
		std::vector<std::shared_ptr<PipeLine_Thread>>::iterator itFewerTask;
		// 最少任务的线程任务量
		uint64_t uiFewerTaskNum = 0;

		// 先查找是否可以根据已有历史任务直接插入
		std::vector<std::shared_ptr<PipeLine_Thread>>::iterator it = m_hdThreads.begin();
		for (i = 0, it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it, ++i)
		{
			uint64_t uiTaskNum = 0;
			iRes = (*it)->AddIfHaveTask(ptr_task, uiTaskNum);
			if (0 == iRes)
			{
				// 0 -- 有相同的任务，并且插入成功
				return 0;
			}
			else if (2 == iRes)
			{
				// 2 -- 有相同的任务，插入失败
				if (0 == i)
				{
					// 首次特殊处理
					uiFewerTaskNum = uiTaskNum;
					itFewerTask = it;
				}

				continue;
			}
			else if (1 == iRes)
			{
				// 1 -- 无相同的任务

				if (0 == i)
				{
					// 首次特殊处理
					uiFewerTaskNum = uiTaskNum;
					itFewerTask = it;
				}
				else
				{
					if (uiTaskNum < uiFewerTaskNum)
					{
						// 此线程比之前的任务数更少
						uiFewerTaskNum = uiTaskNum;
						itFewerTask = it;
					}
				}

				continue;
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "AddIfHaveTask() failed, res=-1";
				return -1;
			}
		}

		// 未发现有相同的线程，插入至任务最少队列中
		iRes = (*itFewerTask)->AddTask(ptr_task);
		if (0 == iRes)
		{
			return 0;
		}
		else
		{
			string strTran;
			string strDebug("AddIfHaveTask() failed, res=");
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

		std::vector<std::shared_ptr<PipeLine_Thread>>::const_iterator it = m_hdThreads.begin();
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
	PipeLine_ThreadPool(void);
};

#endif