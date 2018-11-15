#ifndef __RECORD_NEW_ORDER_HELPER_H__
#define __RECORD_NEW_ORDER_HELPER_H__

#include <memory>

#include "order/define_order_msg.h"

/*
	将收到的订单委托，写到mysql数据库和redis中
	*/
class RecordNewOrderHelper
{
	//
	// function
	//
public:
	RecordNewOrderHelper(void);
	virtual ~RecordNewOrderHelper(void);

	// 将下单消息写入到数据库，包括流水表、买表和卖表，并加入待成交任务
	static int RecordInOrderToDb_Match( std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg );
};

#endif