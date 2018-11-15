#ifndef __LOOP_WORKER_THREADPOOL_H__
#define __LOOP_WORKER_THREADPOOL_H__

#include <vector>

#include "thread_pool_base/ThreadPoolBase.h"
#include "LoopWorker_Thread.h"
#include "thread_pool_base/TaskBase.h"

#include "util/EzLog.h"

class LoopWorker_ThreadPool :
	public ThreadPoolBase<LoopWorker_Thread>
{
//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	//
	// Functions
	//
public:
	LoopWorker_ThreadPool(const unsigned int uiNum)
		: ThreadPoolBase(uiNum), m_scl(keywords::channel = "LoopWorker_ThreadPool")
	{
	}

	virtual ~LoopWorker_ThreadPool(void)
	{
	}

	/*
	分配任务

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int AssignTask(std::shared_ptr<TaskBase>& ptr_task)
	{
		static const std::string ftag("LoopWorker_ThreadPool::AssignTask() ");

		if( m_uiCurrThreadNums > m_uiThreadMaxNums )
		{
			// 已到线程数上限
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "MaxThreadNums reached!";
			return -1;
		}

		std::shared_ptr<LoopWorker_Thread> ptrThread(new LoopWorker_Thread());
		ptrThread->AddTask(ptr_task);

		m_hdThreads.push_back(ptrThread);

		++m_uiCurrThreadNums;

		// 看当前的ThreadPool运行状态决定是否立即启动线程
		if( ThreadPool_Conf::Running == m_pool_state )
		{
			ptrThread->StartThread();
		}

		return 0;
	}

private:
	// 阻止使用默认构造函数
	LoopWorker_ThreadPool(void);
};
#endif
