#include "ShConn_DbfOrder.h"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "simutgw/msg_biz/ProcCancelOrder.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/sof_string.h"

#include "tool_odbc/OTLConn40240.h"

#include "order/StockOrderHelper.h"
#include "simutgw/order/OrderMemoryStoreFactory.h"
#include "simutgw/db_oper/RecordNewOrderHelper.h"

#include "config/conf_mssql_table.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw_config/g_values_sys_run_config.h"

#include "simutgw/biz/AppId_010_OrderValide.h"
#include "simutgw/biz/PreTrade.h"
#include "simutgw/biz/MatchUtil.h"

#include "ShDbfOrderHelper.h"
#include "util/TimeDuration.h"


int ShConn_DbfOrder::GetOneConnSHOrder(const std::string& in_strShConnName,
	std::vector<std::shared_ptr<struct simutgw::OrderMessage>> &io_vecOrder)
{
	/*
	{"rpc":1,"tables":[{"acc":"A645078963","branchid":"     ","bs":"B","checkord":"","date":"20170525",
	"firmid":"     ","ordrec":"0       ","owflag":"LPT","price":"15.000  ","qty":"100     ","rec_num":"1",
	"reff":"J891200002","status":"R","stock":"600446","time":"14:05:31"}]}
	*/
	static const string strTag("ShConn_DbfOrder::GetOneConnSHOrder() ");

	int iRes = 0;
	if (0 == simutgw::g_mapShConns[in_strShConnName].GetCjbh())
	{
		// 重新启动后，需要重新获取cjbh
		iRes = GetSHCjbh(in_strShConnName);
		if (0 != iRes)
		{
			return -1;
		}
	}

	if (0 == simutgw::g_mapShConns[in_strShConnName].GetRecNum())//(0 == simutgw::g_iRec_Num)
	{
		iRes = GetSHRec_Num(in_strShConnName);
		if (0 != iRes)
		{
			return -1;
		}
	}

	OTLConn40240 otlConn;
	iRes = otlConn.Connect(simutgw::g_mapShConns[in_strShConnName].GetConnection());
	if (0 != iRes)
	{
		string strError("连接失败，str=");
		strError += in_strShConnName;
		EzLog::e(strTag, strError);
		return -1;
	}

	string strQuerySql("select top 10000 * from ");
	strQuerySql += simutgw::g_strSH_Ordwth_TableName;
	strQuerySql += " where status='R'";

	otl_stream stream;
	iRes = otlConn.Query(strQuerySql, &stream);
	if (0 != iRes)
	{
		//查询失败
		string strError("查询失败[");
		strError += strQuerySql;
		EzLog::e(strTag, strError);

		return -1;
	}

	// 交易策略
	struct TradePolicyCell currentPolicy;
	
	string strShConn_Name("");
	string strShConn_SettleGroupName("");

	// 获取清算池别名
	// web管理模式下才需要查询
	if (simutgw::WebManMode::WebMode == simutgw::g_iWebManMode)
	{
		std::map<std::string, struct Connection_webConfig>::const_iterator itSh = simutgw::g_mapShConn_webConfig.find(in_strShConnName);
		if (simutgw::g_mapShConn_webConfig.end() == itSh)
		{
			//
			string sDebug("未在 simutgw::g_mapShConn_webConfig 找到 ConnName=");
			sDebug += in_strShConnName;
			EzLog::e(strTag, sDebug);
		}
		else
		{
			strShConn_Name = itSh->second.strWebLinkid;
			strShConn_SettleGroupName = itSh->second.strSettleGroupName;
			currentPolicy.mapLinkRules.insert(itSh->second.mapLinkRules.begin(), itSh->second.mapLinkRules.end());
		}
	}

	currentPolicy.strSettleGroupName = strShConn_SettleGroupName;

	// 获取当前连接的交易策略
	simutgw::g_tradePolicy.Get_Sh(in_strShConnName,
		currentPolicy.iRunMode, currentPolicy.bCheck_Assets,
		currentPolicy.iMatchMode, currentPolicy.iQuotationMatchType, currentPolicy.iPart_Match_Num);

	iRes = ShDbfOrderHelper::SHOrderToFix(otlConn, stream, io_vecOrder, in_strShConnName, currentPolicy);
	if (0 != iRes)
	{
		return -1;
	}

	// 记录链路接收
	size_t tSize = io_vecOrder.size();
	for (size_t i = 0; i < tSize; ++i)
	{
		simutgw::g_counter.IncSh_Link_Receive(in_strShConnName);
	}

	return 0;
}

/*
从ashare_ordwth取委托
*/
int ShConn_DbfOrder::BatchGetSHOrder(vector <std::shared_ptr<struct simutgw::OrderMessage>> &io_vecOrder)
{
	static const string strTag("ShConn_DbfOrder::BatchGetSHOrder() ");

	std::map<string, ShConnection>::iterator it = simutgw::g_mapShConns.begin();
	while (it != simutgw::g_mapShConns.end())
	{
		GetOneConnSHOrder(it->first, io_vecOrder);
		++it;
	}

	return 0;
}

// 校验下单消息，记录入数据库
int ShConn_DbfOrder::Valide_Record_Order(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg)
{
	static const string strTag("ShConn_DbfOrder::Valide_Record_Order() ");

	int iRes = 0;

	//下单消息写入数据库
	if (0 == in_OrderMsg->strMsgType.compare("F"))
	{
		string strReciveLog("Recived SH Cancel Order ordrec=");
		strReciveLog += in_OrderMsg->strClordid;
		strReciveLog += ", sh_conn=";
		strReciveLog += in_OrderMsg->strSessionId;

		EzLog::i(strTag, strReciveLog);
	}
	else
	{
		string strPolicyOut;
		string strCircleID;
		in_OrderMsg->tradePolicy.DebugOut(strPolicyOut);

		string strReciveLog("Recived SH Order reff=");
		strReciveLog += in_OrderMsg->strClordid;
		strReciveLog += ", sh_conn=";
		strReciveLog += in_OrderMsg->strSessionId;
		strReciveLog += strPolicyOut;

		MatchUtil::Get_Order_CircleID(in_OrderMsg, strCircleID);
		strReciveLog += ", CircleID=";
		strReciveLog += strCircleID;

		EzLog::i(strTag, strReciveLog);
	}

	if (0 != in_OrderMsg->tradePolicy.ui64RuleId)
	{
		// 已定义了成交规则
		if (nullptr == in_OrderMsg->tradePolicy.ptrRule_Sh)
		{
			string sDebug("error order cliordid=");
			sDebug += in_OrderMsg->strClordid;
			sDebug +="nullptr Rule Sh, ruleId=";
			string sItoa;
			sof_string::itostr(in_OrderMsg->tradePolicy.ui64RuleId, sItoa);
			sDebug += sItoa;
			EzLog::e(strTag, sDebug);

			return -1;
		}
		
		// ?? check with rule
	}
	else
	{ // 检查消息合法性
		iRes = AppId_010_OrderValide::ValidateOrder(in_OrderMsg);
		if (iRes < 0)
		{
			EzLog::e(strTag, "validate, error order cliordid=" + in_OrderMsg->strClordid);

			// 写入回报队列
			simutgw::g_outMsg_buffer.PushBack(in_OrderMsg);
			return -1;
		}

		// 交易前检查
		iRes = PreTrade::TradePrep(in_OrderMsg);
		if (-2 == iRes)
		{
			in_OrderMsg->bDataError = true;
			in_OrderMsg->enMatchType = simutgw::MatchType::ErrorMatch;
			in_OrderMsg->strErrorCode = simutgw::SH_ERRCODE::c212;
			in_OrderMsg->strError = "价格无效";

			string sDebug("tradeprep, error order reff=");
			sDebug += in_OrderMsg->strClordid;
			sDebug += ",err=";
			sDebug += in_OrderMsg->strError;
			EzLog::e(strTag, sDebug);

			// 写入回报队列
			simutgw::g_outMsg_buffer.PushBack(in_OrderMsg);
			return -1;
		}
		else if (iRes < 0)
		{
			in_OrderMsg->bDataError = true;
			in_OrderMsg->enMatchType = simutgw::MatchType::ErrorMatch;
			in_OrderMsg->strErrorCode = simutgw::SH_ERRCODE::c231;
			in_OrderMsg->strError = "冻结股份失败";

			string sDebug("tradeprep, error order reff=");
			sDebug += in_OrderMsg->strClordid;
			sDebug += ",err=";
			sDebug += in_OrderMsg->strError;
			EzLog::e(strTag, sDebug);

			// 写入回报队列
			simutgw::g_outMsg_buffer.PushBack(in_OrderMsg);
			return -1;
		}
	}
	
	iRes = RecordNewOrderHelper::RecordInOrderToDb_Match(in_OrderMsg);
	if (iRes < 0)
	{
		EzLog::e(strTag, "Write msg to table faild");
	}

	return 0;
}

/*
修改ordwth表的status状态为P
*/
int ShConn_DbfOrder::SetOrdwthStatus(const std::string& in_strShConnName,
	const string &in_strRec_num, const string &in_strStatus)
{
	static const string strTag("ShConn_DbfOrder::SetOrdwthStatus()");

	string strUpdateSql("update ");
	strUpdateSql += simutgw::g_strSH_Ordwth_TableName;
	strUpdateSql += " set status='P' where rec_num=";
	strUpdateSql += in_strRec_num;

	OTLConn40240 con;
	int iRes = 0;
	iRes = con.Connect(simutgw::g_mapShConns[in_strShConnName].GetConnection());
	if (0 != iRes)
	{
		string strError("连接失败，str=");
		strError += in_strShConnName;
		EzLog::e(strTag, strError);
		return -1;
	}

	long lAffectRows;

	iRes = con.Exec(strUpdateSql, lAffectRows);

	if (-1 == iRes)
	{
		string strError("执行失败[");
		strError += strUpdateSql;
		EzLog::e(strTag, strError);
	}
	con.Commit();

	return 0;
}

/*
修改ordwth表的一批记录的status状态为P
*/
int ShConn_DbfOrder::Set_MutilOrd_Status(const std::string& in_strShConnName,
	const vector<std::string> &in_VecRec_num)
{
	static const string strTag("ShConn_DbfOrder::Set_MutilOrd_Status()");

	if (0 == in_VecRec_num.size())
	{
		return 0;
	}

	string strUpdateSqlBase("update ");
	strUpdateSqlBase += simutgw::g_strSH_Ordwth_TableName;
	strUpdateSqlBase += " set status='P' where rec_num in (";

	OTLConn40240 con;
	int iRes = 0;
	iRes = con.Connect(simutgw::g_mapShConns[in_strShConnName].GetConnection());
	if (0 != iRes)
	{
		string strError("连接失败，str=");
		strError += in_strShConnName;
		EzLog::e(strTag, strError);
		return -1;
	}

	long lAffectRows;

	for (size_t i = 0; i < in_VecRec_num.size(); ++i)
	{
		strUpdateSqlBase += in_VecRec_num[i];
		if (i < in_VecRec_num.size() - 1)
		{
			strUpdateSqlBase += ",";
		}
	}

	strUpdateSqlBase += ")";
	iRes = con.Exec(strUpdateSqlBase, lAffectRows);

	if (-1 == iRes)
	{
		string strError("执行失败[");
		strError += strUpdateSqlBase;
		EzLog::e(strTag, strError);
	}
	con.Commit();

	return 0;
}

/*
取上海成交编号
Return:
0 -- 成功
-1 -- 失败
*/
int ShConn_DbfOrder::GetSHCjbh(const std::string& in_strShConnName)
{
	static const string strTag("ShConn_DbfOrder::GetSHCjbh()");

	int iRes = 0;

	OTLConn40240 otlConn;
	iRes = otlConn.Connect(simutgw::g_mapShConns[in_strShConnName].GetConnection());
	if (0 != iRes)
	{
		return -1;
	}

	string strQuerySql("SELECT TOP 1 cjbh FROM ");
	strQuerySql += simutgw::g_strSH_Cjhb_TableName;
	strQuerySql += " ORDER BY cjbh DESC";

	otl_stream stream;
	iRes = otlConn.Query(strQuerySql, &stream);
	if (0 != iRes)
	{
		//查询失败
		string strError("查询失败[");
		strError += strQuerySql;
		EzLog::e(strTag, strError);

		return -1;
	}

	string strCjbh;
	std::map<std::string, struct OTLConn_DF::DataInRow> mapRowData;
	while (otlConn.FetchNextRow(&stream, mapRowData))
	{
		strCjbh = mapRowData["cjbh"].strValue;
	}

	uint64_t ui64Value;
	Tgw_StringUtil::String2UInt64_strtoui64(strCjbh, ui64Value);

	simutgw::g_mapShConns[in_strShConnName].SetCjbh(ui64Value);

	string strTrans;
	//EzLog::i(strTag, sof_string::itostr(simutgw::g_iNextCjbh, strTrans));
	return 0;
}

/*
取上海确认编号
Return:
0 -- 成功
-1 -- 失败
*/
int ShConn_DbfOrder::GetSHRec_Num(const std::string& in_strShConnName)
{
	static const string strTag("ShConn_DbfOrder::GetSHRec_Num()");

	int iRes = 0;

	OTLConn40240 otlConn;
	iRes = otlConn.Connect(simutgw::g_mapShConns[in_strShConnName].GetConnection());
	if (0 != iRes)
	{
		return -1;
	}

	string strQuerySql("SELECT TOP 1 rec_num FROM ");
	strQuerySql += simutgw::g_strSH_Ordwth2_TableName;
	strQuerySql += " ORDER BY rec_num DESC";

	otl_stream stream;
	iRes = otlConn.Query(strQuerySql, &stream);
	if (0 != iRes)
	{
		//查询失败
		string strError("查询失败[");
		strError += strQuerySql;
		EzLog::e(strTag, strError);

		return -1;
	}

	string strRecNum;
	std::map<std::string, struct OTLConn_DF::DataInRow> mapRowData;
	while (otlConn.FetchNextRow(&stream, mapRowData))
	{
		strRecNum = mapRowData["rec_num"].strValue;
	}

	uint64_t ui64Value;
	Tgw_StringUtil::String2UInt64_strtoui64(strRecNum, ui64Value);

	simutgw::g_mapShConns[in_strShConnName].SetRecNum(ui64Value);

	string strTrans;
	//EzLog::i(strTag, sof_string::itostr(simutgw::g_iNextCjbh, strTrans));
	return 0;
}
