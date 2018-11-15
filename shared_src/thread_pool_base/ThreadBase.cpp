#include "ThreadBase.h"

#include <exception>

#include "util/EzLog.h"

ThreadBase::ThreadBase(void)
	: m_scl(keywords::channel = "ThreadBase"),
	m_dThreadId(0),
#ifdef _MSC_VER
	m_hThread(INVALID_HANDLE_VALUE),
	m_hEvent(INVALID_HANDLE_VALUE),
#else
	m_hThread(0),
	m_mutexCond(PTHREAD_MUTEX_INITIALIZER),
	m_condEvent(PTHREAD_COND_INITIALIZER),
#endif
	m_emThreadState(ThreadPool_Conf::STOPPED),
	m_emThreadRunOp(ThreadPool_Conf::Running)
{
	/*
	m_hEvent = CreateEvent(NULL,false,false,NULL);
	if( INVALID_HANDLE_VALUE == m_hEvent )
	{
	std::exception e("CreateEvent error");
	throw e;
	}
	*/
}

ThreadBase::ThreadBase(const uint64_t uiId)
	: m_scl(keywords::channel = "ThreadBase"),
	m_dThreadId((unsigned long)uiId),
#ifdef _MSC_VER
	m_hThread(INVALID_HANDLE_VALUE),
	m_hEvent(INVALID_HANDLE_VALUE),
#else
	m_hThread(0),
	m_mutexCond(PTHREAD_MUTEX_INITIALIZER),
	m_condEvent(PTHREAD_COND_INITIALIZER),
#endif
	m_emThreadState(ThreadPool_Conf::STOPPED),
	m_emThreadRunOp(ThreadPool_Conf::Running)
{
	/*
	m_hEvent = CreateEvent(NULL,false,false,NULL);
	if( INVALID_HANDLE_VALUE == m_hEvent )
	{
	std::exception e("CreateEvent error");
	throw e;
	}
	*/
}

ThreadBase::~ThreadBase(void)
{
#ifdef _MSC_VER
	CloseHandle(m_hEvent);
	m_hEvent = INVALID_HANDLE_VALUE;
#else
	pthread_mutex_destroy(&m_mutexCond);
	pthread_cond_destroy(&m_condEvent);
#endif

}

/*
�����߳�

Return :
0 -- �����ɹ�
-1 -- ����ʧ��
*/
#ifdef _MSC_VER
int ThreadBase::StartThread(LPTHREAD_START_ROUTINE lpFunc)
{
	const std::string ftag("ThreadBase::StartThread() ");

	if (INVALID_HANDLE_VALUE == m_hEvent)
	{
		m_hEvent = CreateEvent(NULL, false, true, NULL);
		if (INVALID_HANDLE_VALUE == m_hEvent)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CreateEvent error";
			std::exception e("CreateEvent error");
			throw e;
		}
	}

	if (INVALID_HANDLE_VALUE == m_hThread)
	{
		m_emThreadState = ThreadPool_Conf::STARTED;
		m_emThreadRunOp = ThreadPool_Conf::Running;

		m_hThread = CreateThread(NULL, 0, lpFunc, this, 0, &m_dThreadId);

		if (INVALID_HANDLE_VALUE == m_hThread)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CreateThread error INVALID_HANDLE_VALUE";
			return -1;
		}

		/*
		if (!EzLog::LogLvlFilter(debug))
		{
		string strTran;
		string strDebug("thread started id=");
		strDebug += sof_string::itostr((uint64_t)m_dThreadId, strTran);
		EzLog::Out(ftag, debug, strDebug );
		}
		*/

		return 0;
	}
	else
	{
		return 1;
	}

	return 1;
}
#else
int ThreadBase::StartThread(PTRFUN lpFunc)
{
	const std::string ftag("ThreadBase::StartThread() ");

	if (0 == m_hThread)
	{
		m_emThreadState = ThreadPool_Conf::STARTED;
		m_emThreadRunOp = ThreadPool_Conf::Running;

		int iRes = pthread_create(&m_hThread, NULL, lpFunc, this);
		if (0 != iRes)
		{
			// reset
			m_hThread = 0;

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "pthread_create errorno=" << iRes;
			std::runtime_error e("pthread_create error");
			throw e;
		}

		return 0;
	}
	else
	{
		return 1;
	}

	return 1;
}
#endif

/*
֪ͨ�ر��̣߳����̹ر�

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int ThreadBase::NotifyStopThread_Immediate(void)
{
	m_emThreadRunOp = ThreadPool_Conf::Immediate;

	SetThreadEvent();

	return 0;
}

/*
֪ͨ�ر��̣߳�������ɺ�ر�

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int ThreadBase::NotifyStopThread_TillTaskFinish(void)
{
	m_emThreadRunOp = ThreadPool_Conf::TillTaskFinish;

	SetThreadEvent();

	return 0;
}

/*
�ر��̣߳��ȴ����е��������

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
#ifdef _MSC_VER
int ThreadBase::WaitStopThread(void)
{
	// const std::string ftag("ThreadBase::WaitStopThread() ");

	if (ThreadPool_Conf::STOPPED == m_emThreadState)
	{
		return 0;
	}

	if (ThreadPool_Conf::Running == m_emThreadRunOp)
	{
		// ����˳�״̬δ�裬������֪ͨ
		m_emThreadRunOp = ThreadPool_Conf::TillTaskFinish;

		SetThreadEvent();
	}

	Sleep(ThreadPool_Conf::g_dwWaitMS_ThreadExit);

	if (ThreadPool_Conf::STOPPED != m_emThreadState)
	{
		// �ȴ��̷߳���
		if (INVALID_HANDLE_VALUE != m_hThread)
		{
			WaitForSingleObject(m_hThread, INFINITE);

			CloseHandle(m_hThread);
			m_hThread = INVALID_HANDLE_VALUE;
			m_emThreadState = ThreadPool_Conf::STOPPED;
			m_dThreadId = 0;
		}
	}

	return 0;
}
#else
int ThreadBase::WaitStopThread(void)
{
	const std::string ftag("ThreadBase::WaitStopThread() ");

	if (ThreadPool_Conf::STOPPED == m_emThreadState)
	{
		return 0;
	}

	if (ThreadPool_Conf::Running == m_emThreadRunOp)
	{
		// ����˳�״̬δ�裬������֪ͨ
		m_emThreadRunOp = ThreadPool_Conf::TillTaskFinish;

		SetThreadEvent();
	}

	usleep(ThreadPool_Conf::g_dwWaitMS_ThreadExit * 1000);

	if (ThreadPool_Conf::STOPPED != m_emThreadState)
	{
		// �ȴ��̷߳���
		if (0 != m_hThread)
		{
			int iRes = pthread_join(m_hThread, NULL);
			if (0 != iRes)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "pthread_join errorno=" << iRes;

				return -1;
			}

			m_hThread = 0;

			m_emThreadState = ThreadPool_Conf::STOPPED;
			m_dThreadId = 0;
		}
	}

	return 0;
}
#endif


/*
�ر��̣߳�����б�Ҫ����ǿ�ƹر�

Param :
bool bForceClose :
true -- ǿ�ƹر�
ʹ�߳��˳����ʹ�̵߳�Whileѭ��break��������Ҫʹ��TerminateThread��TerminateThread����ɺܶ�δ֪����
false -- ��ǿ�ƹر�

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
#ifdef _MSC_VER
int ThreadBase::StopThread(bool bForceClose)
{
	const std::string ftag("ThreadBase::StopThread() ");

	if (ThreadPool_Conf::STOPPED == m_emThreadState)
	{
		return 0;
	}

	if (ThreadPool_Conf::Running != m_emThreadRunOp)
	{
		// ����˳�״̬���裬��ǿ���˳�
		if (ThreadPool_Conf::STOPPED != m_emThreadState && bForceClose)
		{
			TerminateThread(m_hThread, 9);

			{
				std::string strTran;
				string strDebug("thread Terminated id=");
				strDebug += sof_string::itostr((uint64_t)m_dThreadId, strTran);
				BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << strDebug;
			}

			m_hThread = INVALID_HANDLE_VALUE;
			m_emThreadState = ThreadPool_Conf::STOPPED;
			m_dThreadId = 0;
		}
	}
	else
	{
		// ����˳�״̬δ�裬������֪ͨ
		m_emThreadRunOp = ThreadPool_Conf::Immediate;

		SetThreadEvent();

		//�ȴ��̷߳���
		DWORD dwWait = WaitForSingleObject(m_hThread, ThreadPool_Conf::g_dwWaitMS_ThreadExit);
		if (WAIT_TIMEOUT == dwWait && bForceClose)
		{
			TerminateThread(m_hThread, 9);

			{
				std::string strTran;
				string strDebug("thread Terminated id=");
				strDebug += sof_string::itostr((uint64_t)m_dThreadId, strTran);
				BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << strDebug;
			}

			m_hThread = INVALID_HANDLE_VALUE;
			m_emThreadState = ThreadPool_Conf::STOPPED;
			m_dThreadId = 0;
		}
	}

	return 0;
}
#else
int ThreadBase::StopThread(bool bForceClose)
{
	const std::string ftag("ThreadBase::StopThread() ");

	if (ThreadPool_Conf::STOPPED == m_emThreadState)
	{
		return 0;
	}

	if (ThreadPool_Conf::Running != m_emThreadRunOp)
	{
		// ����˳�״̬���裬��ǿ���˳�
		if (ThreadPool_Conf::STOPPED != m_emThreadState && bForceClose)
		{
			int iRes = pthread_cancel(m_hThread);
			if (0 != iRes)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "pthread_cancel in tid=" << m_dThreadId << " errno=" << iRes;
			}

			void* result;
			/* Join with thread to see what its exit status was */
			iRes = pthread_join(m_hThread, &result);
			if (0 != iRes)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "pthread_join in tid=" << m_dThreadId << " errno=" << iRes;
			}

			if (PTHREAD_CANCELED == result)
			{
				BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << "thread was canceled id=" << m_dThreadId;
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "thread wasn't canceled (shouldn't happen!) tid=" << m_dThreadId << " res=" << iRes;
				exit(-1);
			}

			m_hThread = 0;
			m_emThreadState = ThreadPool_Conf::STOPPED;
			m_dThreadId = 0;
		}
	}
	else
	{
		// ����˳�״̬δ�裬������֪ͨ
		m_emThreadRunOp = ThreadPool_Conf::Immediate;

		SetThreadEvent();

		struct timespec ts;
		//�ȴ��̷߳���
		ts.tv_sec = 0;
		ts.tv_nsec = ThreadPool_Conf::g_dwWaitMS_ThreadExit * 1000L;

		int iRes = pthread_timedjoin_np(m_hThread, NULL, &ts);
		if (0 != iRes && bForceClose)
		{
			iRes = pthread_cancel(m_hThread);
			if (0 != iRes)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "pthread_cancel in tid=" << m_dThreadId << " errno=" << iRes;
			}

			void* result;
			/* Join with thread to see what its exit status was */
			iRes = pthread_join(m_hThread, &result);
			if (0 != iRes)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "pthread_join in tid=" << m_dThreadId << " errno=" << iRes;
			}

			if (PTHREAD_CANCELED == result)
			{
				BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << "thread was canceled id=" << m_dThreadId;
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "thread wasn't canceled (shouldn't happen!) tid=" << m_dThreadId << " result=" << result;
				exit(-1);
			}

			/* Free memory allocated by thread */
			free(result);

			m_hThread = 0;
			m_emThreadState = ThreadPool_Conf::STOPPED;
			m_dThreadId = 0;
		}
	}

	return 0;
}
#endif


//����Ϣ��֪ͨ�̴߳���
bool ThreadBase::SetThreadEvent(void)
{
#ifdef _MSC_VER
	if (INVALID_HANDLE_VALUE != m_hEvent)
	{
		SetEvent(m_hEvent);
	}
#else
	pthread_cond_signal(&m_condEvent);
#endif

	return true;
}
