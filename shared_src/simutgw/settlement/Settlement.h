#ifndef __SETTLEMENT_H__
#define __SETTLEMENT_H__

#include "tool_file/TgwDBFOperHelper.h"
#include "tool_mysql/MySqlCnnC602.h"

#include "simutgw/settlement/SZSettle.h"
#include "simutgw/settlement/SHSettle.h"

namespace Stock
{
	// stock_asset���е�һ����¼
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
������
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
	����
	@param const std::vector<std::string>& in_vctSettleGroup : ����ر���
	@param std::string& out_strDay : ��ǰ�����ַ���
	@param std::string& out_strSettlementFilePath : �����ļ�����·��

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int MakeSettlement(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::vector<std::string>& in_vctSettleGroup,
		std::string& out_strDay, std::string& out_strSettlementFilePath);

protected:
	/*
	��ȡ���е�����ر���

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int GetSettleGroupNames(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
		std::vector<string>& out_vctSettGroupNames);

	/*
	stock_asset��stock_etf_asset�����㣬�����ùɷݸ���Ϊ���йɷ�

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Stock_Settle();

	/*
	stock_asset�����㣬�����ùɷݸ���Ϊ���йɷ�

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Stock_Asset_Settle();

	/*
	stock_etf_asset�����㣬�����ùɷݸ���Ϊ���йɷ�

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Stock_ETF_Asset_Settle();

	/*
	stock_asset�����㣬�����ùɷݸ���Ϊ���йɷ�
	��ѯ�����еĹɷݼ�¼

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int GetStock(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
		const std::string& in_strQuery,
		std::vector<struct Stock::StockRecord>& io_vecStockRecord);

	/*
	stock_asset�����㣬�����ùɷݸ���Ϊ���йɷ�
	����һ���ɷݼ�¼

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Update_Stock_Asset(std::shared_ptr<MySqlCnnC602>& mysqlConn, const struct Stock::StockRecord& in_StockRecord);

	/*
	stock_etf_asset�����㣬�����ùɷݸ���Ϊ���йɷ�
	����һ���ɷݼ�¼

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Update_Stock_ETF_Asset(std::shared_ptr<MySqlCnnC602>& mysqlConn, const struct Stock::StockRecord& in_StockRecord);
};

#endif