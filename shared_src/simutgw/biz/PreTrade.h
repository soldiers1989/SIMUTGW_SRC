#ifndef __PRE_TRADE_H__
#define __PRE_TRADE_H__

#include "order/define_order_msg.h"

#include "etf/ETFHelper.h"

/*
	交易前相关处理类

	检查股份是否足额卖出
	检查ETF申购是否超限，成分股是否足够
	进行股份冻结，冻结失败当做废单处理，冻结成功则进入交易队列
*/
class PreTrade
{
public:	
	virtual ~PreTrade();

	/*
		交易准备
		主要是冻结股份

		Return:
		0 -- 成功
		-1 -- 失败
	*/
	static int TradePrep( std::shared_ptr<struct simutgw::OrderMessage>& io_order );

private:
	/*
		取当前的订单类型
	*/
	static int GetTradeType( std::shared_ptr<struct simutgw::OrderMessage>& in_order, enum simutgw::TADE_TYPE& io_type );

	/*
		根据行情判断委托的合法性
		Return:
		0 -- 合法
		-1 -- 不合法
	*/
	static int CheckOrderByQuotation( std::shared_ptr<struct simutgw::OrderMessage>& io_order );

	/*
	查看是否停牌
	Return:
	0 -- 未停牌
	1 -- 已停牌
	*/
	static int CheckTPBZ( std::shared_ptr<struct simutgw::OrderMessage>& io_order, const string& in_strTpbz );

	/*
	查看是否超出涨跌幅
	Return:
	0 -- 未超
	1 -- 已超
	*/
	static int Check_MaxGain_And_MinFall( std::shared_ptr<struct simutgw::OrderMessage>& io_order,
		const simutgw::uint64_t_Money in_ui64mMaxGain, const simutgw::uint64_t_Money in_ui64mMinFall);

	/*
		ETF检查

		Return:
		0 -- 成功
		-1 -- 失败
	*/
	static int ValidateETF( std::shared_ptr<struct simutgw::OrderMessage>& in_order );

	/*
	ETF申购增加成分股交易信息
	*/
	static int AddCreationComponent( std::shared_ptr<struct simutgw::OrderMessage>& io_order,
		const std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
		ETF赎回增加成分股交易信息
	*/
	static int AddRedeptionComponent( std::shared_ptr<struct simutgw::OrderMessage>& io_order,
		const std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

private:
	// 禁止使用默认构造函数
	PreTrade();
};

#endif