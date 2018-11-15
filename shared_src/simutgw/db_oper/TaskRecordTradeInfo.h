#ifndef __TASK_RECORD_TRADE_INFO_H__
#define __TASK_RECORD_TRADE_INFO_H__

#include "thread_pool_base/TaskBase.h"
#include "order/define_order_msg.h"

namespace AsyncDbTask
{
	enum UpdateTaskType
	{
		notype = 0,
		match = 1,
		cancel_succ = 2,
		cancel_fail = 3,
		error = 4
	};
}

/*
	交易后更新数据库消息任务类
*/
class TaskRecordTradeInfo : public TaskBase
{
	//
	// member
	//
	AsyncDbTask::UpdateTaskType m_emType;
	std::shared_ptr<struct simutgw::OrderMessage> m_ptrReport;

	//
	// function
	//
public:
	explicit TaskRecordTradeInfo(unsigned int uiId);
	virtual ~TaskRecordTradeInfo();

	void SetTask(AsyncDbTask::UpdateTaskType& in_emType,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport)
	{
		m_emType = in_emType;
		m_ptrReport = in_ptrReport;
	}

	virtual int TaskProc(void);

private:
	/*
	向数据库中写入成交的数据
	成交
	Param :

	Return :
	0 -- 写入成功
	非0 -- 写入失败
	*/
	int WriteTransInfoInDb();

	/*
	向数据库中写入成交的数据
	撤单成功
	Param :

	Return :
	0 -- 写入成功
	非0 -- 写入失败
	*/
	int WriteTransInfoInDb_CancelSuccess();

	/*
	向数据库中写入成交的数据
	撤单失败
	Param :

	Return :
	0 -- 写入成功
	非0 -- 写入失败
	*/
	int WriteTransInfoInDb_CancelFail();

	/*
	向数据库中写入成交的数据
	废单
	Param :

	Return :
	0 -- 写入成功
	非0 -- 写入失败
	*/
	int WriteTransInfoInDb_Error();
};

#endif