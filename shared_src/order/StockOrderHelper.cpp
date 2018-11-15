#include "StockOrderHelper.h"

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"

#include "tool_json/RapidJsonHelper_tgw.h"

#include "simutgw_config/g_values_sys_run_config.h"

#include "etf/conf_etf_info.h"
#include "quotation/MarketInfoHelper.h"

#include "order/StockHelper.h"

#include "simutgw/stgw_config/g_values_biz.h"

StockOrderHelper::StockOrderHelper(void)
{
}


StockOrderHelper::~StockOrderHelper(void)
{
}

/*
验证下单数据的合法性

Return :
0 -- 合法
非0 -- 不合法
*/
int StockOrderHelper::OrderMsgValidate(std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg,
	int& out_iStockId)
{
	const string ftag("StockOrderHelper::OrderMsgValidate() ");
	if (0 == io_orderMsg->strStockID.length())
	{
		EzLog::e(ftag, "获取股票ID为空");

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "获取股票ID为空";
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	int iResTrans = Tgw_StringUtil::String2Int_atoi(io_orderMsg->strStockID, out_iStockId);
	if (0 != iResTrans)
	{
		string strDebug("转换StockId[");
		strDebug += io_orderMsg->strStockID;
		strDebug += "]为Int失败";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// 订单数量 -- 原始
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strOrderqty_origin, io_orderMsg->ui64Orderqty_origin);
	if (0 != iResTrans)
	{
		string strDebug("转换Orderqty_origin[");
		strDebug += io_orderMsg->strOrderqty_origin;
		strDebug += "]为Int失败";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// 订单数量 -- leavesqty剩余量
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strLeavesQty, io_orderMsg->ui64LeavesQty);
	if (0 != iResTrans)
	{
		string strDebug("转换LeavesQty[");
		strDebug += io_orderMsg->strLeavesQty;
		strDebug += "]为Int失败";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// 价格
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strOrderPrice, io_orderMsg->ui64mOrderPrice);
	if (0 != iResTrans)
	{
		string strDebug("转换Price[");
		strDebug += io_orderMsg->strOrderPrice;
		strDebug += "]为Int失败";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// trade side
	iResTrans = Tgw_StringUtil::String2Int_atoi(io_orderMsg->strSide, io_orderMsg->iSide);
	if (0 != iResTrans)
	{
		string strDebug("转换side[");
		strDebug += io_orderMsg->strSide;
		strDebug += "]为Int失败";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// 买卖方向，1 买/ 2 卖
	if (1 != io_orderMsg->iSide && 2 != io_orderMsg->iSide &&
		0 != io_orderMsg->strSide.compare("D") && 0 != io_orderMsg->strSide.compare("E"))
	{
		string strDebug("side[");
		strDebug += io_orderMsg->strSide;
		strDebug += "]unknown";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// minqty
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strMinQty, io_orderMsg->ui64MinQty);
	if (0 != iResTrans)
	{
		string strDebug("转换MinQty[");
		strDebug += io_orderMsg->strMinQty;
		strDebug += "]为Int失败";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		// 0--深圳A股
		/*
		//	委托类型			TimeInForce OrdType MaxPriceLevels MinQty
		// 普通限价				0			2			0		0
		// 限价全额成交或撤销	3			2			0	=OrderQty
		// 本方最优				0			U			0		0
		//对手方最优剩余转限价	0			1			1		0
		//市价立即成交剩余撤销	3			1			0		0
		//市价全额成交或撤销	3			1			0	=OrderQty
		//市价最优五档全额成交剩余撤销3		1			5		0

		*/
		if (0 == io_orderMsg->strTimeInForce.compare("0") && 0 == io_orderMsg->strOrdType.compare("2")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("0") && 0 == io_orderMsg->ui64MinQty)
		{
			// 普通限价				0			2			0		0
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("3") && 0 == io_orderMsg->strOrdType.compare("2")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("0") &&
			io_orderMsg->ui64Orderqty_origin == io_orderMsg->ui64MinQty)
		{
			// 限价全额成交或撤销	3			2			0	=OrderQty
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("0") && 0 == io_orderMsg->strOrdType.compare("U")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("0") && 0 == io_orderMsg->ui64MinQty)
		{
			// 本方最优				0			U			0		0
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("0") && 0 == io_orderMsg->strOrdType.compare("1")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("1") && 0 == io_orderMsg->ui64MinQty)
		{
			//对手方最优剩余转限价	0			1			1		0
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("3") && 0 == io_orderMsg->strOrdType.compare("1")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("0") && 0 == io_orderMsg->ui64MinQty)
		{
			//市价立即成交剩余撤销	3			1			0		0
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("3") && 0 == io_orderMsg->strOrdType.compare("1")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("0") &&
			io_orderMsg->ui64Orderqty_origin == io_orderMsg->ui64MinQty)
		{
			//市价全额成交或撤销	3			1			0	=OrderQty
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("3") && 0 == io_orderMsg->strOrdType.compare("1")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("5") && 0 == io_orderMsg->ui64MinQty)
		{
			//市价最优五档全额成交剩余撤销3		1			5		0
		}
		else if (0 == io_orderMsg->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
		{
			// 撤单
		}
		else
		{
			string strDebug("未知委托组合market[");
			strDebug += io_orderMsg->strTrade_market;
			strDebug += "].ordType[";
			strDebug += io_orderMsg->strOrdType;
			strDebug += "]";
			EzLog::e(ftag, strDebug);

			io_orderMsg->strErrorCode = simutgw::SZ_ERRCODE::c20005;
			io_orderMsg->bDataError = true;
			io_orderMsg->strError = strDebug;
			io_orderMsg->enMatchType = simutgw::ErrorMatch;

			return -1;
		}
	}
	else if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// 1--上海A股
		if (0 == io_orderMsg->strOrdType.compare("LPT") || 0 == io_orderMsg->strOrdType.compare("ORD") // 普通委托
			|| 0 == io_orderMsg->strOrdType.compare("LRZ") || 0 == io_orderMsg->strOrdType.compare("LRQ") // 融资融券
			|| 0 == io_orderMsg->strOrdType.compare("MPT") // 最优五档即时成交剩余撤销
			|| 0 == io_orderMsg->strOrdType.compare("NPT") // 最优五档即时成交剩余转限价
			)
		{
			// 
		}
		else
		{
			string strDebug("未知委托组合market[");
			strDebug += io_orderMsg->strTrade_market;
			strDebug += "].ordType[";
			strDebug += io_orderMsg->strOrdType;
			strDebug += "]";
			EzLog::e(ftag, strDebug);

			io_orderMsg->strErrorCode = simutgw::SH_ERRCODE::c209;
			io_orderMsg->bDataError = true;
			io_orderMsg->strError = strDebug;
			io_orderMsg->enMatchType = simutgw::ErrorMatch;

			return -1;
		}

	}
	else
	{
		string strDebug("未知市场market[");
		strDebug += io_orderMsg->strTrade_market;
		strDebug += "]";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	if (simutgw::TADE_TYPE::error == io_orderMsg->iTrade_type)
	{
		// 0--A股股票
		// 1--B股股票
		// 2--融资
		// 3--融券
		string strDebug("不支持的交易stockType[");
		string strTran;
		strDebug += sof_string::itostr(io_orderMsg->iTrade_type, strTran);
		strDebug += "]";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	return 0;
}

/*
转换为屏幕打印检视数据

*/
string& StockOrderHelper::OrderToScreenOut(const std::shared_ptr<struct simutgw::OrderMessage>& order,
	string& out_strScreenOut)
{
	static const string ftag("StockOrderHelper::OrderToScreenOut()");

	string strTran;

	// `trade_market` tinyint(4) NOT NULL COMMENT '0--深圳A股，1--上海A股',
	out_strScreenOut = "{\"trade_market\":";
	out_strScreenOut += order->strTrade_market;

	//`trade_type` tinyint(4) NOT NULL COMMENT '0--A股股票',
	out_strScreenOut += ",\"trade_type\":";
	out_strScreenOut += sof_string::itostr(order->iTrade_type, strTran);

	// `sessionid` varchar(50) CHARACTER SET gbk DEFAULT NULL COMMENT '标识唯一session会话',
	out_strScreenOut += ",\"sessionid\":";
	out_strScreenOut += order->strSessionId;

	// `security_account` varchar(12) CHARACTER SET gbk DEFAULT NULL COMMENT '证券账号',
	out_strScreenOut += ",\"security_account\":";
	out_strScreenOut += order->strAccountId;

	// `security_seat` varchar(6) CHARACTER SET gbk DEFAULT NULL COMMENT '席位',
	out_strScreenOut += ",\"security_seat\":";
	out_strScreenOut += order->strSecurity_seat;

	// `trade_group` varchar(10) CHARACTER SET gbk DEFAULT NULL COMMENT '交易圈',
	out_strScreenOut += ",\"trade_group\":";
	out_strScreenOut += order->strTrade_group;

	// `clordid` varchar(10) CHARACTER SET gbk DEFAULT NULL COMMENT '客户订单编号',
	out_strScreenOut += ",\"clordid\":";
	out_strScreenOut += order->strClordid;

	// `msgtype` varchar(6) CHARACTER SET gbk DEFAULT NULL COMMENT '消息类型,D=新订单,F=撤单',
	out_strScreenOut += ",\"msgtype\":";
	out_strScreenOut += order->strMsgType;

	// `orderqty_origin` bigint(20) unsigned DEFAULT NULL COMMENT '订单数量(新订单)/原始订单数量(撤单)',
	out_strScreenOut += ",\"orderqty_origin\":";
	out_strScreenOut += order->strOrderqty_origin;

	// `ordtype` varchar(3) CHARACTER SET gbk DEFAULT NULL COMMENT '订单类型1=市价,2=限价,U=本方最优',
	out_strScreenOut += ",\"ordtype\":";
	out_strScreenOut += order->strOrdType;

	// `order_price` bigint(20) unsigned DEFAULT NULL COMMENT '价格,OrdType为 2 ,即市价委托时填写',
	out_strScreenOut += ",\"order_price\":";
	out_strScreenOut += order->strOrderPrice;

	// `securityid` varchar(8) CHARACTER SET gbk DEFAULT NULL COMMENT '证券代码',
	out_strScreenOut += ",\"securityid\":";
	out_strScreenOut += order->strStockID;

	// `side` varchar(1) CHARACTER SET gbk DEFAULT NULL COMMENT '买卖方向, 1=买，2=卖',
	out_strScreenOut += ",\"side\":";
	out_strScreenOut += order->strSide;

	// `text` varchar(256) CHARACTER SET gbk DEFAULT NULL COMMENT '备注',
	out_strScreenOut += ",\"text\":";
	out_strScreenOut += order->strError;

	// `timeinforce` varchar(1) CHARACTER SET gbk DEFAULT NULL COMMENT '有效时期类型0=当日有效,3=即时成交或取消,9=港股通竞价限价盘,缺省值为 0',
	out_strScreenOut += ",\"timeinforce\":";
	out_strScreenOut += order->strTimeInForce;

	// `applid` varchar(3) CHARACTER SET gbk DEFAULT NULL COMMENT '应用标识',
	out_strScreenOut += ",\"applid\":";
	out_strScreenOut += order->strApplID;

	// `oper_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '发生时间',
	out_strScreenOut += ",\"oper_time\":";
	out_strScreenOut += order->strOper_time;

	// `trade_time` timestamp NULL DEFAULT NULL COMMENT '交易时间',
	out_strScreenOut += ",\"trade_time\":";
	out_strScreenOut += order->strTrade_time;

	out_strScreenOut += ",\"cumqty\":";
	out_strScreenOut += order->strCumQty;

	out_strScreenOut += ",\"leavesqty\":";
	out_strScreenOut += order->strLeavesQty;

	out_strScreenOut += ",\"cashorderqty\":";
	out_strScreenOut += order->strCashorderqty;

	// `is_error` tinyint(4) NOT NULL DEFAULT '0' COMMENT '订单错误标识,0=正确,1=错误',	
	out_strScreenOut += ",\"is_error\":";
	out_strScreenOut += sof_string::itostr(order->bDataError, strTran);

	// `is_proc` tinyint(4) NOT NULL DEFAULT '0' COMMENT '订单处理标识,0=未处理,1=已处理',
	out_strScreenOut += ",\"is_proc\":";
	out_strScreenOut += order->strIsProc;

	// `error_code` varchar(10) COLLATE gbk_bin DEFAULT NULL,
	out_strScreenOut += ",\"error_code\":";
	out_strScreenOut += order->strErrorCode;

	out_strScreenOut += "}";

	return out_strScreenOut;
}

/*
根据股票代码获取股票交易类型
"0" -- A股
"1" -- B股
"2" -- 融资交易
"3" -- 融券交易

深市A股的代码是以000打头。
深圳B股的代码是以200打头。

沪市A股的代码是以600、601或603打头。
沪市B股的代码是以900打头。

Return:
0 -- 成功
非0 -- 失败
*/
int StockOrderHelper::CheckOrder_TradeType(std::shared_ptr<struct simutgw::OrderMessage> &io_OrderMsg)
{
	static const string strTag("StockOrderHelper::CheckOrder_TradeType() ");

	if (io_OrderMsg->strStockID.length() != 6)
	{
		// 股票代码为6位，其他的不合法
		string strError("StockId[");
		strError += io_OrderMsg->strStockID;
		strError += "] illeagal";

		EzLog::e(strTag, strError);
		return -1;
	}
	/*
	*00开头的股票是上证A股，属于大盘股，其中6006开头的股票是最早上市的股票，6016开头的股票为大盘蓝筹股;
	*900开头的股票是上证B股;
	*000开头的股票是深证A股，001、002开头的股票也都属于深证A股，其中002开头的股票是深证A股中小企业股票;
	*200开头的股票是深证B股;
	*300开头的股票是创业板股票;(只有深市)
	*400开头的股票是三板市场股票。
	*/
	string strHead = io_OrderMsg->strStockID.substr(0, 3);
	string strFundHead = io_OrderMsg->strStockID.substr(0, 2); //基金代码

	if (0 == strHead.compare("000") || 0 == strHead.compare("001")
		|| 0 == strHead.compare("002") || 0 == strHead.compare("300")
		|| 0 == strFundHead.compare("15") || 0 == strFundHead.compare("16")
		|| 0 == strFundHead.compare("18"))
	{
		// 深A
		// 如果是融资融券
		if (
			(0 == io_OrderMsg->strCashMargin.compare("2")
			&& (0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
			)
			|| (0 == io_OrderMsg->strCashMargin.compare("3")
			&& (0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
			)
			)
		{
			// 融资交易，融资买入和卖券还款
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::margin_cash;
		}
		else if (
			(0 == io_OrderMsg->strCashMargin.compare("2")
			&& (0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
			)
			|| (
			0 == io_OrderMsg->strCashMargin.compare("3")
			&& (0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
			)
			)
		{
			// 融券交易，融券卖出和买券还券
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::margin_stock;
		}
		else
		{
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::a_trade;
		}
	}
	else if (0 == strHead.compare("600") || 0 == strHead.compare("601")
		|| 0 == strHead.compare("603") || 0 == strFundHead.compare("50")
		|| 0 == strFundHead.compare("51"))
	{
		// 沪A
		if (3 == io_OrderMsg->strOrdType.length() &&
			0 == io_OrderMsg->strOrdType.substr(1, 2).compare("RZ"))
		{
			// 融资
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::margin_cash;
		}
		else if (3 == io_OrderMsg->strOrdType.length() &&
			0 == io_OrderMsg->strOrdType.substr(1, 2).compare("RQ"))
		{
			// 融券
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::margin_stock;
		}
		else
		{
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::a_trade;
		}
	}
	else if (0 == strHead.compare("200")
		|| 0 == strHead.compare("900"))
	{
		// 深B 或 沪B
		io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::b_trade;
	}
	else
	{
		// 未支持
		string strError("StockId[");
		strError += io_OrderMsg->strStockID;
		strError += "] illegal";

		EzLog::e(strTag, strError);
		return -1;
	}

	if (0 == io_OrderMsg->strApplID.compare("120"))
	{
		if (0 == io_OrderMsg->strSide.compare("D"))
		{
			// ETF申购
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::etf_crt;
		}
		else if (0 == io_OrderMsg->strSide.compare("E"))
		{
			// ETF赎回
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::etf_rdp;
		}
	}

	if (0 == io_OrderMsg->strMsgType.compare("F"))
	{
		// 撤单消息
		io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::cancel;
	}

	return 0;
}

/*
判断订单属于哪个交易规则

@param const std::map<uint64_t, std::string>& in_mapLinkRules : 成交配置和通道的关系

Return:
0 -- 成功
非0 -- 失败
*/
int StockOrderHelper::GetOrderMatchRule(std::shared_ptr<struct simutgw::OrderMessage> &io_OrderMsg,
	const std::map<uint64_t, uint64_t>& in_mapLinkRules)
{
	static const string strTag("StockOrderHelper::CheckOrder_TradeType_OrRule() ");

	uint64_t ui64RuleId = 0;

	int iRes = 0;
	if (simutgw::TRADE_MARKET_SH == io_OrderMsg->strTrade_market)
	{
		// 上海市场
		std::shared_ptr<struct MATCH_RULE_SH> ptrRule;

		iRes = simutgw::g_matchRule.GetShRule(io_OrderMsg->jsOrder,
			in_mapLinkRules, ui64RuleId, ptrRule);
		if (0 == iRes)
		{
			io_OrderMsg->tradePolicy.ui64RuleId = ui64RuleId;
			io_OrderMsg->tradePolicy.ptrRule_Sh = ptrRule;

			return 0;
		}
	}
	else if (simutgw::TRADE_MARKET_SZ == io_OrderMsg->strTrade_market)
	{
		// 深圳市场		
		std::shared_ptr<struct MATCH_RULE_SZ> ptrRule;

		iRes = simutgw::g_matchRule.GetSzRule(io_OrderMsg->szfixMsg,
			in_mapLinkRules, ui64RuleId, ptrRule);
		if (0 == iRes)
		{
			io_OrderMsg->tradePolicy.ui64RuleId = ui64RuleId;
			io_OrderMsg->tradePolicy.ptrRule_Sz = ptrRule;

			return 0;
		}
	}

	return -1;
}
