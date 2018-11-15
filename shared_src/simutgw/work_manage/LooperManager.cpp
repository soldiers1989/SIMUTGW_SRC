#include "LooperManager.h"

#include <memory>

#include "simutgw_flowwork/FlowWorkBase.h"

#include "quotation/TgwMarketInfoProc.h"

#include "simutgw/work_manage/Proc_SzConn_Step.h"
#include "simutgw/work_manage/Proc_ShConn_Db_Declare.h"
#include "simutgw/work_manage/Proc_ShConn_Db_Report.h"
#include "simutgw/work_manage/Proc_Distribute_InMsg.h"
#include "simutgw/work_manage/Proc_WorkLoadTrafficStat.h"

#include "util/SystemCounter.h"

#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

LooperManager::LooperManager(void)
	: m_scl(keywords::channel = "LooperManager"), m_looperPool(8)
{
}

LooperManager::~LooperManager(void)
{
}

/*
启动所有模拟撮合相关线程

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int LooperManager::StartFlows(void)
{
	static const string ftag("LooperManager::StartFlows() ");

	int iRes = 0;

	//
	// 分配下单消息缓存
	iRes = Start_Distribute_InMsg();
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Start_Distribute_InMsg failed";

		return -1;
	}
	
	//
	//
	iRes = Start_SzConn();
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Start_SzConn failed";

		return -1;
	}

	//
	//
	iRes = Start_ShConn();
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Start_ShConn failed";

		return -1;
	}

	//
	//
	iRes = Start_Traffic_count();
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Start_Traffic_count failed";

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
int LooperManager::StopFlows(void)
{
	m_looperPool.StopPool();
	return 0;
}


/*
启动相关线程 回报消息处理

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int LooperManager::Start_SzConn(void)
{
	static const string ftag("LooperManager::Start_SzConn() ");

	//
	//启动深圳委托、回报消息处理线程
	if (simutgw::g_bEnable_Sz_Msg_Task)
	{
		Proc_SzConn_Step* ptr_SzMsgTask = new Proc_SzConn_Step();
		if (!ptr_SzMsgTask)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new ptr_SzMsgTask failed";

			return -1;
		}

		ptr_SzMsgTask->SetFlowMatchType(simutgw::Flow_MatchType_SZ_MSG);

		std::shared_ptr<TaskBase> ptr_TaskSzMsg((TaskBase*)ptr_SzMsgTask);
		m_looperPool.AssignTask(ptr_TaskSzMsg);
	}

	return 0;
}

/*
启动相关线程 上海委托、回报消息处理

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int LooperManager::Start_ShConn(void)
{
	static const string ftag("LooperManager::Start_ShConn() ");

	//
	// 启动上海委托、回报消息处理线程
	if (simutgw::g_bEnable_Sh_Msg_Task)
	{
		//
		// 发送上海回报
		Proc_ShConn_Db_Report* ptr_ShReportTask = new Proc_ShConn_Db_Report();
		if (!ptr_ShReportTask)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new ptr_ShReportTask failed";

			return -1;
		}

		ptr_ShReportTask->SetFlowMatchType(simutgw::Flow_MatchType_SH_MSG);

		std::shared_ptr<TaskBase> ptr_TaskShReport((TaskBase*)ptr_ShReportTask);
		m_looperPool.AssignTask(ptr_TaskShReport);

		//
		// 取上海委托
		Proc_ShConn_Db_Declare* ptr_ShMsgTask = new Proc_ShConn_Db_Declare();
		if (!ptr_ShMsgTask)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new ptr_ShMsgTask failed";

			return -1;
		}

		ptr_ShMsgTask->SetFlowMatchType(simutgw::Flow_MatchType_SH_MSG);

		std::shared_ptr<TaskBase> ptr_TaskShMsg((TaskBase*)ptr_ShMsgTask);
		m_looperPool.AssignTask(ptr_TaskShMsg);
	}

	return 0;
}

/*
启动线程
分配下单消息缓存

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int LooperManager::Start_Distribute_InMsg(void)
{
	static const string ftag("LooperManager::Start_Distribute_InMsg() ");

	//启动 分配下单消息缓存
	Proc_Distribute_InMsg* ptr_Distribute = new Proc_Distribute_InMsg();
	if (!ptr_Distribute)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new ptr_DistributeTask failed";

		return -1;
	}

	std::shared_ptr<TaskBase> ptrTask((TaskBase*)ptr_Distribute);
	m_looperPool.AssignTask(ptrTask);

	return 0;
}

/*
启动线程
流量定时统计

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int LooperManager::Start_Traffic_count(void)
{
	static const string ftag("LooperManager::Start_Traffic_count() ");

	//启动深圳买
	Proc_WorkLoadTrafficStat* ptr_Traffic = new Proc_WorkLoadTrafficStat();
	if (!ptr_Traffic)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new Proc_WorkLoadTrafficStat failed";

		return -1;
	}

	std::shared_ptr<TaskBase> ptrTask((TaskBase*)ptr_Traffic);
	m_looperPool.AssignTask(ptrTask);

	simutgw::g_prtTraffic = ptrTask;

	return 0;
}