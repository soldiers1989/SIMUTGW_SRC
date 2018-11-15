#ifndef __ETF_TRADE_VOLUME_BASKET_H__
#define __ETF_TRADE_VOLUME_BASKET_H__

#include <string>
#include <vector>

#include "boost/thread/mutex.hpp"

#include "cache/FrozeVolume.h"

/*
	etf交易时单只ETF累计申购、赎回总额的限制统计缓存类
	*/
class ETF_TradeVolume_Basket
{
	//
	// Members
	//
protected:

	// access mutex
	boost::mutex m_mutex;

	//累计申购总额限制 CreationLimit N18(2) 当天累计可申购的基金份额上限，为 0 表示没有限制，目前只能为整数
	uint64_t m_ui64CreationLimit;
	FrozeVolume m_CreationLimit;

	//累计赎回总额限制 RedemptionLimit N18(2) 当天累计可赎回的基金份额上限，为 0 表示没有限制， 目前只能为整数
	uint64_t m_ui64RedemptionLimit;
	FrozeVolume m_RedemptionLimit;

	//单个账户累计申购总额限制 CreationLimitPerUser N18(2) 单个证券账户当天累计可申购的基金份额上限，
	//为 0 表示没有限制，目前只能为整数单个账户累计赎回总额限制
	uint64_t m_ui64CreationLimitPerUser;
	std::map<std::string, FrozeVolume> m_CreationLimitPerUser;

	//单个账户累计赎回总额限制 RedemptionLimitPerUser N18(2) 单个证券账户当天累计可赎回的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	uint64_t m_ui64RedemptionLimitPerUser;
	std::map<std::string, FrozeVolume> m_RedemptionLimitPerUser;

	//净申购总额限制 NetCreationLimit N18(2) 当天净申购的基金份额上限，为 0表示没有限制，目前只能为整数
	uint64_t m_ui64NetCreationLimit;
	FrozeVolume m_NetCreationLimit;

	//净赎回总额限制 NetRedemptionLimit N18(2) 当天净赎回的基金份额上限，为 0表示没有限制，目前只能为整数
	uint64_t m_ui64NetRedemptionLimit;
	FrozeVolume m_NetRedemptionLimit;

	//单个账户净申购总额限制 NetCreationLimitPerUser N18(2) 单个证券账户当天净申购的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	uint64_t m_ui64NetCreationLimitPerUser;
	std::map<std::string, FrozeVolume> m_NetCreationLimitPerUser;

	//单个账户净赎回总额限制 NetRedemptionLimitPerUser N18(2) 单个证券账户当天净赎回的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	uint64_t m_ui64NetRedemptionLimitPerUser;
	std::map<std::string, FrozeVolume> m_NetRedemptionLimitPerUser;

	//
	// Functions
	//
public:
	ETF_TradeVolume_Basket(void) :
		m_ui64CreationLimit(0), m_CreationLimit(0),
		m_ui64RedemptionLimit(0), m_RedemptionLimit(0),
		m_ui64CreationLimitPerUser(0),
		m_ui64RedemptionLimitPerUser(0),
		m_ui64NetCreationLimit(0), m_NetCreationLimit(0),
		m_ui64NetRedemptionLimit(0), m_NetRedemptionLimit(0),
		m_ui64NetCreationLimitPerUser(0),
		m_ui64NetRedemptionLimitPerUser(0)
	{
	}

	ETF_TradeVolume_Basket(const uint64_t in_CreationLimit, const uint64_t in_RedemptionLimit,
		const uint64_t in_CreationLimitPerUser, const uint64_t in_RedemptionLimitPerUser,
		const uint64_t in_NetCreationLimit, const uint64_t in_NetRedemptionLimit,
		const uint64_t in_NetCreationLimitPerUser, const uint64_t  in_NetRedemptionLimitPerUser)
		:m_ui64CreationLimit(in_CreationLimit), m_CreationLimit(in_CreationLimit),
		m_ui64RedemptionLimit(in_RedemptionLimit), m_RedemptionLimit(in_RedemptionLimit),
		m_ui64CreationLimitPerUser(in_CreationLimitPerUser),
		m_ui64RedemptionLimitPerUser(in_RedemptionLimitPerUser),
		m_ui64NetCreationLimit(in_NetCreationLimit), m_NetCreationLimit(in_NetCreationLimit),
		m_ui64NetRedemptionLimit(in_NetRedemptionLimit), m_NetRedemptionLimit(in_NetRedemptionLimit),
		m_ui64NetCreationLimitPerUser(in_NetCreationLimitPerUser),
		m_ui64NetRedemptionLimitPerUser(in_NetRedemptionLimitPerUser)
	{
	}

	ETF_TradeVolume_Basket(const ETF_TradeVolume_Basket & src)
		:m_ui64CreationLimit(src.m_ui64CreationLimit), m_CreationLimit(src.m_CreationLimit),
		m_ui64RedemptionLimit(src.m_ui64RedemptionLimit), m_RedemptionLimit(src.m_RedemptionLimit),
		m_ui64CreationLimitPerUser(src.m_ui64CreationLimitPerUser),
		m_ui64RedemptionLimitPerUser(src.m_ui64RedemptionLimitPerUser),
		m_ui64NetCreationLimit(src.m_ui64NetCreationLimit), m_NetCreationLimit(src.m_NetCreationLimit),
		m_ui64NetRedemptionLimit(src.m_ui64NetRedemptionLimit),
		m_NetRedemptionLimit(src.m_NetRedemptionLimit),
		m_ui64NetCreationLimitPerUser(src.m_ui64NetCreationLimitPerUser),
		m_ui64NetRedemptionLimitPerUser(src.m_ui64NetRedemptionLimitPerUser)
	{
	}
	
	~ETF_TradeVolume_Basket()
	{
	}

	/*
	冻结申购交易额

	Return:
	0 -- 未超过最大限制额度，冻结成功
	-1 -- 冻结失败
	*/
	int FrozeCreation(const uint64_t in_num, const std::string& in_sCustId);

	/*
	解除冻结申购交易额

	Return:
	0 -- 解除冻结成功
	-1 -- 解除冻结失败
	*/
	int DefrozeCreation(const uint64_t in_num, const std::string& in_sCustId);

	/*
	确认申购交易额

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	int ConfirmCreation(const uint64_t in_num, const std::string& in_sCustId);

	/*
	冻结赎回交易额

	Return:
	0 -- 未超过最大限制额度，冻结成功
	-1 -- 冻结失败
	*/
	int FrozeRedemption(const uint64_t in_num, const std::string& in_sCustId);

	/*
	解除冻结赎回交易额

	Return:
	0 -- 解除冻结成功
	-1 -- 冻结失败
	*/
	int DefrozeRedemption(const uint64_t in_num, const std::string& in_sCustId);

	/*
	确认赎回交易额

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	int ConfirmRedemption(const uint64_t in_num, const std::string& in_sCustId);

private:
	// 禁止使用
	ETF_TradeVolume_Basket& operator =(const ETF_TradeVolume_Basket& src);
};

#endif
