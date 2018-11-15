#ifndef __PROC_DISTRIBUTE_IN_MSG_H__
#define __PROC_DISTRIBUTE_IN_MSG_H__

/*
处理委托、回报消息处理
深圳 STEP 消息
*/

#include "simutgw_flowwork/FlowWorkBase.h"

#include "util/EzLog.h"

class Proc_Distribute_InMsg : public FlowWorkBase
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
	Proc_Distribute_InMsg( void );
	virtual ~Proc_Distribute_InMsg( void );

	virtual int TaskProc( void );

	/*
	处理下单消息缓存

	Return :
	0 -- 处理成功
	-1 -- 处理失败
	*/
	int ProcInMessage();
};

#endif