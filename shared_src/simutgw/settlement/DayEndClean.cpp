#include "DayEndClean.h"

#include <memory>

#include "config/conf_mysql_table.h"
#include "simutgw/stgw_config/g_values_biz.h"

#include "util/EzLog.h"
#include "tool_mysql/MySqlCnnC602.h"

#include "quotation/TgwMarketInfoProc.h"

using namespace std;

DayEndClean::DayEndClean(void)
{
}

DayEndClean::~DayEndClean(void)
{
}

/*
日终清理
Return :
0 -- 成功
非0 -- 失败
*/
int DayEndClean::RunDayEndClean(std::shared_ptr<MySqlCnnC602> &in_mysqlConn)
{
	static const string ftag("DayEndClean::DayEndClean()");

	int iRes = BackupDb(in_mysqlConn);
	if (0 != iRes)
	{
		EzLog::e(ftag, "BackupDb error");

		return -1;
	}

	Reset();

	return 0;
}

/*
备份database
Return :
0 -- 成功
非0 -- 失败
*/
int DayEndClean::BackupDb(std::shared_ptr<MySqlCnnC602> &in_mysqlConn)
{
	static const string ftag("DayEndClean::BackupDb()");

	in_mysqlConn->StartTransaction();

	int iReturn = 0;

	// order_match
	string strMove = "INSERT INTO `order_match_old` ("
		"`id`,`trade_market`,`trade_type`,`sessionid`,`security_account`,"
		"`security_seat`,`trade_group`,`clordid`,`origclordid`,`securityid`,"
		"`securityidsource`,`security_id2`,`side`,`msgtype`,`execid`,"
		"`orderid`,`market`,`orderqty_origin`,`order_price`,`match_type`,"
		"`match_price`,`match_qty`,`match_amount`,`leavesqty`,`cumqty`,"
		"`stock_balance`,`trade_time`,`market_branchid`,`order_time`,`settle_group`,"
		"`is_proc`,`text`) SELECT * from ";
	strMove += simutgw::g_strSQL_OrderMatch_TableName;

	int iRes = MoveDbdata(in_mysqlConn, strMove);
	if (0 != iRes)
	{
		string strDebug("BackupDb ");
		strDebug += simutgw::g_strSQL_OrderMatch_TableName;
		strDebug += " error";

		EzLog::e(ftag, strDebug);

		return -1;
	}

	// order_record
	strMove = "INSERT INTO `order_record_old` ("
		"`id`,`trade_market`,`trade_type`,`sessionid`,`security_account`,"
		"`security_seat`,`trade_group`,`beginstring`,`bodylength`,`checksum`,"
		"`clordid`,`securityidsource`,`msgseqnum`,`msgtype`,`orderqty_origin`,"
		"`leavesqty`,`ordtype`,`origclordid`,`possdupflag`,`order_price`,"
		"`securityid`,`sendercompid`,`sendingtime`,`side`,`targetcompid`,"
		"`text`,`timeinforce`,`transacttime`,`positioneffect`,`stoppx`,"
		"`minqty`,`origsendingtime`,`cashorderqty`,`coveredoruncovered`,`nopartyids`,"
		"`ownertype`,`orderrestrictions`,`cashmargin`,`confirmid`,`maxpricelevels`,"
		"`applid`,`market_branchid`,`oper_time`,`is_error`,`is_proc`,"
		"`error_msg`) SELECT * from ";
	strMove += simutgw::g_strSQL_NewOrder_TableName;

	iRes = MoveDbdata(in_mysqlConn, strMove);
	if (0 != iRes)
	{
		string strDebug("BackupDb ");
		strDebug += simutgw::g_strSQL_NewOrder_TableName;
		strDebug += " error";

		EzLog::e(ftag, strDebug);

		return -1;
	}

	// order_record
	strMove = "INSERT INTO `order_report_old` ("
		"`id`,`trade_market`,`trade_type`,`sessionid`,`security_account`,"
		"`security_seat`,`trade_group`,`beginstring`,`bodylength`,`checksum`,"
		"`clordid`,`securityidsource`,`msgseqnum`,`msgtype`,`orderqty_origin`,"
		"`leavesqty`,`ordtype`,`origclordid`,`possdupflag`,`order_price`,"
		"`securityid`,`sendercompid`,`sendingtime`,`side`,`targetcompid`,"
		"`text`,`timeinforce`,`transacttime`,`positioneffect`,`stoppx`,"
		"`minqty`,`origsendingtime`,`cashorderqty`,`coveredoruncovered`,`nopartyids`,"
		"`ownertype`,`orderrestrictions`,`cashmargin`,`confirmid`,`maxpricelevels`,"
		"`applid`,`market_branchid`,`oper_time`,`is_error`,`is_proc`,"
		"`error_msg`) SELECT * from ";
	strMove += simutgw::g_strSQL_OrderReport_TableName;

	iRes = MoveDbdata(in_mysqlConn, strMove);
	if (0 != iRes)
	{
		string strDebug("BackupDb ");
		strDebug += simutgw::g_strSQL_NewOrder_TableName;
		strDebug += " error";

		EzLog::e(ftag, strDebug);

		return -1;
	}

	// Truncate tables
	iRes = TruncateTable(in_mysqlConn, simutgw::g_strSQL_NewOrder_TableName);
	if (0 != iRes)
	{
		string strDebug("TruncateTable ");
		strDebug += simutgw::g_strSQL_NewOrder_TableName;
		strDebug += " error";
		EzLog::e(ftag, strDebug);

		return -1;
	}

	iRes = TruncateTable(in_mysqlConn, simutgw::g_strSQL_OrderMatch_TableName);
	if (0 != iRes)
	{
		string strDebug("TruncateTable ");
		strDebug += simutgw::g_strSQL_OrderMatch_TableName;
		strDebug += " error";
		EzLog::e(ftag, strDebug);

		return -1;
	}

	iRes = TruncateTable(in_mysqlConn, simutgw::g_strSQL_OrderReport_TableName);
	if (0 != iRes)
	{
		string strDebug("TruncateTable ");
		strDebug += simutgw::g_strSQL_OrderReport_TableName;
		strDebug += " error";
		EzLog::e(ftag, strDebug);

		return -1;
	}

	//
	iReturn = in_mysqlConn->Commit();

	return iReturn;
}

/*
将数据库表数据复制至另一个表
Return :
0 -- 成功
非0 -- 失败
*/
int DayEndClean::MoveDbdata(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const string& in_strMoveCmd)
{
	static const std::string ftag("DayEndClean::MoveDbdata()");

	int iReturn = 0;

	// order report
	string strMove(in_strMoveCmd);
	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	// 整表插入数据
	int iRes = in_mysqlConn->Query(strMove, &pResultSet, ulAffectedRows);
	if (2 == iRes)
	{
	}
	else
	{
		string strTrans;
		string strDebug("执行");
		strDebug += strMove;
		strDebug += "错误res=";
		strDebug += sof_string::itostr(iRes, strTrans);
		EzLog::e(ftag, strDebug);

		iReturn = -1;
	}

	return iReturn;
}

/*
truncate表
Return :
0 -- 成功
非0 -- 失败
*/
int DayEndClean::TruncateTable(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const string& in_strTableName)
{
	static const string ftag("DayEndClean::TruncateTable()");

	int iReturn = 0;

	// order report
	string strSql("TRUNCATE TABLE ");
	strSql += in_strTableName;

	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	// 整表插入数据
	int iRes = in_mysqlConn->Query(strSql, &pResultSet, ulAffectedRows);
	if (2 == iRes)
	{
	}
	else
	{
		string strTrans;
		string strDebug("执行");
		strDebug += strSql;
		strDebug += "错误res=";
		strDebug += sof_string::itostr(iRes, strTrans);
		EzLog::e(ftag, strDebug);

		iReturn = -1;
	}

	return iReturn;
}

/*
重置bLogOn = false;
g_iReportIndex = 0;
g_iRec_Num = 0;
g_iRec_Num2 = 0;
//记录上海确认的主机订单号
g_iTeordernum = 1;
//记录上海回报的成交编号
g_iCjbh = 1;
Return :
0 -- 成功
非0 -- 失败
*/
int DayEndClean::Reset(void)
{
	//simutgw::g_mapSzConns.clear();
	//simutgw::bLogOn = false;

	//simutgw::g_iReportIndex = 0;

	//simutgw::g_iRec_Num = 0;

	//simutgw::g_iRec_Num2 = 0;

	////记录上海确认的主机订单号
	//simutgw::g_iTeordernum = 1;

	////记录上海回报的成交编号
	//simutgw::g_iNextCjbh = 0;

	return 0;
}
