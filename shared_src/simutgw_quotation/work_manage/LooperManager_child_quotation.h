#ifndef __LOOPER_MANAGER_CHILD_QUOTATION_H__
#define __LOOPER_MANAGER_CHILD_QUOTATION_H__

/*
需要常驻循环处理的管理类

给子进程 行情处理专用
*/

#include "thread_pool_loopworker/LoopWorker_ThreadPool.h"

class LooperManager_child_quotation
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	LoopWorker_ThreadPool m_looperPool;

	//
	// Functions
	//
public:
	LooperManager_child_quotation(void);
	virtual ~LooperManager_child_quotation(void);

	/*
	启动所有模拟撮合相关线程

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int StartFlows( void );


	/*
	关闭所有模拟撮合相关线程

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int StopFlows( void );

protected:
	/*
	启动相关线程 行情处理

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int Start_MarketInfo( void );
};

#endif