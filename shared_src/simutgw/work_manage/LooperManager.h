#ifndef __LOOPER__MANAGER_H__
#define __LOOPER__MANAGER_H__

/*
需要常驻循环处理的管理类
*/

#include "thread_pool_loopworker/LoopWorker_ThreadPool.h"

class LooperManager
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
	LooperManager(void);
	virtual ~LooperManager(void);

	/*
	启动所有模拟撮合相关线程

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int StartFlows(void);


	/*
	关闭所有模拟撮合相关线程

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int StopFlows(void);

protected:

	/*
	启动相关线程 深圳委托、回报消息处理

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int Start_SzConn(void);

	/*
	启动相关线程 上海委托、回报消息处理

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int Start_ShConn(void);

	/*
	启动线程
	分配下单消息缓存

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int Start_Distribute_InMsg(void);

	/*
	启动线程
	流量定时统计

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int Start_Traffic_count(void);
};

#endif