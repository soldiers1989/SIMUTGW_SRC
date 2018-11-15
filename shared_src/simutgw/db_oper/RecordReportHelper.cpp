#include "RecordReportHelper.h"
#include "simutgw/db_oper/TaskRecordReport.h"

#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"


RecordReportHelper::RecordReportHelper(void)
{
}

RecordReportHelper::~RecordReportHelper(void)
{
}

// 记录处理流水
int RecordReportHelper::RecordReportToDb(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg)
{
	static const string ftag("RecordReportHelper::RecordReportToDb() ");
	{
		string strDebug("Record Report clordid[");
		strDebug += in_OrderMsg->strClordid;
		EzLog::i(ftag, strDebug);
	}

	int iRes = 0;

	if (simutgw::SysRunMode::NormalMode == in_OrderMsg->tradePolicy.iRunMode)
	{
		// 3 -- 普通模式 记录数据库
		TaskRecordReport* task = new TaskRecordReport(simutgw::g_uidTaskGen.GetId());
		task->SetOrder(in_OrderMsg);

		std::shared_ptr<TaskBase> base(dynamic_cast<TaskBase*>(task));
		simutgw::g_asyncDbwriter.AssignTask(base);
	}

	return iRes;
}

