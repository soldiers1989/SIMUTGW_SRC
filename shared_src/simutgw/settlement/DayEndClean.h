#ifndef __DAYEND_CLEAN_H__
#define __DAYEND_CLEAN_H__

#include <memory>

#include "tool_mysql/MySqlCnnC602.h"

/*
日终清理类
*/
class DayEndClean
{
	//
	// Members
	//
private:

	//
	// Functions
	//

public:
	DayEndClean(void);
	virtual ~DayEndClean(void);

	/*
	日终清理
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int RunDayEndClean(std::shared_ptr<MySqlCnnC602> &in_mysqlConn);

protected:
	/*
	备份database
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int BackupDb(std::shared_ptr<MySqlCnnC602> &in_mysqlConn);

	/*
	将数据库表数据复制至另一个表
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int MoveDbdata(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const string& in_strMoveCmd);

	/*
	truncate表
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int TruncateTable(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const string& in_strTableName);

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
	int Reset(void);
};

#endif


