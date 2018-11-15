#include "SzConn_StepOrder.h"
#include "simutgw/msg_biz/ProcCancelOrder.h"

#include "order/StockOrderHelper.h"

#include "tool_mysql/MySqlCnnC602.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/sof_string.h"

#include "tool_redis/Tgw_RedisHelper.h"

#include "simutgw/db_oper/RecordNewOrderHelper.h"

#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "simutgw/biz/AppId_010_OrderValide.h"
#include "simutgw/biz/AppId_120_OrderValide.h"
#include "simutgw/biz/PreTrade.h"
#include "simutgw/biz/MatchUtil.h"

// 校验下单消息，记录入数据库
int SzConn_StepOrder::Valide_Record_Order(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg)
{
	static const string strTag("SzConn_StepOrder::Valide_Record_Order() ");

	string strValue = in_OrderMsg->strSenderCompID;
	if (simutgw::g_mapSzConns.end() != simutgw::g_mapSzConns.find(strValue))
	{
		if (!simutgw::g_mapSzConns[strValue].GetLogSta())
		{
			// 如果未登录上		
			return 0;
		}
	}
	else
	{
		return 0;
	}

	int iRes = 0;
	//下单消息写入数据库
	if (0 == in_OrderMsg->strMsgType.compare("F"))
	{
		string strReciveLog("Recived SZ Cancel Order cliordid=");
		strReciveLog += in_OrderMsg->strClordid;
		strReciveLog += ", client=";
		strReciveLog += in_OrderMsg->strSenderCompID;

		EzLog::i(strTag, strReciveLog);
	}
	else if (0 == in_OrderMsg->strMsgType.compare("D"))
	{
		string strPolicyOut;
		string strCircleID;
		in_OrderMsg->tradePolicy.DebugOut(strPolicyOut);

		string strReciveLog("Recived SZ Order cliordid=");
		strReciveLog += in_OrderMsg->strClordid;
		strReciveLog += ", client=";
		strReciveLog += in_OrderMsg->strSenderCompID;
		strReciveLog += strPolicyOut;

		MatchUtil::Get_Order_CircleID(in_OrderMsg, strCircleID);
		strReciveLog += ", CircleID=";
		strReciveLog += strCircleID;

		EzLog::i(strTag, strReciveLog);
	}
	else
	{
		//nothing
	}


	if (0 != in_OrderMsg->tradePolicy.ui64RuleId)
	{
		// 已定义了成交规则
		if (nullptr == in_OrderMsg->tradePolicy.ptrRule_Sz)
		{
			string sDebug("error order cliordid=");
			sDebug += in_OrderMsg->strClordid;
			sDebug += "nullptr Rule Sz, ruleId=";
			string sItoa;
			sof_string::itostr(in_OrderMsg->tradePolicy.ui64RuleId, sItoa);
			sDebug += sItoa;
			EzLog::e(strTag, sDebug);

			return -1;
		}

		// ?? check with rule
	}
	else
	{
		// 检查消息合法性
		if (0 == in_OrderMsg->strApplID.compare("010"))
		{
			iRes = AppId_010_OrderValide::ValidateOrder(in_OrderMsg);
		}
		else if (0 == in_OrderMsg->strApplID.compare("120"))
		{
			iRes = AppId_120_OrderValide::ValidateOrder(in_OrderMsg);
		}
		else
		{
			in_OrderMsg->bDataError = true;
			in_OrderMsg->enMatchType = simutgw::MatchType::ErrorMatch;
			in_OrderMsg->strErrorCode = simutgw::SZ_ERRCODE::c20005;
			simutgw::g_outMsg_buffer.PushBack(in_OrderMsg);

			std::string strError("error applid=");
			strError += in_OrderMsg->strApplID;
			strError += "order cliordid=";
			strError += in_OrderMsg->strClordid;

			in_OrderMsg->strError = strError;
			EzLog::e(strTag, strError);

			return -1;
		}

		if (iRes < 0)
		{
			// 写入回报队列
			simutgw::g_outMsg_buffer.PushBack(in_OrderMsg);

			EzLog::e(strTag, "validate, error order cliordid=" + in_OrderMsg->strClordid);

			return -1;
		}

		// 交易前检查
		iRes = PreTrade::TradePrep(in_OrderMsg);
		if (-2 == iRes)
		{
			in_OrderMsg->bDataError = true;
			in_OrderMsg->enMatchType = simutgw::MatchType::ErrorMatch;
			in_OrderMsg->strErrorCode = simutgw::SZ_ERRCODE::c20009;
			in_OrderMsg->strError = "价格无效";

			// 写入回报队列
			simutgw::g_outMsg_buffer.PushBack(in_OrderMsg);

			string sDebug("tradeprep, error order cliordid=");
			sDebug += in_OrderMsg->strClordid;
			sDebug += ",err=";
			sDebug += in_OrderMsg->strError;
			EzLog::e(strTag, sDebug);

			return -1;
		}
		else if (iRes < 0)
		{
			in_OrderMsg->bDataError = true;
			in_OrderMsg->enMatchType = simutgw::MatchType::ErrorMatch;
			in_OrderMsg->strErrorCode = simutgw::SZ_ERRCODE::c20022;
			in_OrderMsg->strError = "冻结股份失败";

			// 写入回报队列
			simutgw::g_outMsg_buffer.PushBack(in_OrderMsg);

			string sDebug("tradeprep, error order cliordid=");
			sDebug += in_OrderMsg->strClordid;
			sDebug += ",err=";
			sDebug += in_OrderMsg->strError;
			EzLog::e(strTag, sDebug);

			return -1;
		}
	}

	iRes = RecordNewOrderHelper::RecordInOrderToDb_Match(in_OrderMsg);
	if (iRes < 0)
	{
		EzLog::e(strTag, "Write msg to table faild");

		return -1;
	}

	return 0;
}
