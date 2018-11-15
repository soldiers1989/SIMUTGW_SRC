#ifndef __SZ_CONN_STEP_ORDER_H__
#define __SZ_CONN_STEP_ORDER_H__

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "config/conf_fix.h"

#include "order/define_order_msg.h"

/*
深圳step消息处理类
提供map写入数据库的方法
*/

class SzConn_StepOrder
{
	/*
	member
	*/
private:

	/*
	function
	*/

public:
	virtual ~SzConn_StepOrder(void);

	// 校验下单消息，记录入数据库
	static int Valide_Record_Order(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg);

private:
	// 禁止使用默认构造函数
	SzConn_StepOrder(void);
};

#endif