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
	// 线程资源
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
	启动线程

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	virtual int StartThread( void );

	/*
	插入任务

	Param :

	Return :
	0 -- 插入成功
	2 -- 数量已满，不允许插入
	-1 -- 插入失败
	*/
	int AddTask( T_PTR_TaskPriorityBase& in_ptrTask );

	/*
	获取队列数据大小

	Return :

	*/
	uint64_t GetQueueSize( void )
	{
		return m_prtQueTask.QueueSize();
	}

	/*
	获取当前任务情况

	Return :
	0 -- 成功
	-1 -- 失败
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