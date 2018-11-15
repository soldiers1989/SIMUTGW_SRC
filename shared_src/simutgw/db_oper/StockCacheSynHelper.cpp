#include "StockCacheSynHelper.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/db_oper/TaskStockCacheSyn.h"
#include "order/StockOrderHelper.h"
#include "order/StockHelper.h"

StockCacheSynHelper::StockCacheSynHelper()
{
}


StockCacheSynHelper::~StockCacheSynHelper()
{
}

/*
�ڴ���DB�ɷ���Ϣͬ��
��ͨ�ɷ�����
*/
int StockCacheSynHelper::SellSyn( const std::string& strAccount, const std::string& strStockID )
{
	static const std::string strTag( "StockCacheSynHelper::SellSyn() " );

	uint64_t ui64stkOrg, ui64stkUpd, ui64rdpOrg, ui64rdpUpd, ui64avlOrg, ui64avlUpd, ui64lastOrg;
	ui64stkOrg = ui64stkUpd = ui64rdpOrg = ui64rdpUpd = ui64avlOrg = ui64avlUpd = ui64lastOrg = 0;

	int iRes = simutgw::g_mapUserStock[strAccount][strStockID]->SellDBSyn( ui64stkOrg, ui64stkUpd,
		ui64rdpOrg, ui64rdpUpd, ui64avlOrg, ui64avlUpd, ui64lastOrg );
	if ( 0 != iRes )
	{
		std::string strDebug( "SellDBSyn ʧ��" );
		EzLog::e( strTag, strDebug );
		return -1;
	}

	std::string strTrans, strUpdate( "UPDATE `stock_asset` SET `stock_balance`=" );
	strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
	strUpdate += ",`stock_etf_redemption_balance`=";
	strUpdate += sof_string::itostr( ui64rdpUpd, strTrans );
	strUpdate += ",`stock_available`=";
	strUpdate += sof_string::itostr( ui64avlUpd, strTrans );
	strUpdate += ",`stock_last_balance`=";
	strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
	strUpdate += ",`oper_time`=now() WHERE `account_id`='";
	strUpdate += strAccount;
	strUpdate += "' AND `stock_id`='";
	strUpdate += strStockID;
	strUpdate += "' AND `stock_balance`=";
	strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
	strUpdate += " AND `stock_etf_redemption_balance`=";
	strUpdate += sof_string::itostr( ui64rdpOrg, strTrans );
	strUpdate += " AND `stock_available`=";
	strUpdate += sof_string::itostr( ui64avlOrg, strTrans );
	strUpdate += " AND `stock_last_balance`=";
	strUpdate += sof_string::itostr( ui64lastOrg, strTrans );

	iRes = ExcUpdateSql( strUpdate );
	if ( 0 != iRes )
	{
		std::string strDebug( "ͬ���ɷ���Ϣʧ��" );
		EzLog::e( strTag, strDebug );
	}

	return iRes;
}

/*
�ڴ���DB�ɷ���Ϣͬ��
etf����
*/
int StockCacheSynHelper::SellEtfSyn( const std::string& strAccount, const std::string& strStockID )
{
	static const std::string strTag( "StockCacheSynHelper::SellEtfSyn() " );

	uint64_t ui64stkOrg, ui64stkUpd, ui64crtOrg, ui64crtUpd, ui64avlOrg, ui64avlUpd, ui64lastOrg;
	ui64stkOrg = ui64stkUpd = ui64crtOrg = ui64crtUpd = ui64avlOrg = ui64avlUpd = ui64lastOrg = 0;

	int iRes = simutgw::g_mapUserStock[strAccount][strStockID]->SellEtfDBSyn( ui64stkOrg, ui64stkUpd,
		ui64crtOrg, ui64crtUpd, ui64avlOrg, ui64avlUpd, ui64lastOrg );
	if ( 0 != iRes )
	{
		std::string strDebug( "SellEtfDBSyn ʧ��" );
		EzLog::e( strTag, strDebug );
	}
	
	std::string strTrans, strUpdate( "UPDATE `stock_etf_asset` SET `stock_balance`=" );
	strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
	strUpdate += ",`stock_creation_balance`=";
	strUpdate += sof_string::itostr( ui64crtUpd, strTrans );
	strUpdate += ",`stock_available`=";
	strUpdate += sof_string::itostr( ui64avlUpd, strTrans );
	strUpdate += ",`stock_last_balance`=";
	strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
	strUpdate += ",`oper_time`=now() WHERE `account_id`='";
	strUpdate += strAccount;
	strUpdate += "' AND `stock_id`='";
	strUpdate += strStockID;
	strUpdate += "' AND `stock_balance`=";
	strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
	strUpdate += " AND `stock_creation_balance`=";
	strUpdate += sof_string::itostr( ui64crtOrg, strTrans );
	strUpdate += " AND `stock_available`=";
	strUpdate += sof_string::itostr( ui64avlOrg, strTrans );
	strUpdate += " AND `stock_last_balance`=";
	strUpdate += sof_string::itostr( ui64lastOrg, strTrans );

	iRes = ExcUpdateSql( strUpdate );
	if ( 0 != iRes )
	{
		std::string strDebug( "ͬ���ɷ���Ϣʧ��" );
		EzLog::e( strTag, strDebug );
	}

	return iRes;
}

/*
�ڴ���DB�ɷ���Ϣͬ��
��ͨ�ɷ�����
*/
int StockCacheSynHelper::AddSyn( const std::string& strAccount, const std::string& strStockID )
{
	static const std::string strTag( "StockCacheSynHelper::AddSyn() " );

	uint64_t ui64stkOrg, ui64stkUpd, ui64actOrg, ui64actUpd, ui64lastOrg;
	ui64stkOrg = ui64stkUpd = ui64actOrg = ui64actUpd = ui64lastOrg = 0;

	int iRes = simutgw::g_mapUserStock[strAccount][strStockID]->AddActSyn( ui64stkOrg, ui64stkUpd,
		ui64actOrg, ui64actUpd, ui64lastOrg );

	std::string strTrans, strUpdate;
	if ( simutgw::g_mapUserStock[strAccount][strStockID]->CheckStore() )
	{
		strUpdate = "UPDATE `stock_asset` SET `stock_balance`=";
		strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
		strUpdate += ",`stock_auction_purchase_balance`=";
		strUpdate += sof_string::itostr( ui64actUpd, strTrans );
		strUpdate += ",`stock_last_balance`=";
		strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
		strUpdate += ",`oper_time`=now() WHERE `account_id`='";
		strUpdate += strAccount;
		strUpdate += "' AND `stock_id`='";
		strUpdate += strStockID;
		strUpdate += "' AND `stock_balance`=";
		strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
		strUpdate += " AND `stock_auction_purchase_balance`=";
		strUpdate += sof_string::itostr( ui64actOrg, strTrans );
		strUpdate += " AND `stock_last_balance`=";
		strUpdate += sof_string::itostr( ui64lastOrg, strTrans );

		simutgw::g_mapUserStock[strAccount][strStockID]->SetStore();
	}
	else
	{
		strUpdate = "INSERT INTO `stock_asset` (`account_id`,`stock_id`,`trade_market`,\
`stock_balance`,`stock_auction_purchase_balance`,`stock_staple_purchase_balance`,\
`stock_etf_redemption_balance`,`stock_available`,`stock_last_balance`,\
`is_close`,`oper_time`,`operator`) VALUES('";
		strUpdate += strAccount;
		strUpdate += "','";
		strUpdate += strStockID;
		strUpdate += "',";
		std::string strTradeMarket;
		StockHelper::GetStockTradeMarket(strStockID, strTradeMarket);
		if (!strTradeMarket.empty())
		{
			strUpdate += strTradeMarket;
		}
		else
		{
			strUpdate += "0";
		}
		strUpdate += ",";
		strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
		strUpdate += ",";
		strUpdate += sof_string::itostr( ui64actUpd, strTrans );
		strUpdate += ",0,0,0,0,0,now(),'SYS')";
	}

	iRes = ExcUpdateSql( strUpdate );
	if ( 0 != iRes )
	{
		std::string strDebug( "ͬ���ɷ���Ϣʧ��" );
		EzLog::e( strTag, strDebug );
	}

	// �����ݿ����м�¼
	simutgw::g_mapUserStock[strAccount][strStockID]->SetStore();

	return iRes;
}

/*
�ڴ���DB�ɷ���Ϣͬ��
etf����
*/
int StockCacheSynHelper::AddEtfSyn( const std::string& strAccount, const std::string& strStockID )
{
	static const std::string strTag( "StockCacheSynHelper::AddEtfSyn() " );

	uint64_t ui64stkOrg, ui64stkUpd, ui64actOrg, ui64actUpd, ui64lastOrg;

	int iRes = simutgw::g_mapUserStock[strAccount][strStockID]->AddEtfSyn( ui64stkOrg, ui64stkUpd,
		ui64actOrg, ui64actUpd, ui64lastOrg );

	std::string strTrans, strUpdate;
	if ( simutgw::g_mapUserStock[strAccount][strStockID]->CheckStore() )
	{
		strUpdate = "UPDATE `stock_etf_asset` SET `stock_balance`=";
		strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
		strUpdate += ",`stock_auction_purchase_balance`=";
		strUpdate += sof_string::itostr( ui64actUpd, strTrans );
		strUpdate += ",`stock_last_balance`=";
		strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
		strUpdate += ",`oper_time`=now() WHERE `account_id`='";
		strUpdate += strAccount;
		strUpdate += "' AND `stock_id`='";
		strUpdate += strStockID;
		strUpdate += "' AND `stock_balance`=";
		strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
		strUpdate += " AND `stock_auction_purchase_balance`=";
		strUpdate += sof_string::itostr( ui64actOrg, strTrans );
		strUpdate += " AND `stock_last_balance`=";
		strUpdate += sof_string::itostr( ui64lastOrg, strTrans );

		simutgw::g_mapUserStock[strAccount][strStockID]->SetStore();
	}
	else
	{
		strUpdate = "INSERT INTO `stock_etf_asset` (`account_id`,`stock_id`,`trade_market`,\
`stock_balance`,`stock_auction_purchase_balance`,`stock_staple_purchase_balance`,\
`stock_creation_balance`,`stock_available`,`stock_last_balance`,\
`is_close`,`oper_time`,`operator`) VALUES('";
		strUpdate += strAccount;
		strUpdate += "','";
		strUpdate += strStockID;
		strUpdate += "',";
		std::string strTradeMarket;
		StockHelper::GetStockTradeMarket(strStockID, strTradeMarket);
		if (!strTradeMarket.empty())
		{
			strUpdate += strTradeMarket;
		}
		else
		{
			strUpdate += "0";
		}
		strUpdate += ",";
		strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
		strUpdate += ",";
		strUpdate += sof_string::itostr( ui64actUpd, strTrans );
		strUpdate += ",0,0,0,0,0,now(),'SYS')";
	}

	iRes = ExcUpdateSql( strUpdate );
	if ( 0 != iRes )
	{
		std::string strDebug( "ͬ���ɷ���Ϣʧ��" );
		EzLog::e( strTag, strDebug );
	}

	// �����ݿ����м�¼
	simutgw::g_mapUserStock[strAccount][strStockID]->SetStore();

	return iRes;
}

/*
�ڴ���DB�ɷ���Ϣͬ��
�깺ETF����
*/
int StockCacheSynHelper::CrtEtfSyn( const std::string& strAccount, const std::string& strStockID )
{
	static const std::string strTag( "StockCacheSynHelper::CrtEtfSyn() " );

	uint64_t ui64stkOrg, ui64stkUpd, ui64crtOrg, ui64crtUpd, ui64lastOrg;

	int iRes = simutgw::g_mapUserStock[strAccount][strStockID]->CreationEtfDBSyn( ui64stkOrg, ui64stkUpd,
		ui64crtOrg, ui64crtUpd, ui64lastOrg );

	std::string strTrans, strUpdate;
	if ( simutgw::g_mapUserStock[strAccount][strStockID]->CheckStore() )
	{
		strUpdate = "UPDATE `stock_etf_asset` SET `stock_balance`=";
		strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
		strUpdate += ",`stock_creation_balance`=";
		strUpdate += sof_string::itostr( ui64crtUpd, strTrans );
		strUpdate += ",`stock_last_balance`=";
		strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
		strUpdate += ",`oper_time`=now() WHERE `account_id`='";
		strUpdate += strAccount;
		strUpdate += "' AND `stock_id`='";
		strUpdate += strStockID;
		strUpdate += "' AND `stock_balance`=";
		strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
		strUpdate += " AND `stock_creation_balance`=";
		strUpdate += sof_string::itostr( ui64crtOrg, strTrans );
		strUpdate += " AND `stock_last_balance`=";
		strUpdate += sof_string::itostr( ui64lastOrg, strTrans );

		simutgw::g_mapUserStock[strAccount][strStockID]->SetStore();
	}
	else
	{
		strUpdate = "INSERT INTO `stock_etf_asset` (`account_id`,`stock_id`,`trade_market`,\
`stock_balance`,`stock_auction_purchase_balance`,`stock_staple_purchase_balance`,\
`stock_creation_balance`,`stock_available`,`stock_last_balance`,\
`is_close`,`oper_time`,`operator`) VALUES('";
		strUpdate += strAccount;
		strUpdate += "','";
		strUpdate += strStockID;
		strUpdate += "',";
		std::string strTradeMarket;
		StockHelper::GetStockTradeMarket(strStockID, strTradeMarket);
		if (!strTradeMarket.empty())
		{
			strUpdate += strTradeMarket;
		}
		else
		{
			strUpdate += "0";
		}
		strUpdate += ",";
		strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
		strUpdate += ",0,0,";
		strUpdate += sof_string::itostr( ui64crtUpd, strTrans );
		strUpdate += ",0,0,0,now(),'SYS')";
	}

	iRes = ExcUpdateSql( strUpdate );
	if ( 0 != iRes )
	{
		std::string strDebug( "ͬ���ɷ���Ϣʧ��" );
		EzLog::e( strTag, strDebug );
	}

	// �����ݿ����м�¼
	simutgw::g_mapUserStock[strAccount][strStockID]->SetStore();

	return iRes;
}

/*
�ڴ���DB�ɷ���Ϣͬ��
�깺�ɷֹɼ���
*/
int StockCacheSynHelper::CrtComponentSyn( const std::string& strAccount, const std::string& strStockID )
{
	static const std::string strTag( "StockCacheSynHelper::CrtComponentSyn() " );

	uint64_t ui64stkOrg, ui64stkUpd, ui64actOrg, ui64actUpd, ui64avlOrg, ui64avlUpd, ui64lastOrg;
	ui64stkOrg = ui64stkUpd = ui64actOrg = ui64actUpd = ui64avlOrg = ui64avlUpd = ui64lastOrg = 0;

	int iRes = simutgw::g_mapUserStock[strAccount][strStockID]->CreationComponentDBSyn(
		ui64stkOrg, ui64stkUpd, ui64actOrg, ui64actUpd, ui64avlOrg, ui64avlUpd, ui64lastOrg );
	if ( 0 != iRes )
	{
		std::string strDebug( "CreationComponentDBSynʧ��" );
		EzLog::e( strTag, strDebug );
	}
	
	std::string strTrans, strUpdate;
	strUpdate = "UPDATE `stock_asset` SET `stock_balance`=";
	strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
	strUpdate += ",`stock_auction_purchase_balance`=";
	strUpdate += sof_string::itostr( ui64actUpd, strTrans );
	strUpdate += ",`stock_available`=";
	strUpdate += sof_string::itostr( ui64avlUpd, strTrans );
	strUpdate += ",`stock_last_balance`=";
	strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
	strUpdate += ",`oper_time`=now() WHERE `account_id`='";
	strUpdate += strAccount;
	strUpdate += "' AND `stock_id`='";
	strUpdate += strStockID;
	strUpdate += "' AND `stock_balance`=";
	strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
	strUpdate += " AND `stock_auction_purchase_balance`=";
	strUpdate += sof_string::itostr( ui64actOrg, strTrans );
	strUpdate += " AND `stock_available`=";
	strUpdate += sof_string::itostr( ui64avlOrg, strTrans );
	strUpdate += " AND `stock_last_balance`=";
	strUpdate += sof_string::itostr( ui64lastOrg, strTrans );

	iRes = ExcUpdateSql( strUpdate );
	if ( 0 != iRes )
	{
		std::string strDebug( "ͬ���ɷ���Ϣʧ��" );
		EzLog::e( strTag, strDebug );
	}

	return iRes;
}

/*
�ڴ���DB�ɷ���Ϣͬ��
���ETF����
*/
int StockCacheSynHelper::RdpEtfSyn( const std::string& strAccount, const std::string& strStockID )
{
	static const std::string strTag( "StockCacheSynHelper::RdpEtfSyn() " );

	uint64_t ui64stkOrg, ui64stkUpd, ui64actOrg, ui64actUpd, ui64avlOrg, ui64avlUpd, ui64lastOrg;
	ui64stkOrg = ui64stkUpd = ui64actOrg = ui64actUpd = ui64avlOrg = ui64avlUpd = ui64lastOrg = 0;

	int iRes = simutgw::g_mapUserStock[strAccount][strStockID]->RdpEtfSyn(
		ui64stkOrg, ui64stkUpd, ui64actOrg, ui64actUpd, ui64avlOrg, ui64avlUpd, ui64lastOrg );
	if ( 0 != iRes )
	{
		std::string strDebug( "RdpEtfSynʧ��" );
		EzLog::e( strTag, strDebug );
	}
	
	std::string strTrans, strUpdate( "UPDATE `stock_etf_asset` SET `stock_balance`=" );
	strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
	strUpdate += ",`stock_auction_purchase_balance`=";
	strUpdate += sof_string::itostr( ui64actUpd, strTrans );
	strUpdate += ",`stock_available`=";
	strUpdate += sof_string::itostr( ui64avlUpd, strTrans );
	strUpdate += ",`stock_last_balance`=";
	strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
	strUpdate += ",`oper_time`=now() WHERE `account_id`='";
	strUpdate += strAccount;
	strUpdate += "' AND `stock_id`='";
	strUpdate += strStockID;
	strUpdate += "' AND `stock_balance`=";
	strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
	strUpdate += " AND `stock_auction_purchase_balance`=";
	strUpdate += sof_string::itostr( ui64actOrg, strTrans );
	strUpdate += " AND `stock_available`=";
	strUpdate += sof_string::itostr( ui64avlOrg, strTrans );
	strUpdate += " AND `stock_last_balance`=";
	strUpdate += sof_string::itostr( ui64lastOrg, strTrans );

	iRes = ExcUpdateSql( strUpdate );
	if ( 0 != iRes )
	{
		std::string strDebug( "ͬ���ɷ���Ϣʧ��" );
		EzLog::e( strTag, strDebug );
	}

	return iRes;
}

/*
�ڴ���DB�ɷ���Ϣͬ��
��سɷֹ�����
*/
int StockCacheSynHelper::RdpComponentSyn( const std::string& strAccount, const std::string& strStockID )
{
	static const std::string strTag( "StockCacheSynHelper::RdpComponentSyn() " );

	uint64_t ui64stkOrg, ui64stkUpd, ui64rdpOrg, ui64rdpUpd, ui64lastOrg;

	int iRes = simutgw::g_mapUserStock[strAccount][strStockID]->AddRdpComponentSyn( ui64stkOrg, ui64stkUpd,
		ui64rdpOrg, ui64rdpUpd, ui64lastOrg );
	if ( 0 != iRes )
	{
		std::string strDebug( "AddRdpComponentSynʧ��" );
		EzLog::e( strTag, strDebug );
	}
	
	std::string strTrans, strUpdate;
	if ( simutgw::g_mapUserStock[strAccount][strStockID]->CheckStore() )
	{
		strUpdate = "UPDATE `stock_asset` SET `stock_balance`=";
		strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
		strUpdate += ",`stock_etf_redemption_balance`=";
		strUpdate += sof_string::itostr( ui64rdpUpd, strTrans );
		strUpdate += ",`stock_last_balance`=";
		strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
		strUpdate += ",`oper_time`=now() WHERE `account_id`='";
		strUpdate += strAccount;
		strUpdate += "' AND `stock_id`='";
		strUpdate += strStockID;
		strUpdate += "' AND `stock_balance`=";
		strUpdate += sof_string::itostr( ui64stkOrg, strTrans );
		strUpdate += " AND `stock_etf_redemption_balance`=";
		strUpdate += sof_string::itostr( ui64rdpOrg, strTrans );
		strUpdate += " AND `stock_last_balance`=";
		strUpdate += sof_string::itostr( ui64lastOrg, strTrans );

		simutgw::g_mapUserStock[strAccount][strStockID]->SetStore();
	}
	else
	{
		strUpdate = "INSERT INTO `stock_asset` (`account_id`,`stock_id`,`trade_market`,\
`stock_balance`,`stock_auction_purchase_balance`,`stock_staple_purchase_balance`,\
`stock_etf_redemption_balance`,`stock_available`,`stock_last_balance`,\
`is_close`,`oper_time`,`operator`) VALUES('";
		strUpdate += strAccount;
		strUpdate += "','";
		strUpdate += strStockID;
		strUpdate += "',";
		std::string strTradeMarket;
		StockHelper::GetStockTradeMarket(strStockID, strTradeMarket);
		if (!strTradeMarket.empty())
		{
			strUpdate += strTradeMarket;
		}
		else
		{
			strUpdate += "0";
		}
		strUpdate += ",0,0,";
		strUpdate += sof_string::itostr( ui64stkUpd, strTrans );
		strUpdate += ",";
		strUpdate += sof_string::itostr( ui64rdpUpd, strTrans );
		strUpdate += ",0,0,0,0,now(),'SYS')";
	}

	iRes = ExcUpdateSql( strUpdate );
	if ( 0 != iRes )
	{
		std::string strDebug( "ͬ���ɷ���Ϣʧ��" );
		EzLog::e( strTag, strDebug );
	}

	// �����ݿ����м�¼
	simutgw::g_mapUserStock[strAccount][strStockID]->SetStore();

	return iRes;
}

/*
ִ�и���sql���
*/
int StockCacheSynHelper::ExcUpdateSql( const std::string& strUpdate )
{
	//static const std::string strTag( "StockCacheSynHelper::ExcUpdateSql() " );

	// 3 -- ��ͨģʽ ��¼���ݿ�
	TaskStockCacheSyn* Task( new TaskStockCacheSyn( 1 ) );
	Task->SetSql( strUpdate );
	std::shared_ptr<TaskBase> ptrBase( (TaskBase*) Task );
	simutgw::g_asyncDbwriter.AssignTask( ptrBase );


	return 0;
}