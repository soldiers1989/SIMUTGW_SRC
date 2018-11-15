#include "RecordTradeInfo.h"
#include "simutgw/db_oper/RecordMatchTableHelper.h"
#include "simutgw/db_oper/TaskRecordTradeInfo.h"

#include "order/StockOrderHelper.h"

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"

#include "config/conf_mysql_table.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

RecordTradeInfo::RecordTradeInfo()
{
}


RecordTradeInfo::~RecordTradeInfo()
{
}

/*
向数据库中写入成交的数据
成交
Param :
下单的方向

Return :
0 -- 写入成功
非0 -- 写入失败
*/
int RecordTradeInfo::WriteTransInfoInDb( std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport )
{
	static const string strTag( "RecordTradeInfo::WriteTransInfoInDb()" );

	if ( simutgw::SysRunMode::NormalMode == in_ptrReport->tradePolicy.iRunMode )
	{
		// 3 -- 普通模式 记录数据库
		AsyncDbTask::UpdateTaskType type = AsyncDbTask::UpdateTaskType::match;
		TaskRecordTradeInfo* Task( new TaskRecordTradeInfo( 1 ) );
		Task->SetTask( type, in_ptrReport );
		std::shared_ptr<TaskBase> ptrBase( (TaskBase*) Task );
		simutgw::g_asyncDbwriter.AssignTask( ptrBase );
	}

	return 0;
}

/*
向数据库中写入成交的数据
撤单成功
Param :

Return :
0 -- 写入成功
非0 -- 写入失败
*/
int RecordTradeInfo::WriteTransInfoInDb_CancelSuccess( std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport )
{
	static const string strTag( "RecordTradeInfo::WriteTransInfoInDb()" );

	if ( simutgw::SysRunMode::NormalMode == in_ptrReport->tradePolicy.iRunMode )
	{
		// 3 -- 普通模式 记录数据库
		AsyncDbTask::UpdateTaskType type = AsyncDbTask::UpdateTaskType::cancel_succ;
		TaskRecordTradeInfo* Task( new TaskRecordTradeInfo( 1 ) );
		Task->SetTask( type, in_ptrReport );
		std::shared_ptr<TaskBase> ptrBase( (TaskBase*) Task );
		simutgw::g_asyncDbwriter.AssignTask( ptrBase );
	}

	return 0;
}

/*
向数据库中写入成交的数据
撤单失败
Param :

Return :
0 -- 写入成功
非0 -- 写入失败
*/
int RecordTradeInfo::WriteTransInfoInDb_CancelFail( std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport )
{
	static const string strTag( "RecordTradeInfo::WriteTransInfoInDb()" );

	if ( simutgw::SysRunMode::NormalMode == in_ptrReport->tradePolicy.iRunMode )
	{
		// 3 -- 普通模式 记录数据库
		AsyncDbTask::UpdateTaskType type = AsyncDbTask::UpdateTaskType::cancel_fail;
		TaskRecordTradeInfo* Task( new TaskRecordTradeInfo( 1 ) );
		Task->SetTask( type, in_ptrReport );
		std::shared_ptr<TaskBase> ptrBase( (TaskBase*) Task );
		simutgw::g_asyncDbwriter.AssignTask( ptrBase );
	}

	return 0;
}

/*
向数据库中写入成交的数据
废单
Param :

Return :
0 -- 写入成功
非0 -- 写入失败
*/
int RecordTradeInfo::WriteTransInfoInDb_Error( std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport )
{
	static const string strTag( "RecordTradeInfo::WriteTransInfoInDb()" );

	if ( simutgw::SysRunMode::NormalMode == in_ptrReport->tradePolicy.iRunMode )
	{
		// 3 -- 普通模式 记录数据库
		AsyncDbTask::UpdateTaskType type = AsyncDbTask::UpdateTaskType::error;
		TaskRecordTradeInfo* Task( new TaskRecordTradeInfo( 1 ) );
		Task->SetTask( type, in_ptrReport );
		std::shared_ptr<TaskBase> ptrBase( (TaskBase*) Task );
		simutgw::g_asyncDbwriter.AssignTask( ptrBase );
	}

	return 0;
}

/*
更新order_record表字段
Param :

Return :
0 -- 写入成功
非0 -- 写入失败
*/
int RecordTradeInfo::UpdateRecordTable( std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::shared_ptr<MySqlCnnC602> &in_mysqlConn )
{
	static const string strTag( "RecordTradeInfo::UpdateRecordTable() " );

	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		string strQueryString, strItoa;

		strQueryString = "UPDATE ";
		strQueryString += simutgw::g_strSQL_NewOrder_TableName;
		strQueryString += " SET `is_proc`=1 WHERE `sessionid`=\"";
		strQueryString += in_ptrReport->strSessionId;
		strQueryString += "\" AND `trade_market`=\"";
		strQueryString += in_ptrReport->strTrade_market;
		strQueryString += "\" AND `clordid`=\"";
		strQueryString += in_ptrReport->strClordid;
		strQueryString += "\"";

		int iRes = in_mysqlConn->Query( strQueryString, &pResultSet, ulAffectedRows );
		if ( 2 == iRes )
		{
			// 是更新
			if ( 1 != ulAffectedRows )
			{
				// 失败
				string strDebug( "运行[" );
				strDebug += strQueryString;
				strDebug += "]得到AffectedRows=";
				strDebug += sof_string::itostr( (uint64_t) ulAffectedRows, strItoa );
				EzLog::e( strTag, strDebug );

				return -1;
			}
		}
		else
		{
			string strDebug( "运行[" );
			strDebug += strQueryString;
			strDebug += "]得到Res=";
			strDebug += sof_string::itostr( iRes, strItoa );
			EzLog::e( strTag, strDebug );

			return -1;
		}
	}
	catch ( exception& e )
	{
		EzLog::ex( strTag, e );

		return -1;
	}

	return 0;
}

