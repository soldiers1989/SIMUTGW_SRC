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
����ǰ��麯��
Return:
0 -- �Ϸ�
-1 -- ���Ϸ�
*/
int AppId_120_OrderValide::ValidateOrder( std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg )
{
	const static string ftag( "AppId_120_OrderValide::ValidateOrder() " );

	if ( io_orderMsg->strClordid.empty() )
	{
		EzLog::e( ftag, "�ͻ��������Ϊ��" );

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "�ͻ��������Ϊ��";
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	if ( 6 != io_orderMsg->strStockID.length() )
	{
		EzLog::e( ftag, "֤ȯ�������" );

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "֤ȯ�������";
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	int iStockID = 0;
	int iResTrans = Tgw_StringUtil::String2Int_atoi( io_orderMsg->strStockID, iStockID );
	if ( 0 != iResTrans )
	{
		string strDebug( "ת��֤ȯ����[" );
		strDebug += io_orderMsg->strStockID;
		strDebug += "]ΪIntʧ��";
		EzLog::e( ftag, strDebug );

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// �������� -- ԭʼ
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64( io_orderMsg->strOrderqty_origin, io_orderMsg->ui64Orderqty_origin );
	if ( 0 != iResTrans )
	{
		string strDebug( "ת��Orderqty_origin[" );
		strDebug += io_orderMsg->strOrderqty_origin;
		strDebug += "]ΪIntʧ��";
		EzLog::e( ftag, strDebug );

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// �������� -- leavesqtyʣ����
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64( io_orderMsg->strLeavesQty, io_orderMsg->ui64LeavesQty );
	if ( 0 != iResTrans )
	{
		string strDebug( "ת��LeavesQty[" );
		strDebug += io_orderMsg->strLeavesQty;
		strDebug += "]ΪIntʧ��";
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
			strDebug += "]����";
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
			strDebug += "]����";
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
			string strDebug( "ת��Price[" );
			strDebug += io_orderMsg->strOrderPrice;
			strDebug += "]ΪIntʧ��";
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
