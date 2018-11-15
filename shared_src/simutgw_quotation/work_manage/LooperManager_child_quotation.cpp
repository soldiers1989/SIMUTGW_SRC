#include "LooperManager_child_quotation.h"

#include "simutgw_flowwork/FlowWorkBase.h"

#include "quotation/TgwMarketInfoProc.h"

#include "util/SystemCounter.h"

#include "simutgw_config/g_values_sys_run_config.h"

LooperManager_child_quotation::LooperManager_child_quotation(void)
	: m_scl(keywords::channel = "LooperManager_child_quotation"), m_looperPool(2)
{
}

LooperManager_child_quotation::~LooperManager_child_quotation(void)
{
}

/*
启动所有模拟撮合相关线程

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int LooperManager_child_quotation::StartFlows(void)
{
	static const string ftag("LooperManager_child_quotation::StartFlows() ");

	int iRes = 0;
	
	//
	// 行情处理
	iRes = Start_MarketInfo();
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Start_MarketInfo failed";

		return -1;
	}

	// start pool
	iRes = m_looperPool.StartPool();
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "StartPool failed";

		return -1;
	}

	return iRes;
}

/*
关闭所有模拟撮合相关线程

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int LooperManager_child_quotation::StopFlows(void)
{
	m_looperPool.StopPool();
	return 0;
}

/*
启动相关线程 行情处理

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int LooperManager_child_quotation::Start_MarketInfo(void)
{
	static const string ftag("LooperManager_child_quotation::Start_MarketInfo() ");

	//
	//启动处理行情线程
	TgwMarketInfoProc* ptr_QutationTask = new TgwMarketInfoProc();
	if (!ptr_QutationTask)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new ptr_QutationTask failed";

		return -1;
	}

	std::shared_ptr<TaskBase> ptr_TaskQutation((TaskBase*)ptr_QutationTask);
	m_looperPool.AssignTask(ptr_TaskQutation);

	return 0;
}