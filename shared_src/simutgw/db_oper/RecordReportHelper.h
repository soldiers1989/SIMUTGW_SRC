#ifndef __RECORD_HELPER_HELPER_H__
#define __RECORD_HELPER_HELPER_H__

#include <memory>

#include "order/define_order_msg.h"

/*
	将处理的委托写到report表
*/
class RecordReportHelper
{
	//
	// function
	//
public:
	RecordReportHelper(void);
	virtual ~RecordReportHelper(void);

	// 记录处理流水
	static int RecordReportToDb( std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg );
};

#endif