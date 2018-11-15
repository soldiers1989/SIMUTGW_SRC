#ifndef __PROC_BASE_ORDER_VALID_H__
#define __PROC_BASE_ORDER_VALID_H__

#include <memory>

#include "thread_pool_priority/TaskPriorityBase.h"

#include "order/define_order_msg.h"

/*
处理交易信息验证的基类
*/
class ProcBase_Order_Valid : public TaskPriorityBase
{
	//
	// member
	//
protected:
	std::shared_ptr<struct simutgw::OrderMessage> m_orderMsg;

	//
	// function
	//
public:
	ProcBase_Order_Valid(const uint64_t uiId) : TaskPriorityBase(uiId, QueueTask::Level_Check_Order)
	{
	}

	virtual ~ProcBase_Order_Valid()
	{
	}

	void SetOrderMsg( std::shared_ptr<struct simutgw::OrderMessage>& in_msg )
	{
		m_orderMsg = in_msg;
		
		m_iKey = TaskPriorityBase::GenerateKey( in_msg->strStockID, in_msg->strSide, false );
	}

	/*
	处理回调
	*/
	virtual int TaskProc( void )
	{
		int iRes = ValidateOrder();
		return iRes;
	}

protected:
	/*
	交易前检查函数
	Return:
	0 -- 合法
	-1 -- 不合法
	*/
	virtual int ValidateOrder() = 0;
};

#endif