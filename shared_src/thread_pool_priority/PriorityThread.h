#ifndef __PRIORITY_THREAD_H__
#define __PRIORITY_THREAD_H__

#include "thread_pool_base/ThreadBase.h"
#include "TaskPriorityQueue.h"

class PriorityThread : public ThreadBase
{
	//
	// Members
	//
protected:
	//
	// �߳���Դ
	//
	TaskPriorityQueue m_prtQueTask;

	//
	// Functions
	//
public:
	PriorityThread()
	{
	}

	explicit PriorityThread( const uint64_t uiId )
		: ThreadBase( uiId )
	{
	}

	virtual ~PriorityThread()
	{
	}

	/*
	�����߳�

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	virtual int StartThread( void );

	/*
	��������

	Param :

	Return :
	0 -- ����ɹ�
	2 -- �������������������
	-1 -- ����ʧ��
	*/
	int AddTask( T_PTR_TaskPriorityBase& in_ptrTask );

	/*
	��ȡ�������ݴ�С

	Return :

	*/
	uint64_t GetQueueSize( void )
	{
		return m_prtQueTask.QueueSize();
	}

	/*
	��ȡ��ǰ�������

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int GetTaskAssignInfo( std::string& out_strTaskInfo );

protected:
#ifdef _MSC_VER
	static DWORD WINAPI Run(LPVOID  pParam);
#else
	static void* Run(void*  pParam);
#endif
};

#endif