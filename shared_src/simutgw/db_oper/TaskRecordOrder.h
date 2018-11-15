#ifndef __TASK_RECORD_ORDER_H__
#define __TASK_RECORD_ORDER_H__

#include "util/EzLog.h"
#include "thread_pool_base/TaskBase.h"

#include "order/define_order_msg.h"

#include "tool_mysql/MySqlCnnC602.h"

/*
	记录新订单类
	*/
class TaskRecordOrder : public TaskBase
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
	explicit TaskRecordOrder(const uint64_t uiId);
	virtual ~TaskRecordOrder();

	void SetOrder(std::shared_ptr<struct simutgw::OrderMessage> &in_orderMsg)
	{
		m_orderMsg = std::shared_ptr<struct simutgw::OrderMessage>(new struct simutgw::OrderMessage(*in_orderMsg));
	}

	virtual int TaskProc(void);

private:
	//将下单消息写入到数据库，包括流水表、买表和卖表
	int RecordOrderInDb();

	/*
	将下单消息插入到表中
	Param :
	Return :
	0 -- 成功
	1 -- 重单
	*/
	int RecordNewOrder(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);

	/*
	将下单消息插入到表中
	Param :
	iType :
	0 -- order_record

	Return :
	*/
	int InsertDb_neworder_buy_sell(int iType, std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);

	/*
	将撤单消息插入到表中
	Param :
	Return :
	0 -- 成功
	1 -- 重单
	*/
	int InsertDbCancelOrder(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);
};

#endif