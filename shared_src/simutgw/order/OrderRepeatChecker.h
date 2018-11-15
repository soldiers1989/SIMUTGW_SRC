#ifndef __ORDER_REPEAT_CHECKER_H__
#define __ORDER_REPEAT_CHECKER_H__

#include "OrderMemoryStoreFactory.h"

#include "simutgw_flowwork/FlowWorkBase.h"

/*
检查消息重单
*/
class OrderRepeatChecker
{
	//
	// Members
	//
private:

	//
	// Functions
	//
public:
	virtual ~OrderRepeatChecker( void );

	/*
	检查当前单是否有重单

	Return :
	ture -- 是重单
	false -- 不是重单
	*/
	static bool CheckIfRepeatOrder( std::shared_ptr<struct simutgw::OrderMessage>& in_order );

	/*
	向redis中记录当前单号
	Param :

	Return :
	0 -- 写入成功
	非0 -- 写入失败
	*/
	static int RecordOrderInRedis( std::shared_ptr<struct simutgw::OrderMessage>& orderMsg );

	/*
	清理redis中重单缓存

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	static int DayEndCleanUpInRedis(void);
protected:
	/*
	检查redis中当前单是否有重单

	Return :
	ture -- 是重单
	false -- 不是重单
	*/
	static bool CheckIfRepeatOrder_Redis( std::shared_ptr<struct simutgw::OrderMessage>& in_order );

	/*
	检查mysql中当前单是否有重单

	Return :
	ture -- 是重单
	false -- 不是重单
	*/
	static bool CheckIfRepeatOrder_MySql( std::shared_ptr<struct simutgw::OrderMessage>& in_order );

private:
	// 禁止使用默认构造函数
	OrderRepeatChecker( void );
};

#endif