#ifndef __TASK_VALID_ORDER_H__
#define __TASK_VALID_ORDER_H__

#include <memory>

#include "simutgw_flowwork/FlowWorkBase.h"

#include "simutgw/stgw_config/g_values_inner.h"

#include "thread_pool_priority/TaskPriorityBase.h"

/*
下单数据检验task
*/
class Task_ValidOrder : public TaskPriorityBase
{
	//
	// member
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	std::shared_ptr<struct simutgw::OrderMessage> m_orderMsg;

	//
	// Functions
	//
public:
	explicit Task_ValidOrder(const uint64_t uiId) : TaskPriorityBase(uiId, QueueTask::Level_Check_Order),
		m_scl(keywords::channel = "Task_ValidOrder")
	{
	}

	virtual ~Task_ValidOrder()
	{
	}

	void SetOrderMsg(std::shared_ptr<struct simutgw::OrderMessage>& in_msg)
	{
		m_orderMsg = in_msg;
		m_iKey = TaskPriorityBase::GenerateKey( in_msg->strStockID, in_msg->strSide, false );
	}

	/*
	处理回调
	*/
	virtual int TaskProc(void)
	{
		int iRes = ValidateOrder();

		return iRes;
	}

private:
	/*
	交易前检查函数
	Return:
	0 -- 合法
	-1 -- 不合法
	*/
	int ValidateOrder();
};

#endif
