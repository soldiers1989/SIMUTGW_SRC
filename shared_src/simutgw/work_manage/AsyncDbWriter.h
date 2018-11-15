#ifndef __ASYNC_DB_WRITER_H__
#define __ASYNC_DB_WRITER_H__

#include <memory>

/*
异步对DataBase进行写入的类
*/

#include "thread_pool_pipeline/PipeLine_ThreadPool.h"

class AsyncDbWriter
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	PipeLine_ThreadPool m_pipelinePool;

	//
	// Functions
	//
public:
	AsyncDbWriter( void );
	virtual ~AsyncDbWriter( void );

	/*
	启动线程

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int Start( void );

	/*
	关闭线程

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int Stop( void );

	/*
	分配任务

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int AssignTask( std::shared_ptr<TaskBase>& ptr_task );

	/*
	获取当前任务情况

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int GetTaskAssignInfo(std::string& out_strTaskInfo)
	{
		// static const std::string ftag("ThreadPool::AssignTask() ");

		m_pipelinePool.GetTaskAssignInfo(out_strTaskInfo);

		return 0;
	}

protected:

};

#endif