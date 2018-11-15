#ifndef __PROC_Cancel_ORDER_H__
#define __PROC_Cancel_ORDER_H__

#include "simutgw_config/config_define.h"
#include "simutgw/stgw_config/g_values_biz.h"

#include "order/define_order_msg.h"

/*
*��������Ϣ��
*/

class ProcCancelOrder
{
	/*
	member
	*/
private:

	/*
	function
	*/
public:
	ProcCancelOrder( void );
	virtual ~ProcCancelOrder( void );

	/*
		�����ʳ���������������û�г�������
	*/
	static int ProcSingleCancelOrder_Without_Request( 
		std::shared_ptr<struct simutgw::OrderMessage>& in_cancelPtr );
};

#endif
