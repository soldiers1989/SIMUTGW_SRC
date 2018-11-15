#ifndef __THREAD_CONFIG_H__
#define __THREAD_CONFIG_H__

#ifdef _MSC_VER
#include <Windows.h>
#else

#endif

namespace ThreadPool_Conf
{
	enum ThreadState
	{
		STARTED = 0,
		STOPPED = 1
	};

	// 线程运行时策略
	enum ThreadRunningOption
	{
		// 运行
		Running = 0,

		// 立刻退出
		Immediate = 1,

		// 等待任务处理完成
		TillTaskFinish = 2
	};


	// 线程开始后的等待延迟时间
	static const unsigned long g_dwWaitMS_AfterStartThread = 300;

	// 线程处理循环中Event的等待时间
	static const unsigned long g_dwWaitMS_Event = 10;

	// 等待线程关闭时间
	static const unsigned long g_dwWaitMS_ThreadExit = 500;

}


#endif