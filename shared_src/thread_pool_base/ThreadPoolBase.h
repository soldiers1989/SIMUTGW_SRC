#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <vector>
#include <memory>

#include "ThreadBase.h"

#include "util/EzLog.h"

/*
�̳߳�
*/
template<typename T>
class ThreadPoolBase
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// ���������߳�����
	unsigned int m_uiThreadMaxNums;

	// ���������߳�����
	unsigned int m_uiCurrThreadNums;

	// �����̵߳ľ��
	std::vector<std::shared_ptr<T>> m_hdThreads;

	volatile enum ThreadPool_Conf::ThreadState m_pool_state;

	//
	// Functions
	//
public:
	explicit ThreadPoolBase(const unsigned int uiNum)
		: m_scl(keywords::channel = "ThreadPoolBase"),
		m_uiThreadMaxNums(uiNum), m_uiCurrThreadNums(0),
		m_pool_state(ThreadPool_Conf::STOPPED)
	{
	}

	virtual ~ThreadPoolBase(void)
	{
		StopPool();
	}

	unsigned int GetCurrentThreadNum(void) const
	{
		return m_uiCurrThreadNums;
	}


	/*
	�����̳߳�

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	virtual int StartPool(void)
	{
		const string ftag("ThreadPoolBase::StartPool() ");

		m_pool_state = ThreadPool_Conf::STARTED;

		typename std::vector<std::shared_ptr<T>>::iterator it = m_hdThreads.begin();
		for ( it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it )
		{
			int iRes = ( *it )->StartThread();
			if ( 0 != iRes )
			{
				EzLog::e(ftag, "start error!");
				return -1;
			}
		}

#ifdef _MSC_VER
		Sleep(ThreadPool_Conf::g_dwWaitMS_AfterStartThread);
#else
		usleep(ThreadPool_Conf::g_dwWaitMS_AfterStartThread * 1000L);
#endif

		return 0;
	}

	/*
	�ر��̳߳�

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	virtual int StopPool(void)
	{
		const string ftag("ThreadPoolBase::StopPool() ");

		if ( 0 == m_hdThreads.size() )
		{
			m_pool_state = ThreadPool_Conf::STOPPED;
			return 0;
		}

		// ��֪ͨ�ر�
		typename std::vector<std::shared_ptr<T>>::iterator it = m_hdThreads.begin();
		for ( it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it )
		{
			( *it )->NotifyStopThread_Immediate();
		}

		{
			string strDebug("Notified all thread, wait a moment");
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strDebug;
		}

		// Sleep���߳��˳�ʱ��
#ifdef _MSC_VER
		Sleep(ThreadPool_Conf::g_dwWaitMS_ThreadExit);
#else
		usleep(ThreadPool_Conf::g_dwWaitMS_ThreadExit * 1000L);
#endif

		// ȷ�ϲ�ǿ�йر�
		it = m_hdThreads.begin();
		while ( it != m_hdThreads.end() )
		{
			( *it )->StopThread();
			++it;
		}

		m_hdThreads.clear();

		m_pool_state = ThreadPool_Conf::STOPPED;

		return 0;
		}

	/*
	�ȴ����е�������ɣ��ٹر��̳߳�

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	virtual int StopPool_WaitAllFinish(void)
	{
		const string ftag("ThreadPoolBase::StopPool_WaitAllFinish() ");

		// ��֪ͨ�ر�
		typename std::vector<std::shared_ptr<T>>::iterator it = m_hdThreads.begin();
		for ( it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it )
		{
			( *it )->NotifyStopThread_TillTaskFinish();
		}

		{
			string strDebug("Notified all thread, wait a moment");
			BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << strDebug;
		}

		for ( it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it )
		{
			( *it )->WaitStopThread();
		}

		m_hdThreads.clear();

		m_pool_state = ThreadPool_Conf::STOPPED;

		return 0;
	}

private:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	ThreadPoolBase(void);
	};

#endif