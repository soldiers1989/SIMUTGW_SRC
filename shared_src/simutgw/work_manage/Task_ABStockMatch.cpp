#include "Task_ABStockMatch.h"

#include <memory>

#include "GenTaskHelper.h"

#include "json/json.h"

#include "simutgw/biz/MatchUtil.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "util/SystemCounter.h"
#include "util/TimeDuration.h"

#include "simutgw/order/OrderMemoryStoreFactory.h"
#include "order/StockOrderHelper.h"

#include "quotation/MarketInfoHelper.h"
#include "simutgw/msg_biz/ProcCancelOrder.h"

#include "simutgw/db_oper/RecordNewOrderHelper.h"
#include "simutgw/db_oper/DbUserInfoAsset.h"

#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/stgw_config/sys_function.h"

#include "cache/UserStockHelper.h"

/*
交易成交函数
Return:
0 -- 成交
-1 -- 失败
*/
int Task_ABStockMatch::MatchOrder()
{
	static const string ftag("Task_ABStockMatch::MatchOrder() ");

	TimeDuration dra;
	dra.Begin();

	int iRes = 0;
	// 成交
	switch (m_orderMsg->tradePolicy.iMatchMode)
	{
	case simutgw::SysMatchMode::EnAbleQuta:
		iRes = ABStockEnableQuoMatch(m_orderMsg);
		break;

	case simutgw::SysMatchMode::SimulMatchAll:
		iRes = ABStockMatch_All(m_orderMsg);
		break;

	case simutgw::SysMatchMode::SimulMatchByDivide:
		iRes = ABStockMatch_Divide(m_orderMsg);
		break;

	case simutgw::SysMatchMode::SimulNotMatch:
		iRes = ABStockMatch_UnMatch(m_orderMsg);
		break;

	case simutgw::SysMatchMode::SimulErrMatch:
		iRes = ABStockMatch_Error(m_orderMsg);
		break;

	case simutgw::SysMatchMode::SimulMatchPart:
		iRes = ABStockMatch_Part(m_orderMsg);
		break;

	default:
		// 
		string strValue;
		string strError("Match mode[");
		strError += sof_string::itostr(m_orderMsg->tradePolicy.iMatchMode, strValue);
		strError += "] doesn't support";
		EzLog::e(ftag, strError);
		break;
	}

	return iRes;
}

/*
处理启用行情的交易

Return:
0 -- 成功
-1 -- 失败
*/
int Task_ABStockMatch::ABStockEnableQuoMatch(std::shared_ptr<struct simutgw::OrderMessage>& ptrObj)
{
	static const string ftag("Task_ABStockMatch::ABStockEnableQuoMatch() ");

	enum simutgw::MatchType iMatchRes = Quot_MatchOrder(ptrObj);

	if (iMatchRes != simutgw::NotMatch)
	{
		string strItoa;
		string strDebug("订单clordid[");
		strDebug += ptrObj->strClordid;
		strDebug += "]成交结果[";
		strDebug += sof_string::itostr(iMatchRes, strItoa);
		strDebug += "],price[";
		strDebug += sof_string::itostr(ptrObj->ui64mPrice_matched, strItoa);
		strDebug += "],cjsl[";
		strDebug += sof_string::itostr(ptrObj->ui64Orderqty_matched, strItoa);
		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
	}

	//判断是否成交
	if (simutgw::MatchAll == iMatchRes)
	{
		//成交
		if (0 == ptrObj->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchAll();
		}
		else if (0 == ptrObj->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchAll();
		}
	}
	else if (simutgw::MatchPart == iMatchRes)
	{
		//部分成交
		if (0 == ptrObj->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchPart();
		}
		else if (0 == ptrObj->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchPart();
		}

		// 限价或者剩余挂单 插入成交队列
		if (simutgw::deltype_ordinary_limit == ptrObj->enDelType ||
			simutgw::deltype_match_at_market_in_five_and_remainder_hold == ptrObj->enDelType ||
			simutgw::deltype_the_oppsite_side_optimaland_remainder_hold == ptrObj->enDelType)
		{
			GenTaskHelper::GenTask_Match(ptrObj);
		}
		else
		{
			// nothing
			BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << "clordid=" << ptrObj->strClordid << ", unkown DelType=" << ptrObj->enDelType;
		}
	}
	else if (simutgw::NotMatch == iMatchRes)
	{
		//未成交
		// 将交易挂单，再插回去

		// 插入成交队列
		GenTaskHelper::GenTask_Match(ptrObj);
	}
	else if (simutgw::OutOfRange == iMatchRes)
	{
		// 超出涨跌幅
		// 将交易结束
		ptrObj->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("委托clordid[");
		sDebug += ptrObj->strClordid;
		sDebug += "]超出涨跌幅";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;
		// 写入回报队列
		simutgw::g_outMsg_buffer.PushBack(ptrObj);
		return -1;
	}
	else if (simutgw::StopTrans == iMatchRes)
	{
		// 股票已停牌
		// 将交易结束
		ptrObj->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("委托clordid[");
		sDebug += ptrObj->strClordid;
		sDebug += "]股票已停牌";

		EzLog::e(ftag, sDebug);

		// 写入回报队列
		simutgw::g_outMsg_buffer.PushBack(ptrObj);
		return -1;
	}
	else if (simutgw::ErrorMatch == iMatchRes)
	{
		// 不合法交易
		ptrObj->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("委托clordid[");
		sDebug += ptrObj->strClordid;
		sDebug += "]ErrorMatch";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

		// 写入回报队列
		simutgw::g_outMsg_buffer.PushBack(ptrObj);
		return -1;
	}
	else
	{
		//
		BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << "clordid=" << ptrObj->strClordid << ", unkown MatchType=" << iMatchRes;
	}

	return 0;
}

/*
处理实盘模拟交易

Param :
下单的方向
const int iOrderDirection :
1 -- 买单
2 -- 卖单

Return :
simutgw::MatchType
{
//全部成交
MatchAll = 0,
//部分成交
MatchPart = 1,
//未成交
NotMatch = -1,
// 超出涨跌幅
OutOfRange = -2,
// 停牌
StopTrans = -3,
// 交易不合法
ErrorMatch = -4
};
*/
enum simutgw::MatchType Task_ABStockMatch::Quot_MatchOrder(
	std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{
	static const string ftag("Task_ABStockMatch::Quot_MatchOrder()");

	string strCircleId;
	MatchUtil::Get_Order_CircleID(orderMsg, strCircleId);

	//从redis里读取对应的行情
	// read redis 
	string strRedisRes;

	Json::Value jsonRedisCmd;
	string strRedisCmd;

	enum simutgw::MatchType enRes = simutgw::NotMatch;

	try
	{
		simutgw::uint64_t_Money ui64mMaxGain = 0;
		simutgw::uint64_t_Money ui64mMinFall = 0;
		uint64_t ui64Cjsl = 0;
		simutgw::uint64_t_Money ui64mCjje = 0;
		string strHqsj;
		string strTpbz;

		//得到行情
		int iRes = MarketInfoHelper::GetCurrQuotGapByCircle(orderMsg->strStockID,
			orderMsg->strSide, strCircleId, orderMsg->tradePolicy.iQuotationMatchType,
			ui64mMaxGain, ui64mMinFall, ui64Cjsl, ui64mCjje, strHqsj, strTpbz);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "订单clordid[" << orderMsg->strClordid
				<< "], stockid=" << orderMsg->strStockID << ", GetQuotation error";

			orderMsg->enMatchType = simutgw::NotMatch;
			return simutgw::NotMatch;
		}

		// 市场 剩余成交数量
		uint64_t ui64Cjsl_Remain = 0;
		// 市场 剩余成交金额
		simutgw::uint64_t_Money ui64mCjje_Remain = 0;
		enRes = Quot_MatchByMarket(orderMsg, ui64mMaxGain, ui64mMinFall,
			ui64Cjsl, ui64mCjje, ui64Cjsl_Remain, ui64mCjje_Remain,
			strTpbz);
		if (enRes != simutgw::MatchAll && enRes != simutgw::MatchPart)
		{
			return enRes;
		}

		// 将交易数据写入数据库
		// 写入回报队列
		simutgw::g_outMsg_buffer.PushBack(orderMsg);
		if (0 != iRes)
		{
			enRes = simutgw::NotMatch;
			orderMsg->enMatchType = simutgw::NotMatch;
			return simutgw::NotMatch;
		}

		// 回写交易后的容量数据
		iRes = MarketInfoHelper::SetCurrQuotGapByCircle(orderMsg, strCircleId, ui64Cjsl_Remain,
			ui64mCjje_Remain, strHqsj);
		if (0 != iRes)
		{
			enRes = simutgw::NotMatch;
			orderMsg->enMatchType = simutgw::NotMatch;
			return simutgw::NotMatch;
		}

		return enRes;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
		return simutgw::NotMatch;
	}
}

/*
实盘 按照市场、成交方式成交
Return :
simutgw::MatchType
{
//全部成交
MatchAll = 0,
//部分成交
MatchPart = 1,
//未成交
NotMatch = -1,
// 超出涨跌幅
OutOfRange = -2,
// 停牌
StopTrans = -3,
// 交易不合法
ErrorMatch = -4
};
*/
enum simutgw::MatchType Task_ABStockMatch::Quot_MatchByMarket(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const simutgw::uint64_t_Money in_ui64mMaxGain, const simutgw::uint64_t_Money in_ui64mMinFall,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain,
	const string& in_strTpbz)
{
	static const string ftag("Task_ABStockMatch::Quot_MatchByMarket()");

	io_orderMsg->enMatchType = simutgw::NotMatch;

	//  查看是否停牌
	if (0 > MatchUtil::CheckTPBZ(io_orderMsg, in_strTpbz))
	{
		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "订单clordid[" << io_orderMsg->strClordid
			<< "], stockid=" << io_orderMsg->strStockID << ", TPBZ error, not match";

		return io_orderMsg->enMatchType;
	}

	// 进行计算
	if (0 == in_ui64Cjsl || 0 == in_ui64mCjje)
	{
		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "订单clordid[" << io_orderMsg->strClordid
			<< "], stockid=" << io_orderMsg->strStockID
			<< ", quot Cjsl=" << in_ui64Cjsl << ", Cjje=" << in_ui64mCjje << ", not match";

		// 市场容量余量已见底，不能成交
		io_orderMsg->enMatchType = simutgw::NotMatch;

		return io_orderMsg->enMatchType;
	}

	switch (io_orderMsg->enDelType)
	{
	case simutgw::deltype_ordinary_limit:
		// 是否超出涨跌幅
		if (0 > MatchUtil::Check_MaxGain_And_MinFall(io_orderMsg, in_ui64mMaxGain, in_ui64mMinFall))
		{
			BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "订单clordid[" << io_orderMsg->strClordid
				<< "], stockid=" << io_orderMsg->strStockID
				<< ", out price range quot MaxGain=" << in_ui64mMaxGain << ", MinFall=" << in_ui64mMinFall << ", not match";

			io_orderMsg->enMatchType = simutgw::OutOfRange;
			return io_orderMsg->enMatchType;
		}

		Match_Ordinary_Limit(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_the_side_optimal:
		Match_Optimal(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_the_oppsite_side_optimaland_remainder_hold:
		Match_Optimal(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_match_at_market_and_remainder_cancel:
		Match_Optimal_And_Remainder_Cancel(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_all_match_at_market_or_cancel:
		Match_All_Market_Or_Cancel(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_match_at_market_in_five_and_remainder_cancel:
		Match_Optimal_And_Remainder_Cancel(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_match_at_market_in_five_and_remainder_hold:
		Match_Optimal(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	default:

		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "订单clordid[" << io_orderMsg->strClordid
			<< "], stockid=" << io_orderMsg->strStockID
			<< ", unkown delType" << io_orderMsg->enDelType;

		io_orderMsg->enMatchType = simutgw::OutOfRange;
		return io_orderMsg->enMatchType;
		break;
	}

	return io_orderMsg->enMatchType;
}

/*
	处理委托类型 -- 普通限价

	处理方式：
	按照限价成交，未成交部分挂单
	*/
int Task_ABStockMatch::Match_Ordinary_Limit(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain)
{
	static const string strTag("Task_ABStockMatch::Match_Ordinary_Limit() ");

	// 计算市场的均价
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;

	// 判断成交类型
	if (0 > MatchUtil::Check_Match_Method(io_orderMsg, in_ui64Cjsl, in_ui64mCjje))
	{
		return 0;
	}

	// 进行撮合成交，分为部分及全部成交
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// 市场容量不够，只能部分成交
		io_orderMsg->enMatchType = simutgw::MatchPart;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = 0;
		out_ui64mCjje_Remain = 0;

		// 部分成交的数量
		uint64_t ui64MatchedOrderqty = in_ui64Cjsl;

		//
		// 已成交部分
		// 委托数量
		io_orderMsg->ui64Orderqty_matched = ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// 价格	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// 委托金额
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * ui64MatchedOrderqty;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);

		//
		// 未成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_unmatched = io_orderMsg->ui64LeavesQty - ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		io_orderMsg->ui64LeavesQty = io_orderMsg->ui64Orderqty_unmatched;

		//累计成交量
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin - io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strCumQty);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_unmatched = io_orderMsg->ui64Orderqty_unmatched * io_orderMsg->ui64mOrderPrice;

		// 成交，委托价格strOrderPrice变为原样
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}
	else
	{
		// 市场容量足够，可以全部成交
		io_orderMsg->enMatchType = simutgw::MatchAll;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = in_ui64Cjsl - io_orderMsg->ui64LeavesQty;
		out_ui64mCjje_Remain = in_ui64mCjje - io_orderMsg->ui64LeavesQty * ui64mAveragePrice;

		//
		// 已成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_matched = io_orderMsg->ui64LeavesQty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// 价格	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * io_orderMsg->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);
		//
		// 未成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_unmatched = 0;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		//累计成交量
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin, io_orderMsg->strCumQty);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_unmatched = 0;

		// 成交，委托价格strOrderPrice变为原样
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}

	UserStockHelper::UpdateAfterTrade(io_orderMsg);

	// 生成内部订单号
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//唯一执行id
	io_orderMsg->strExecID = strTransId;
	//唯一订单id
	io_orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '交易时间',
	TimeStringUtil::GetCurrTimeInTradeType(io_orderMsg->strTrade_time);

	return 0;
}

/*
	处理委托类型 -- 本方最优、对手方最优

	处理方式：
	按照市场均价成交，未成交部分的转成限价挂单
	*/
int Task_ABStockMatch::Match_Optimal(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain)
{
	static const string strTag("Task_ABStockMatch::Match_Optimal() ");

	// 计算市场的均价
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;

	// 判断成交类型
	if (0 > MatchUtil::Check_Match_Method(io_orderMsg, in_ui64Cjsl, in_ui64mCjje, false))
	{
		return 0;
	}

	// 进行撮合成交，分为部分及全部成交
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// 市场容量不够，只能部分成交
		io_orderMsg->enMatchType = simutgw::MatchPart;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = 0;
		out_ui64mCjje_Remain = 0;

		// 部分成交的数量
		uint64_t ui64MatchedOrderqty = in_ui64Cjsl;

		//
		// 已成交部分
		// 委托数量
		io_orderMsg->ui64Orderqty_matched = ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// 价格	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// 委托金额
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * ui64MatchedOrderqty;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);

		//
		// 未成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_unmatched = io_orderMsg->ui64LeavesQty - ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		io_orderMsg->ui64LeavesQty = io_orderMsg->ui64Orderqty_unmatched;

		//累计成交量
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin - io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strCumQty);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_unmatched = io_orderMsg->ui64Orderqty_unmatched * io_orderMsg->ui64mOrderPrice;

		// 成交，委托价格strOrderPrice变为原样
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}
	else
	{
		// 市场容量足够，可以全部成交
		io_orderMsg->enMatchType = simutgw::MatchAll;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = in_ui64Cjsl - io_orderMsg->ui64LeavesQty;
		out_ui64mCjje_Remain = in_ui64mCjje - io_orderMsg->ui64LeavesQty * ui64mAveragePrice;

		//
		// 已成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_matched = io_orderMsg->ui64LeavesQty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// 价格	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * io_orderMsg->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);
		//
		// 未成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_unmatched = 0;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		//累计成交量
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin, io_orderMsg->strCumQty);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_unmatched = 0;

		// 成交，委托价格strOrderPrice变为原样
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}

	UserStockHelper::UpdateAfterTrade(io_orderMsg);

	// 生成内部订单号
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//唯一执行id
	io_orderMsg->strExecID = strTransId;
	//唯一订单id
	io_orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '交易时间',
	TimeStringUtil::GetCurrTimeInTradeType(io_orderMsg->strTrade_time);

	return 0;
}

/*
	处理委托类型
	--市价立即成交剩余撤销、市价最优五档全额成交剩余撤销

	处理方式：
	按照市场均价成交，未成交部分的撤单
	*/
int Task_ABStockMatch::Match_Optimal_And_Remainder_Cancel(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain)
{
	static const string strTag("Task_ABStockMatch::Match_Optimal_And_Remainder_Cancel() ");

	// 计算市场的均价
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;

	// 判断成交类型
	if (0 > MatchUtil::Check_Match_Method(io_orderMsg, in_ui64Cjsl, in_ui64mCjje, false))
	{
		return 0;
	}

	// 进行撮合成交，分为部分及全部成交
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// 市场容量不够，只能部分成交
		io_orderMsg->enMatchType = simutgw::MatchPart;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = 0;
		out_ui64mCjje_Remain = 0;

		// 部分成交的数量
		uint64_t ui64MatchedOrderqty = in_ui64Cjsl;

		//
		// 已成交部分
		// 委托数量
		io_orderMsg->ui64Orderqty_matched = ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// 价格	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// 委托金额
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * ui64MatchedOrderqty;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);

		//
		// 未成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_unmatched = io_orderMsg->ui64LeavesQty - ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		io_orderMsg->ui64LeavesQty = io_orderMsg->ui64Orderqty_unmatched;

		//累计成交量
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin - io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strCumQty);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_unmatched = io_orderMsg->ui64Orderqty_unmatched * io_orderMsg->ui64mOrderPrice;

		// 成交，委托价格strOrderPrice变为原样
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);

		// 剩余撤单
		ProcCancelOrder::ProcSingleCancelOrder_Without_Request(io_orderMsg);
	}
	else
	{
		// 市场容量足够，可以全部成交
		io_orderMsg->enMatchType = simutgw::MatchAll;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = in_ui64Cjsl - io_orderMsg->ui64LeavesQty;
		out_ui64mCjje_Remain = in_ui64mCjje - io_orderMsg->ui64LeavesQty * ui64mAveragePrice;

		//
		// 已成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_matched = io_orderMsg->ui64LeavesQty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// 价格	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * io_orderMsg->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);
		//
		// 未成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_unmatched = 0;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		//累计成交量
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin, io_orderMsg->strCumQty);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_unmatched = 0;

		// 成交，委托价格strOrderPrice变为原样
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}

	UserStockHelper::UpdateAfterTrade(io_orderMsg);

	// 生成内部订单号
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//唯一执行id
	io_orderMsg->strExecID = strTransId;
	//唯一订单id
	io_orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '交易时间',
	TimeStringUtil::GetCurrTimeInTradeType(io_orderMsg->strTrade_time);

	return 0;
}

/*
	处理委托类型
	--市价全额成交或撤销

	处理方式：
	按照市价全部成交，或者全部撤单
	*/
int Task_ABStockMatch::Match_All_Market_Or_Cancel(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain)
{
	static const string strTag("Task_ABStockMatch::Match_All_Limit_Or_Cancel() ");

	// 计算市场的均价
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;

	// 判断成交类型
	if (0 > MatchUtil::Check_Match_Method(io_orderMsg, in_ui64Cjsl, in_ui64mCjje, false))
	{
		return 0;
	}

	// 进行撮合成交，分为部分及全部成交
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// 市场容量不够，只能部分成交
		io_orderMsg->enMatchType = simutgw::MatchPart;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = 0;
		out_ui64mCjje_Remain = 0;

		// 部分成交的数量
		uint64_t ui64MatchedOrderqty = in_ui64Cjsl;

		//
		// 已成交部分
		// 委托数量
		io_orderMsg->ui64Orderqty_matched = ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// 价格	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// 委托金额
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * ui64MatchedOrderqty;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);

		//
		// 未成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_unmatched = io_orderMsg->ui64LeavesQty - ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		io_orderMsg->ui64LeavesQty = io_orderMsg->ui64Orderqty_unmatched;

		//累计成交量
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin - io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strCumQty);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_unmatched = io_orderMsg->ui64Orderqty_unmatched * io_orderMsg->ui64mOrderPrice;

		// 成交，委托价格strOrderPrice变为原样
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);

		// 剩余撤单
		ProcCancelOrder::ProcSingleCancelOrder_Without_Request(io_orderMsg);
	}
	else
	{
		// 市场容量足够，可以全部成交
		io_orderMsg->enMatchType = simutgw::MatchAll;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = in_ui64Cjsl - io_orderMsg->ui64LeavesQty;
		out_ui64mCjje_Remain = in_ui64mCjje - io_orderMsg->ui64LeavesQty * ui64mAveragePrice;

		//
		// 已成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_matched = io_orderMsg->ui64LeavesQty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// 价格	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * io_orderMsg->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);
		//
		// 未成交部分
		// 订单数量
		io_orderMsg->ui64Orderqty_unmatched = 0;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		//累计成交量
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin, io_orderMsg->strCumQty);
		// 订单金额
		io_orderMsg->ui64mCashorderqty_unmatched = 0;

		// 成交，委托价格strOrderPrice变为原样
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}

	UserStockHelper::UpdateAfterTrade(io_orderMsg);

	// 生成内部订单号
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//唯一执行id
	io_orderMsg->strExecID = strTransId;
	//唯一订单id
	io_orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '交易时间',
	TimeStringUtil::GetCurrTimeInTradeType(io_orderMsg->strTrade_time);

	return 0;
}

/*
处理普通AB股委托
全部成交
Return:
0 -- 成功
-1 -- 失败
*/
int Task_ABStockMatch::ABStockMatch_All(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{
	static const string ftag("Task_ABStockMatch::ABStockMatch_All() ");

	// 预估 成交时的成交数量
	uint64_t ui64Cjsl_predict_match = 0;
	// 预估 成交时的成交金额
	simutgw::uint64_t_Money ui64mCjje_predict_match = 0;

	// 计算市场的均价
	simutgw::uint64_t_Money ui64mAveragePrice = 0;

	// 因为不需要看行情，所以成交均价即为下单价格
	ui64mAveragePrice = orderMsg->ui64mOrderPrice;

	// 市场容量足够，可以全部成交

	// 预估 成交时的成交数量
	ui64Cjsl_predict_match = orderMsg->ui64LeavesQty;
	// 预估 成交时的成交金额
	ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;

	//
	// 市场容量足够，可以全部成交
	orderMsg->enMatchType = simutgw::MatchAll;
	orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
	orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
	orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

	//
	// 已成交部分
	// 订单数量
	orderMsg->ui64Orderqty_matched = orderMsg->ui64LeavesQty;
	sof_string::itostr(orderMsg->ui64Orderqty_matched, orderMsg->strLastQty);
	// 价格	
	orderMsg->ui64mPrice_matched = ui64mAveragePrice;
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mPrice_matched, orderMsg->strLastPx, 4);
	// 订单金额
	orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * orderMsg->ui64Orderqty_matched;
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mCashorderqty_matched, orderMsg->strCashorderqty, 4);
	//
	// 未成交部分
	// 订单数量
	orderMsg->ui64Orderqty_unmatched = 0;
	sof_string::itostr(orderMsg->ui64Orderqty_unmatched, orderMsg->strLeavesQty);
	//累计成交量
	sof_string::itostr(orderMsg->ui64Orderqty_origin, orderMsg->strCumQty);
	// 订单金额
	orderMsg->ui64mCashorderqty_unmatched = 0;

	// 成交，委托价格strOrderPrice变为原样
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mOrderPrice, orderMsg->strOrderPrice, 4);

	// 生成内部订单号
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//唯一执行id
	orderMsg->strExecID = strTransId;
	//唯一订单id
	orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '交易时间',
	TimeStringUtil::GetCurrTimeInTradeType(orderMsg->strTrade_time);

	{
		string strItoa;
		string strDebug("订单clordid[");
		strDebug += orderMsg->strClordid;
		strDebug += "]成交结果[";
		strDebug += sof_string::itostr(simutgw::MatchAll, strItoa);
		strDebug += "],price[";
		strDebug += sof_string::itostr(orderMsg->ui64mPrice_matched, strItoa);
		strDebug += "],cjsl[";
		strDebug += sof_string::itostr(orderMsg->ui64Orderqty_matched, strItoa);
		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
	}

	//成交
	if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchAll();
	}
	else if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchAll();
	}

	UserStockHelper::UpdateAfterTrade(orderMsg);

	// 写入回报队列
	simutgw::g_outMsg_buffer.PushBack(orderMsg);

	return 0;
}

/*
处理普通AB股委托
分笔成交
Return:
0 -- 成功
-1 -- 失败
*/
int Task_ABStockMatch::ABStockMatch_Divide(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{
	static const string ftag("Task_ABStockMatch::ABStockMatch_Divide() ");

	// 预估 成交时的成交数量
	uint64_t ui64Cjsl_predict_match = 0;
	// 预估 成交时的成交金额
	simutgw::uint64_t_Money ui64mCjje_predict_match = 0;

	// 计算市场的均价
	simutgw::uint64_t_Money ui64mAveragePrice = 0;

	// 因为不需要看行情，所以成交均价即为下单价格
	ui64mAveragePrice = orderMsg->ui64mOrderPrice;

	// 平均一笔的数量
	uint64_t ui64PartQty = orderMsg->ui64Orderqty_origin / orderMsg->tradePolicy.iPart_Match_Num;

	if (0 == ui64PartQty)
	{
		// 当成一笔成交 最后一笔
		orderMsg->enMatchType = simutgw::MatchAll;
		// 预估 成交时的成交数量
		ui64Cjsl_predict_match = orderMsg->ui64LeavesQty;
	}
	else
	{
		if (orderMsg->ui64LeavesQty >= (2 * ui64PartQty))
		{
			//  不是最后一笔
			orderMsg->enMatchType = simutgw::MatchPart;
			// 预估 成交时的成交数量
			ui64Cjsl_predict_match = ui64PartQty;
		}
		else if (orderMsg->ui64LeavesQty >= ui64PartQty)
		{
			// 最后一笔
			orderMsg->enMatchType = simutgw::MatchAll;
			// 预估 成交时的成交数量
			ui64Cjsl_predict_match = orderMsg->ui64LeavesQty;
		}
		else
		{
			// 不够分笔 最后一笔
			orderMsg->enMatchType = simutgw::MatchAll;
			// 预估 成交时的成交数量
			ui64Cjsl_predict_match = orderMsg->ui64LeavesQty;
		}
	}

	// 预估 成交时的成交金额
	ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;

	//
	orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
	orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
	orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

	//
	// 已成交部分
	// 订单数量
	orderMsg->ui64Orderqty_matched = ui64Cjsl_predict_match;
	sof_string::itostr(orderMsg->ui64Orderqty_matched, orderMsg->strLastQty);
	// 价格	
	orderMsg->ui64mPrice_matched = ui64mAveragePrice;
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mPrice_matched, orderMsg->strLastPx, 4);
	// 订单金额
	orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * orderMsg->ui64Orderqty_matched;
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mCashorderqty_matched, orderMsg->strCashorderqty, 4);
	//
	// 未成交部分
	// 订单数量
	orderMsg->ui64Orderqty_unmatched = orderMsg->ui64LeavesQty - orderMsg->ui64Orderqty_matched;
	orderMsg->ui64LeavesQty = orderMsg->ui64Orderqty_unmatched;
	sof_string::itostr(orderMsg->ui64Orderqty_unmatched, orderMsg->strLeavesQty);
	//累计成交量
	sof_string::itostr(orderMsg->ui64Orderqty_matched, orderMsg->strCumQty);
	// 订单金额
	orderMsg->ui64mCashorderqty_unmatched = 0;

	// 成交，委托价格strOrderPrice变为原样
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mOrderPrice, orderMsg->strOrderPrice, 4);

	// 生成内部订单号
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//唯一执行id
	orderMsg->strExecID = strTransId;
	//唯一订单id
	orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '交易时间',
	TimeStringUtil::GetCurrTimeInTradeType(orderMsg->strTrade_time);

	{
		string strItoa;
		string strDebug("订单orderid[");
		strDebug += orderMsg->strClordid;
		strDebug += "]成交结果[";
		strDebug += sof_string::itostr(simutgw::MatchAll, strItoa);
		strDebug += "],price[";
		strDebug += sof_string::itostr(orderMsg->ui64mPrice_matched, strItoa);
		strDebug += "],cjsl[";
		strDebug += sof_string::itostr(orderMsg->ui64Orderqty_matched, strItoa);
		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
	}

	//成交
	if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchPart();
	}
	else if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchPart();
	}

	UserStockHelper::UpdateAfterTrade(orderMsg);

	// 写入回报队列
	simutgw::g_outMsg_buffer.PushBack(orderMsg);

	if (orderMsg->ui64LeavesQty != 0)
	{
		// 回写到order中
		GenTaskHelper::GenTask_Match(orderMsg);
	}

	return 0;
}

/*
处理普通AB股委托
不成交
Return:
0 -- 成功
-1 -- 失败
*/
int Task_ABStockMatch::ABStockMatch_UnMatch(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{
	static const string ftag("Task_ABStockMatch::ABStockMatch_UnMatch() ");

	GenTaskHelper::GenTask_Match(orderMsg);

	return 0;
}

/*
处理普通AB股委托
废单
Return:
0 -- 成功
-1 -- 失败
*/
int Task_ABStockMatch::ABStockMatch_Error(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{

	//不合法交易
	if (orderMsg->ui64LeavesQty == orderMsg->ui64Orderqty_origin)
	{
		// 由于废单只能是整条废单，如果已经成交了一部分，则不能作为废单
		orderMsg->enMatchType = simutgw::ErrorMatch;
		orderMsg->bDataError = true;
		orderMsg->strError = "模拟错误应答";
		if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			orderMsg->strErrorCode = simutgw::SZ_ERRCODE::c20001;
		}
		else if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			orderMsg->strErrorCode = simutgw::SH_ERRCODE::c215;
		}

		// 写入回报队列
		simutgw::g_outMsg_buffer.PushBack(orderMsg);
	}
	else
	{
		ABStockMatch_UnMatch(orderMsg);
	}

	return 0;
}

/*
处理普通AB股委托
部分成交
Return:
0 -- 成功
-1 -- 失败
*/
int Task_ABStockMatch::ABStockMatch_Part(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{
	static const string ftag("Task_ABStockMatch::ABStockMatch_Part() ");

	if (orderMsg->ui64LeavesQty != orderMsg->ui64Orderqty_origin)
	{
		// 剩余一半挂单
		GenTaskHelper::GenTask_Match(orderMsg);
	}
	else
	{
		// 预估 成交时的成交数量
		uint64_t ui64Cjsl_predict_match = 0;
		// 预估 成交时的成交金额
		simutgw::uint64_t_Money ui64mCjje_predict_match = 0;

		// 计算市场的均价
		simutgw::uint64_t_Money ui64mAveragePrice = 0;

		// 因为不需要看行情，所以成交均价即为下单价格
		ui64mAveragePrice = orderMsg->ui64mOrderPrice;

		// 总数量的一半
		uint64_t ui64PartQty = orderMsg->ui64Orderqty_origin / 2;

		// 特殊情况，总数量为1
		if (orderMsg->ui64Orderqty_origin == 1)
		{
			ui64PartQty = orderMsg->ui64Orderqty_origin;
			orderMsg->enMatchType = simutgw::MatchAll;
		}
		else
		{
			orderMsg->enMatchType = simutgw::MatchPart;
		}
		ui64Cjsl_predict_match = ui64PartQty;

		// 预估 成交时的成交金额
		ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;

		//
		orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
		orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		//
		// 已成交部分
		// 订单数量
		orderMsg->ui64Orderqty_matched = ui64Cjsl_predict_match;
		sof_string::itostr(orderMsg->ui64Orderqty_matched, orderMsg->strLastQty);
		// 价格	
		orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(orderMsg->ui64mPrice_matched, orderMsg->strLastPx, 4);
		// 订单金额
		orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * orderMsg->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(orderMsg->ui64mCashorderqty_matched, orderMsg->strCashorderqty, 4);
		//
		// 未成交部分
		// 订单数量
		orderMsg->ui64Orderqty_unmatched = orderMsg->ui64LeavesQty - orderMsg->ui64Orderqty_matched;
		orderMsg->ui64LeavesQty = orderMsg->ui64Orderqty_unmatched;
		sof_string::itostr(orderMsg->ui64Orderqty_unmatched, orderMsg->strLeavesQty);
		//累计成交量
		sof_string::itostr(orderMsg->ui64Orderqty_matched, orderMsg->strCumQty);
		// 订单金额
		orderMsg->ui64mCashorderqty_unmatched = 0;

		// 成交，委托价格strOrderPrice变为原样
		Tgw_StringUtil::iLiToStr(orderMsg->ui64mOrderPrice, orderMsg->strOrderPrice, 4);

		// 生成内部订单号
		string strTransId;
		TimeStringUtil::ExRandom15(strTransId);
		//唯一执行id
		orderMsg->strExecID = strTransId;
		//唯一订单id
		orderMsg->strOrderID = strTransId;

		// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '交易时间',
		TimeStringUtil::GetCurrTimeInTradeType(orderMsg->strTrade_time);

		{
			string strItoa;
			string strDebug("订单orderid[");
			strDebug += orderMsg->strClordid;
			strDebug += "]成交结果[";
			strDebug += sof_string::itostr(simutgw::MatchAll, strItoa);
			strDebug += "],price[";
			strDebug += sof_string::itostr(orderMsg->ui64mPrice_matched, strItoa);
			strDebug += "],cjsl[";
			strDebug += sof_string::itostr(orderMsg->ui64Orderqty_matched, strItoa);
			BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
		}

		//成交
		if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchPart();
		}
		else if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchPart();
		}

		UserStockHelper::UpdateAfterTrade(orderMsg);

		// 写入回报队列
		simutgw::g_outMsg_buffer.PushBack(orderMsg);

		if (orderMsg->ui64LeavesQty != 0)
		{
			// 回写到order中
			GenTaskHelper::GenTask_Match(orderMsg);
		}
	}

	return 0;
}