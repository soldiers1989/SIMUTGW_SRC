#include "PreTrade.h"

#include "simutgw_config/g_values_sys_run_config.h"

#include "cache/UserStockHelper.h"

#include "quotation/MarketInfoHelper.h"

#include "simutgw/biz/MatchUtil.h"

/*
����׼��
��Ҫ�Ƕ���ɷ�

Return:
0 -- �ɹ�
-1 -- ����ɷ�ʧ��
-2 -- ������ʧ��
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
			// �ɷֹ�
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

		// �ɷֹ�
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
ȡ��ǰ�Ķ�������
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

	// ����
	if (0 == in_order->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
	{
		io_type = simutgw::TADE_TYPE::cancel;
	}

	in_order->iTrade_type = io_type;

	return 0;
}

/*
���������ж�ί�еĺϷ���
Return:
0 -- �Ϸ�
-1 -- ���Ϸ�
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

	//�õ�����
	int iRes = MarketInfoHelper::GetCurrQuotGapByCircle_RecentPrice(io_order->strStockID,
		strCircleID, ui64mMaxGain, ui64mMinFall, ui64Cjsl, ui64mCjje, strHqsj, strTpbz);
	if (0 != iRes)
	{
		// ��ȡʧ��
		return 0;
	}

	//  �鿴�Ƿ�ͣ��
	if (0 > CheckTPBZ(io_order, strTpbz))
	{
		return -1;
	}

	// ���м���
	if (0 == io_order->strOrdType.compare("2")
		|| 0 == io_order->strOrdType.compare("LPT")
		|| 0 == io_order->strOrdType.compare("LRZ")
		|| 0 == io_order->strOrdType.compare("LRQ"))
	{
		// �Ƿ񳬳��ǵ���
		if (0 > Check_MaxGain_And_MinFall(io_order, ui64mMaxGain, ui64mMinFall))
		{
			return -1;
		}
	}

	return 0;
}

/*
�鿴�Ƿ�ͣ��
Return:
0 -- δͣ��
1 -- ��ͣ��
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
		io_order->strError = "�ù���ͣ��";

		if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			// ����
			io_order->strErrorCode = simutgw::SZ_ERRCODE::c20007;
		}
		else if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			// �Ϻ�
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
�鿴�Ƿ񳬳��ǵ���
Return:
0 -- δ��
1 -- �ѳ�
*/
int PreTrade::Check_MaxGain_And_MinFall(std::shared_ptr<struct simutgw::OrderMessage>& io_order,
	const simutgw::uint64_t_Money in_ui64mMaxGain, const simutgw::uint64_t_Money in_ui64mMinFall)
{
	static const string strTag("PreTrade::Check_MaxGain_And_MinFall() ");

	int iReturn = 0;

	// ��������¹�Ʊ��û���ǵ����ƣ��������
	if ((0 == in_ui64mMaxGain) && (0 == in_ui64mMinFall))
	{

	}
	else
	{
		// �����ǵ���
		if (in_ui64mMaxGain < io_order->ui64mOrderPrice || in_ui64mMinFall > io_order->ui64mOrderPrice)
		{
			// �����ǵ�ͣ�۸�Χ
			io_order->enMatchType = simutgw::OutOfRange;

			string strTransTmp;
			io_order->strError = "���ǵ���λ.max";
			io_order->strError += sof_string::itostr(in_ui64mMaxGain, strTransTmp);
			io_order->strError += ".min";
			io_order->strError += sof_string::itostr(in_ui64mMinFall, strTransTmp);

			io_order->bDataError = true;
			if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
			{
				// ����
				io_order->strErrorCode = simutgw::SZ_ERRCODE::c20009;
			}
			else if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
			{
				// �Ϻ�
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
ETF���

Return:
0 -- �ɹ�
-1 -- ʧ��
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
		// �Ƿ������깺
		if (!ptrEtf->bCreation)
		{
			// �������깺
			in_order->enMatchType = simutgw::MatchType::ErrorMatch;
			return -1;
		}
	}
	else
	{
		// �Ƿ��������
		if (!ptrEtf->bRedemption)
		{
			// ���������
			in_order->enMatchType = simutgw::MatchType::ErrorMatch;
			return -1;
		}
	}

	// �Ƿ�С����С�깺��ص�λ
	if (ptrEtf->ui64CreationRedemptionUnit > in_order->ui64Orderqty_origin)
	{
		// С����С�깺��ص�λ
		in_order->enMatchType = simutgw::MatchType::ErrorMatch;
		return -1;
	}

	return 0;
}

/*
ETF�깺���ӳɷֹɽ�����Ϣ
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
ETF������ӳɷֹɽ�����Ϣ
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