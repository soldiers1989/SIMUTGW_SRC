#ifndef __PROC_SH_CONN_DB_REPORT_H__
#define __PROC_SH_CONN_DB_REPORT_H__

/*
上海 数据库报盘 回报消息处理
*/

#include <map>
#include <vector>
#include <string>


#include "simutgw_flowwork/FlowWorkBase.h"

#include "util/EzLog.h"

class Proc_ShConn_Db_Report : public FlowWorkBase
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
	Proc_ShConn_Db_Report(void);
	virtual ~Proc_ShConn_Db_Report(void);

	virtual int TaskProc( void );
	
	/*
	处理上海的消息

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int ProcShReport();

protected:
	/*
	发送一条上海回报或确认

	Return:
	0 -- 成功
	其他 -- 失败
	*/
	int Send_SH_ReportOrConfirm(map<string, vector<string>>& in_mapUpdate);
};

#endif