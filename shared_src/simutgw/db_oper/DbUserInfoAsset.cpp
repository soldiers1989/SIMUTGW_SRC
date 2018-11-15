#include "DbUserInfoAsset.h"

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"

#include "tool_json/RapidJsonHelper_tgw.h"

#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "etf/conf_etf_info.h"
#include "quotation/MarketInfoHelper.h"

/*
在数据库中获取用户的余额等数据

Return :
0 -- 获取成功
<0 -- 获取失败
1 -- 余券不足
*/
int DbUserInfoAsset::GetDbUserData(std::shared_ptr<struct simutgw::OrderMessage>& ptrObj)
{
	static const string strTag("StockOrderHelper::GetDbUserData()");

	string strQueryString;

	int iReturn = 0;


	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		// 查询用户信息及余额
		strQueryString = "SELECT * FROM `account` WHERE `security_account`=\"";
		strQueryString += ptrObj->strAccountId;
		strQueryString += "\"";

		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(strTag, "Get Connection is NULL");

			return -1;
		}

		int iRes = mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
			if (0 != mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				ptrObj->strTrade_group = mapRowData["trade_group"].strValue;
			}
			else
			{
				string strDebug("account=[");
				strDebug += ptrObj->strAccountId;
				strDebug += "] not exists";
				EzLog::e(strTag, strDebug);

				iReturn = -3;
			}

			// 释放
			mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

		}
		else
		{
			string strItoa;
			string strDebug("运行[");
			strDebug += strQueryString;
			strDebug += "]得到";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(strTag, strDebug);

			iReturn = -2;
		}

		if (0 != iReturn)
		{
			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
			return iReturn;
		}

		//  查询用户股票持有情况
		strQueryString = "SELECT * FROM `stock_asset` WHERE `account_id`=\"";
		strQueryString += ptrObj->strAccountId;
		strQueryString += "\" AND `stock_id`=\"";
		strQueryString += ptrObj->strStockID;
		strQueryString += "\"";

		iRes = mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
			if (0 != mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				// 证券持有数量，股份余额
				string strStockBalance(mapRowData["stock_balance"].strValue);

				uint64_t ui64StockNum = 0;

				int iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStockBalance, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64StockBalance_beforematch = ui64StockNum;
					ptrObj->userHold.ui64StockBalance_aftermatch = ui64StockNum;

					if (!strStockBalance.empty())
					{
						ptrObj->bIsUserHoldingCurrentStock = true;
					}
					else
					{
						ptrObj->bIsUserHoldingCurrentStock = false;
					}


					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_balance=[";
					strDebug += strStockBalance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 竞价买入量
				string str_stock_auction_purchase_balance(
					mapRowData["stock_auction_purchase_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
					str_stock_auction_purchase_balance, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64Stock_auction_purchase_beforematch = ui64StockNum;
					ptrObj->userHold.ui64Stock_auction_purchase_aftermatch = ui64StockNum;

					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_auction_purchase_balance=[";
					strDebug += str_stock_auction_purchase_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 大宗买入量
				string str_stock_staple_purchase_balance(
					mapRowData["stock_staple_purchase_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
					str_stock_staple_purchase_balance, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64Stock_staple_purchase_beforematch = ui64StockNum;
					ptrObj->userHold.ui64Stock_staple_purchase_aftermatch = ui64StockNum;

					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_staple_purchase_balance=[";
					strDebug += str_stock_staple_purchase_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// etf赎回量，可竞价卖出 或 etf申购量，可竞价卖出
				string str_stock_etf_redemption_balance(
					mapRowData["stock_etf_redemption_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
					str_stock_etf_redemption_balance, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64Stock_etfredemption_creation_beforematch = ui64StockNum;
					ptrObj->userHold.ui64Stock_etfredemption_creation_aftermatch = ui64StockNum;

					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_etf_redemption_balance=[";
					strDebug += str_stock_etf_redemption_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 证券持有可用余额，可用于申购etf份额和可竞价卖出
				string strStockAvaliable(mapRowData["stock_available"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStockAvaliable, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64StockAvailable_beforematch = ui64StockNum;
					ptrObj->userHold.ui64StockAvailable_aftermatch = ui64StockNum;

					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_avaliable=[";
					strDebug += strStockAvaliable;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 上次余额
				string strStock_last_balance(mapRowData["stock_last_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStock_last_balance, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64Stock_last_balance = ui64StockNum;

					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_last_balance=[";
					strDebug += strStock_last_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}


				if ((0 == ptrObj->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == ptrObj->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
					&& (ptrObj->userHold.ui64StockAvailable_beforematch < ptrObj->ui64Orderqty_origin))
				{
					// 卖出可用证券数量不足
					iReturn = 1;
				}
			}
			else
			{
				// 无此股票信息
				ptrObj->userHold.ui64StockBalance_beforematch = 0;
				ptrObj->userHold.ui64StockBalance_aftermatch = 0;
				//
				ptrObj->userHold.ui64Stock_auction_purchase_beforematch = 0;
				ptrObj->userHold.ui64Stock_auction_purchase_aftermatch = 0;
				//
				ptrObj->userHold.ui64Stock_staple_purchase_beforematch = 0;
				ptrObj->userHold.ui64Stock_staple_purchase_aftermatch = 0;
				//
				ptrObj->userHold.ui64Stock_etfredemption_creation_beforematch = 0;
				ptrObj->userHold.ui64Stock_etfredemption_creation_aftermatch = 0;
				//
				ptrObj->userHold.ui64StockAvailable_beforematch = 0;
				ptrObj->userHold.ui64StockAvailable_aftermatch = 0;

				ptrObj->userHold.ui64Stock_last_balance = 0;
				ptrObj->bIsUserHoldingCurrentStock = false;

				iReturn = 0;
			}

			// 释放
			mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
		}
		else
		{
			string strItoa;
			string strDebug("运行[");
			strDebug += strQueryString;
			strDebug += "]得到";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(strTag, strDebug);

			iReturn = -2;
		}

	}
	catch (exception& e)
	{
		EzLog::ex(strTag, e);

		return -1;
	}

	return iReturn;
}

/*
在数据库中获取用户的帐户相关数据

Return :
0 -- 获取成功
<0 -- 获取失败
*/
int DbUserInfoAsset::GetDb_UserAcntInfo(std::shared_ptr<struct simutgw::OrderMessage>& ptrObj)
{
	static const string strTag("StockOrderHelper::GetDb_UserAcntInfo()");

	string strQueryString;

	int iReturn = 0;


	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		// 查询用户信息及余额
		strQueryString = "SELECT * FROM `account` WHERE `security_account`=\"";
		strQueryString += ptrObj->strAccountId;
		strQueryString += "\"";

		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(strTag, "Get Connection is NULL");

			return -1;
		}

		int iRes = mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
			if (0 != mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				ptrObj->strTrade_group = mapRowData["trade_group"].strValue;
			}
			else
			{
				string strDebug("account=[");
				strDebug += ptrObj->strAccountId;
				strDebug += "] not exists";
				EzLog::e(strTag, strDebug);

				iReturn = -3;
			}

			// 释放
			mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

		}
		else
		{
			string strItoa;
			string strDebug("运行[");
			strDebug += strQueryString;
			strDebug += "]得到";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(strTag, strDebug);

			iReturn = -2;
		}

		if (0 != iReturn)
		{
			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
			return iReturn;
		}
	}
	catch (exception& e)
	{
		EzLog::ex(strTag, e);

		return -1;
	}

	return iReturn;
}

/*
在数据库中获取用户的ETF持仓

Return :
0 -- 获取成功
<0 -- 获取失败
*/
int DbUserInfoAsset::GetDb_User_Etf(std::shared_ptr<struct simutgw::OrderMessage>& ptrObj)
{
	static const string strTag("DbUserInfoAsset::GetDb_User_Etf()");

	string strQueryString;

	int iReturn = 0;

	try
	{
		int iRes = 0;
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		// 初始化
		ptrObj->userHold.ui64StockBalance_beforematch = 0;
		ptrObj->userHold.ui64StockBalance_aftermatch = 0;
		//
		ptrObj->userHold.ui64Stock_auction_purchase_beforematch = 0;
		ptrObj->userHold.ui64Stock_auction_purchase_aftermatch = 0;
		//
		ptrObj->userHold.ui64Stock_staple_purchase_beforematch = 0;
		ptrObj->userHold.ui64Stock_staple_purchase_aftermatch = 0;
		//
		ptrObj->userHold.ui64Stock_etfredemption_creation_beforematch = 0;
		ptrObj->userHold.ui64Stock_etfredemption_creation_aftermatch = 0;
		//
		ptrObj->userHold.ui64StockAvailable_beforematch = 0;
		ptrObj->userHold.ui64StockAvailable_aftermatch = 0;

		ptrObj->userHold.ui64Stock_last_balance = 0;
		ptrObj->bIsUserHoldingCurrentStock = false;

		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(strTag, "Get Connection is NULL");

			return -1;
		}

		//  查询用户股票持有情况
		strQueryString = "SELECT * FROM `stock_etf_asset` WHERE `account_id`=\"";
		strQueryString += ptrObj->strAccountId;
		strQueryString += "\" AND `stock_id`=\"";
		strQueryString += ptrObj->strStockID;
		strQueryString += "\"";

		iRes = mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
			if (0 != mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				// 持有etf份额，股份余额
				string strStockBalance(mapRowData["stock_balance"].strValue);

				uint64_t ui64StockNum = 0;

				int iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStockBalance, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64StockBalance_beforematch = ui64StockNum;
					ptrObj->userHold.ui64StockBalance_aftermatch = ui64StockNum;

					if (!strStockBalance.empty())
					{
						ptrObj->bIsUserHoldingCurrentStock = true;
					}
					else
					{
						ptrObj->bIsUserHoldingCurrentStock = false;
					}

					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_balance=[";
					strDebug += strStockBalance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 竞价买入量
				string str_stock_auction_purchase_balance(
					mapRowData["stock_auction_purchase_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
					str_stock_auction_purchase_balance, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64Stock_auction_purchase_beforematch = ui64StockNum;
					ptrObj->userHold.ui64Stock_auction_purchase_aftermatch = ui64StockNum;

					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_auction_purchase_balance=[";
					strDebug += str_stock_auction_purchase_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 大宗买入量
				string str_stock_staple_purchase_balance(
					mapRowData["stock_staple_purchase_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
					str_stock_staple_purchase_balance, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64Stock_staple_purchase_beforematch = ui64StockNum;
					ptrObj->userHold.ui64Stock_staple_purchase_aftermatch = ui64StockNum;

					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_staple_purchase_balance=[";
					strDebug += str_stock_staple_purchase_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// etf赎回量，可竞价卖出 或 etf申购量，可竞价卖出
				string str_stock_creation_balance(
					mapRowData["stock_creation_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
					str_stock_creation_balance, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64Stock_etfredemption_creation_beforematch = ui64StockNum;
					ptrObj->userHold.ui64Stock_etfredemption_creation_aftermatch = ui64StockNum;

					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_creation_balance=[";
					strDebug += str_stock_creation_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 证券持有可用etf份额，可赎回和可竞价卖出
				string strStockAvaliable(mapRowData["stock_available"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStockAvaliable, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64StockAvailable_beforematch = ui64StockNum;
					ptrObj->userHold.ui64StockAvailable_aftermatch = ui64StockNum;

					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_avaliable=[";
					strDebug += strStockAvaliable;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 上次余额
				string strStock_last_balance(mapRowData["stock_last_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStock_last_balance, ui64StockNum);
				if (0 == iTranRes)
				{
					ptrObj->userHold.ui64Stock_last_balance = ui64StockNum;

					iReturn = 0;
				}
				else
				{
					string strDebug("account=[");
					strDebug += ptrObj->strAccountId;
					strDebug += "] stockId=[";
					strDebug += ptrObj->strStockID;
					strDebug += "] stock_last_balance=[";
					strDebug += strStock_last_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}
			}
			else
			{
				// 无此股票信息				

				iReturn = 0;
			}

			// 释放
			mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
		}
		else
		{
			string strItoa;
			string strDebug("运行[");
			strDebug += strQueryString;
			strDebug += "]得到";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(strTag, strDebug);

			iReturn = -2;
		}

	}
	catch (exception& e)
	{
		EzLog::ex(strTag, e);

		return -1;
	}

	return iReturn;
}

/*
在数据库中获取用户的单只股票的持仓

Return :
0 -- 获取成功，用户持有该只股票
1 -- 获取失败，用户不持有该只股票
-1 -- 获取失败，其他类型的错误
*/
int DbUserInfoAsset::GetDb_UserHoldStock(std::shared_ptr<MySqlCnnC602>& mysqlConn,
	const std::string& in_sAccountId, const std::string& in_sStockId,
	std::shared_ptr<struct simutgw::TradeStock>& out_ptrUserStock)
{
	static const string strTag("DbUserInfoAsset::GetDb_UserHoldStock()");

	if (in_sAccountId.empty() || in_sStockId.empty())
	{
		return -1;
	}

	std::string strQueryString;

	int iReturn = 0;
	int iRes = 0;
	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	// 初始化	
	out_ptrUserStock->ui64StockBalance_beforematch = 0;
	out_ptrUserStock->ui64StockBalance_aftermatch = 0;
	//
	out_ptrUserStock->ui64Stock_auction_purchase_beforematch = 0;
	out_ptrUserStock->ui64Stock_auction_purchase_aftermatch = 0;
	//
	out_ptrUserStock->ui64Stock_staple_purchase_beforematch = 0;
	out_ptrUserStock->ui64Stock_staple_purchase_aftermatch = 0;
	//
	out_ptrUserStock->ui64Stock_etfredemption_creation_beforematch = 0;
	out_ptrUserStock->ui64Stock_etfredemption_creation_aftermatch = 0;
	//
	out_ptrUserStock->ui64StockAvailable_beforematch = 0;
	out_ptrUserStock->ui64StockAvailable_aftermatch = 0;

	//  查询用户股票持有情况
	strQueryString = "SELECT * FROM `stock_asset` WHERE `account_id`=\"";
	strQueryString += in_sStockId;
	strQueryString += "\" AND `stock_id`=\"";
	strQueryString += in_sAccountId;
	strQueryString += "\"";

	iRes = mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
	if (1 == iRes)
	{
		// select
		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		if (0 != mysqlConn->FetchNextRow(&pResultSet, mapRowData))
		{
			// 证券持有数量，股份余额
			string strStockBalance(mapRowData["stock_balance"].strValue);

			uint64_t ui64StockNum = 0;

			int iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStockBalance, ui64StockNum);
			if (0 == iTranRes)
			{
				out_ptrUserStock->ui64StockBalance_beforematch = ui64StockNum;
				out_ptrUserStock->ui64StockBalance_aftermatch = ui64StockNum;

				if (!strStockBalance.empty())
				{

				}
				else
				{

				}

				iReturn = 0;
			}
			else
			{
				string strDebug("account=[");
				strDebug += in_sAccountId;
				strDebug += "] stockId=[";
				strDebug += in_sStockId;
				strDebug += "] stock_balance=[";
				strDebug += strStockBalance;
				strDebug += "] trans error";
				EzLog::e(strTag, strDebug);

				iReturn = -1;
			}

			// 竞价买入量
			string str_stock_auction_purchase_balance(
				mapRowData["stock_auction_purchase_balance"].strValue);
			iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
				str_stock_auction_purchase_balance, ui64StockNum);
			if (0 == iTranRes)
			{
				out_ptrUserStock->ui64Stock_auction_purchase_beforematch = ui64StockNum;
				out_ptrUserStock->ui64Stock_auction_purchase_aftermatch = ui64StockNum;

				iReturn = 0;
			}
			else
			{
				string strDebug("account=[");
				strDebug += in_sAccountId;
				strDebug += "] stockId=[";
				strDebug += in_sStockId;
				strDebug += "] stock_auction_purchase_balance=[";
				strDebug += str_stock_auction_purchase_balance;
				strDebug += "] trans error";
				EzLog::e(strTag, strDebug);

				iReturn = -1;
			}

			// 大宗买入量
			string str_stock_staple_purchase_balance(
				mapRowData["stock_staple_purchase_balance"].strValue);
			iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
				str_stock_staple_purchase_balance, ui64StockNum);
			if (0 == iTranRes)
			{
				out_ptrUserStock->ui64Stock_staple_purchase_beforematch = ui64StockNum;
				out_ptrUserStock->ui64Stock_staple_purchase_aftermatch = ui64StockNum;

				iReturn = 0;
			}
			else
			{
				string strDebug("account=[");
				strDebug += in_sAccountId;
				strDebug += "] stockId=[";
				strDebug += in_sStockId;
				strDebug += "] stock_staple_purchase_balance=[";
				strDebug += str_stock_staple_purchase_balance;
				strDebug += "] trans error";
				EzLog::e(strTag, strDebug);

				iReturn = -1;
			}

			// etf赎回量，可竞价卖出 或 etf申购量，可竞价卖出
			string str_stock_etf_redemption_balance(
				mapRowData["stock_etf_redemption_balance"].strValue);
			iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
				str_stock_etf_redemption_balance, ui64StockNum);
			if (0 == iTranRes)
			{
				out_ptrUserStock->ui64Stock_etfredemption_creation_beforematch = ui64StockNum;
				out_ptrUserStock->ui64Stock_etfredemption_creation_aftermatch = ui64StockNum;

				iReturn = 0;
			}
			else
			{
				string strDebug("account=[");
				strDebug += in_sAccountId;
				strDebug += "] stockId=[";
				strDebug += in_sStockId;
				strDebug += "] stock_etf_redemption_balance=[";
				strDebug += str_stock_etf_redemption_balance;
				strDebug += "] trans error";
				EzLog::e(strTag, strDebug);

				iReturn = -1;
			}

			// 证券持有可用余额，可用于申购etf份额和可竞价卖出
			string strStockAvaliable(mapRowData["stock_available"].strValue);
			iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStockAvaliable, ui64StockNum);
			if (0 == iTranRes)
			{
				out_ptrUserStock->ui64StockAvailable_beforematch = ui64StockNum;
				out_ptrUserStock->ui64StockAvailable_aftermatch = ui64StockNum;

				iReturn = 0;
			}
			else
			{
				string strDebug("account=[");
				strDebug += in_sAccountId;
				strDebug += "] stockId=[";
				strDebug += in_sStockId;
				strDebug += "] stock_avaliable=[";
				strDebug += strStockAvaliable;
				strDebug += "] trans error";
				EzLog::e(strTag, strDebug);

				iReturn = -1;
			}

			// 上次余额
			string strStock_last_balance(mapRowData["stock_last_balance"].strValue);
			iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStock_last_balance, ui64StockNum);
			if (0 == iTranRes)
			{
				out_ptrUserStock->ui64Stock_last_balance = ui64StockNum;

				iReturn = 0;
			}
			else
			{
				string strDebug("account=[");
				strDebug += in_sAccountId;
				strDebug += "] stockId=[";
				strDebug += in_sStockId;
				strDebug += "] stock_last_balance=[";
				strDebug += strStock_last_balance;
				strDebug += "] trans error";
				EzLog::e(strTag, strDebug);

				iReturn = -1;
			}
		}
		else
		{
			// 无此股票信息
			iReturn = 0;
		}

		// 释放
		mysqlConn->FreeResult(&pResultSet);
		pResultSet = NULL;

	}

	return iReturn;
}

/*
从数据库加载股份信息

Return:
0 -- 成功
-1 -- 失败
*/
int DbUserInfoAsset::LoadUserStockFromDB(const std::string& strAccount, const std::string& strStockID,
struct UserStockInfo& out_usrStkInfo)
{
	static const std::string strTag("DbUserInfoAsset::LoadUserStockFromDB() ");

	//	从mysql连接池取连接
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (NULL == mysqlConn)
	{
		//	取出的mysql连接为NULL

		//	归还连接
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		EzLog::e(strTag, "Get Connection is NULL");

		return -1;
	}

	enum StockHelper::StockType stkType = StockHelper::Ordinary;
	int iRes = GetStockType(strStockID, stkType);
	if (0 != iRes)
	{
		return -1;
	}

	std::string strQuery;
	strQuery = "SELECT `stock_balance`,`stock_auction_purchase_balance`,\
			   			   `stock_staple_purchase_balance`,`stock_available`,`stock_last_balance`,";

	switch (stkType)
	{
	case StockHelper::Ordinary:
		strQuery += "`stock_etf_redemption_balance` FROM `stock_asset`";
		break;

	case StockHelper::Etf:
		strQuery += "`stock_creation_balance` FROM `stock_etf_asset`";
		break;
	default:
		break;
	}

	strQuery += " WHERE `account_id`='";
	strQuery += strAccount;
	strQuery += "' AND `stock_id`='";
	strQuery += strStockID;
	strQuery += "'";

	int iReturn = 0;
	MYSQL_RES *pResultSet = NULL;
	try
	{
		mysqlConn->StartTransaction();

		unsigned long ulAffectedRows = 0;
		iRes = mysqlConn->Query(strQuery, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
			if (0 != mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				// 持有etf份额，股份余额
				string strStockBalance(mapRowData["stock_balance"].strValue);

				int iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStockBalance, out_usrStkInfo.ui64stkBalance);
				if (0 == iTranRes)
				{
				}
				else
				{
					string strDebug("account=[");
					strDebug += strAccount;
					strDebug += "] stockId=[";
					strDebug += strStockID;
					strDebug += "] stock_balance=[";
					strDebug += strStockBalance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 竞价买入量
				string str_stock_auction_purchase_balance(
					mapRowData["stock_auction_purchase_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
					str_stock_auction_purchase_balance, out_usrStkInfo.ui64stk_act_pch_Balance);
				if (0 == iTranRes)
				{
				}
				else
				{
					string strDebug("account=[");
					strDebug += strAccount;
					strDebug += "] stockId=[";
					strDebug += strStockID;
					strDebug += "] stock_auction_purchase_balance=[";
					strDebug += str_stock_auction_purchase_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 大宗买入量
				string str_stock_staple_purchase_balance(
					mapRowData["stock_staple_purchase_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
					str_stock_staple_purchase_balance, out_usrStkInfo.ui64stk_stp_pch_Balance);
				if (0 == iTranRes)
				{
				}
				else
				{
					string strDebug("account=[");
					strDebug += strAccount;
					strDebug += "] stockId=[";
					strDebug += strStockID;
					strDebug += "] stock_staple_purchase_balance=[";
					strDebug += str_stock_staple_purchase_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// etf申购量，可竞价卖出
				string str_stock_creation_balance(
					mapRowData["stock_creation_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(
					str_stock_creation_balance, out_usrStkInfo.ui64stk_crt_Balance);
				if (0 == iTranRes)
				{
				}
				else
				{
					string strDebug("account=[");
					strDebug += strAccount;
					strDebug += "] stockId=[";
					strDebug += strStockID;
					strDebug += "] stock_creation_balance=[";
					strDebug += str_stock_creation_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 证券持有可用etf份额，可赎回和可竞价卖出
				string strStockAvaliable(mapRowData["stock_available"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStockAvaliable, out_usrStkInfo.ui64stk_avl_Balance);
				if (0 == iTranRes)
				{
				}
				else
				{
					string strDebug("account=[");
					strDebug += strAccount;
					strDebug += "] stockId=[";
					strDebug += strStockID;
					strDebug += "] stock_avaliable=[";
					strDebug += strStockAvaliable;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// 上次余额
				string strStock_last_balance(mapRowData["stock_last_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStock_last_balance, out_usrStkInfo.ui64stk_last_Balance);
				if (0 == iTranRes)
				{
				}
				else
				{
					string strDebug("account=[");
					strDebug += strAccount;
					strDebug += "] stockId=[";
					strDebug += strStockID;
					strDebug += "] stock_last_balance=[";
					strDebug += strStock_last_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}

				// etf赎回量，可竞价卖出
				string strStock_etf_redemption_balance(mapRowData["stock_etf_redemption_balance"].strValue);
				iTranRes = Tgw_StringUtil::String2UInt64_strtoui64(strStock_etf_redemption_balance,
					out_usrStkInfo.ui64stk_etf_rdp_Balance);
				if (0 == iTranRes)
				{
				}
				else
				{
					string strDebug("account=[");
					strDebug += strAccount;
					strDebug += "] stockId=[";
					strDebug += strStockID;
					strDebug += "] stock_last_balance=[";
					strDebug += strStock_etf_redemption_balance;
					strDebug += "] trans error";
					EzLog::e(strTag, strDebug);

					iReturn = -1;
				}
			}
			else
			{
				// 无此股票信息				
				iReturn = -1;
			}

			// 释放
			mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
		}
	}
	catch (...)
	{
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
	}


	return iReturn;
}
