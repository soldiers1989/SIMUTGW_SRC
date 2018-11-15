#ifndef __STGW_ORDER_HELPER_H__
#define __STGW_ORDER_HELPER_H__

#include "order/define_order_msg.h"

// order消息helper类
class SZ_STEP_OrderHelper
{
	// 
	// member
	//

	//
	// function
	//
public:
	SZ_STEP_OrderHelper();
	virtual ~SZ_STEP_OrderHelper();

	// 新订单转结构体
	static int NewOrder2Struct(const FIX::Message& message,
		std::shared_ptr<struct simutgw::OrderMessage>& ptrorder);

private:
	// 取nopartyids重复组的数据，包括账号、席位和营业部代码
	static int GetNopartyIds(const FIX::FieldMap& message,
		std::shared_ptr<struct simutgw::OrderMessage>& ptrorder);
};

#endif