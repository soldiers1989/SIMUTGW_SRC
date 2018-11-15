#ifndef __STGW_ORDER_HELPER_H__
#define __STGW_ORDER_HELPER_H__

#include "order/define_order_msg.h"

// order��Ϣhelper��
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

	// �¶���ת�ṹ��
	static int NewOrder2Struct(const FIX::Message& message,
		std::shared_ptr<struct simutgw::OrderMessage>& ptrorder);

private:
	// ȡnopartyids�ظ�������ݣ������˺š�ϯλ��Ӫҵ������
	static int GetNopartyIds(const FIX::FieldMap& message,
		std::shared_ptr<struct simutgw::OrderMessage>& ptrorder);
};

#endif