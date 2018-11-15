#ifndef __TASK_RECORD_REPORT_H__
#define __TASK_RECORD_REPORT_H__

#include "thread_pool_base/TaskBase.h"

#include "order/define_order_msg.h"

#include "util/EzLog.h"

#include "tool_mysql/MySqlCnnC602.h"

/*
	��¼��������
	*/
class TaskRecordReport : public TaskBase
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
	explicit TaskRecordReport(const uint64_t uiId);
	virtual ~TaskRecordReport();

	void SetOrder(std::shared_ptr<struct simutgw::OrderMessage> &in_orderMsg)
	{
		m_orderMsg = std::shared_ptr<struct simutgw::OrderMessage>(new struct simutgw::OrderMessage(*in_orderMsg));
	}

	virtual int TaskProc(void);

private:
	//�����׽��д�뵽���ݿ⣬������ˮ����������
	int RecordReportInDb();
	/*
	���µ���Ϣ���뵽����
	Param :

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int InsertDb_Report(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);
};

#endif