#ifndef __TASK_CANCEL_ORDER_H__
#define __TASK_CANCEL_ORDER_H__

#include <memory>

#include "simutgw_config/config_define.h"
#include "order/define_order_msg.h"

#include "simutgw/biz/MatchUtil.h"
#include "thread_pool_priority/TaskPriorityBase.h"

/*
处理Etf交易task
*/
class Task_CancelOrder : public TaskPriorityBase
{
	//
	// member
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;
	
	std::shared_ptr<struct simutgw::OrderMessage> m_orderMsg;

	//
	// function
	//
public:
	explicit Task_CancelOrder( const uint64_t uiId ) :
		TaskPriorityBase(uiId, QueueTask::Level_Cancel_Order),
		m_scl(keywords::channel = "Task_CancelOrder")
	{
	}

	void SetOrderMsg(std::shared_ptr<struct simutgw::OrderMessage>& in_msg)
	{
		m_orderMsg = std::shared_ptr<struct simutgw::OrderMessage>(new struct simutgw::OrderMessage(*in_msg));

		m_iKey = TaskPriorityBase::GenerateKey( in_msg->strStockID, in_msg->strSide, true );

		m_strClordid = in_msg->strClordid;
	}

	/*
	处理回调
	*/
	virtual int TaskProc(void)
	{
		int iRes = ProcSingleCancelOrder();
		return iRes;
	}

protected:
	/*
	撤单失败，无原始订单，插入到撤单队列
	*/
	int ProcCancelFail_OrigNotFound();

	/*
	撤单失败，原始订单已经被处理
	*/
	int ProcCancelFail_OrigIsProc();

	/*
	在内存队列中找出原始订单
	return:
	0 -- 已找到
	-1 -- 未找到
	*/
	int GetOrigOrderInMemory();

	/*
	撤单成功
	*/
	int ProcCancelSuccess(std::shared_ptr<TaskPriorityBase>& origOrderTask);

	/*
	处理单笔撤单，有撤单请求
	*/
	int ProcSingleCancelOrder();

	/*
	处理单笔撤单，主动撤单，没有撤单请求
	*/
	int ProcSingleCancelOrder_Without_Request();
};

#endif