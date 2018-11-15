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
����step��Ϣ������
�ṩmapд�����ݿ�ķ���
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

	// У���µ���Ϣ����¼�����ݿ�
	static int Valide_Record_Order(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg);

private:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	SzConn_StepOrder(void);
};

#endif