#ifndef __PROC_BASE_ORDER_MATCH_H__
#define __PROC_BASE_ORDER_MATCH_H__

#include <memory>

#include "thread_pool_base/TaskBase.h"

#include "order/define_order_msg.h"
#include "util/WorkCounter.h"

/*
处理交易成交的基类
*/
class ProcBase_Order_Match : public TaskBase
{
	//
	// member
	//
protected:
	std::shared_ptr<struct simutgw::OrderMessage> m_orderMsg;

	std::shared_ptr<WorkCounter> m_WorkCounter;

	//
	// function
	//
public:
	ProcBase_Order_Match( const unsigned int uiId ) : TaskBase( uiId )
	{
	}

	virtual ~ProcBase_Order_Match()
	{
	}

	void SetOrderMsg( std::shared_ptr<struct simutgw::OrderMessage>& in_msg )
	{
		m_orderMsg = in_msg;
	}
	
	/*
	处理回调
	*/
	virtual int TaskProc( void )
	{
		int iRes = MatchOrder();
		return iRes;
	}

protected:
	/*
	交易成交函数
	Return:
	0 -- 成交
	-1 -- 失败
	*/
	virtual int MatchOrder() = 0;
};

#endif