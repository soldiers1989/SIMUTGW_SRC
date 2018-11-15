#ifndef __ETF_TRADE_VOLUME_H__
#define __ETF_TRADE_VOLUME_H__

#include <string>
#include <vector>
#include <memory>

#include "boost/thread/mutex.hpp"

#include "conf_etf_info.h"
#include "ETF_TradeVolume_Basket.h"

/*
	etf交易时累计申购、赎回总额的限制统计缓存类
	*/
class ETF_TradeVolume
{
	//
	// Members
	//
protected:
	std::map<std::string, ETF_TradeVolume_Basket> m_etfsTv;

	//
	// Functions
	//
public:
	ETF_TradeVolume( void )
	{ }

	virtual ~ETF_TradeVolume()
	{ }

	/*
	冻结申购交易额

	Return:
	0 -- 未超过最大限制额度，冻结成功
	-1 -- 冻结失败
	*/
	int FrozeCreation( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

	/*
	解除冻结申购交易额

	Return:
	0 -- 解除冻结成功
	-1 -- 冻结失败
	*/
	int DefrozeCreation( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId );

	/*
	确认申购交易额

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	int ConfirmCreation( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId );

	/*
	冻结赎回交易额

	Return:
	0 -- 未超过最大限制额度，冻结成功
	-1 -- 冻结失败
	*/
	int FrozeRedemption( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

	/*
	解除冻结赎回交易额

	Return:
	0 -- 解除冻结成功
	-1 -- 解除冻结失败
	*/
	int DefrozeRedemption( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId );

	/*
	确认赎回交易额

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	int ConfirmRedemption( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId );

};

#endif