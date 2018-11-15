#include "PriorityThread.h"
#include "util/EzLog.h"

/*
��������

Param :
const bool in_bIsUnorder : true ������������
false ������������

Return :
0 -- ����ɹ�
2 -- �������������������
-1 -- ����ʧ��
*/
int PriorityThread::AddTask(T_PTR_TaskPriorityBase& in_ptrTask)
{
	uint64_t uiCurrentTaskNum;

	// set thread queue pointer
	in_ptrTask->SetTaskQueue(&m_prtQueTask);

	int iRes = m_prtQueTask.AddTask(in_ptrTask, uiCurrentTaskNum, in_ptrTask->GetIsMatchOrdered());

	if (2 >= uiCurrentTaskNum)
	{
		// �����ǰ��������̫�٣��п������̻߳��ڵȴ�Event
		SetThreadEvent();
	}

	return iRes;
}

/*
��ȡ��ǰ�������

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int PriorityThread::GetTaskAssignInfo(std::string& out_strTaskInfo)
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
int PriorityThread::StartThread(void)
{
	// static const std::string ftag("PipeLine_NewThread::StartThread() ");

	int iRes = ThreadBase::StartThread(&PriorityThread::Run);

	return iRes;
}

#ifdef _MSC_VER
DWORD WINAPI PriorityThread::Run(LPVOID  pParam)
{
	static const std::string ftag("PriorityThread::Run() ");

	PriorityThread* pThread = static_cast<PriorityThread*>(pParam);

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

		//long long lastTime = 0;
		//TimeDuration dra;

		int iRes = 0;
		while ((0 == iRes) &&
			((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
			(ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp)))
		{
			// ȡ��������
			std::shared_ptr<TaskPriorityBase> out_ptrTask;
			iRes = pThread->m_prtQueTask.GetTask(out_ptrTask);
			if (0 == iRes)
			{
				//dra.Begin();
				//lastTime = 0;

				if (NULL == out_ptrTask)
				{
					continue;
				}

				out_ptrTask->TaskProc();

				//dra.End_Us(lastTime );
				//if ( !EzLog::LogLvlFilter( debug ) )
				//{
				//	std::string strTrans;
				//	std::string strOutput( "Proc Time=" );
				//	strOutput += sof_string::itostr( lastTime, strTrans );
				//	EzLog::i( ftag, strOutput );
				//}
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
void* PriorityThread::Run(void*  pParam)
{
	static const std::string ftag("PriorityThread::Run() ");

	PriorityThread* pThread = static_cast<PriorityThread*>(pParam);

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
			std::shared_ptr<TaskPriorityBase> out_ptrTask;
			iRes = pThread->m_prtQueTask.GetTask(out_ptrTask);
			if (0 == iRes)
			{
				//dra.Begin();
				//lastTime = 0;

				if (NULL == out_ptrTask)
				{
					continue;
				}

				out_ptrTask->TaskProc();

				//dra.End_Us(lastTime );
				//if ( !EzLog::LogLvlFilter( debug ) )
				//{
				//	std::string strTrans;
				//	std::string strOutput( "Proc Time=" );
				//	strOutput += sof_string::itostr( lastTime, strTrans );
				//	EzLog::i( ftag, strOutput );
				//}
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
