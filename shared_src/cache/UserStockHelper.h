#ifndef __USER_STOCK_HELPER_H__
#define __USER_STOCK_HELPER_H__

#include <vector>
#include <memory>

#include "etf/conf_etf_info.h"
#include "order/define_order_msg.h"
#include "order/StockHelper.h"

// 
struct UserStockInfo
{
	std::string strAccount;
	std::string strStockID;
	// `stock_balance` '证券持有数量，股份余额',
	uint64_t ui64stkBalance;

	// `stock_auction_purchase_balance` '竞价买入量',
	uint64_t ui64stk_act_pch_Balance;

	// `stock_staple_purchase_balance` '大宗买入量',
	uint64_t ui64stk_stp_pch_Balance;

	// `stock_etf_redemption_balance` '普通股票etf赎回量，可竞价卖出',
	uint64_t ui64stk_etf_rdp_Balance;

	// `stock_creation_balance` 'etf申购量，可竞价卖出',
	uint64_t ui64stk_crt_Balance;

	// `stock_available`'持有可用余额，若是普通股票，可用于申购etf份额和可竞价卖出；若是etf，可用etf份额，可赎回和可竞价卖出',
	uint64_t ui64stk_avl_Balance;

	// `stock_last_balance` '上次余额',
	uint64_t ui64stk_last_Balance;
};

/*
	用户股份信息Helper类

	提供的外部函数：
	冻结用户（账户）的股份、etf
	解冻用户（账户）的股份、etf
	确认用户（账户）的股份、etf
	增加用户（账户）的股份、etf
	清空所有缓存的股份信息

	内部函数：
	查找需要进行操作的用户股份信息
	从数据库加载用户股份信息
	判断普通股票、etf
	*/
class UserStockHelper
{
	//
	// memebr
	//

	//
	// function
	//
public:
	UserStockHelper();
	virtual ~UserStockHelper();

	/*
	日终时清空缓存
	*/
	static void DayEndCleanUp(void);

	/*
	交易后更新用户股份
	*/
	static int UpdateAfterTrade(std::shared_ptr<struct simutgw::OrderMessage>& io_order);

	/*
	撤单成功后更新用户股份
	*/
	static int UpdateAfterCancel(std::shared_ptr<struct simutgw::OrderMessage>& io_order);

	//------------------------------股份卖出冻结股份--------------------------------//
	/*
	冻结普通股票股份

	Param:
	out_ui64etf -- 源于etf赎回的股份数量
	out_ui64avl -- 源于可用股份数量

	Return:
	0	-- 冻结成功
	-1	-- 冻结失败
	*/
	static int SellFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl);

	/*
	解除冻结普通股票股份

	Param:
	in_ui64etf -- 源于etf赎回的股份数量
	in_ui64avl -- 源于可用股份数量

	Return:
	0	-- 解除冻结成功
	-1	-- 解除冻结失败
	*/
	static int SellDeFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
	确认普通股份交易额，将从冻结的部分扣除相应额度
	Param:
	in_ui64etf -- 源于etf赎回的股份数量
	in_ui64avl -- 源于可用股份数量

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	static int SellConfirm(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64etf, uint64_t in_ui64avl);

	//------------------------------etf卖出冻结etf份额--------------------------------//
	/*
	冻结etf股份
	Param:
	out_ui64etf -- 源于申购的份额
	out_ui64avl -- 源于可用etf份额

	Return:
	0	-- 冻结成功
	-1	-- 冻结失败
	*/
	static int SellEtfFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl);

	/*
	解除冻结etf股份
	Param:
	in_ui64etf -- 源于申购的份额
	in_ui64avl -- 源于可用etf份额

	Return:
	0	-- 解除冻结成功
	-1	-- 解除冻结失败
	*/
	static int SellEtfDeFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
	确认etf交易额，将从冻结的部分扣除相应额度
	Param:
	in_ui64etf -- 源于申购的份额
	in_ui64avl -- 源于可用etf份额

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	static int SellEtfConfirm(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64etf, uint64_t in_ui64avl);

	//------------------------------etf申购冻结成分股--------------------------------//
	/*
	申购etf的所有成分股股份是否足够
	Param:
	ui64EtfQty -- ETF数量
	out_ui64CashCompnents -- 现金替代总金额

	Return:
	0	-- 足够
	-1	-- 不足
	*/
	static int CreationQuery(const std::string& strAccount, const uint64_t ui64EtfQty,
		const std::shared_ptr<struct simutgw::SzETF>& ptrEtf,
		std::vector<std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent>> &io_vecFrozeCompnent);

	/*
	冻结申购etf的成分股股份

	Return:
	0	-- 冻结成功
	-1	-- 冻结失败
	*/
	static int CreationFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl);

	/*
	解除冻结申购etf的成分股股份

	Param:
	in_ui64act -- 源于竞价买入股份数量
	in_ui64avl -- 源于可用股份数量

	Return:
	0	-- 解除冻结成功
	-1	-- 解除冻结失败
	*/
	static int CreationDeFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64act, uint64_t in_ui64avl);

	/*
	确认申购etf的成分股交易额，将从冻结的部分扣除相应额度
	Param:
	in_ui64act -- 源于竞价买入股份数量
	in_ui64avl -- 源于可用股份数量

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	static int CreationConfirm(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64act, uint64_t in_ui64avl);

	//------------------------------etf赎回冻结etf份额--------------------------------//
	/*
	冻结赎回etf的份额

	Return:
	0	-- 冻结成功
	-1	-- 冻结失败
	*/
	static int RedemptionFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl);

	/*
	解除冻结赎回etf的份额

	Param:
	in_ui64act -- 源于竞价买入股份数量
	in_ui64avl -- 源于可用股份数量

	Return:
	0	-- 解除冻结成功
	-1	-- 解除冻结失败
	*/
	static int RedemptionDeFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64act, uint64_t in_ui64avl);

	/*
	确认赎回etf的份额，将从冻结的部分扣除相应额度
	Param:
	in_ui64act -- 源于竞价买入股份数量
	in_ui64avl -- 源于可用股份数量

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	static int RedemptionConfirm(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64act, uint64_t in_ui64avl);

	//------------------------------增加股份--------------------------------//
	/*
	竞价买入增加（普通股份、etf份额）

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int AddAct(const std::string& strAccount, const std::string& strStockID, uint64_t ui64StpNum);

	/*
	申购买入增加(etf份额)

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int AddCrt(const std::string& strAccount, const std::string& strStockID, uint64_t ui64CrtNum);

	/*
	赎回etf份额时增加(成分股股份)

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int AddRdp(const std::string& strAccount, const std::string& strStockID, uint64_t ui64RdpNum);

private:
	/*
		查找用户股份信息
		若内存中有，返回成功
		若内存中无，则查询数据库，有加载进内存并返回成功，没有返回失败

		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int LookUp(const std::string& strAccount, const std::string& strStockID, enum StockHelper::StockType& out_stkType);


	/*
		从数据库加载股份信息

		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int LoadUserStockFromDBAndStore(const std::string& strAccount, const std::string& strStockID);

	/*
		交易后更新用户股份--普通交易(包括etf买卖)
		*/
	static int UpdateTrade(std::shared_ptr<struct simutgw::OrderMessage>& io_order);

	/*
	交易后更新用户股份--ETF申赎
	*/
	static int UpdateETFCrtRdp(std::shared_ptr<struct simutgw::OrderMessage>& io_order);

	/*
	撤单后更新用户股份--普通交易(包括etf买卖)
	*/
	static int UpdateCancel(std::shared_ptr<struct simutgw::OrderMessage>& io_order);
};

#endif