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
	�����̳߳ض���

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
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
	��������

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int AssignTask(std::shared_ptr<TaskBase>& ptr_task)
	{
		static const string ftag("PipeLine_ThreadPool::AssignTask() ");

		int iRes = 0;
		int i = 0;

		// ����������߳̾��
		std::vector<std::shared_ptr<PipeLine_Thread>>::iterator itFewerTask;
		// ����������߳�������
		uint64_t uiFewerTaskNum = 0;

		// �Ȳ����Ƿ���Ը���������ʷ����ֱ�Ӳ���
		std::vector<std::shared_ptr<PipeLine_Thread>>::iterator it = m_hdThreads.begin();
		for (i = 0, it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it, ++i)
		{
			uint64_t uiTaskNum = 0;
			iRes = (*it)->AddIfHaveTask(ptr_task, uiTaskNum);
			if (0 == iRes)
			{
				// 0 -- ����ͬ�����񣬲��Ҳ���ɹ�
				return 0;
			}
			else if (2 == iRes)
			{
				// 2 -- ����ͬ�����񣬲���ʧ��
				if (0 == i)
				{
					// �״����⴦��
					uiFewerTaskNum = uiTaskNum;
					itFewerTask = it;
				}

				continue;
			}
			else if (1 == iRes)
			{
				// 1 -- ����ͬ������

				if (0 == i)
				{
					// �״����⴦��
					uiFewerTaskNum = uiTaskNum;
					itFewerTask = it;
				}
				else
				{
					if (uiTaskNum < uiFewerTaskNum)
					{
						// ���̱߳�֮ǰ������������
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

		// δ��������ͬ���̣߳��������������ٶ�����
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
	��ȡ��ǰ�������

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
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
	// ��ֹʹ��Ĭ�Ϲ��캯��
	PipeLine_ThreadPool(void);
};

#endif