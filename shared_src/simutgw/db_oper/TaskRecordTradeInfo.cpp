#include "TaskRecordTradeInfo.h"

#include "simutgw/db_oper/RecordTradeInfo.h"
#include "simutgw/db_oper/RecordMatchTableHelper.h"

#include "simutgw/stgw_config/g_values_inner.h"

TaskRecordTradeInfo::TaskRecordTradeInfo(unsigned int uiId) :TaskBase(uiId)
{
	m_emType = AsyncDbTask::UpdateTaskType::notype;
	m_ptrReport.reset();
}


TaskRecordTradeInfo::~TaskRecordTradeInfo()
{
}

int TaskRecordTradeInfo::TaskProc(void)
{
	switch (m_emType)
	{
	case AsyncDbTask::UpdateTaskType::match:
		WriteTransInfoInDb();
		break;

	case AsyncDbTask::UpdateTaskType::cancel_succ:
		WriteTransInfoInDb_CancelSuccess();
		break;

	case AsyncDbTask::UpdateTaskType::cancel_fail:
		WriteTransInfoInDb_CancelFail();
		break;

	case AsyncDbTask::UpdateTaskType::error:
		//WriteTransInfoInDb_Error();
		break;

	default:
		break;
	}

	return 0;
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
int TaskRecordTradeInfo::WriteTransInfoInDb()
{
	static const string strTag("TaskRecordTradeInfo::WriteTransInfoInDb()");

	try
	{
		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(strTag, "Get Connection is NULL");

			return -1;
		}

		mysqlConn->StartTransaction();

		int iRes = 0;

		//记录到成交流水表
		iRes = RecordMatchTableHelper::RecordMatchInfo(m_ptrReport, mysqlConn);

		if (0 == iRes)
		{
			mysqlConn->Commit();
		}
		else
		{
			mysqlConn->RollBack();
		}

		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
	}
	catch (exception& e)
	{
		EzLog::ex(strTag, e);

		return -1;
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
int TaskRecordTradeInfo::WriteTransInfoInDb_CancelSuccess()
{
	static const string strTag("TaskRecordTradeInfo::WriteTransInfoInDb()");

	try
	{
		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(strTag, "Get Connection is NULL");

			return -1;
		}

		mysqlConn->StartTransaction();
		int iRes = RecordTradeInfo::UpdateRecordTable(m_ptrReport, mysqlConn);

		//if (0 == iRes)
		//{
		//	std::shared_ptr<struct simutgw::OrderMessage> ptrOrig(new struct simutgw::OrderMessage);
		//	ptrOrig->strSide = m_ptrReport->strSide;
		//	ptrOrig->strClordid = m_ptrReport->strOrigClordid;
		//	ptrOrig->strSessionId = m_ptrReport->strSessionId;
		//	ptrOrig->strTrade_market = m_ptrReport->strTrade_market;

		//	ptrOrig->enMatchType = simutgw::MatchAll;

		//	iRes = RecordTradeInfo::UpdateBuySellTable(ptrOrig, mysqlConn);
		//}
		//else
		//{
		//	mysqlConn->RollBack();
		//}

		if (0 != iRes)
		{
			mysqlConn->RollBack();
		}
		else
		{
			mysqlConn->Commit();
		}

		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
	}
	catch (exception& e)
	{
		EzLog::ex(strTag, e);

		return -1;
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
int TaskRecordTradeInfo::WriteTransInfoInDb_CancelFail()
{
	static const string strTag("TaskRecordTradeInfo::WriteTransInfoInDb()");

	try
	{
		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(strTag, "Get Connection is NULL");

			return -1;
		}

		mysqlConn->StartTransaction();
		int iRes = RecordTradeInfo::UpdateRecordTable(m_ptrReport, mysqlConn);

		if (0 != iRes)
		{
			mysqlConn->RollBack();
		}
		else
		{
			mysqlConn->Commit();
		}

		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
	}
	catch (exception& e)
	{
		EzLog::ex(strTag, e);

		return -1;
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
int TaskRecordTradeInfo::WriteTransInfoInDb_Error()
{
	static const string strTag("TaskRecordTradeInfo::WriteTransInfoInDb_Error()");
	
	return 0;
}