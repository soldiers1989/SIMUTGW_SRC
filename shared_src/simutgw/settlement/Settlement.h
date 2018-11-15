#ifndef __SETTLEMENT_H__
#define __SETTLEMENT_H__

#include "tool_file/TgwDBFOperHelper.h"
#include "tool_mysql/MySqlCnnC602.h"

#include "simutgw/settlement/SZSettle.h"
#include "simutgw/settlement/SHSettle.h"

namespace Stock
{
	// stock_asset表中的一条记录
	struct StockRecord
	{
		std::string strId;
		//`stock_balance`,
		uint64_t ui64StockBalance;
		//`stock_auction_purchase_balance`,
		uint64_t ui64ActBalance;
		//`stock_staple_purchase_balance`,
		uint64_t ui64StpBalance;
		//`stock_etf_redemption_balance`,
		uint64_t ui64RdpBalance;
		//`stock_creation_balance`
		uint64_t ui64CrtBalance;
		//`stock_available`
		uint64_t ui64AvlBalance;
	};
}

/*
结算类
*/
class Settlement
{

	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	//
	// Functions
	//
public:
	Settlement(void);
	virtual ~Settlement(void);

	/*
	结算
	@param const std::vector<std::string>& in_vctSettleGroup : 清算池别名
	@param std::string& out_strDay : 当前日期字符串
	@param std::string& out_strSettlementFilePath : 清算文件所在路径

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int MakeSettlement(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::vector<std::string>& in_vctSettleGroup,
		std::string& out_strDay, std::string& out_strSettlementFilePath);

protected:
	/*
	获取所有的清算池别名

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int GetSettleGroupNames(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
		std::vector<string>& out_vctSettGroupNames);

	/*
	stock_asset、stock_etf_asset表清算，将可用股份更新为持有股份

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Stock_Settle();

	/*
	stock_asset表清算，将可用股份更新为持有股份

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Stock_Asset_Settle();

	/*
	stock_etf_asset表清算，将可用股份更新为持有股份

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Stock_ETF_Asset_Settle();

	/*
	stock_asset表清算，将可用股份更新为持有股份
	查询出所有的股份记录

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int GetStock(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
		const std::string& in_strQuery,
		std::vector<struct Stock::StockRecord>& io_vecStockRecord);

	/*
	stock_asset表清算，将可用股份更新为持有股份
	更新一条股份记录

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Update_Stock_Asset(std::shared_ptr<MySqlCnnC602>& mysqlConn, const struct Stock::StockRecord& in_StockRecord);

	/*
	stock_etf_asset表清算，将可用股份更新为持有股份
	更新一条股份记录

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Update_Stock_ETF_Asset(std::shared_ptr<MySqlCnnC602>& mysqlConn, const struct Stock::StockRecord& in_StockRecord);
};

#endif