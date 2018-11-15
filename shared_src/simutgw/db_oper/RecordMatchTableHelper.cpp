#include "RecordMatchTableHelper.h"
#include "simutgw/db_oper/TaskRecordComponentMatch.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "tool_string/Tgw_StringUtil.h"

RecordMatchTableHelper::RecordMatchTableHelper()
{
}


RecordMatchTableHelper::~RecordMatchTableHelper()
{
}

/*
普通委托(包括etf申赎)记录到流水表
*/
int RecordMatchTableHelper::RecordMatchInfo(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::shared_ptr<MySqlCnnC602> &in_mysqlConn)
{
	static const string strTag("RecordMatchTableHelper::RecordMatchInfo() ");

	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		string strQueryString, strItoa;

		// 记录入流水单
		// 在买单表中记录已处理
		strQueryString = "INSERT INTO `order_match`	(`sessionid`,`trade_market`,`trade_type`,`security_account`,`security_seat`,"
			"`trade_group`,`clordid`,`securityid`,`securityidsource`,`security_id2`,"
			"`side`,`msgtype`,`execid`, `orderid`,`market`,"
			"`orderqty_origin`,`order_price`,`match_type`,`match_price`,`match_qty`,"
			"`match_amount`,`leavesqty`,`cumqty`,`stock_balance`,`trade_time`,"
			"`market_branchid`,`order_time`,`settle_group`) VALUES (\"";
		// 1-5 
		strQueryString += in_ptrReport->strSessionId;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strTrade_market;
		strQueryString += "\",\"";
		strQueryString += sof_string::itostr(in_ptrReport->iTrade_type, strItoa);
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strAccountId;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strSecurity_seat;
		strQueryString += "\",\"";

		// 6-10 `trade_group`,`clordid`,`securityid`,`securityidsource`,`security_id2`,
		strQueryString += in_ptrReport->strTrade_group;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strClordid;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strStockID;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strSecurityIDSource;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strStockID2;
		strQueryString += "\",\"";

		// 11-15 `side`,`msgtype`,`execid`, `orderid`,`market`,
		strQueryString += in_ptrReport->strSide;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strMsgType;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strExecID;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strOrderID;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strTrade_market;
		strQueryString += "\",\"";

		// 16-20 `orderqty_origin`,`order_price`,`match_type`,`match_price`,`match_qty`,
		strQueryString += in_ptrReport->strOrderqty_origin;
		strQueryString += "\",\"";
		uint64_t uiValue;
		string strValue;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_ptrReport->strOrderPrice, uiValue);
		strQueryString += sof_string::itostr(uiValue, strValue);
		strQueryString += "\",\"";
		strQueryString += sof_string::itostr(in_ptrReport->enMatchType, strItoa);
		strQueryString += "\",\"";
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_ptrReport->strLastPx, uiValue);
		strQueryString += sof_string::itostr(uiValue, strValue);
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strLastQty;
		strQueryString += "\",\"";

		// 21-25 `match_amount`,`leavesqty`,`cumqty`,`stock_balance`,`trade_time`,
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_ptrReport->strCashorderqty, uiValue);
		strQueryString += sof_string::itostr(uiValue, strValue);
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strLeavesQty;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strCumQty;
		strQueryString += "\",\"";
		strQueryString += sof_string::itostr(in_ptrReport->userHold.ui64StockBalance_beforematch, strItoa);
		strQueryString += "\",now(),\"";

		// 26-30 `market_branchid`,`order_time`,`settle_group`
		strQueryString += in_ptrReport->strMarket_branchid;
		strQueryString += "\",\"";
		strQueryString += in_ptrReport->strOper_time;
		
		if (in_ptrReport->tradePolicy.strSettleGroupName.empty())
		{
			strQueryString += "\",null";
		}
		else
		{
			strQueryString += "\",\"";
			strQueryString += in_ptrReport->tradePolicy.strSettleGroupName;
			strQueryString += "\"";
		}		
		strQueryString += " )";

		int iRes = in_mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (2 == iRes)
		{
			// 是更新
			if (1 != ulAffectedRows)
			{
				// 失败
				string strDebug("运行["), strItoa;
				strDebug += strQueryString;
				strDebug += "]得到AffectedRows=";
				strDebug += sof_string::itostr((uint64_t)ulAffectedRows, strItoa);
				EzLog::e(strTag, strDebug);

				return -1;
			}
		}
		else
		{
			string strDebug("运行["), strItoa;
			strDebug += strQueryString;
			strDebug += "]得到Res=";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(strTag, strDebug);

			return -1;
		}
	}
	catch (exception& e)
	{
		EzLog::ex(strTag, e);

		return -1;
	}

	return 0;
}

/*
etf申赎成分股记录到流水表
*/
int RecordMatchTableHelper::RecordCompnentMatchInfo(std::shared_ptr<struct simutgw::OrderMessage> in_ptrReport,
	const std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent>& in_ptrFrozeComponent)
{
	static const string strTag("RecordMatchTableHelper::RecordCompnentMatchInfo() ");

	if (simutgw::SysRunMode::NormalMode == in_ptrReport->tradePolicy.iRunMode)
	{
		// 3 -- 普通模式 记录数据库
		TaskRecordComponentMatch* task = new TaskRecordComponentMatch(0);
		task->SetReportAndComponent(in_ptrReport, in_ptrFrozeComponent);

		std::shared_ptr<TaskBase> base(dynamic_cast<TaskBase*>(task));
		simutgw::g_asyncDbwriter.AssignTask(base);
	}

	return 0;
}