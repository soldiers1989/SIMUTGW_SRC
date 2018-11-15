#ifndef __MARKET_INFO_HELPER_H__
#define __MARKET_INFO_HELPER_H__

#include <string>
#include <memory>

#include "config/conf_msg.h"
#include "order/define_order_msg.h"

class MarketInfoHelper
{
	//
	// Members
	//

	//
	// Functions
	//
public:
	/*
	获取当前储存的实时行情
	Return:
	0 -- 获取成功
	非0 -- 获取失败
	*/
	static int GetCurrentQuotationByStockId(const std::string& in_cstrZqdm,
		uint64_t& out_ui64Cjsl, uint64_t& out_ui64Cjje, std::string& out_strHqsj);

	/*
	获取当前储存的交易圈实时行情容量
	如果当前交易圈容量没有，则取总容量的，如果总容量也没有，则返回失败
	Return:
	0 -- 获取成功
	非0 -- 获取失败
	*/
	static int GetCurrQuotGapByCircle(const std::string& in_cstrZqdm,
		const std::string& in_strSide, const std::string& in_cstrTradeCircle,
		simutgw::QuotationType& in_quotType,
		simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
		uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje, std::string& out_strHqsj,
		std::string& out_strTpbz);

	/*
	储存当前储存的交易圈实时行情容量
	Return:
	0 -- 储存成功
	非0 -- 储存失败
	*/
	static int SetCurrQuotGapByCircle(const std::string& in_cstrZqdm, const std::string& in_cstrTradeCircle,
		const uint64_t& in_cui64Cjsl, const uint64_t& in_cui64Cjje, const std::string& in_cstrHqsj);

	/*
	储存当前储存的交易圈实时行情容量
	Return:
	0 -- 储存成功
	非0 -- 储存失败
	*/
	static int SetCurrQuotGapByCircle(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg,
		const std::string& in_cstrTradeCircle, const uint64_t& in_cui64Cjsl,
		const uint64_t& in_cui64Cjje, const std::string& in_cstrHqsj);

	/*
	获取当前储存的交易圈实时行情容量
	如果当前交易圈容量没有，则取总容量的，如果总容量也没有，则返回失败

	最近成交价

	Return:
	0 -- 获取成功
	非0 -- 获取失败
	*/
	static int GetCurrQuotGapByCircle_RecentPrice(const std::string& in_cstrZqdm, const std::string& in_cstrTradeCircle,
		simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
		uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje, std::string& out_strHqsj,
		std::string& out_strTpbz);

	/*
	查询当前的静态行情信息
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int GetCurrStaticQuot(const std::string& in_cstrZqdm, uint64_t& out_ui64Zrsp, std::string& out_strTPBZ);

protected:

	/*
	获取当前储存的交易圈实时行情容量
	如果当前交易圈容量没有，则取总容量的，如果总容量也没有，则返回失败
	取成交总量和成交金额
	Return:
	0 -- 获取成功
	非0 -- 获取失败
	*/
	static int GetCurrQuotGapByCircle(const std::string& in_cstrZqdm, const std::string& in_cstrTradeCircle,
		simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
		uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje, std::string& out_strHqsj,
		std::string& out_strTpbz);

	/*
	获取当前储存的交易圈实时行情容量
	如果当前交易圈容量没有，则取总容量的，如果总容量也没有，则返回失败

	区间段均价

	Return:
	0 -- 获取成功
	非0 -- 获取失败
	*/
	static int GetCurrQuotGapByCircle_AveragePrice(const std::string& in_cstrZqdm, const std::string& in_cstrTradeCircle,
		simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
		uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje,	std::string& out_strHqsj,
		std::string& out_strTpbz);

	/*
	获取当前储存的交易圈实时行情容量
	如果当前交易圈容量没有，则取总容量的，如果总容量也没有，则返回失败

	买一卖一

	Return:
	0 -- 获取成功
	非0 -- 获取失败
	*/
	static int GetCurrQuotGapByCircle_SellBuyPrice(const std::string& in_cstrZqdm, 
		const std::string& in_strSide, const std::string& in_cstrTradeCircle,
		simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
		uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje,	std::string& out_strHqsj,
		std::string& out_strTpbz);

private:
	// 防止被使用
	MarketInfoHelper(void);
	virtual ~MarketInfoHelper(void);
};

#endif