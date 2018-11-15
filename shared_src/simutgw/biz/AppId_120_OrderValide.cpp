#include "AppId_120_OrderValide.h"

#include "json/json.h"

#include "simutgw/order/OrderMemoryStoreFactory.h"
#include "order/StockOrderHelper.h"
#include "simutgw/db_oper/RecordNewOrderHelper.h"

#include "simutgw/db_oper/DbUserInfoAsset.h"

#include "simutgw/msg_biz/ProcCancelOrder.h"

#include "quotation/MarketInfoHelper.h"

#include "util/SystemCounter.h"
#include "tool_string/Tgw_StringUtil.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/sys_function.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "etf/ETFHelper.h"
#include "cache/UserStockHelper.h"

/*
交易前检查函数
Return:
0 -- 合法
-1 -- 不合法
*/
int AppId_120_OrderValide::ValidateOrder( std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg )
{
	const static string ftag( "AppId_120_OrderValide::ValidateOrder() " );

	if ( io_orderMsg->strClordid.empty() )
	{
		EzLog::e( ftag, "客户订单编号为空" );

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "客户订单编号为空";
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	if ( 6 != io_orderMsg->strStockID.length() )
	{
		EzLog::e( ftag, "证券代码错误" );

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "证券代码错误";
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	int iStockID = 0;
	int iResTrans = Tgw_StringUtil::String2Int_atoi( io_orderMsg->strStockID, iStockID );
	if ( 0 != iResTrans )
	{
		string strDebug( "转换证券代码[" );
		strDebug += io_orderMsg->strStockID;
		strDebug += "]为Int失败";
		EzLog::e( ftag, strDebug );

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// 订单数量 -- 原始
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64( io_orderMsg->strOrderqty_origin, io_orderMsg->ui64Orderqty_origin );
	if ( 0 != iResTrans )
	{
		string strDebug( "转换Orderqty_origin[" );
		strDebug += io_orderMsg->strOrderqty_origin;
		strDebug += "]为Int失败";
		EzLog::e( ftag, strDebug );

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// 订单数量 -- leavesqty剩余量
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64( io_orderMsg->strLeavesQty, io_orderMsg->ui64LeavesQty );
	if ( 0 != iResTrans )
	{
		string strDebug( "转换LeavesQty[" );
		strDebug += io_orderMsg->strLeavesQty;
		strDebug += "]为Int失败";
		EzLog::e( ftag, strDebug );

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	if ( simutgw::TRADE_MARKET_SZ == io_orderMsg->strMarket )
	{
		// side
		if ( !( "D" == io_orderMsg->strSide || "E" == io_orderMsg->strSide ) )
		{
			string strDebug( "side[" );
			strDebug += io_orderMsg->strSide;
			strDebug += "]错误";
			EzLog::e( ftag, strDebug );

			io_orderMsg->bDataError = true;
			io_orderMsg->strError = strDebug;
			io_orderMsg->enMatchType = simutgw::ErrorMatch;

			return -1;
		}

		// ordtype
		if ( "2" != io_orderMsg->strOrdType )
		{
			string strDebug( "ordtype[" );
			strDebug += io_orderMsg->strSide;
			strDebug += "]错误";
			EzLog::e( ftag, strDebug );

			io_orderMsg->bDataError = true;
			io_orderMsg->strError = strDebug;
			io_orderMsg->enMatchType = simutgw::ErrorMatch;

			return -1;
		}

		// price
		iResTrans = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64( io_orderMsg->strOrderPrice, io_orderMsg->ui64mOrderPrice );
		if ( 0 != iResTrans )
		{
			string strDebug( "转换Price[" );
			strDebug += io_orderMsg->strOrderPrice;
			strDebug += "]为Int失败";
			EzLog::e( ftag, strDebug );

			io_orderMsg->bDataError = true;
			io_orderMsg->strError = strDebug;
			io_orderMsg->enMatchType = simutgw::ErrorMatch;

			return -1;
		}
	}

	iResTrans = StockOrderHelper::CheckOrder_TradeType(io_orderMsg);
	if ( 0 != iResTrans )
	{
		return iResTrans;
	}

	return 0;
}
