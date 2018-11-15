#include "AppId_010_OrderValide.h"

#include "order/StockOrderHelper.h"
#include "tool_string/Tgw_StringUtil.h"


/*
����ǰ��麯��
Return:
0 -- �Ϸ�
-1 -- ���Ϸ�
*/
int AppId_010_OrderValide::ValidateOrder(std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg)
{
	const static string strTag("AppId_010_OrderValide::ValidateOrder() ");

	// �������
	int iRes = BasicValidate(io_orderMsg);
	if (0 != iRes)
	{
		return iRes;
	}

	// ������һ���Ѿ�����
	if (0 == io_orderMsg->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
	{
		return 0;
	}

	// ��ͨ�ֻ����׻�������ȯ
	iRes = StockOrderHelper::CheckOrder_TradeType(io_orderMsg);
	if (0 != iRes)
	{
		return iRes;
	}

	// �޼ۻ��м�
	iRes = GetOrder_DEL_TYPE(io_orderMsg);
	if (0 != iRes)
	{
		return iRes;
	}

	switch (io_orderMsg->enDelType)
	{
	case simutgw::deltype_ordinary_limit:
	{
		// �޼�
		int iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strOrderPrice, io_orderMsg->ui64mOrderPrice);
		if (0 != iResTrans)
		{
			string strDebug("ת��Price[");
			strDebug += io_orderMsg->strOrderPrice;
			strDebug += "]ΪIntʧ��";
			EzLog::e(strTag, strDebug);

			io_orderMsg->bDataError = true;
			io_orderMsg->strError = strDebug;
			io_orderMsg->enMatchType = simutgw::ErrorMatch;

			return -1;
		}
	}

	break;

	default:
		break;
	}


	return 0;
}

/*
����ǰ��麯��
��һЩ�������
Return:
0 -- �Ϸ�
-1 -- ���Ϸ�
*/
int AppId_010_OrderValide::BasicValidate(std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg)
{
	const static string ftag("AppId_010_OrderValide::ValidateOrder() ");

	if (io_orderMsg->strClordid.empty())
	{
		EzLog::e(ftag, "�ͻ��������Ϊ��");

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "�ͻ��������Ϊ��";
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	if (6 != io_orderMsg->strStockID.length())
	{
		EzLog::e(ftag, "֤ȯ�������");

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "֤ȯ�������";
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	int iStockID = 0;
	int iResTrans = Tgw_StringUtil::String2Int_atoi(io_orderMsg->strStockID, iStockID);
	if (0 != iResTrans)
	{
		string strDebug("ת��֤ȯ����[");
		strDebug += io_orderMsg->strStockID;
		strDebug += "]ΪIntʧ��";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// �������� -- ԭʼ
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strOrderqty_origin, io_orderMsg->ui64Orderqty_origin);
	if (0 != iResTrans)
	{
		string strDebug("ת��Orderqty_origin[");
		strDebug += io_orderMsg->strOrderqty_origin;
		strDebug += "]ΪIntʧ��";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// �������� -- leavesqtyʣ����
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strLeavesQty, io_orderMsg->ui64LeavesQty);
	if (0 != iResTrans)
	{
		string strDebug("ת��LeavesQty[");
		strDebug += io_orderMsg->strLeavesQty;
		strDebug += "]ΪIntʧ��";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// trade side
	if (0 == io_orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == io_orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
	{
		io_orderMsg->iSide = 1;
	}
	else if (0 == io_orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == io_orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
	{
		io_orderMsg->iSide = 2;
	}
	else
	{
		string strDebug("ת��side[");
		strDebug += io_orderMsg->strSide;
		strDebug += "]ΪIntʧ��";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// ��������1 ��/ 2 ��
	if (1 != io_orderMsg->iSide && 2 != io_orderMsg->iSide)
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
		string strDebug("ת��MinQty[");
		strDebug += io_orderMsg->strMinQty;
		strDebug += "]ΪIntʧ��";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	if (0 == io_orderMsg->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
	{
		if (io_orderMsg->strOrigClordid.empty())
		{
			EzLog::e(ftag, "�ͻ�ԭʼ�������Ϊ��");

			io_orderMsg->bDataError = true;
			io_orderMsg->strError = "�ͻ�ԭʼ�������Ϊ��";
			io_orderMsg->enMatchType = simutgw::ErrorMatch;

			return -1;
		}
	}

	return 0;
}

/*
ȡ���걨ί�е�����

Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int AppId_010_OrderValide::GetOrder_DEL_TYPE(std::shared_ptr<struct simutgw::OrderMessage> &orderMsg)
{
	static const string strTag("AppId_010_OrderValide::GetOrder_DEL_TYPE() ");

	int iReturn = 0;

	// ���ж��г�
	if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		// �����г�
		uint64_t ui64MinQty = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(orderMsg->strMinQty, ui64MinQty);
		if (0 == orderMsg->strOrdType.compare("2") &&
			0 == orderMsg->strMaxPriceLevels.compare("0"))
		{
			//	ί������			TimeInForce OrdType MaxPriceLevels MinQty
			// ��ͨ�޼�				0			2			0		0
			// �޼�ȫ��ɽ�����	3			2			0	=OrderQty
			if (0 == orderMsg->strTimeInForce.compare("0") &&
				0 == ui64MinQty)
			{
				orderMsg->enDelType = simutgw::deltype_ordinary_limit;
			}
			//else if (0 == orderMsg->strTimeInForce.compare("3") &&
			//	0 == orderMsg->strMinQty.compare(orderMsg->strOrderqty_origin))
			//{
			//	orderMsg->enDelType = simutgw::deltype_all_match_within_limit_or_cancel;
			//}
			else
			{
				// ����
				iReturn = -1;
			}

		}
		else if (0 == orderMsg->strOrdType.compare("U"))
		{
			//	ί������			TimeInForce OrdType MaxPriceLevels MinQty
			// ��������				0			U			0		0
			if (0 == orderMsg->strTimeInForce.compare("0") &&
				0 == orderMsg->strMaxPriceLevels.compare("0") &&
				0 == ui64MinQty)
			{
				orderMsg->enDelType = simutgw::deltype_the_side_optimal;
			}
			else
			{
				iReturn = -1;
			}

		}
		else if (0 == orderMsg->strOrdType.compare("1"))
		{
			//	ί������			TimeInForce OrdType MaxPriceLevels MinQty
			//���ַ�����ʣ��ת�޼�	0			1			1		0
			//�м������ɽ�ʣ�೷��	3			1			0		0
			//�м�ȫ��ɽ�����	3			1			0	=OrderQty
			//�м������嵵ȫ��ɽ�ʣ�೷��3		1			5		0
			if (0 == orderMsg->strTimeInForce.compare("0") &&
				0 == orderMsg->strMaxPriceLevels.compare("1") &&
				0 == ui64MinQty)
			{
				orderMsg->enDelType = simutgw::deltype_the_oppsite_side_optimaland_remainder_hold;
			}
			else if (0 == orderMsg->strTimeInForce.compare("3") &&
				0 == orderMsg->strMaxPriceLevels.compare("0") &&
				0 == ui64MinQty)
			{
				orderMsg->enDelType = simutgw::deltype_match_at_market_and_remainder_cancel;
			}
			else if (0 == orderMsg->strTimeInForce.compare("3") &&
				0 == orderMsg->strMaxPriceLevels.compare("0") &&
				ui64MinQty == orderMsg->ui64Orderqty_origin)
			{
				orderMsg->enDelType = simutgw::deltype_all_match_at_market_or_cancel;
			}
			else if (0 == orderMsg->strTimeInForce.compare("3") &&
				0 == orderMsg->strMaxPriceLevels.compare("5") &&
				0 == ui64MinQty)
			{
				orderMsg->enDelType = simutgw::deltype_match_at_market_in_five_and_remainder_cancel;
			}
			else
			{
				iReturn = -1;
			}
		}
		else
		{
			// ����
			iReturn = -1;
		}

	}
	else if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// �Ϻ��г�
		if (0 == orderMsg->strOrdType.compare("LPT")
			|| 0 == orderMsg->strOrdType.compare("LRZ")
			|| 0 == orderMsg->strOrdType.compare("LRQ"))
		{
			// ��ͨ�޼�
			// ����
			// ��ȯ
			orderMsg->enDelType = simutgw::deltype_ordinary_limit;
		}
		else if (0 == orderMsg->strOrdType.compare("MPT"))
		{
			// �����嵵��ʱ�ɽ�ʣ�೷��
			orderMsg->enDelType = simutgw::deltype_match_at_market_in_five_and_remainder_cancel;
		}
		else if (0 == orderMsg->strOrdType.compare("NPT"))
		{
			// �����嵵��ʱ�ɽ�ʣ��ת�޼�
			orderMsg->enDelType = simutgw::deltype_match_at_market_in_five_and_remainder_hold;
		}
		else
		{
			iReturn = -1;
		}

	}
	else
	{
		string strError("Error trade market[");
		strError += orderMsg->strTrade_market;

		orderMsg->strError = strError;
		orderMsg->bDataError = true;
		orderMsg->enMatchType = simutgw::ErrorMatch;

		EzLog::e(strTag, strError);

		return -1;
	}

	if (0 > iReturn)
	{
		string strError("Unkonwn order type");

		orderMsg->strError = strError;
		orderMsg->bDataError = true;
		orderMsg->enMatchType = simutgw::ErrorMatch;

		EzLog::e(strTag, strError);
	}

	return iReturn;
}
