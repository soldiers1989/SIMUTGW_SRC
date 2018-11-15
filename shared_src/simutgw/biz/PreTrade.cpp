#include "PreTrade.h"

#include "simutgw_config/g_values_sys_run_config.h"

#include "cache/UserStockHelper.h"

#include "quotation/MarketInfoHelper.h"

#include "simutgw/biz/MatchUtil.h"

/*
交易准备
主要是冻结股份

Return:
0 -- 成功
-1 -- 冻结股份失败
-2 -- 行情检查失败
*/
int PreTrade::TradePrep(std::shared_ptr<struct simutgw::OrderMessage>& io_order)
{
	static const std::string strTag("PreTrade::TradePrep() ");

	enum simutgw::TADE_TYPE type = simutgw::TADE_TYPE::error;
	GetTradeType(io_order, type);

	int iRes = 0;

	switch (type)
	{
	case simutgw::TADE_TYPE::error:
		return -1;
		break;

	case simutgw::TADE_TYPE::margin_cash:
	case simutgw::TADE_TYPE::margin_stock:
		break;

	case simutgw::TADE_TYPE::a_trade:
	case simutgw::TADE_TYPE::b_trade:
	case simutgw::TADE_TYPE::etf_buy:
		if (simutgw::SysMatchMode::EnAbleQuta == io_order->tradePolicy.iMatchMode)
		{
			iRes = CheckOrderByQuotation(io_order);
			if (0 != iRes)
			{
				return -2;
			}
		}

		if (io_order->tradePolicy.bCheck_Assets && 
			(0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2)))
		{
			iRes = UserStockHelper::SellFroze(io_order->strAccountId, io_order->strStockID,
				io_order->ui64LeavesQty, io_order->sellCps.ui64Etf_rdp, io_order->sellCps.ui64Avl);
		}

		break;

	case simutgw::TADE_TYPE::etf_sell:

		if (simutgw::SysMatchMode::EnAbleQuta == io_order->tradePolicy.iMatchMode)
		{
			iRes = CheckOrderByQuotation(io_order);
			if (0 != iRes)
			{
				return -2;
			}
		}

		if (io_order->tradePolicy.bCheck_Assets)
		{
			iRes = UserStockHelper::SellEtfFroze(io_order->strAccountId, io_order->strStockID,
				io_order->ui64LeavesQty, io_order->sellETFCps.ui64Etf_crt, io_order->sellETFCps.ui64Avl);
		}

		break;

	case simutgw::TADE_TYPE::etf_crt:
	{
		iRes = ValidateETF(io_order);
		if (0 != iRes)
		{
			return -1;
		}

		std::shared_ptr<struct simutgw::SzETF> ptrEtf(new struct simutgw::SzETF);
		iRes = ETFHelper::Query(io_order->strStockID, ptrEtf);
		if (0 != iRes)
		{
			io_order->enMatchType = simutgw::MatchType::ErrorMatch;
			return -1;
		}

		if (io_order->tradePolicy.bCheck_Assets)
		{
			iRes = UserStockHelper::CreationQuery(io_order->strAccountId, io_order->ui64Orderqty_origin,
				ptrEtf, io_order->vecFrozeComponent);
		}
		else
		{
			// 成分股
			AddCreationComponent(io_order, ptrEtf);
		}

		break;
	}

	case simutgw::TADE_TYPE::etf_rdp:
	{
		iRes = ValidateETF(io_order);
		if (0 != iRes)
		{
			return -1;
		}

		std::shared_ptr<struct simutgw::SzETF> ptrEtf(new struct simutgw::SzETF);
		iRes = ETFHelper::Query(io_order->strStockID, ptrEtf);
		if (0 != iRes)
		{
			io_order->enMatchType = simutgw::MatchType::ErrorMatch;
			return -1;
		}

		if (io_order->tradePolicy.bCheck_Assets)
		{
			iRes = UserStockHelper::RedemptionFroze(io_order->strAccountId, io_order->strStockID,
				io_order->ui64LeavesQty, io_order->rdpETFCps.ui64Act_pch, io_order->rdpETFCps.ui64Avl);
		}

		// 成分股
		AddRedeptionComponent(io_order, ptrEtf);

		break;
	}

	default:
		break;
	}

	if (0 != iRes)
	{
		io_order->enMatchType = simutgw::MatchType::ErrorMatch;
	}
	return iRes;
}

/*
取当前的订单类型
*/
int PreTrade::GetTradeType(std::shared_ptr<struct simutgw::OrderMessage>& in_order, enum simutgw::TADE_TYPE& io_type)
{
	// static const std::string strTag("PreTrade::GetTradeType() ");

	io_type = simutgw::TADE_TYPE::error;

	if (6 != in_order->strStockID.length())
	{
		return 0;
	}

	bool bETF = false;
	std::string strHead = in_order->strStockID.substr(0, 2);
	if (0 == strHead.compare("15") || 0 == strHead.compare("16")
		|| 0 == strHead.compare("18") || 0 == strHead.compare("50")
		|| 0 == strHead.compare("51"))
	{
		bETF = true;
	}
	else
	{
	}

	if (0 == in_order->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == in_order->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
	{
		if (bETF)
		{
			io_type = simutgw::TADE_TYPE::etf_buy;
		}
		else
		{
			io_type = simutgw::TADE_TYPE::a_trade;
		}
	}
	else if (0 == in_order->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == in_order->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
	{
		if (bETF)
		{
			io_type = simutgw::TADE_TYPE::etf_sell;
		}
		else
		{
			io_type = simutgw::TADE_TYPE::a_trade;
		}
	}
	else if (0 == in_order->strSide.compare("D"))
	{
		if (bETF)
		{
			io_type = simutgw::TADE_TYPE::etf_crt;
		}
		else
		{
		}
	}
	else if (0 == in_order->strSide.compare("E"))
	{
		if (bETF)
		{
			io_type = simutgw::TADE_TYPE::etf_rdp;
		}
		else
		{
		}
	}
	else
	{

	}

	// 撤单
	if (0 == in_order->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
	{
		io_type = simutgw::TADE_TYPE::cancel;
	}

	in_order->iTrade_type = io_type;

	return 0;
}

/*
根据行情判断委托的合法性
Return:
0 -- 合法
-1 -- 不合法
*/
int PreTrade::CheckOrderByQuotation(std::shared_ptr<struct simutgw::OrderMessage>& io_order)
{
	// static const std::string strTag("PreTrade::CheckOrderByQuotation() ");

	std::string strCircleID;

	MatchUtil::Get_Order_CircleID(io_order, strCircleID);

	simutgw::uint64_t_Money ui64mMaxGain = 0;
	simutgw::uint64_t_Money ui64mMinFall = 0;
	uint64_t ui64Cjsl = 0;
	simutgw::uint64_t_Money ui64mCjje = 0;
	string strHqsj;
	string strTpbz;

	//得到行情
	int iRes = MarketInfoHelper::GetCurrQuotGapByCircle_RecentPrice(io_order->strStockID,
		strCircleID, ui64mMaxGain, ui64mMinFall, ui64Cjsl, ui64mCjje, strHqsj, strTpbz);
	if (0 != iRes)
	{
		// 获取失败
		return 0;
	}

	//  查看是否停牌
	if (0 > CheckTPBZ(io_order, strTpbz))
	{
		return -1;
	}

	// 进行计算
	if (0 == io_order->strOrdType.compare("2")
		|| 0 == io_order->strOrdType.compare("LPT")
		|| 0 == io_order->strOrdType.compare("LRZ")
		|| 0 == io_order->strOrdType.compare("LRQ"))
	{
		// 是否超出涨跌幅
		if (0 > Check_MaxGain_And_MinFall(io_order, ui64mMaxGain, ui64mMinFall))
		{
			return -1;
		}
	}

	return 0;
}

/*
查看是否停牌
Return:
0 -- 未停牌
1 -- 已停牌
*/
int PreTrade::CheckTPBZ(std::shared_ptr<struct simutgw::OrderMessage>& io_order,
	const string& in_strTpbz)
{
	static const string strTag("PreTrade::CheckTPBZ() ");

	int iReturn = 0;

	if (0 != in_strTpbz.compare("F"))
	{
		io_order->enMatchType = simutgw::StopTrans;

		io_order->bDataError = true;
		io_order->strError = "该股已停牌";

		if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			// 深圳
			io_order->strErrorCode = simutgw::SZ_ERRCODE::c20007;
		}
		else if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			// 上海
			io_order->strErrorCode = simutgw::SH_ERRCODE::c203;
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
int PreTrade::Check_MaxGain_And_MinFall(std::shared_ptr<struct simutgw::OrderMessage>& io_order,
	const simutgw::uint64_t_Money in_ui64mMaxGain, const simutgw::uint64_t_Money in_ui64mMinFall)
{
	static const string strTag("PreTrade::Check_MaxGain_And_MinFall() ");

	int iReturn = 0;

	// 极少情况下股票会没有涨跌限制，如刚上市
	if ((0 == in_ui64mMaxGain) && (0 == in_ui64mMinFall))
	{

	}
	else
	{
		// 计算涨跌幅
		if (in_ui64mMaxGain < io_order->ui64mOrderPrice || in_ui64mMinFall > io_order->ui64mOrderPrice)
		{
			// 超出涨跌停价格范围
			io_order->enMatchType = simutgw::OutOfRange;

			string strTransTmp;
			io_order->strError = "超涨跌价位.max";
			io_order->strError += sof_string::itostr(in_ui64mMaxGain, strTransTmp);
			io_order->strError += ".min";
			io_order->strError += sof_string::itostr(in_ui64mMinFall, strTransTmp);

			io_order->bDataError = true;
			if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
			{
				// 深圳
				io_order->strErrorCode = simutgw::SZ_ERRCODE::c20009;
			}
			else if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
			{
				// 上海
				io_order->strErrorCode = simutgw::SH_ERRCODE::c212;
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
ETF检查

Return:
0 -- 成功
-1 -- 失败
*/
int PreTrade::ValidateETF(std::shared_ptr<struct simutgw::OrderMessage>& in_order)
{
	// static const std::string strTag("PreTrade::ValidateETF() ");

	in_order->enMatchType = simutgw::NotMatch;

	std::shared_ptr<struct simutgw::SzETF> ptrEtf(new struct simutgw::SzETF);

	int iRes = ETFHelper::Query(in_order->strStockID, ptrEtf);
	if (0 != iRes)
	{
		return -1;
	}

	if (0 == in_order->strSide.compare("D"))
	{
		// 是否允许申购
		if (!ptrEtf->bCreation)
		{
			// 不允许申购
			in_order->enMatchType = simutgw::MatchType::ErrorMatch;
			return -1;
		}
	}
	else
	{
		// 是否允许赎回
		if (!ptrEtf->bRedemption)
		{
			// 不允许赎回
			in_order->enMatchType = simutgw::MatchType::ErrorMatch;
			return -1;
		}
	}

	// 是否小于最小申购赎回单位
	if (ptrEtf->ui64CreationRedemptionUnit > in_order->ui64Orderqty_origin)
	{
		// 小于最小申购赎回单位
		in_order->enMatchType = simutgw::MatchType::ErrorMatch;
		return -1;
	}

	return 0;
}

/*
ETF申购增加成分股交易信息
*/
int PreTrade::AddCreationComponent(std::shared_ptr<struct simutgw::OrderMessage>& io_order,
	const std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	// static const std::string strTag("PreTrade::AddCreationComponent() ");

	uint64_t ui64Times = io_order->ui64Orderqty_origin / ptrEtf->ui64CreationRedemptionUnit;

	for (size_t st = 0; st < ptrEtf->vecComponents.size(); ++st)
	{
		std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent> ptrComp(new
		struct simutgw::EtfCrt_FrozeComponent);

		ptrComp->strSecurityID = ptrEtf->vecComponents[st].strUnderlyingSecurityID;
		ptrComp->ui64act_pch_count = ptrEtf->vecComponents[st].ui64ComponentShare * ui64Times;
		ptrComp->ui64avl_count = 0;
		ptrComp->ui64Cash = ptrEtf->vecComponents[st].ui64mRedemptionCashSubstitute * ui64Times;
		io_order->vecFrozeComponent.push_back(ptrComp);
	}

	return 0;
}

/*
ETF赎回增加成分股交易信息
*/
int PreTrade::AddRedeptionComponent(std::shared_ptr<struct simutgw::OrderMessage>& io_order,
	const std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	// static const std::string strTag("PreTrade::AddRedeptionComponent() ");

	uint64_t ui64Times = io_order->ui64Orderqty_origin / ptrEtf->ui64CreationRedemptionUnit;

	for (size_t st = 0; st < ptrEtf->vecComponents.size(); ++st)
	{
		std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent> ptrComp(new
		struct simutgw::EtfCrt_FrozeComponent);

		ptrComp->strSecurityID = ptrEtf->vecComponents[st].strUnderlyingSecurityID;
		ptrComp->ui64rdp_count = ptrEtf->vecComponents[st].ui64ComponentShare * ui64Times;
		ptrComp->ui64Cash = ptrEtf->vecComponents[st].ui64mRedemptionCashSubstitute * ui64Times;

		io_order->vecFrozeComponent.push_back(ptrComp);
	}

	return 0;
}