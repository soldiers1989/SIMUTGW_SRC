#ifndef __TASK_RECORD_REPORT_H__
#define __TASK_RECORD_REPORT_H__

#include "thread_pool_base/TaskBase.h"

#include "order/define_order_msg.h"

#include "util/EzLog.h"

#include "tool_mysql/MySqlCnnC602.h"

/*
	记录处理结果类
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
	//将交易结果写入到数据库，包括流水表、买表和卖表
	int RecordReportInDb();
	/*
	将下单消息插入到表中
	Param :

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int InsertDb_Report(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);
};

#endif