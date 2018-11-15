#ifndef __THREAD_BASE_H__
#define __THREAD_BASE_H__

#include "ThreadConfig.h"

#include "util/EzLog.h"

#ifdef _MSC_VER

#else
#include <pthread.h>
#endif

/*
Thread ������
*/
class ThreadBase
{
	//
	// Members
	//
protected:
	//
	// �߳�����
	//
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	unsigned long m_dThreadId;

#ifdef _MSC_VER
	// �����̵߳ľ��
	HANDLE m_hThread;

	// ֪ͨ�̴߳������л���
	HANDLE m_hEvent;
#else
	// �����̵߳ľ��
	pthread_t m_hThread;

	// ֪ͨ�̴߳������л���
	pthread_mutex_t m_mutexCond; 
	pthread_cond_t m_condEvent;
#endif

	// ��ǰ�߳�״̬
	volatile enum ThreadPool_Conf::ThreadState m_emThreadState;

	// ��ǰ�߳�����ʱ����
	volatile enum ThreadPool_Conf::ThreadRunningOption m_emThreadRunOp;

	//
	// Functions
	//
public:
	ThreadBase(void);
	explicit ThreadBase(const uint64_t uiId);
	virtual ~ThreadBase(void);

	void SetThreadId(const uint64_t uiId)
	{
		m_dThreadId = (unsigned long)uiId;
	}

	const uint64_t GetThreadId(void) const
	{
		return m_dThreadId;
	}

	/*
	�����߳�

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
#ifdef _MSC_VER
	virtual int StartThread(LPTHREAD_START_ROUTINE lpFunc);
#else
	typedef void* (*PTRFUN)(void*);
	virtual int StartThread(PTRFUN lpFunc);
#endif
	/*
	֪ͨ�ر��̣߳����̹ر�

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int NotifyStopThread_Immediate(void);

	/*
	֪ͨ�ر��̣߳�������ɺ�ر�

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int NotifyStopThread_TillTaskFinish(void);

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
	int StopThread(bool bForceClose = false);

	/*
	�ر��̣߳��ȴ����е��������

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int WaitStopThread(void);

	//����Ϣ��֪ͨ�̴߳���
	bool SetThreadEvent(void);

protected:
#ifdef _MSC_VER
#else
	/*
	// pthread_cleanup_push�Ļص�����
	static void PthreadCleanup(void* arg)
	{
		pthread_mutex_unlock((pthread_mutex_t*)arg);
	}
	*/
	/*
	pthread_cleanup_push(PthreadCleanup, &pThread->m_mutexCond);
		
	//�ȴ��߳��źţ���ʱ		
	pthread_mutex_lock(&pThread->m_mutexCond);

	struct timespec tmRestrict;
	tmRestrict.tv_sec = 0;
	tmRestrict.tv_nsec += ThreadPool_Conf::g_dwWaitMS_Event * 1000L;
	pthread_cond_timedwait(&pThread->m_condEvent, &pThread->m_mutexCond, &tmRestrict);

	pthread_mutex_unlock(&pThread->m_mutexCond);
	
	// do something
	pthread_cleanup_pop(0);
	
	// NOTE:
	// ����Ҫע����ǣ�����̴߳���pthread_CANCEL_ASYNCHRONOUS״̬����������ξ��п��ܳ�����ΪCANCEL�¼��п�����pthread_cleanup_push()��pthread_mutex_lock()֮�䷢����
	// ������pthread_mutex_unlock()��pthread_cleanup_pop()֮�䷢�����Ӷ�����������unlockһ����û�м�����mutex��������ɴ���
	// ��ˣ���ʹ����������ʱ�򣬶�Ӧ����ʱ���ó�pthread_CANCEL_DEFERREDģʽ��Ϊ�ˣ�POSIX��Linuxʵ���л��ṩ��
	// һ�Բ���֤����ֲ��pthread_cleanup_push_defer_np()/pthread_cleanup_pop_defer_np()��չ����
	*/
#endif
};

#endif
