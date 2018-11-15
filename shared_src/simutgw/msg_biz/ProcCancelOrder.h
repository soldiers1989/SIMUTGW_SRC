#ifndef __PROC_Cancel_ORDER_H__
#define __PROC_Cancel_ORDER_H__

#include "simutgw_config/config_define.h"
#include "simutgw/stgw_config/g_values_biz.h"

#include "order/define_order_msg.h"

/*
*处理撤单消息类
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
		处理单笔撤单，主动撤单，没有撤单请求
	*/
	static int ProcSingleCancelOrder_Without_Request( 
		std::shared_ptr<struct simutgw::OrderMessage>& in_cancelPtr );
};

#endif
