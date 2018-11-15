#include "OrderRepeatChecker.h"

#include "simutgw/order/OrderRepeatChecker.h"
#include "order/StockOrderHelper.h"
#include "simutgw/db_oper/RecordNewOrderHelper.h"

#include "tool_redis/Tgw_RedisHelper.h"

#include "simutgw/stgw_config/sys_function.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "config/conf_mysql_table.h"

/*
检查当前单是否有重单

Return :
ture -- 是重单
false -- 不是重单
*/
bool OrderRepeatChecker::CheckIfRepeatOrder( std::shared_ptr<struct simutgw::OrderMessage>& in_order )
{
	static const string ftag( "OrderRepeatChecker::CheckIfRepeatOrder() " );

	bool bHaveRepeat = CheckIfRepeatOrder_Redis( in_order );

	return bHaveRepeat;
}

/*
检查redis中当前单是否有重单

Return :
ture -- 是重单
false -- 不是重单
*/
bool OrderRepeatChecker::CheckIfRepeatOrder_Redis( std::shared_ptr<struct simutgw::OrderMessage>& in_order )
{
	static const string ftag( "OrderRepeatChecker::CheckIfRepeatOrder_Redis() " );

	bool bHaveRepeat = true;

	// read redis

	// 读取订单Id
	vector<string> vctArgs;
	vctArgs.push_back("HGET");
	vctArgs.push_back(simutgw::g_redis_Key_OrderId_Record);
	vctArgs.push_back(in_order->strClordid);

	//
	std::string strRedisRes;
	std::vector<StgwRedisReply> vectRedisRes_Array;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv(vctArgs, nullptr, &strRedisRes,
		nullptr, &vectRedisRes_Array, nullptr);
	if ( RedisReply_nil == emPcbCallRes )
	{
		// 无记录，不是重单
		bHaveRepeat = false;
	}
	else if ( RedisReply_string == emPcbCallRes )
	{
		// 有记录，是重单
		bHaveRepeat = true;
	}
	else
	{
		string strTran;
		sof_string::itostr( emPcbCallRes, strTran );

		string strDebug( "Redis执行错误，cmd=[" );
		for (size_t i = 0; i < vctArgs.size(); ++i)
		{
			strDebug += vctArgs[i];
			strDebug += " ";
		}
		strDebug += "],res=[";
		strDebug += strTran;
		strDebug += "]. str=[";
		strDebug += strRedisRes;
		strDebug += "]";

		EzLog::w(ftag, strDebug);

		bHaveRepeat = true;
	}
	return bHaveRepeat;
}

/*
检查mysql中当前单是否有重单

Return :
ture -- 是重单
false -- 不是重单
*/
bool OrderRepeatChecker::CheckIfRepeatOrder_MySql( std::shared_ptr<struct simutgw::OrderMessage>& in_order )
{
	static const string ftag( "OrderRepeatChecker::CheckIfRepeatOrder_MySql() " );

	bool bHaveRepeat = true;

	//	从mysql连接池取连接
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if ( nullptr == mysqlConn )
	{
		//	取出的mysql连接为NULL

		//	归还连接
		simutgw::g_mysqlPool.ReleaseConnection( mysqlConn );

		EzLog::e( ftag, "Get Connection is NULL" );

		return true;
	}

	MYSQL_RES *pResultSet = nullptr;

	try
	{
		string strCheck = "SELECT `clordid` FROM ";
		strCheck += simutgw::g_strSQL_NewOrder_TableName;

		strCheck += " WHERE `clordid`='";
		strCheck += in_order->strClordid;
		strCheck += "';";

		// result set rows
		unsigned long ulAffectedRows = 0;

		//查询
		int iRes = mysqlConn->Query( strCheck, &pResultSet, ulAffectedRows );
		if ( 1 != iRes )
		{
			string strItoa;
			string sDebug( "运行[" );
			sDebug += strCheck;
			sDebug += "]得到";
			sDebug += sof_string::itostr( iRes, strItoa );
			EzLog::e( ftag, sDebug );

			mysqlConn->FreeResult( &pResultSet );

			//	归还连接
			simutgw::g_mysqlPool.ReleaseConnection( mysqlConn );

			return true;
		}

		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		if ( 0 == mysqlConn->FetchNextRow( &pResultSet, mapRowData ) )
		{
			bHaveRepeat = false;
		}
		else
		{
			bHaveRepeat = true;
		}

		mysqlConn->FreeResult( &pResultSet );
		pResultSet = nullptr;

		//	归还连接
		simutgw::g_mysqlPool.ReleaseConnection( mysqlConn );
	}
	catch ( exception& e )
	{
		mysqlConn->FreeResult( &pResultSet );
		pResultSet = nullptr;

		EzLog::ex( ftag, e );

		//	归还连接
		simutgw::g_mysqlPool.ReleaseConnection( mysqlConn );

		return true;
	}

	return bHaveRepeat;
}

/*
向redis中记录当前单号
Param :

Return :
0 -- 写入成功
非0 -- 写入失败
*/
int OrderRepeatChecker::RecordOrderInRedis( std::shared_ptr<struct simutgw::OrderMessage>& orderMsg )
{
	static const string ftag( "OrderRepeatChecker::RecordOrderInRedis() " );

	//
	vector<string> vctArgs;

	vctArgs.push_back("HSET");
	vctArgs.push_back(simutgw::g_redis_Key_OrderId_Record);
	vctArgs.push_back(orderMsg->strClordid);
	vctArgs.push_back("1");

	//
	long long llRedisRes = 0;
	std::string strRedisRes;
	std::vector<StgwRedisReply> vectArray;
	size_t stElemSize = 0;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv(vctArgs, &llRedisRes, &strRedisRes,
		nullptr, &vectArray, &stElemSize );
	if (RedisReply_integer != emPcbCallRes)
	{
		string strTran;
		sof_string::itostr(emPcbCallRes, strTran);

		string strDebug("Redis执行错误，cmd=[");
		for (size_t i = 0; i < vctArgs.size(); ++i)
		{
			strDebug += vctArgs[i];
			strDebug += " ";
		}
		strDebug += "],res=[";
		strDebug += strTran;
		strDebug += "]. str=[";
		strDebug += strRedisRes;
		strDebug += "]";

		EzLog::w(ftag, strDebug);

		return -1;
	}
	else
	{
		
	}

	return 0;
}

/*
清理redis中重单缓存

Return :
0 -- 成功
非0 -- 失败
*/
int OrderRepeatChecker::DayEndCleanUpInRedis(void)
{
	static const string ftag("OrderRepeatChecker::DayEndCleanUpInRedis() ");

	//
	vector<string> vctArgs;

	vctArgs.push_back("DEL");
	vctArgs.push_back(simutgw::g_redis_Key_OrderId_Record);

	//
	long long llRedisRes = 0;
	std::string strRedisRes;
	std::vector<StgwRedisReply> vectArray;
	size_t stElemSize = 0;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv(vctArgs, &llRedisRes, &strRedisRes,
		nullptr, &vectArray, &stElemSize);
	if ( RedisReply_integer != emPcbCallRes )
	{
		string strTran;
		sof_string::itostr(emPcbCallRes, strTran);

		string strDebug("Redis执行错误，cmd=[");
		for ( size_t i = 0; i < vctArgs.size(); ++i )
		{
			strDebug += vctArgs[i];
			strDebug += " ";
		}
		strDebug += "],res=[";
		strDebug += strTran;
		strDebug += "]. str=[";
		strDebug += strRedisRes;
		strDebug += "]";

		EzLog::w(ftag, strDebug);

		return -1;
	}
	else
	{

	}

	return 0;
}