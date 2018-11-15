#ifndef __TASK_RECORD_COMPONENT_MATCH_H__
#define __TASK_RECORD_COMPONENT_MATCH_H__

#include "thread_pool_base/TaskBase.h"

#include "order/define_order_msg.h"

/*
	记录etf成分股成交记录类
*/
class TaskRecordComponentMatch : public TaskBase
{
	//
	// member
	//
	std::shared_ptr<struct simutgw::OrderMessage> m_ptrReport;
	std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent> m_ptrFrozeComponent;
	
	//
	// function
	//
public:
	explicit TaskRecordComponentMatch(const unsigned int uiId);
	virtual ~TaskRecordComponentMatch();
	
	int SetReportAndComponent(std::shared_ptr<struct simutgw::OrderMessage> in_ptrReport,
		std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent> in_ptrFrozeComponent)
	{
		m_ptrReport = in_ptrReport;
		m_ptrFrozeComponent = in_ptrFrozeComponent;

		return 0;
	}

	virtual int TaskProc(void);

private:
	/*
	etf申赎成分股记录到流水表
	*/
	int RecordCompnentMatchInfo();
};

#endif