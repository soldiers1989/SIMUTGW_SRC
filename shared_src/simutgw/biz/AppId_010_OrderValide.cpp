#include "AppId_010_OrderValide.h"

#include "order/StockOrderHelper.h"
#include "tool_string/Tgw_StringUtil.h"


/*
交易前检查函数
Return:
0 -- 合法
-1 -- 不合法
*/
int AppId_010_OrderValide::ValidateOrder(std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg)
{
	const static string strTag("AppId_010_OrderValide::ValidateOrder() ");

	// 基本检查
	int iRes = BasicValidate(io_orderMsg);
	if (0 != iRes)
	{
		return iRes;
	}

	// 撤单上一步已经检查过
	if (0 == io_orderMsg->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
	{
		return 0;
	}

	// 普通现货交易或融资融券
	iRes = StockOrderHelper::CheckOrder_TradeType(io_orderMsg);
	if (0 != iRes)
	{
		return iRes;
	}

	// 限价或市价
	iRes = GetOrder_DEL_TYPE(io_orderMsg);
	if (0 != iRes)
	{
		return iRes;
	}

	switch (io_orderMsg->enDelType)
	{
	case simutgw::deltype_ordinary_limit:
	{
		// 限价
		int iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strOrderPrice, io_orderMsg->ui64mOrderPrice);
		if (0 != iResTrans)
		{
			string strDebug("转换Price[");
			strDebug += io_orderMsg->strOrderPrice;
			strDebug += "]为Int失败";
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
交易前检查函数
做一些基本检查
Return:
0 -- 合法
-1 -- 不合法
*/
int AppId_010_OrderValide::BasicValidate(std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg)
{
	const static string ftag("AppId_010_OrderValide::ValidateOrder() ");

	if (io_orderMsg->strClordid.empty())
	{
		EzLog::e(ftag, "客户订单编号为空");

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "客户订单编号为空";
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	if (6 != io_orderMsg->strStockID.length())
	{
		EzLog::e(ftag, "证券代码错误");

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "证券代码错误";
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	int iStockID = 0;
	int iResTrans = Tgw_StringUtil::String2Int_atoi(io_orderMsg->strStockID, iStockID);
	if (0 != iResTrans)
	{
		string strDebug("转换证券代码[");
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
		string strDebug("转换MinQty[");
		strDebug += io_orderMsg->strMinQty;
		strDebug += "]为Int失败";
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
			EzLog::e(ftag, "客户原始订单编号为空");

			io_orderMsg->bDataError = true;
			io_orderMsg->strError = "客户原始订单编号为空";
			io_orderMsg->enMatchType = simutgw::ErrorMatch;

			return -1;
		}
	}

	return 0;
}

/*
取得申报委托的类型

Return:
0 -- 成功
-1 -- 失败
*/
int AppId_010_OrderValide::GetOrder_DEL_TYPE(std::shared_ptr<struct simutgw::OrderMessage> &orderMsg)
{
	static const string strTag("AppId_010_OrderValide::GetOrder_DEL_TYPE() ");

	int iReturn = 0;

	// 先判断市场
	if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		// 深圳市场
		uint64_t ui64MinQty = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(orderMsg->strMinQty, ui64MinQty);
		if (0 == orderMsg->strOrdType.compare("2") &&
			0 == orderMsg->strMaxPriceLevels.compare("0"))
		{
			//	委托类型			TimeInForce OrdType MaxPriceLevels MinQty
			// 普通限价				0			2			0		0
			// 限价全额成交或撤销	3			2			0	=OrderQty
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
				// 错误
				iReturn = -1;
			}

		}
		else if (0 == orderMsg->strOrdType.compare("U"))
		{
			//	委托类型			TimeInForce OrdType MaxPriceLevels MinQty
			// 本方最优				0			U			0		0
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
			//	委托类型			TimeInForce OrdType MaxPriceLevels MinQty
			//对手方最优剩余转限价	0			1			1		0
			//市价立即成交剩余撤销	3			1			0		0
			//市价全额成交或撤销	3			1			0	=OrderQty
			//市价最优五档全额成交剩余撤销3		1			5		0
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
			// 错误
			iReturn = -1;
		}

	}
	else if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// 上海市场
		if (0 == orderMsg->strOrdType.compare("LPT")
			|| 0 == orderMsg->strOrdType.compare("LRZ")
			|| 0 == orderMsg->strOrdType.compare("LRQ"))
		{
			// 普通限价
			// 融资
			// 融券
			orderMsg->enDelType = simutgw::deltype_ordinary_limit;
		}
		else if (0 == orderMsg->strOrdType.compare("MPT"))
		{
			// 最优五档即时成交剩余撤销
			orderMsg->enDelType = simutgw::deltype_match_at_market_in_five_and_remainder_cancel;
		}
		else if (0 == orderMsg->strOrdType.compare("NPT"))
		{
			// 最优五档即时成交剩余转限价
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
