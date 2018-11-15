#include "ReportMsg_Sh.h"

#include "simutgw/db_oper/RecordTradeInfo.h"
#include "simutgw/db_oper/RecordReportHelper.h"

#include "order/StockOrderHelper.h"
#include "simutgw/order/OrderMemoryStoreFactory.h"

#include "tool_redis/Tgw_RedisHelper.h"

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"
#include "util/TimeDuration.h"
#include "util/SystemCounter.h"

#include "config/conf_mssql_table.h"
#include "simutgw/stgw_config/g_values_biz.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "match_rule/RuleWordProc_ShDb.h"

src::severity_channel_logger<trivial::severity_level, std::string>
ReportMsg_Sh::m_scl(keywords::channel = "ReportMsg_Sh");

ReportMsg_Sh::ReportMsg_Sh()
{
}


ReportMsg_Sh::~ReportMsg_Sh()
{
}

/*
处理一条回报

Reutrn:
0 -- 成功
-1 -- 失败
*/
int ReportMsg_Sh::ProcSingleReport(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	vector<string>& out_vectSqlStr)
{
	static const string ftag("ReportMsg_Sh::ProcSingleReport() ");

	int iRes = 0;

	string strReport;

	if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		//	深圳
		string strSendLog("wrong place SZ Report clordid=");
		strSendLog += in_ptrReport->strClordid;
		strSendLog += (", client=");
		strSendLog += in_ptrReport->strSenderCompID;

		EzLog::e(ftag, strSendLog);
	}
	else if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// 上海
		if (0 != in_ptrReport->tradePolicy.ui64RuleId)
		{
			// 已定义了成交规则
			if (nullptr == in_ptrReport->tradePolicy.ptrRule_Sh)
			{
				string sDebug("error order cliordid=");
				sDebug += in_ptrReport->strClordid;
				sDebug += "nullptr Rule Sh, ruleId=";
				string sItoa;
				sof_string::itostr(in_ptrReport->tradePolicy.ui64RuleId, sItoa);
				sDebug += sItoa;
				EzLog::e(ftag, sDebug);

				return -1;
			}

			// report with rule
			iRes = FixToSHReport_JsonRule(in_ptrReport, out_vectSqlStr);
		}
		else
		{
			/* 先更新表 */
			iRes = RecordTradeInfo::WriteTransInfoInDb(in_ptrReport);

			iRes = FixToSHReport(in_ptrReport, out_vectSqlStr);
		}

		if (0 != iRes)
		{
			return iRes;
		}

		string strSendLog("Sended SH Cjhb reff=");
		strSendLog += in_ptrReport->strClordid;
		strSendLog += ", sh_conn=";
		strSendLog += in_ptrReport->strSessionId;

		EzLog::i(ftag, strSendLog);
	}
	else
	{
		//nothing
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "clordid=" << in_ptrReport->strClordid
			<< ", client=" << in_ptrReport->strSenderCompID << " error Trade_market=" << in_ptrReport->strTrade_market;

		return -1;
	}

	return iRes;
}


/*
FIX协议格式数据转换成FIX协议格式上海确认，返回一个sql串

Reutrn:
0 -- 成功
-1 -- 失败
*/
int ReportMsg_Sh::FixToSHConfirm(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::vector<std::string>& out_vectSqlStr)
{
	static const string strTag("ReportMsg_Sh::FixToSHConfirm() ");

	string strInsertConfirmSql = "INSERT INTO ";
	strInsertConfirmSql += simutgw::g_strSH_Ordwth2_TableName;
	strInsertConfirmSql += " (rec_num,date,time,reff,acc,stock,"
		"bs,price,qty,status,qty2,remark,status1,teordernum,owflag,ordrec,firmid,branchid,checkord) VALUES(";

	size_t pos = in_ptrReport->strTransactTime.find("-");
	if (string::npos == pos)
	{
		string strError("委托clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]TransactTime[";
		strError += in_ptrReport->strTransactTime;
		strError += "]格式不对";
		EzLog::e(strTag, strError);

		return -1;
	}
	//成交日期，格式为YYYYMMDD, strTransactTime:YYYYMMDD-HH:MM:SS.sss
	string strTransDate = in_ptrReport->strTransactTime.substr(0, pos);

	size_t posTime = in_ptrReport->strTransactTime.find(".");
	if (string::npos == posTime)
	{
		string strError("委托clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]TransactTime[";
		strError += in_ptrReport->strTransactTime;
		strError += "]格式不对";
		EzLog::e(strTag, strError);

		return -1;
	}
	//成交时间，格式为HH:MM:SS
	string strTransTime = in_ptrReport->strTransactTime.substr(pos + 1, posTime - pos - 1);

	string strValue;

	//rec_num
	uint64_t ui64Value = 0;
	string strName = in_ptrReport->strSessionId;
	// 先找session
	if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
	{
		ui64Value = simutgw::g_mapShConns[strName].IncRecNum();
	}
	else
	{
		std::string strError("上海connection[");
		strError += strName;
		strError += "]丢失";
		EzLog::e(strTag, strError);
		return -1;
	}

	strInsertConfirmSql += sof_string::itostr(ui64Value, strValue);
	strInsertConfirmSql += ",'";

	//date,time,reff,acc,stock,
	strInsertConfirmSql += strTransDate;
	strInsertConfirmSql += "','";

	strInsertConfirmSql += strTransTime;
	strInsertConfirmSql += "','";

	if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_NEW_ORDER))
	{
		strInsertConfirmSql += in_ptrReport->strClordid;
	}
	else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL) ||
		0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_CANCELREJECT) ||
		0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		//撤单时与strOrigClordid相反
		strInsertConfirmSql += in_ptrReport->strOrigClordid;
	}
	else
	{
		std::string strError("委托clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]Msgtype[";
		strError += in_ptrReport->strMsgType;
		strError += "]错误";
		EzLog::e(strTag, strError);
		return -1;
	}
	strInsertConfirmSql += "','";

	strInsertConfirmSql += in_ptrReport->strAccountId;
	strInsertConfirmSql += "','";

	strInsertConfirmSql += in_ptrReport->strStockID;
	strInsertConfirmSql += "','";

	//bs,price,qty,status,qty2,
	strInsertConfirmSql += in_ptrReport->strSide;
	strInsertConfirmSql += "','";

	Tgw_StringUtil::iLiToStr(in_ptrReport->ui64mOrderPrice, strValue, 3);

	strInsertConfirmSql += strValue;
	strInsertConfirmSql += "','";


	strInsertConfirmSql += in_ptrReport->strOrderqty_origin;
	strInsertConfirmSql += "','";

	/*
	接收状态，
	‘F’表示交易所后台判断该订单为废单。
	‘E’表示交易所前台判断该订单为废单；此时remark（字段12：错误信息）给出错误代码。
	‘?’表示通信故障。
	‘O’表示上交所成功接收该笔申报。
	‘W’表示上交所成功接受该笔撤单；
	当订单类型为最优五档即时成交剩余自动撤销的市价订单时，且订单有效时，该字段取值为‘W’。

	*/
	if (simutgw::ErrorMatch == in_ptrReport->enMatchType)
	{
		strValue = "F";
	}
	else if (0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		strValue = "W";
	}
	else
	{
		if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_NEW_ORDER))
		{
			//	
			strValue = "O";
		}
		else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
		{
			//	
			strValue = "W";
		}
		else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_CANCELREJECT))
		{
			strValue = "F";
		}
		else
		{
			std::string strError("委托clordid[");
			strError += in_ptrReport->strClordid;
			strError += "]Msgtype[";
			strError += in_ptrReport->strMsgType;
			strError += "]错误";
			EzLog::e(strTag, strError);
			return -1;
		}
	}

	strInsertConfirmSql += strValue;
	strInsertConfirmSql += "',";

	//qty2
	/*
	撤单数量，
	对于限价订单申报记录，该字段为空；
	对于撤单记录，该字段为实际撤单返回数量；
	对于最优五档即时成交剩余自动撤销的市价订单，如果申报部分成交，该字段取值为自动撤单的数量；如果申报全部成交，则该字段取值为0。

	*/
	if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_NEW_ORDER) ||
		0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_CANCELREJECT))
	{
		strInsertConfirmSql += "'','";
	}
	else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL) ||
		0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += in_ptrReport->strLastQty;
		strInsertConfirmSql += "','";
	}
	else
	{
		std::string strError("委托clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]Msgtype[";
		strError += in_ptrReport->strMsgType;
		strError += "]错误";
		EzLog::e(strTag, strError);
		return -1;
	}

	// remark,status1,teordernum,owflag,ordrec
	if (0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		strInsertConfirmSql += "0";
	}
	else
	{
		if (in_ptrReport->strErrorCode.empty())
		{
			strInsertConfirmSql += " ";
		}
		else
		{
			strInsertConfirmSql += in_ptrReport->strErrorCode;
		}
	}
	strInsertConfirmSql += "','";

	if (0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		strInsertConfirmSql += "R";
	}
	else
	{
		strInsertConfirmSql += "R";
		//strInsertConfirmSql += "P";
	}
	strInsertConfirmSql += "','";

	if (0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		uint64_t ui64Value = 0;
		// 先找session
		if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
		{
			ui64Value = simutgw::g_mapShConns[strName].IncRecNum();
		}
		strInsertConfirmSql += sof_string::itostr(ui64Value, strValue);
	}
	else
	{
		//strInsertConfirmSql += sof_string::itostr(simutgw::g_iTeordernum, strValue);
		uint64_t ui64Value = 0;
		// 先找session
		if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
		{
			ui64Value = simutgw::g_mapShConns[strName].GetTeordNum();
		}
		strInsertConfirmSql += sof_string::itostr(ui64Value, strValue);
	}
	strInsertConfirmSql += "','";

	if (0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		strInsertConfirmSql += "WTH";
	}
	else
	{
		strInsertConfirmSql += in_ptrReport->strOrdType;
	}
	strInsertConfirmSql += "',";

	if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_NEW_ORDER) ||
		0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_CANCELREJECT))
	{
		strInsertConfirmSql += "0,";
	}
	else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL) ||
		0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		//撤单时与strClordid相反
		strInsertConfirmSql += "'";
		strInsertConfirmSql += in_ptrReport->strClordid;
		strInsertConfirmSql += "',";
	}
	else
	{
		std::string strError("委托clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]Msgtype[";
		strError += in_ptrReport->strMsgType;
		strError += "]错误";
		EzLog::e(strTag, strError);
		return -1;
	}

	//	firmid,branchid,checkord
	if (!in_ptrReport->strConfirmID.empty())
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += in_ptrReport->strConfirmID;
		strInsertConfirmSql += "','";
	}
	else
	{
		strInsertConfirmSql += "null,'";
	}

	strInsertConfirmSql += in_ptrReport->strMarket_branchid;
	strInsertConfirmSql += "',0x00000000000000000000000000000000);";

	out_vectSqlStr.push_back(strInsertConfirmSql);
	return 0;
}

/*
FIX协议格式数据 按JSON配置规则 转换成FIX协议格式上海确认，返回一个sql串

Reutrn:
0 -- 成功
-1 -- 失败
*/
int ReportMsg_Sh::FixToSHConfirm_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::vector<std::string>& out_vectSqlStr)
{
	static const string ftag("ReportMsg_Sh::FixToSHConfirm_JsonRule() ");

	if (!in_ptrReport->tradePolicy.ptrRule_Sh->docRuleConfirm.IsObject())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] error Rule match, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

		return -1;
	}

	RuleWordProc_ShDb shdbRuleProc;
	int iResolveRes = 0;
	string strResolveValue("");

	rapidjson::Value& elem = in_ptrReport->tradePolicy.ptrRule_Sh->docRuleConfirm;
	if (!elem.IsObject())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] error Rule confirm, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

		return -1;
	}

	string strInsertConfirmSql = "INSERT INTO ";
	strInsertConfirmSql += simutgw::g_strSH_Ordwth2_TableName;
	strInsertConfirmSql += " (rec_num,date,time,reff,acc,stock,"
		"bs,price,qty,status,qty2,remark,status1,teordernum,owflag,ordrec,firmid,branchid,checkord) VALUES(";


	string strValue;

	//rec_num
	iResolveRes = shdbRuleProc.ResolveRule(elem, "rec_num", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- rec_num";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	//date,time,reff,acc,stock,
	iResolveRes = shdbRuleProc.ResolveRule(elem, "date", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- date";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "time", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- time";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "reff", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- reff";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "acc", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- acc";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "stock", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- stock";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	//bs,price,qty,status,qty2,
	iResolveRes = shdbRuleProc.ResolveRule(elem, "bs", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- bs";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "price", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- price";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "qty", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- qty";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}
	/*
	接收状态，
	‘F’表示交易所后台判断该订单为废单。
	‘E’表示交易所前台判断该订单为废单；此时remark（字段12：错误信息）给出错误代码。
	‘?’表示通信故障。
	‘O’表示上交所成功接收该笔申报。
	‘W’表示上交所成功接受该笔撤单；
	当订单类型为最优五档即时成交剩余自动撤销的市价订单时，且订单有效时，该字段取值为‘W’。

	*/
	iResolveRes = shdbRuleProc.ResolveRule(elem, "status", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- status";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}


	//qty2
	/*
	撤单数量，
	对于限价订单申报记录，该字段为空；
	对于撤单记录，该字段为实际撤单返回数量；
	对于最优五档即时成交剩余自动撤销的市价订单，如果申报部分成交，该字段取值为自动撤单的数量；如果申报全部成交，则该字段取值为0。

	*/
	iResolveRes = shdbRuleProc.ResolveRule(elem, "qty2", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- qty2";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	// remark,status1,teordernum,owflag,ordrec
	iResolveRes = shdbRuleProc.ResolveRule(elem, "remark", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- remark";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "status1", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- status1";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "teordernum", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- teordernum";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "owflag", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- owflag";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "ordrec", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- ordrec";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	//	firmid,branchid,checkord
	iResolveRes = shdbRuleProc.ResolveRule(elem, "firmid", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- firmid";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "branchid", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- branchid";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "checkord", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- checkord";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "0x00000000000000000000000000000000";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "'";
	}

	strInsertConfirmSql += ");";

	out_vectSqlStr.push_back(strInsertConfirmSql);
	return 0;
}

/*
FIX协议格式数据转换成FIX协议格式上海回报，返回一个sql串
*/
int ReportMsg_Sh::FixToSHReport(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	vector<string>& out_vectSqlStr)
{
	static const string strTag("ReportMsg_Sh::FixToSHReport() ");

	string strInsertReportSql = "INSERT INTO ";
	strInsertReportSql += simutgw::g_strSH_Cjhb_TableName;
	strInsertReportSql += "(gddm,gdxm,bcrq,cjbh,gsdm,cjsl,bcye,zqdm,"
		"sbsj,cjsj,cjjg,cjje,sqbh,bs,mjbh) VALUES('";

	size_t pos = in_ptrReport->strTransactTime.find("-");
	if (string::npos == pos)
	{
		string strError("委托clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]TransactTime[";
		strError += in_ptrReport->strTransactTime;
		strError += "]格式不对";
		EzLog::e(strTag, strError);

		return -1;
	}
	//成交日期，格式为YYYYMMDD, strTransactTime:YYYYMMDD-HH:MM:SS.sss
	string strTransDate = in_ptrReport->strTransactTime.substr(0, pos);

	size_t posTime = in_ptrReport->strTransactTime.find(".");
	if (string::npos == posTime)
	{
		string strError("委托clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]TransactTime[";
		strError += in_ptrReport->strTransactTime;
		strError += "]格式不对";
		EzLog::e(strTag, strError);

		return -1;
	}
	//成交时间，格式为HHMMSS
	string strTransTime = in_ptrReport->strTransactTime.substr(pos + 1, posTime - pos - 1);
	while (true)
	{
		if (string::npos == strTransTime.find(":"))
		{
			break;
		}

		strTransTime.replace(strTransTime.find(":"), 1, "");
	}

	//gddm,gdxm,bcrq,cjbh,gsdm,
	strInsertReportSql += in_ptrReport->strAccountId;
	strInsertReportSql += "','";

	strInsertReportSql += "1','";

	strInsertReportSql += strTransDate;
	strInsertReportSql += "','";

	uint64_t ui64Value = 0;
	string strName = in_ptrReport->strSessionId;
	// 先找session
	if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
	{
		ui64Value = simutgw::g_mapShConns[strName].IncCjbh();
	}
	else
	{
		std::string strError("上海connection[");
		strError += strName;
		strError += "]丢失";
		EzLog::e(strTag, strError);
		return -1;
	}

	sof_string::itostr(ui64Value, in_ptrReport->strOrderID);
	strInsertReportSql += in_ptrReport->strOrderID;
	strInsertReportSql += "','";

	strInsertReportSql += in_ptrReport->strSecurity_seat;
	strInsertReportSql += "','";

	//cjsl,bcye,zqdm,sbsj,cjsj
	strInsertReportSql += in_ptrReport->strLastQty;
	strInsertReportSql += "','";

	strInsertReportSql += "0000000001','";

	strInsertReportSql += in_ptrReport->strStockID;
	strInsertReportSql += "','";

	//sbsj
	strInsertReportSql += strTransTime;
	strInsertReportSql += "','";

	//cjsj
	strInsertReportSql += strTransTime;
	strInsertReportSql += "','";

	//cjjg,cjje,sqbh,bs,mjbh
	// cjjg	成交价格，单位为人民币元(A股、基金、债券)或美元(B股)或每百元资金的年收益率(国债回购)。精度为小数点后3位。
	string strItoa;
	strInsertReportSql += Tgw_StringUtil::iLiToStr(in_ptrReport->ui64mPrice_matched, strItoa, 3);
	strInsertReportSql += "','";


	/*
	cjje 成交金额。精度为小数点后2位。
	如果实际成交金额超过999,999,999.99，系统会填写-1，请使用该字段的市场参与者特殊处理，
	柜台系统应能识别并及时处理该字段溢出异常，可采取如自行计算或拆分调整，盘后采用登记结算数据或其他方式处理，做好该异常的识别和处理。
	*/
	//strInsertReportSql += in_ptrReport->strCashorderqty;
	if (in_ptrReport->ui64mCashorderqty_matched > 999999999990)
	{
		strInsertReportSql += "-1";
	}
	else
	{
		strInsertReportSql += Tgw_StringUtil::iLiToStr(in_ptrReport->ui64mCashorderqty_matched, strItoa, 2);
	}
	strInsertReportSql += "','";

	strInsertReportSql += in_ptrReport->strClordid;
	strInsertReportSql += "','";

	strInsertReportSql += in_ptrReport->strSide;

	strInsertReportSql += "','";

	strInsertReportSql += "66666";
	strInsertReportSql += "');";

	out_vectSqlStr.push_back(strInsertReportSql);

	return 0;
}


/*
FIX协议格式数据 按JSON配置规则 转换成FIX协议格式上海回报，返回一个sql串

Reutrn:
0 -- 成功
-1 -- 失败
*/
int ReportMsg_Sh::FixToSHReport_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	vector<string>& out_vectSqlStr)
{
	static const string ftag("ReportMsg_Sh::FixToSHReport_JsonRule() ");

	if (!in_ptrReport->tradePolicy.ptrRule_Sh->docRuleMatch.IsArray())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
			<< "] error Rule match, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

		return -1;
	}

	RuleWordProc_ShDb shdbRuleProc;
	int iResolveRes = 0;
	string strResolveValue("");

	for (rapidjson::SizeType i = 0; i < in_ptrReport->tradePolicy.ptrRule_Sh->docRuleMatch.Size(); ++i)
	{
		rapidjson::Value& elem = in_ptrReport->tradePolicy.ptrRule_Sh->docRuleMatch[i];
		if (!elem.IsObject())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] error Rule match, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

			return -1;
		}

		string strInsertReportSql = "INSERT INTO ";
		strInsertReportSql += simutgw::g_strSH_Cjhb_TableName;
		strInsertReportSql += "(gddm,gdxm,bcrq,cjbh,gsdm,cjsl,bcye,zqdm,"
			"sbsj,cjsj,cjjg,cjje,sqbh,bs,mjbh) VALUES(";

		//gddm,gdxm,bcrq,cjbh,gsdm,
		iResolveRes = shdbRuleProc.ResolveRule(elem, "gddm", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- gddm";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "gdxm", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- gdxm";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "bcrq", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- bcrq";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "cjbh", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- cjbh";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "gsdm", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- gsdm";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		//cjsl,bcye,zqdm,sbsj,cjsj
		iResolveRes = shdbRuleProc.ResolveRule(elem, "cjsl", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- cjsl";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "bcye", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- bcye";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "zqdm", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- zqdm";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "sbsj", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- sbsj";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "cjsj", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- cjsj";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		//cjjg,cjje,sqbh,bs,mjbh
		// cjjg	成交价格，单位为人民币元(A股、基金、债券)或美元(B股)或每百元资金的年收益率(国债回购)。精度为小数点后3位。
		iResolveRes = shdbRuleProc.ResolveRule(elem, "cjjg", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- cjjg";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		/*
		cjje 成交金额。精度为小数点后2位。
		如果实际成交金额超过999,999,999.99，系统会填写-1，请使用该字段的市场参与者特殊处理，
		柜台系统应能识别并及时处理该字段溢出异常，可采取如自行计算或拆分调整，盘后采用登记结算数据或其他方式处理，做好该异常的识别和处理。
		*/
		iResolveRes = shdbRuleProc.ResolveRule(elem, "cjje", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- cjje";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "sqbh", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- sqbh";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "bs", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- bs";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "mjbh", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "委托clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- mjbh";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "'";
		}

		strInsertReportSql += ");";

		out_vectSqlStr.push_back(strInsertReportSql);
	}

	return 0;
}

/*
处理上海撤单

@param std::string& out_strSql_confirm : 写入确认表的数据

Return:
0 -- 成功
-1 -- 失败
*/
int ReportMsg_Sh::ProcSHCancelOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::vector<std::string>& out_vectSqlStr)
{
	static const string strTag("ReportMsg_Sh::ProcSHCancelOrder() ");

	int iRes = 0;

	if (in_ptrReport->enMatchType == simutgw::CancelMatch)
	{
		if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_EXECREPORT))
		{
			// 撤单成功
			RecordTradeInfo::WriteTransInfoInDb_CancelSuccess(in_ptrReport);

			iRes = FixToSHConfirm(in_ptrReport, out_vectSqlStr);

			if (0 == iRes)
			{
				string strSendLog("Sended SH Cancel Report reff=");
				strSendLog += in_ptrReport->strClordid;
				strSendLog += ", sh_conn=";
				strSendLog += in_ptrReport->strSessionId;

				EzLog::i(strTag, strSendLog);
			}

			simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchCancel();

		}
		else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_CANCELREJECT))
		{
			/* 先更新表*/
			RecordTradeInfo::WriteTransInfoInDb_CancelFail(in_ptrReport);

			iRes = FixToSHConfirm(in_ptrReport, out_vectSqlStr);

			if (0 == iRes)
			{
				string strSendLog("Sended SH Cancel Report reff=");
				strSendLog += in_ptrReport->strClordid;

				EzLog::i(strTag, strSendLog);
			}
		}

	}

	return 0;
}

/*
处理上海错误订单


Return:
0 -- 成功
-1 -- 失败
1 -- 无回报
*/
int ReportMsg_Sh::ProcSHErrorOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	vector<string>& out_vectSqlStr)
{
	static const string strTag("ReportMsg_Sh::ProcSHErrorOrder() ");

	int iRes = 0;

	//从mysql连接池取连接
	RecordTradeInfo::WriteTransInfoInDb_Error(in_ptrReport);

	string strReject, strStepReject;
	if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_NEW_ORDER))
	{

		// 买卖单
		iRes = FixToSHConfirm(in_ptrReport, out_vectSqlStr);

	}
	else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
	{
		// 撤单

	}

	simutgw::g_counter.GetSh_InnerCounter()->Inc_Error();

	string strSendLog("Sended SH Ordwth2 reff=");
	strSendLog += in_ptrReport->strClordid;

	EzLog::i(strTag, strSendLog);

	return 0;
}

/*
处理上海确认

Return:
0 -- 成功
-1 -- 失败
1 -- 无回报
*/
int ReportMsg_Sh::ProcSHConfirmOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::vector<std::string>& out_vectSqlStr)
{
	static const string ftag("ReportMsg_Sh::ProcSHConfirmOrder() ");

	int iRes = 0;

	if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		//	深圳
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "wrong place SZ Report clordid=" << in_ptrReport->strClordid
			<< ", client=" << in_ptrReport->strSenderCompID;

		return -1;
	}
	else if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// 上海
		if (0 != in_ptrReport->tradePolicy.ui64RuleId)
		{
			// 已定义了成交规则
			if (nullptr == in_ptrReport->tradePolicy.ptrRule_Sh)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error order cliordid=" << in_ptrReport->strClordid
					<< "nullptr Rule Sh, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

				return -1;
			}

			// report with rule
			iRes = FixToSHConfirm_JsonRule(in_ptrReport, out_vectSqlStr);
		}
		else
		{
			iRes = FixToSHConfirm(in_ptrReport, out_vectSqlStr);
		}

		if (0 != iRes)
		{
			return -1;
		}

		return 0;
	}

	BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "clordid=" << in_ptrReport->strClordid
		<< ", client=" << in_ptrReport->strSenderCompID << " error Trade_market=" << in_ptrReport->strTrade_market;

	return -1;
}

/*
处理上海回报业务
Return:
0 -- 成功
-1 -- 失败
1 -- 无回报
*/
int ReportMsg_Sh::Get_SHReport(map<string, vector<string>>& out_mapUpdate)
{
	static const string strTag("ReportMsg_Sh::ProcSHReport() ");

	string strSzReport;
	vector<string> vectSqlStr;

	int iMaxLoopcount = 20000;
	for (int i = 0; i < iMaxLoopcount; ++i)
	{
		// 取上海回报
		std::shared_ptr<struct simutgw::OrderMessage> ptrReport(new struct simutgw::OrderMessage);

		int iRes = simutgw::g_outMsg_buffer.PopFront_sh(ptrReport);
		if (iRes < 0)
		{
			EzLog::e(strTag, "ReadReport() faild");

			continue;
		}
		else if (iRes > 0)
		{
			// 无回报
			return 0;
		}
		else
		{
			vectSqlStr.clear();

			// 记录处理流水
			RecordReportHelper::RecordReportToDb(ptrReport);
			if (ptrReport->enMatchType == simutgw::MatchAll)
			{
				// 处理正常成交回报
				iRes = ProcSingleReport(ptrReport, vectSqlStr);
				if (0 == iRes)
				{
					//simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchAll();
				}
			}
			else if (ptrReport->enMatchType == simutgw::MatchPart)
			{
				// 处理正常成交回报
				iRes = ProcSingleReport(ptrReport, vectSqlStr);
				if (0 == iRes)
				{
					//simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchPart();
				}
			}
			else if (ptrReport->enMatchType == simutgw::CancelMatch)
			{
				// 处理撤单回报
				iRes = ProcSHCancelOrder(ptrReport, vectSqlStr);

				if (0 == iRes)
				{
					simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchCancel();
				}
			}
			else if (ptrReport->enMatchType == simutgw::ErrorMatch)
			{
				// 处理废单回报
				iRes = ProcSHErrorOrder(ptrReport, vectSqlStr);

				if (0 == iRes)
				{
					simutgw::g_counter.GetSh_InnerCounter()->Inc_Error();
				}
			}
			else if (ptrReport->enMatchType == simutgw::NotMatch)
			{
				// 确认
				iRes = ProcSHConfirmOrder(ptrReport, vectSqlStr);
				if (0 == iRes)
				{
					//simutgw::g_counter.GetSh_InnerCounter()->Inc_Confirm();
				}
			}
			else
			{
				std::string strTrans, strError("clordid[");
				strError += ptrReport->strClordid;
				strError += "]MatchType[";
				strError += sof_string::itostr(ptrReport->enMatchType, strTrans);
				EzLog::e(strTag, strError);

				continue;
			}

			if (!vectSqlStr.empty())
			{
				if (out_mapUpdate.end() == out_mapUpdate.find(ptrReport->strSessionId))
				{
					vector<string> vecSql;
					out_mapUpdate.insert(make_pair(ptrReport->strSessionId, vecSql));
				}

				out_mapUpdate[ptrReport->strSessionId].insert(out_mapUpdate[ptrReport->strSessionId].end(),
					vectSqlStr.begin(), vectSqlStr.end());
			}

		}
	}

	return 0;
}
