#ifndef __TASK_CANCEL_ORDER_H__
#define __TASK_CANCEL_ORDER_H__

#include <memory>

#include "simutgw_config/config_define.h"
#include "order/define_order_msg.h"

#include "simutgw/biz/MatchUtil.h"
#include "thread_pool_priority/TaskPriorityBase.h"

/*
����Etf����task
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
	����ص�
	*/
	virtual int TaskProc(void)
	{
		int iRes = ProcSingleCancelOrder();
		return iRes;
	}

protected:
	/*
	����ʧ�ܣ���ԭʼ���������뵽��������
	*/
	int ProcCancelFail_OrigNotFound();

	/*
	����ʧ�ܣ�ԭʼ�����Ѿ�������
	*/
	int ProcCancelFail_OrigIsProc();

	/*
	���ڴ�������ҳ�ԭʼ����
	return:
	0 -- ���ҵ�
	-1 -- δ�ҵ�
	*/
	int GetOrigOrderInMemory();

	/*
	�����ɹ�
	*/
	int ProcCancelSuccess(std::shared_ptr<TaskPriorityBase>& origOrderTask);

	/*
	�����ʳ������г�������
	*/
	int ProcSingleCancelOrder();

	/*
	�����ʳ���������������û�г�������
	*/
	int ProcSingleCancelOrder_Without_Request();
};

#endif