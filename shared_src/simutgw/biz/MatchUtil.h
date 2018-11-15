#ifndef __MATCH_UTIL_H__
#define __MATCH_UTIL_H__

#include <memory>

#include "order/define_order_msg.h"

/*
撮合交易通用方法类
*/
class MatchUtil
{
	//
	// Members
	//
protected:

	//
	// Functions
	//
public:	
	virtual ~MatchUtil( void );

	/*
	查看是否停牌
	Return:
	0 -- 未停牌
	1 -- 已停牌
	*/
	static int CheckTPBZ(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg, const string& in_strTpbz);

	/*
	查看是否超出涨跌幅
	Return:
	0 -- 未超
	1 -- 已超
	*/
	static int Check_MaxGain_And_MinFall(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const simutgw::uint64_t_Money in_ui64mMaxGain,const simutgw::uint64_t_Money in_ui64mMinFall);

	/*
	判断成交类型
	Param:
	bLimit -- true 是限价
	-- false 市价
	Return:
	0 -- 可成交，部分或者全部
	-1 -- 不成交，错误或者挂单
	*/
	static int Check_Match_Method(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl,const simutgw::uint64_t_Money in_ui64mCjje, bool bLimit = true);

	/*
	判断成交类型，补差资金和股份
	Param:
	bLimit -- true 是限价
	-- false 市价
	Return:
	0 -- 可成交，部分或者全部
	-1 -- 不成交，错误或者挂单
	*/
	static int Check_Match_Method_WithoutAccount(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl,const simutgw::uint64_t_Money in_ui64mCjje, bool bLimit = true);

	/*
	 取一笔委托的行情交易圈
	*/
	static void Get_Order_CircleID(const std::shared_ptr<struct simutgw::OrderMessage>& orderMsg, 
		string& out_strCircleID);

private:
	// 禁止使用默认构造函数
	MatchUtil( void );
};

#endif
