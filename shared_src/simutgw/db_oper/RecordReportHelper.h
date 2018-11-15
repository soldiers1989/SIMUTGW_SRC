#ifndef __RECORD_HELPER_HELPER_H__
#define __RECORD_HELPER_HELPER_H__

#include <memory>

#include "order/define_order_msg.h"

/*
	�������ί��д��report��
*/
class RecordReportHelper
{
	//
	// function
	//
public:
	RecordReportHelper(void);
	virtual ~RecordReportHelper(void);

	// ��¼������ˮ
	static int RecordReportToDb( std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg );
};

#endif