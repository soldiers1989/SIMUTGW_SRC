#ifndef __RECORD_NEW_ORDER_HELPER_H__
#define __RECORD_NEW_ORDER_HELPER_H__

#include <memory>

#include "order/define_order_msg.h"

/*
	���յ��Ķ���ί�У�д��mysql���ݿ��redis��
	*/
class RecordNewOrderHelper
{
	//
	// function
	//
public:
	RecordNewOrderHelper(void);
	virtual ~RecordNewOrderHelper(void);

	// ���µ���Ϣд�뵽���ݿ⣬������ˮ������������������ɽ�����
	static int RecordInOrderToDb_Match( std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg );
};

#endif