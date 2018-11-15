#include "MatchUtil.h"

#include "order/StockOrderHelper.h"

#include "tool_string/Tgw_StringUtil.h"

#include "simutgw/order/OrderMemoryStoreFactory.h"

#include "config/conf_mysql_table.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "simutgw/db_oper/RecordNewOrderHelper.h"
#include "cache/UserStockHelper.h"

/*
查看是否停牌
Return:
0 -- 未停牌
1 -- 已停牌
*/
int MatchUtil::CheckTPBZ(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const string& in_strTpbz)
{
	static const string strTag("MatchUtil::CheckTPBZ() ");

	int iReturn = 0;

	if (0 != in_strTpbz.compare("F"))
	{
		io_orderMsg->enMatchType = simutgw::StopTrans;

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "该股已停牌";

		if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			// 深圳
			io_orderMsg->strErrorCode = simutgw::SZ_ERRCODE::c20007;
		}
		else if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			// 上海
			io_orderMsg->strErrorCode = simutgw::SH_ERRCODE::c203;
		}
		else
		{
			// 
		}

		iReturn = -1;
	}

	return iReturn;
}

/*
查看是否超出涨跌幅
Return:
0 -- 未超
1 -- 已超
*/
int MatchUtil::Check_MaxGain_And_MinFall(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const simutgw::uint64_t_Money in_ui64mMaxGain, const simutgw::uint64_t_Money in_ui64mMinFall)
{
	static const string strTag("MatchUtil::Check_MaxGain_And_MinFall() ");

	int iReturn = 0;

	// 极少情况下股票会没有涨跌限制，如刚上市
	if ((0 == in_ui64mMaxGain) && (0 == in_ui64mMinFall))
	{

	}
	else
	{
		// 计算涨跌幅
		if (in_ui64mMaxGain < io_orderMsg->ui64mOrderPrice || in_ui64mMinFall > io_orderMsg->ui64mOrderPrice)
		{
			// 超出涨跌停价格范围
			io_orderMsg->enMatchType = simutgw::OutOfRange;

			string strTransTmp;
			io_orderMsg->strError = "超涨跌价位.max";
			io_orderMsg->strError += sof_string::itostr(in_ui64mMaxGain, strTransTmp);
			io_orderMsg->strError += ".min";
			io_orderMsg->strError += sof_string::itostr(in_ui64mMinFall, strTransTmp);

			io_orderMsg->bDataError = true;
			if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
			{
				// 深圳
				io_orderMsg->strErrorCode = simutgw::SZ_ERRCODE::c20009;
			}
			else if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
			{
				// 上海
				io_orderMsg->strErrorCode = simutgw::SH_ERRCODE::c212;
			}
			else
			{
				// 
			}

			iReturn = -1;
		}
	}

	return iReturn;
}

/*
判断成交类型
Param:
bLimit -- true 是限价
-- false 市价

Return:
0 -- 可成交，部分或者全部
-1 -- 不成交，错误或者挂单
*/
int MatchUtil::Check_Match_Method(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	bool bLimit)
{
	static const string strTag("MatchUtil::Check_Match_Method() ");

	// 预估 成交时的成交数量
	uint64_t ui64Cjsl_predict_match = 0;
	// 预估 成交时的成交金额
	simutgw::uint64_t_Money ui64mCjje_predict_match = 0;

	// 计算市场的均价
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;

	// 判断撮合成交类型，预估成交数量及金额，分为部分及全部成交
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// 市场容量不够，只能部分成交

		// 预估 成交时的成交数量
		ui64Cjsl_predict_match = in_ui64Cjsl;
		// 预估 成交时的成交金额
		ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;

	}
	else
	{
		// 市场容量足够，可以全部成交

		// 预估 成交时的成交数量
		ui64Cjsl_predict_match = io_orderMsg->ui64LeavesQty;
		// 预估 成交时的成交金额
		ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;
	}

	// 买卖价格及库存判断
	if (1 == io_orderMsg->iSide)
	{
		// 买单
		if (bLimit && io_orderMsg->ui64mOrderPrice < ui64mAveragePrice)
		{
			// 限价，买价比均价还低
			io_orderMsg->enMatchType = simutgw::NotMatch;

			string strTransTmp;
			io_orderMsg->strError = "买价比均价还低.average:";
			io_orderMsg->strError += sof_string::itostr(ui64mAveragePrice, strTransTmp);

			return -1;
		}
	}
	else if (2 == io_orderMsg->iSide)
	{
		// 卖单
		if (bLimit && io_orderMsg->ui64mOrderPrice > ui64mAveragePrice)
		{
			// 限价，卖价比均价还高

			io_orderMsg->enMatchType = simutgw::NotMatch;

			string strTransTmp;
			io_orderMsg->strError = "卖价比均价还高.average:";
			io_orderMsg->strError += sof_string::itostr(ui64mAveragePrice, strTransTmp);

			return -1;
		}
	}
	else
	{
		string strTransTmp;
		io_orderMsg->strError = "未知买卖方向.side:";
		io_orderMsg->strError += io_orderMsg->strSide;

		io_orderMsg->enMatchType = simutgw::ErrorMatch;
		io_orderMsg->bDataError = true;

		return -1;
	}

	return 0;
}

/*
判断成交类型，补差资金和股份
Param:
bLimit -- true 是限价
-- false 市价
Return:
0 -- 可成交，部分或者全部
-1 -- 不成交，错误或者挂单
*/
int MatchUtil::Check_Match_Method_WithoutAccount(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje, bool bLimit)
{
	static const string strTag("MatchUtil::Check_Match_Method_WithoutAccount() ");

	// 预估 成交时的成交数量
	uint64_t ui64Cjsl_predict_match = 0;
	// 预估 成交时的成交金额
	simutgw::uint64_t_Money ui64mCjje_predict_match = 0;

	// 计算市场的均价
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;


	// 判断撮合成交类型，预估成交数量及金额，分为部分及全部成交
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// 市场容量不够，只能部分成交

		// 预估 成交时的成交数量
		ui64Cjsl_predict_match = in_ui64Cjsl;
		// 预估 成交时的成交金额
		ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;

	}
	else
	{
		// 市场容量足够，可以全部成交

		// 预估 成交时的成交数量
		ui64Cjsl_predict_match = io_orderMsg->ui64LeavesQty;
		// 预估 成交时的成交金额
		ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;
	}

	// 买卖价格及库存判断
	if (1 == io_orderMsg->iSide)
	{
		// 买单
		if (bLimit && io_orderMsg->ui64mOrderPrice < ui64mAveragePrice)
		{
			// 限价，买价比均价还低
			io_orderMsg->enMatchType = simutgw::NotMatch;

			string strTransTmp;
			io_orderMsg->strError = "买价比均价还低.average:";
			io_orderMsg->strError += sof_string::itostr(ui64mAveragePrice, strTransTmp);

			return -1;
		}
	}
	else if (2 == io_orderMsg->iSide)
	{
		// 卖单
		if (bLimit && io_orderMsg->ui64mOrderPrice > ui64mAveragePrice)
		{
			// 限价，卖价比均价还高
			io_orderMsg->enMatchType = simutgw::NotMatch;

			string strTransTmp;
			io_orderMsg->strError = "卖价比均价还高.average:";
			io_orderMsg->strError += sof_string::itostr(ui64mAveragePrice, strTransTmp);

			return -1;
		}
	}
	else
	{
		string strTransTmp;
		io_orderMsg->strError = "未知买卖方向.side:";
		io_orderMsg->strError += io_orderMsg->strSide;
		io_orderMsg->bDataError = true;

		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	return 0;
}

/*
取一笔委托的行情交易圈
*/
void MatchUtil::Get_Order_CircleID(const std::shared_ptr<struct simutgw::OrderMessage>& orderMsg,
	string& out_strCircleID)
{
	out_strCircleID = orderMsg->strTrade_group;
	if (out_strCircleID.empty())
	{
		out_strCircleID = orderMsg->strMarket_branchid;
	}

	if (out_strCircleID.empty())
	{
		out_strCircleID = "C1";
	}
}