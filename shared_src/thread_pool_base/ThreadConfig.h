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

	// �߳�����ʱ����
	enum ThreadRunningOption
	{
		// ����
		Running = 0,

		// �����˳�
		Immediate = 1,

		// �ȴ����������
		TillTaskFinish = 2
	};


	// �߳̿�ʼ��ĵȴ��ӳ�ʱ��
	static const unsigned long g_dwWaitMS_AfterStartThread = 300;

	// �̴߳���ѭ����Event�ĵȴ�ʱ��
	static const unsigned long g_dwWaitMS_Event = 10;

	// �ȴ��̹߳ر�ʱ��
	static const unsigned long g_dwWaitMS_ThreadExit = 500;

}


#endif