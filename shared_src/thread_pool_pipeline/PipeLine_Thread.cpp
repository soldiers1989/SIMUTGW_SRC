#include "PipeLine_Thread.h"

#include "util/EzLog.h"

PipeLine_Thread::PipeLine_Thread(void)
	: ThreadBase()
{
}

PipeLine_Thread::~PipeLine_Thread(void)
{
}


/*
��������������Ƿ�����HashKey��ͬ������������У��Ͳ����������û�У��򲻲���

Param :
unsigned int& out_CurrentTaskNum : ��ǰ���߳�������������

Return :
0 -- ����ͬ�����񣬲��Ҳ���ɹ�
1 -- ����ͬ������
2 -- ����ͬ�����񣬲���ʧ��
*/
int PipeLine_Thread::AddIfHaveTask(std::shared_ptr<TaskBase>& ptr_task,
	uint64_t& out_CurrentTaskNum)
{
	int iRes = m_prtQueTask.AddIfHaveTask(ptr_task, out_CurrentTaskNum);

	return iRes;
}

/*
��������

Return :
0 -- ����ɹ�
-1 -- ����ʧ��
*/
int PipeLine_Thread::AddTask(std::shared_ptr<TaskBase>& ptr_task)
{
	uint64_t uiCurrentTaskNum;
	int iRes = m_prtQueTask.AddTask(ptr_task, uiCurrentTaskNum);

	if (2 >= uiCurrentTaskNum)
	{
		// �����ǰ��������̫�٣��п������̻߳��ڵȴ�Event
		SetThreadEvent();
	}

	return iRes;
}


/*
��ȡ����

Return :
0 -- ����ɹ�
-1 -- ����ʧ��
*/
int PipeLine_Thread::GetTask(std::shared_ptr<TaskBase>& ptr_task)
{
	int iRes = m_prtQueTask.GetTask(ptr_task);

	return iRes;
}

/*
��ȡ��ǰ�������

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int PipeLine_Thread::GetTaskAssignInfo(std::string& out_strTaskInfo)
{
	int iRes = m_prtQueTask.GetTaskAssignInfo(out_strTaskInfo);

	return iRes;
}

/*
�����߳�

Return :
0 -- �����ɹ�
-1 -- ����ʧ��
*/
int PipeLine_Thread::StartThread(void)
{
	// static const std::string ftag("PipeLine_Thread::StartThread() ");

	int iRes = ThreadBase::StartThread(&PipeLine_Thread::Run);

	return iRes;
}

#ifdef _MSC_VER
DWORD WINAPI PipeLine_Thread::Run(LPVOID  pParam)
{
	static const std::string ftag("PipeLine_Thread::Run() ");

	PipeLine_Thread* pThread = static_cast<PipeLine_Thread*>(pParam);

	pThread->m_prtQueTask.SetQueueId(pThread->m_dThreadId);

	while ((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
		(ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp))
	{
		//�ȴ��߳��źţ���ʱ
		if (INVALID_HANDLE_VALUE != pThread->m_hEvent)
		{
			WaitForSingleObject(pThread->m_hEvent, ThreadPool_Conf::g_dwWaitMS_Event);
		}
		else
		{
			EzLog::e(ftag, "m_hEvent INVALID_HANDLE_VALUE");
		}

		int iRes = 0;
		while ((0 == iRes) &&
			((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
			(ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp)))
		{
			// ȡ��������
			std::shared_ptr<TaskBase> out_ptrTask;
			iRes = pThread->m_prtQueTask.GetTask(out_ptrTask);
			if (0 == iRes)
			{
				if (NULL == out_ptrTask)
				{
					continue;
				}

				// Run task
				out_ptrTask->TaskProc();
			}
		}

		if (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp)
		{
			// ��������ϣ��˳�
			break;
		}

	}

	/*
	if (!EzLog::LogLvlFilter(trace))
	{
	string strTran;
	string strDebug("thread finished id=");
	strDebug += sof_string::itostr((uint64_t)pThread->m_dThreadId, strTran);
	EzLog::Out(ftag, trace, strDebug );
	}
	*/

	pThread->m_emThreadState = ThreadPool_Conf::STOPPED;
	pThread->m_dThreadId = 0;

	return 0;
}
#else
void* PipeLine_Thread::Run(void* pParam)
{
	static const std::string ftag("PipeLine_Thread::Run() ");

	PipeLine_Thread* pThread = static_cast<PipeLine_Thread*>(pParam);

	pThread->m_prtQueTask.SetQueueId(pThread->m_dThreadId);

	while ((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
		(ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp))
	{
		//�ȴ��߳��źţ���ʱ		
		pthread_mutex_lock(&pThread->m_mutexCond);

		// The  pthread_cond_timedwait()   function   shall   be   equivalent   to
		// pthread_cond_wait(),  except  that an error is returned if the absolute
		// time specified by abstime  passes  (that  is,  system  time  equals  or
		// exceeds  abstime) before the condition cond is signaled or broadcasted,
		// or if the absolute time specified by abstime has already been passed at
		// the time of the call.
		struct timeval now;
		struct timespec tmRestrict;

		gettimeofday(&now, NULL);

		tmRestrict.tv_sec = now.tv_sec;
		tmRestrict.tv_nsec = now.tv_usec * 1000 + ThreadPool_Conf::g_dwWaitMS_Event * 1000L;
		pthread_cond_timedwait(&pThread->m_condEvent, &pThread->m_mutexCond, &tmRestrict);

		pthread_mutex_unlock(&pThread->m_mutexCond);

		int iRes = 0;
		while ((0 == iRes) &&
			((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
			(ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp)))
		{
			// ȡ��������
			std::shared_ptr<TaskBase> out_ptrTask;
			iRes = pThread->m_prtQueTask.GetTask(out_ptrTask);
			if (0 == iRes)
			{
				if (NULL == out_ptrTask)
				{
					continue;
				}

				out_ptrTask->TaskProc();
			}
		}

		if (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp)
		{
			// ��������ϣ��˳�
			break;
		}

	}

	/*
	if (!EzLog::LogLvlFilter(trace))
	{
	string strTran;
	string strDebug("thread finished id=");
	strDebug += sof_string::itostr((uint64_t)pThread->m_dThreadId, strTran);
	EzLog::Out(ftag, trace, strDebug );
	}
	*/

	pThread->m_emThreadState = ThreadPool_Conf::STOPPED;
	pThread->m_dThreadId = 0;

	return 0;
}
#endif

