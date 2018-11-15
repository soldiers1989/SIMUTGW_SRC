#ifndef __USER_STOCK_VOLUME_H__
#define __USER_STOCK_VOLUME_H__

#include "cache/FrozeVolume.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"

/*
	用户股份容量
	*/
class UserStockVolume
{
	//
	// member
	//
private:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// 证券账号
	std::string m_strUserAccount;
	// 证券代码
	std::string m_strSecurityID;

	// DB中是否存在
	bool m_bStored;

	// `stock_balance` '证券持有数量，股份余额',
	FrozeVolume m_stkBalance;

	// `stock_auction_purchase_balance` '竞价买入量',
	FrozeVolume m_stk_act_pch_Balance;

	// `stock_staple_purchase_balance` '大宗买入量',
	FrozeVolume m_stk_stp_pch_Balance;

	// `stock_etf_redemption_balance` '普通股票etf赎回量，可竞价卖出',
	FrozeVolume m_stk_etf_rdp_Balance;

	// `stock_creation_balance` 'etf申购量，可竞价卖出',
	FrozeVolume m_stk_crt_Balance;

	// `stock_available`'持有可用余额，若是普通股票，可用于申购etf份额和可竞价卖出；若是etf，可用etf份额，可赎回和可竞价卖出',
	FrozeVolume m_stk_avl_Balance;

	// `stock_last_balance` '上次余额',
	FrozeVolume m_stk_last_Balance;

	// 最近一次访问时间
	unsigned long m_ullast_access_time;

	// 到期时间间隔，默认十分钟600s
	unsigned long m_ultime_interval;

	// 是否到期
	bool m_bExpire;

	//
	// function
	//
public:
	UserStockVolume();

	UserStockVolume(bool bStored, uint64_t stkBalance, uint64_t stk_act_pch_Balance, uint64_t stk_stp_pch_Balance,
		uint64_t stk_etf_rdp_Balance, uint64_t stk_crt_Balance, uint64_t stk_avl_Balance,
		uint64_t stk_last_Balance, unsigned long ulinterval = 600);

	virtual ~UserStockVolume();

	/* 查看是否在mysql中有记录 */
	bool CheckStore()
	{
		return m_bStored;
	}

	/* 查看是否在mysql中有记录 */
	void SetStore(bool b = true)
	{
		m_bStored = b;
	}

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
	int SellFroze(uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl);

	/*
		解除冻结普通股票股份

		Param:
		in_ui64etf -- 源于etf赎回的股份数量
		in_ui64avl -- 源于可用股份数量

		Return:
		0	-- 解除冻结成功
		-1	-- 解除冻结失败
		*/
	int SellDeFroze(uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
	确认普通股份交易额，将从冻结的部分扣除相应额度
	Param:
	in_ui64etf -- 源于etf赎回的股份数量
	in_ui64avl -- 源于可用股份数量

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	int SellConfirm(uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
		与数据库信息同步，更新最近同步计数和未冻结计数
		*/
	int SellDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_etf_origin, uint64_t& out_etf_update,
		uint64_t &out_avl_origin, uint64_t& out_avl_update, uint64_t& out_last_origin);

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
	int SellEtfFroze(uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl);

	/*
	解除冻结etf股份
	Param:
	in_ui64etf -- 源于申购的份额
	in_ui64avl -- 源于可用etf份额

	Return:
	0	-- 解除冻结成功
	-1	-- 解除冻结失败
	*/
	int SellEtfDeFroze(uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
	确认etf交易额，将从冻结的部分扣除相应额度
	Param:
	in_ui64etf -- 源于申购的份额
	in_ui64avl -- 源于可用etf份额

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	int SellEtfConfirm(uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
	与数据库信息同步，更新最近同步计数和未冻结计数
	*/
	int SellEtfDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_etf_origin, uint64_t& out_etf_update,
		uint64_t &out_avl_origin, uint64_t& out_avl_update, uint64_t& out_last_origin);

	//------------------------------etf申购冻结成分股--------------------------------//
	/*
		申购成分股的最大可用股份查询

		Return:
		0 -- 足够
		-1 -- 不够
		*/
	int CreationQuery(uint64_t ui64Query, uint64_t &out_ui64max);

	/*
	冻结申购etf的成分股股份

	Return:
	0	-- 冻结成功
	-1	-- 冻结失败
	*/
	int CreationFroze(uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl);

	/*
	解除冻结申购etf的成分股股份

	Param:
	in_ui64act -- 源于竞价买入股份数量
	in_ui64avl -- 源于可用股份数量

	Return:
	0	-- 解除冻结成功
	-1	-- 解除冻结失败
	*/
	int CreationDeFroze(uint64_t in_ui64act, uint64_t in_ui64avl);

	/*
	与数据库信息同步，更新最近同步计数和未冻结计数
	申购成分股同步
	*/
	int CreationComponentDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_act_origin, uint64_t& out_act_update,
		uint64_t &out_avl_origin, uint64_t& out_avl_update, uint64_t& out_last_origin);

	/*
	与数据库信息同步，更新最近同步计数和未冻结计数
	申购ETF同步
	*/
	int CreationEtfDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_crt_origin, uint64_t& out_crt_update, uint64_t& out_last_origin);

	/*
	确认申购etf的成分股交易额，将从冻结的部分扣除相应额度
	Param:
	in_ui64act -- 源于竞价买入股份数量
	in_ui64avl -- 源于可用股份数量

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	int CreationConfirm(uint64_t in_ui64act, uint64_t in_ui64avl);

	//------------------------------etf赎回冻结etf份额--------------------------------//
	/*
	冻结赎回etf的份额
	Param:
	out_ui64act -- 源于买入的份额
	out_ui64avl -- 源于可用etf份额

	Return:
	0	-- 冻结成功
	-1	-- 冻结失败
	*/
	int RedemptionFroze(uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl);

	/*
	解除冻结赎回etf的份额

	Param:
	in_ui64act -- 源于竞价买入股份数量
	in_ui64avl -- 源于可用股份数量

	Return:
	0	-- 解除冻结成功
	-1	-- 解除冻结失败
	*/
	int RedemptionDeFroze(uint64_t in_ui64act, uint64_t in_ui64avl);

	/*
	确认赎回etf的份额，将从冻结的部分扣除相应额度
	Param:
	in_ui64act -- 源于竞价买入股份数量
	in_ui64avl -- 源于可用股份数量

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	int RedemptionConfirm(uint64_t in_ui64act, uint64_t in_ui64avl);

	//------------------------------增加股份--------------------------------//
	/*
		竞价买入增加（普通股份、etf份额）

		Return:
		0 -- 成功
		-1 -- 失败
		*/
	int AddAct(uint64_t ui64StpNum);

	/*
	申购买入增加(etf份额)

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int AddCrt(uint64_t ui64CrtNum);

	/*
	赎回etf份额时增加(成分股股份)

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int AddRdp(uint64_t ui64RdpNum);

	/*
	与数据库信息同步，更新最近同步计数和未冻结计数
	竞价买入增加
	*/
	int AddActSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_act_origin, uint64_t& out_act_update, uint64_t& out_last_origin);

	/*
	与数据库信息同步，更新最近同步计数和未冻结计数
	申购增加(etf份额)
	*/
	int AddEtfSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_etf_origin, uint64_t& out_etf_update, uint64_t& out_last_origin);

	/*
	与数据库信息同步，更新最近同步计数和未冻结计数
	赎回减少(etf份额)
	*/
	int RdpEtfSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_act_origin, uint64_t& out_act_update,
		uint64_t &out_avl_origin, uint64_t& out_avl_update,
		uint64_t& out_last_origin);

	/*
	与数据库信息同步，更新最近同步计数和未冻结计数
	赎回etf份额时增加(成分股股份)
	*/
	int AddRdpComponentSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_rdp_origin, uint64_t& out_rdp_update, uint64_t& out_last_origin);


private:
	/*
		查看是否过期
		Return:
		true -- 过期
		false -- 未过期
		*/
	bool IsExpired();

	/*
		冻结总额
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	int FrozeStkBalance(uint64_t ui64Froze);

	/*
	解冻总额
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int DeFrozeStkBalance(uint64_t ui64Froze);

	/*
		更新缓存信息，由于已经过期

		Return:
		0 -- 成功
		-1 -- 失败
		*/
	int UpdateUserStock();
};

#endif