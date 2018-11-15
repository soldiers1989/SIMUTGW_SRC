#ifndef __PROC_BASE_ORDER_VALID_H__
#define __PROC_BASE_ORDER_VALID_H__

#include <memory>

#include "thread_pool_priority/TaskPriorityBase.h"

#include "order/define_order_msg.h"

/*
��������Ϣ��֤�Ļ���
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
	����ص�
	*/
	virtual int TaskProc( void )
	{
		int iRes = ValidateOrder();
		return iRes;
	}

protected:
	/*
	����ǰ��麯��
	Return:
	0 -- �Ϸ�
	-1 -- ���Ϸ�
	*/
	virtual int ValidateOrder() = 0;
};

#endif