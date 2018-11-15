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
	�����̳߳ض���

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	virtual int InitPool(void);

	/*
	��������,����key������

	Param :

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int AssignTaskWithKey(T_PTR_TaskPriorityBase& ptr_task)
	{
		static const string ftag("Priority_ThreadPool::AssignTaskWithKey() ");

		int iRes = 0;

		// ��task��key���Ե�ǰ�߳�����ȡ�࣬�õ����䵽�ĸ��߳�
		int iMod = ptr_task->GetKey() % m_uiCurrThreadNums;

		// ���±����
		iRes = m_hdThreads[iMod]->AddTask(ptr_task);
		if (0 == iRes)
		{
			// 0 -- ����ɹ�
			return 0;
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "AddTask() failed, res=-1";
			return -1;
		}
	}

	/*
	�������� ȡ��С�Ķ���

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int AssignTaskInMini(T_PTR_TaskPriorityBase& ptr_task)
	{
		static const string ftag("PipeLine_ThreadPool::AssignTaskInMini() ");

		int iRes = 0;
		int i = 0;

		// ����������߳̾��
		std::vector<std::shared_ptr<PriorityThread>>::iterator itFewerTask;
		// ����������߳�������
		uint64_t uiFewerTaskNum = UINT_MAX;

		// �Ȳ�����С��������в���
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

		// �������������ٶ�����
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
	��ȡ��ǰ�������

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
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
	// ��ֹʹ��Ĭ�Ϲ��캯��
	PriorityThreadPool(void);
};

#endif