#include "ShDbfOrderHelper.h"

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
#include "tool_string/TimeStringUtil.h"
#include "tool_string/sof_string.h"

#include "order/StockOrderHelper.h"
#include "simutgw/order/OrderMemoryStoreFactory.h"
#include "simutgw/db_oper/RecordNewOrderHelper.h"

#include "simutgw_config/g_values_sys_run_config.h"

#include "config/conf_mssql_table.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "simutgw/biz/AppId_010_OrderValide.h"
#include "simutgw/biz/PreTrade.h"

#include "ShConn_DbfOrder.h"

/*
上海委托数据转换成FIX协议格式
*/
int ShDbfOrderHelper::SHOrderToFix(OTLConn40240& otlConn, otl_stream& in_stream,
	vector<std::shared_ptr<struct simutgw::OrderMessage>> &io_vecOrder,
	const string &strSessionid, const struct TradePolicyCell& in_policy)
{
	static const string strTag("ShDbfOrderHelper::SHOrderToFix() ");

	vector<string> vecRecNum;
	string strKey, strDate, strTime;
	std::map<std::string, struct OTLConn_DF::DataInRow> mapRowData;
	while (otlConn.FetchNextRow(&in_stream, mapRowData))
	{
		vecRecNum.push_back(mapRowData["rec_num"].strValue);

		//解析Arrary中的一条order
		std::shared_ptr<struct simutgw::OrderMessage> shOrder(new struct simutgw::OrderMessage);

		// 写入策略
		shOrder->tradePolicy = in_policy;

		// 委托收到时间
		shOrder->tRcvTime = time(NULL);

		rapidjson::Document::AllocatorType& jsAlloc = shOrder->jsOrder.GetAllocator();

		// rec_num 记录编号，连续递增且唯一
		shOrder->strMsgSeqNum = mapRowData["rec_num"].strValue;
		shOrder->jsOrder.AddMember("rec_num", rapidjson::Value(mapRowData["rec_num"].strValue.c_str(), jsAlloc), jsAlloc);

		if (0 == simutgw::g_mapShConns[strSessionid].GetRecNum())//(0 == simutgw::g_iRec_Num)
		{
			//Tgw_StringUtil::String2UInt64_strtoui64(shOrder->strMsgSeqNum, simutgw::g_iRec_Num);
			uint64_t ui64Value = 0;
			Tgw_StringUtil::String2UInt64_strtoui64(shOrder->strMsgSeqNum, ui64Value);
			simutgw::g_mapShConns[strSessionid].SetRecNum(ui64Value + 1);
		}
		//EzLog::Out("rec_num: ", info, (int)simutgw::g_iRec_Num);

		//date 写入日期 YYYYMMDD
		strDate = mapRowData["date"].strValue;
		shOrder->jsOrder.AddMember("date", rapidjson::Value(mapRowData["date"].strValue.c_str(), jsAlloc), jsAlloc);

		//time 写入时间 HH:MM:SS
		strTime = mapRowData["time"].strValue;
		shOrder->jsOrder.AddMember("time", rapidjson::Value(mapRowData["time"].strValue.c_str(), jsAlloc), jsAlloc);

		//reff 会员内部订单号
		shOrder->strClordid = mapRowData["reff"].strValue;
		shOrder->jsOrder.AddMember("reff", rapidjson::Value(mapRowData["reff"].strValue.c_str(), jsAlloc), jsAlloc);

		//acc 证券账号
		shOrder->strAccountId = mapRowData["acc"].strValue;
		shOrder->jsOrder.AddMember("acc", rapidjson::Value(mapRowData["acc"].strValue.c_str(), jsAlloc), jsAlloc);

		//stock 证券代码
		shOrder->strStockID = mapRowData["stock"].strValue;
		shOrder->jsOrder.AddMember("stock", rapidjson::Value(mapRowData["stock"].strValue.c_str(), jsAlloc), jsAlloc);

		//bs 买卖方向
		shOrder->strSide = mapRowData["bs"].strValue;
		if (0 == shOrder->strSide.compare("b"))
		{
			shOrder->strSide = "B";
		}
		else if (0 == shOrder->strSide.compare("s"))
		{
			shOrder->strSide = "S";
		}

		shOrder->jsOrder.AddMember("bs", rapidjson::Value(mapRowData["bs"].strValue.c_str(), jsAlloc), jsAlloc);

		//price 申报价格
		shOrder->strOrderPrice = mapRowData["price"].strValue;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(shOrder->strOrderPrice, shOrder->ui64mOrderPrice);
		sof_string::itostr(shOrder->ui64mOrderPrice, shOrder->strOrderPrice);

		shOrder->jsOrder.AddMember("price", rapidjson::Value(mapRowData["price"].strValue.c_str(), jsAlloc), jsAlloc);

		//qty 申报数量
		shOrder->strOrderqty_origin = mapRowData["qty"].strValue;
		Tgw_StringUtil::String2UInt64_strtoui64(shOrder->strOrderqty_origin, shOrder->ui64Orderqty_origin);

		shOrder->jsOrder.AddMember("qty", rapidjson::Value(mapRowData["qty"].strValue.c_str(), jsAlloc), jsAlloc);

		//leavesqty
		shOrder->strLeavesQty = shOrder->strOrderqty_origin;
		//"status"))
		// nothing 发送状态，‘R’或‘r’表示该记录还没有发送，‘P’表示已发往上交所后台

		//owflag 订单类型标志
		shOrder->strOrdType = mapRowData["owflag"].strValue;
		shOrder->jsOrder.AddMember("owflag", rapidjson::Value(mapRowData["owflag"].strValue.c_str(), jsAlloc), jsAlloc);

		//ordrec 撤单编号
		shOrder->strOrigClordid = mapRowData["ordrec"].strValue;
		shOrder->jsOrder.AddMember("ordrec", rapidjson::Value(mapRowData["ordrec"].strValue.c_str(), jsAlloc), jsAlloc);

		//firmid B股结算会员代码，对于A股投资者取值无意义
		shOrder->strConfirmID = mapRowData["firmid"].strValue;
		shOrder->jsOrder.AddMember("firmid", rapidjson::Value(mapRowData["firmid"].strValue.c_str(), jsAlloc), jsAlloc);

		//branchid 营业部代码
		shOrder->strMarket_branchid = mapRowData["branchid"].strValue;
		shOrder->jsOrder.AddMember("branchid", rapidjson::Value(mapRowData["branchid"].strValue.c_str(), jsAlloc), jsAlloc);

		//"checkord"
		//nothing 校验码,二进制

		// 上海A股
		shOrder->strTrade_market = simutgw::TRADE_MARKET_SH;

		// transacttime YYYYMMDD-HH:MM:SS.sss
		shOrder->strTransactTime = strDate;
		shOrder->strTransactTime += "-";
		shOrder->strTransactTime += strTime;
		shOrder->strTransactTime += ".000";

		// `oper_time` YYYY-MM-DD HH:MM:SS
		TimeStringUtil::GetCurrTimeInTradeType(shOrder->strOper_time);

		//seesionid
		shOrder->strSessionId = strSessionid;

		//msgtype
		if (0 == shOrder->strOrdType.compare("WTH"))
		{
			shOrder->strMsgType = "F";

			//撤单时，需要交换reff和ordrec
			string strValue = shOrder->strOrigClordid;
			shOrder->strOrigClordid = shOrder->strClordid;
			shOrder->strClordid = strValue;
		}
		else
		{
			shOrder->strMsgType = "D";
		}

		// web管理模式下才需要查询
		if (simutgw::WebManMode::WebMode == simutgw::g_iWebManMode)
		{
			// 判断股票的成交配置规则
			StockOrderHelper::GetOrderMatchRule(shOrder, shOrder->tradePolicy.mapLinkRules);
		}

		// 判断是否已有规则
		if (0 == shOrder->tradePolicy.ui64RuleId)
		{
			// 无成交配置规则时按代码交易走
			// 判断股票交易类型
			int iRes = StockOrderHelper::CheckOrder_TradeType(shOrder);
			if (0 != iRes)
			{
				// 不取该条委托
				ShConn_DbfOrder::SetOrdwthStatus(strSessionid, shOrder->strMsgSeqNum, "P");

				shOrder->enMatchType = simutgw::ErrorMatch;
				shOrder->bDataError = true;

				// 写入回报队列
				simutgw::g_outMsg_buffer.PushBack(shOrder);

				continue;
			}
		}

		io_vecOrder.push_back(shOrder);

	} // while (valIter != doc["tables"].End())

	ShConn_DbfOrder::Set_MutilOrd_Status(strSessionid, vecRecNum);

	return 0;
}

/*
检查上海委托是否是支持的业务类型

Return:
0 -- 支持
-1 -- 不支持
*/
int ShDbfOrderHelper::Validate_SH_Order(std::shared_ptr<struct simutgw::OrderMessage> &shOrder)
{
	static const string strTag("ShDbfOrderHelper::Validate_SH_Order() ");

	int iRes = 0;

	//排除一些非交易业务
	if (0 == shOrder->strAccountId.compare("799988") && 0 == shOrder->strConfirmID.compare("939988"))
	{
		shOrder->bDataError = true;
		shOrder->enMatchType = simutgw::ErrorMatch;
		shOrder->strErrorCode = "269";
		iRes = -1;
	}
	else if (0 == shOrder->strStockID.compare("799999") && 1 == shOrder->ui64mOrderPrice &&
		1 == shOrder->ui64Orderqty_origin)
	{
		// 指定登记
		shOrder->bDataError = true;
		shOrder->enMatchType = simutgw::ErrorMatch;
		shOrder->strErrorCode = "269";
		iRes = -1;
	}
	else if (shOrder->strStockID.empty() && shOrder->strAccountId.empty() && shOrder->strClordid.empty())
	{
		//nothing
		shOrder->bDataError = true;
		shOrder->enMatchType = simutgw::ErrorMatch;
		shOrder->strErrorCode = "269";
		iRes = -1;
	}
	else
	{

	}

	return iRes;
}