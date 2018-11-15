#ifndef __PROC_SZ_CONN_STEP_H__
#define __PROC_SZ_CONN_STEP_H__

/*
处理委托、回报消息处理
深圳 STEP 消息
*/

#include "simutgw_flowwork/FlowWorkBase.h"

#include "util/EzLog.h"

class Proc_SzConn_Step : public FlowWorkBase
{
	//
	// member
	//
private:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;
	
	//
	// function
	//
public:
	Proc_SzConn_Step( void );
	virtual ~Proc_SzConn_Step( void );

	virtual int TaskProc( void );
	
	/*
	处理深圳的消息

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int ProcSzMessage();
};

#endif