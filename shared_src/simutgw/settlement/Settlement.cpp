#include "Settlement.h"
#include "simutgw/settlement/SZSettle.h"
#include "simutgw/settlement/SHSettle.h"

#include "boost/filesystem.hpp"
#include "boost/date_time.hpp"

#include "config/conf_msg.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "order/define_order_msg.h"

#include "tool_file/FileOperHelper.h"

#include "tool_string/Tgw_StringUtil.h"

#include "tool_mysql/MysqlConnectionPool.h"

Settlement::Settlement(void)
	:m_scl(keywords::channel = "Settlement")
{
}

Settlement::~Settlement(void)
{
}

/*
结算
@param const std::vector<std::string>& in_vctSettleGroup : 清算池别名
@param std::string& out_strDay : 当前日期字符串
@param std::string& out_strSettlementFilePath : 清算文件所在路径

Return :
0 -- 成功
非0 -- 失败
*/
int Settlement::MakeSettlement(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::vector<std::string>& in_vctSettleGroup,
	std::string& out_strDay, std::string& out_strSettlementFilePath)
{
	static const string ftag("Settlement::MakeSettlement() ");

	int iRes = 0;

	string strDay;
	string strFilePath;
	iRes = FileOperHelper::MakeDirDate_InParentPath("statement", strDay, strFilePath);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "MakeDirDate failed, path=" << strFilePath;
		return -1;
	}

	const std::vector<string>* pvctNames = nullptr;

	std::vector<string> vctSettGroupNames_local;

	if (0 == in_vctSettleGroup.size())
	{
		// 统计有多少SettleGroup
		iRes = GetSettleGroupNames(in_mysqlConn, vctSettGroupNames_local);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetSettleGroupNames failed";
			return -1;
		}

		pvctNames = &vctSettGroupNames_local;
	}
	else
	{
		pvctNames = &in_vctSettleGroup;
	}

	SHSettle sh;
	SZSettle sz;
	for (size_t i = 0; i < pvctNames->size(); ++i)
	{
		iRes = sh.MakeSettlementDetails(in_mysqlConn, (*pvctNames)[i], strFilePath);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Shanghai Market settle datails failed";
			return -1;
		}

		iRes = sz.MakeSettlementDetails(in_mysqlConn, (*pvctNames)[i], strFilePath);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Shenzhen Market settle datails failed";
			return -1;
		}
	}

	iRes = sh.MakeSettlementSummary(in_mysqlConn, strFilePath);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Shanghai Market settle summary failed";
		return -1;
	}

	iRes = sz.MakeSettlementSummary(in_mysqlConn, strFilePath);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Shenzhen Market settle summary failed";
		return -1;
	}

	iRes = Stock_Settle();
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Stock settle failed";
		return -1;
	}

	out_strDay = strDay;
	out_strSettlementFilePath = strFilePath;

	return 0;
}

/*
获取所有的清算池别名

Return :
0 -- 成功
非0 -- 失败
*/
int Settlement::GetSettleGroupNames(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
	std::vector<string>& out_vctSettGroupNames)
{
	static const string ftag("Settlement::GetSettleGroupNames() ");

	int iReturn = 0;
	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		// 查询
		string strQueryString = "SELECT `settle_group` FROM `order_match` GROUP BY `settle_group`;";

		int iRes = in_mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
			while (0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				if (mapRowData["settle_group"].bIsNull)
				{
					// IS NULL
					out_vctSettGroupNames.push_back("");
				}
				else
				{
					out_vctSettGroupNames.push_back(mapRowData["settle_group"].strValue);
				}
			}

			// 释放
			in_mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;
		}
	}
	catch (std::exception& e)
	{
		EzLog::ex(ftag, e);

		iReturn = -1;
	}

	return iReturn;
}

/*
stock_asset表清算，将可用股份更新为持有股份

Return :
0 -- 成功
非0 -- 失败
*/
int Settlement::Stock_Settle()
{
	static const string ftag("Settlement::Stock_Settle() ");

	int iRes = Stock_Asset_Settle();
	if (0 != iRes)
	{
		EzLog::e(ftag, "Stock_Asset_Settle() Failed");

		return -1;
	}

	iRes = Stock_ETF_Asset_Settle();
	if (0 != iRes)
	{
		EzLog::e(ftag, "Stock_ETF_Asset_Settle() Failed");

		return -1;
	}

	return 0;
}

/*
stock_asset表清算，将可用股份更新为持有股份

Return :
0 -- 成功
非0 -- 失败
*/
int Settlement::Stock_Asset_Settle()
{
	static const string ftag("Settlement::Stock_Settle() ");

	//从mysql连接池取连接
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (NULL == mysqlConn)
	{
		//取出的mysql连接为NULL

		//归还连接
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		EzLog::e(ftag, "Get Connection is NULL");

		return -1;
	}

	std::string strQueryString("SELECT `id`,`stock_balance`,`stock_auction_purchase_balance`,"
		"`stock_staple_purchase_balance`,`stock_etf_redemption_balance`,`stock_available` FROM `stock_asset`");
	try
	{
		mysqlConn->StartTransaction();

		vector<struct Stock::StockRecord> vecStockRecord;
		// 先查询出股份记录
		int iRes = GetStock(mysqlConn, strQueryString, vecStockRecord);
		if (0 > iRes)
		{
			EzLog::e(ftag, "Get Stock faild");
			return -1;
		}

		// 更新每条股份信息
		for (size_t i = 0; i < vecStockRecord.size(); ++i)
		{
			Update_Stock_Asset(mysqlConn, vecStockRecord[i]);
		}

		mysqlConn->Commit();
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
	}

	//归还连接
	simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

	return 0;
}

/*
stock_etf_asset表清算，将可用股份更新为持有股份

Return :
0 -- 成功
非0 -- 失败
*/
int Settlement::Stock_ETF_Asset_Settle()
{
	static const string ftag("Settlement::Stock_Settle() ");

	//从mysql连接池取连接
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (NULL == mysqlConn)
	{
		//取出的mysql连接为NULL

		//归还连接
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		EzLog::e(ftag, "Get Connection is NULL");

		return -1;
	}

	std::string strQueryString("SELECT `id`,`stock_balance`,`stock_auction_purchase_balance`,"
		"`stock_staple_purchase_balance`,`stock_creation_balance`,`stock_available` FROM `stock_etf_asset`");
	try
	{
		mysqlConn->StartTransaction();

		vector<struct Stock::StockRecord> vecStockRecord;
		// 先查询出股份记录
		int iRes = GetStock(mysqlConn, strQueryString, vecStockRecord);
		if (0 > iRes)
		{
			EzLog::e(ftag, "Get Stock faild");
			return -1;
		}

		// 更新每条股份信息
		for (size_t i = 0; i < vecStockRecord.size(); ++i)
		{
			Update_Stock_ETF_Asset(mysqlConn, vecStockRecord[i]);
		}

		mysqlConn->Commit();
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
	}

	//归还连接
	simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

	return 0;
}


/*
stock_asset表清算，将可用股份更新为持有股份
查询出所有的股份记录

Return :
0 -- 成功
非0 -- 失败
*/
int Settlement::GetStock(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
	const std::string& in_strQuery,
	std::vector<struct Stock::StockRecord>& io_vecStockRecord)
{
	static const string ftag("Settlement::GetStock() ");

	int iReturn = 0;
	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		int iRes = in_mysqlConn->Query(in_strQuery, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
			while (0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				struct Stock::StockRecord record;
				record.strId = mapRowData["id"].strValue;
				Tgw_StringUtil::String2UInt64_strtoui64(mapRowData["stock_balance"].strValue,
					record.ui64StockBalance);
				Tgw_StringUtil::String2UInt64_strtoui64(mapRowData["stock_auction_purchase_balance"].strValue,
					record.ui64ActBalance);
				Tgw_StringUtil::String2UInt64_strtoui64(mapRowData["stock_staple_purchase_balance"].strValue,
					record.ui64StpBalance);
				Tgw_StringUtil::String2UInt64_strtoui64(mapRowData["stock_etf_redemption_balance"].strValue,
					record.ui64RdpBalance);
				Tgw_StringUtil::String2UInt64_strtoui64(mapRowData["stock_creation_balance"].strValue,
					record.ui64CrtBalance);
				Tgw_StringUtil::String2UInt64_strtoui64(mapRowData["stock_available"].strValue,
					record.ui64AvlBalance);

				// uint64_t ui64Tmp = record.ui64ActBalance + record.ui64StpBalance + record.ui64RdpBalance + record.ui64AvlBalance;

				if (record.ui64StockBalance == record.ui64AvlBalance &&
					0 == record.ui64ActBalance && 0 == record.ui64StpBalance &&
					0 == record.ui64RdpBalance && 0 == record.ui64CrtBalance)
				{
					continue;
				}
				else
				{
					//
					io_vecStockRecord.push_back(record);
				}
			}

			// 释放
			in_mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

		}
		else
		{
			string strItoa;
			string strDebug("运行[");
			strDebug += in_strQuery;
			strDebug += "]得到";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(ftag, strDebug);

			iReturn = -2;
		}

	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		iReturn = -1;
	}

	return iReturn;
}

/*
stock_asset表清算，将可用股份更新为持有股份
更新一条股份记录

Return :
0 -- 成功
非0 -- 失败
*/
int Settlement::Update_Stock_Asset(std::shared_ptr<MySqlCnnC602>& mysqlConn,
	const struct Stock::StockRecord& in_stockRecord)
{
	static const string ftag("Settlement::Update_Stock_Asset() ");

	int iReturn = 0;
	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		uint64_t ui64Tmp = 0;
		std::string strTrans;

		ui64Tmp = in_stockRecord.ui64ActBalance + in_stockRecord.ui64StpBalance +
			in_stockRecord.ui64RdpBalance + in_stockRecord.ui64AvlBalance;
		// 更新股份记录
		string strQueryString = "UPDATE `stock_asset` SET `stock_balance`=";
		strQueryString += sof_string::itostr(ui64Tmp, strTrans);
		strQueryString += ", `stock_available`=";
		strQueryString += sof_string::itostr(ui64Tmp, strTrans);
		strQueryString += ", `stock_auction_purchase_balance`=0, `stock_staple_purchase_balance`=0,`stock_etf_redemption_balance`=0,`oper_time` = now()";
		strQueryString += " WHERE `id`=";
		strQueryString += in_stockRecord.strId;
		strQueryString += " AND `stock_balance`=";
		strQueryString += sof_string::itostr(in_stockRecord.ui64StockBalance, strTrans);
		strQueryString += " AND `stock_available`=";
		strQueryString += sof_string::itostr(in_stockRecord.ui64AvlBalance, strTrans);

		int iRes = mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (2 == iRes)
		{
			// 是更新
			if (1 != ulAffectedRows)
			{
				// 失败
				string strItoa;
				string strDebug("运行[");
				strDebug += strQueryString;
				strDebug += "]得到AffectedRows=";
				strDebug += sof_string::itostr((uint64_t)ulAffectedRows, strItoa);
				EzLog::e(ftag, strDebug);

				return -1;
			}
		}
		else
		{
			string strItoa;
			string strDebug("运行[");
			strDebug += strQueryString;
			strDebug += "]得到Res=";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(ftag, strDebug);

			return -1;
		}
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		iReturn = -1;
	}

	return iReturn;
}

/*
stock_etf_asset表清算，将可用股份更新为持有股份
更新一条股份记录

Return :
0 -- 成功
非0 -- 失败
*/
int Settlement::Update_Stock_ETF_Asset(std::shared_ptr<MySqlCnnC602>& mysqlConn, const struct Stock::StockRecord& in_stockRecord)
{
	static const string ftag("Settlement::Update_Stock_ETF_Asset() ");

	int iReturn = 0;
	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		uint64_t ui64Tmp = 0;
		std::string strTrans;

		ui64Tmp = in_stockRecord.ui64ActBalance + in_stockRecord.ui64StpBalance +
			in_stockRecord.ui64CrtBalance + in_stockRecord.ui64AvlBalance;
		// 更新股份记录
		string strQueryString = "UPDATE `stock_etf_asset` SET `stock_balance`=";
		strQueryString += sof_string::itostr(ui64Tmp, strTrans);
		strQueryString += ", `stock_available`=";
		strQueryString += sof_string::itostr(ui64Tmp, strTrans);
		strQueryString += ", `stock_auction_purchase_balance`=0, `stock_staple_purchase_balance`=0,`stock_creation_balance`=0,`oper_time` = now()";
		strQueryString += " WHERE `id`=";
		strQueryString += in_stockRecord.strId;
		strQueryString += " AND `stock_balance`=";
		strQueryString += sof_string::itostr(in_stockRecord.ui64StockBalance, strTrans);
		strQueryString += " AND `stock_available`=";
		strQueryString += sof_string::itostr(in_stockRecord.ui64AvlBalance, strTrans);

		int iRes = mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (2 == iRes)
		{
			// 是更新
			if (1 != ulAffectedRows)
			{
				// 失败
				string strItoa;
				string strDebug("运行[");
				strDebug += strQueryString;
				strDebug += "]得到AffectedRows=";
				strDebug += sof_string::itostr((uint64_t)ulAffectedRows, strItoa);
				EzLog::e(ftag, strDebug);

				return -1;
			}
		}
		else
		{
			string strItoa;
			string strDebug("运行[");
			strDebug += strQueryString;
			strDebug += "]得到Res=";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(ftag, strDebug);

			return -1;
		}
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		iReturn = -1;
	}

	return iReturn;
}