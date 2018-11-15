#ifndef __TASK_SH_RULE_MATCH_H__
#define __TASK_SH_RULE_MATCH_H__

#include <memory>

#include "simutgw_config/config_define.h"
#include "order/define_order_msg.h"

#include "simutgw/biz/MatchUtil.h"
#include "thread_pool_priority/TaskPriorityBase.h"

/*
处理上海按成交规则task
*/
class Task_ShRule_Match : public TaskPriorityBase
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
	explicit Task_ShRule_Match(const uint64_t uiId) :
		TaskPriorityBase(uiId, QueueTask::Level_Match_Order),
		m_scl(keywords::channel = "Task_ShRule_Match")
	{
	}

	void SetOrderMsg(std::shared_ptr<struct simutgw::OrderMessage>& in_msg)
	{
		m_orderMsg = std::shared_ptr<struct simutgw::OrderMessage>(new struct simutgw::OrderMessage(*in_msg));

		m_iKey = TaskPriorityBase::GenerateKey( in_msg->strStockID, in_msg->strSide, false );

		m_strSide = in_msg->strSide;
		m_ui64Price = in_msg->ui64mOrderPrice;
		m_strClordid = in_msg->strClordid;
		m_tOrderTime = in_msg->tRcvTime;
	}

	/*
	处理回调
	*/
	virtual int TaskProc(void)
	{
		int iRes = MatchOrder();
		return iRes;
	}

	virtual ~Task_ShRule_Match(void)
	{
	}

protected:
	/*
	交易成交函数
	Return:
	0 -- 成交
	-1 -- 失败
	*/
	virtual int MatchOrder();
};

#endif