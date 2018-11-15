#ifndef __PROC_SH_CONN_DB_DECLARE_H__
#define __PROC_SH_CONN_DB_DECLARE_H__

/*
上海 数据库报盘 委托消息
*/

#include "simutgw_flowwork/FlowWorkBase.h"

#include "util/EzLog.h"

class Proc_ShConn_Db_Declare : public FlowWorkBase
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
	Proc_ShConn_Db_Declare(void);
	virtual ~Proc_ShConn_Db_Declare(void);

	virtual int TaskProc( void );
	
	/*
	处理上海的消息

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int ProcShMessage();
};

#endif