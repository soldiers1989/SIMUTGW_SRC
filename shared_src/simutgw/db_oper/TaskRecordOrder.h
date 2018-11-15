#ifndef __TASK_RECORD_ORDER_H__
#define __TASK_RECORD_ORDER_H__

#include "util/EzLog.h"
#include "thread_pool_base/TaskBase.h"

#include "order/define_order_msg.h"

#include "tool_mysql/MySqlCnnC602.h"

/*
	��¼�¶�����
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
	//���µ���Ϣд�뵽���ݿ⣬������ˮ����������
	int RecordOrderInDb();

	/*
	���µ���Ϣ���뵽����
	Param :
	Return :
	0 -- �ɹ�
	1 -- �ص�
	*/
	int RecordNewOrder(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);

	/*
	���µ���Ϣ���뵽����
	Param :
	iType :
	0 -- order_record

	Return :
	*/
	int InsertDb_neworder_buy_sell(int iType, std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);

	/*
	��������Ϣ���뵽����
	Param :
	Return :
	0 -- �ɹ�
	1 -- �ص�
	*/
	int InsertDbCancelOrder(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);
};

#endif