#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <vector>
#include <memory>

#include "ThreadBase.h"

#include "util/EzLog.h"

/*
线程池
*/
template<typename T>
class ThreadPoolBase
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// 最大允许的线程数量
	unsigned int m_uiThreadMaxNums;

	// 现在已有线程数量
	unsigned int m_uiCurrThreadNums;

	// 已有线程的句柄
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
	启动线程池

	Return :
	0 -- 成功
	-1 -- 失败
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
	关闭线程池

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	virtual int StopPool(void)
	{
		const string ftag("ThreadPoolBase::StopPool() ");

		if ( 0 == m_hdThreads.size() )
		{
			m_pool_state = ThreadPool_Conf::STOPPED;
			return 0;
		}

		// 先通知关闭
		typename std::vector<std::shared_ptr<T>>::iterator it = m_hdThreads.begin();
		for ( it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it )
		{
			( *it )->NotifyStopThread_Immediate();
		}

		{
			string strDebug("Notified all thread, wait a moment");
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strDebug;
		}

		// Sleep给线程退出时间
#ifdef _MSC_VER
		Sleep(ThreadPool_Conf::g_dwWaitMS_ThreadExit);
#else
		usleep(ThreadPool_Conf::g_dwWaitMS_ThreadExit * 1000L);
#endif

		// 确认并强行关闭
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
	等待所有的任务都完成，再关闭线程池

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	virtual int StopPool_WaitAllFinish(void)
	{
		const string ftag("ThreadPoolBase::StopPool_WaitAllFinish() ");

		// 先通知关闭
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
	// 阻止使用默认构造函数
	ThreadPoolBase(void);
	};

#endif