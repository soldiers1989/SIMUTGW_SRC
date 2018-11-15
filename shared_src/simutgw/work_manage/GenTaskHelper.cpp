#include "GenTaskHelper.h"

#include <memory>

#include "simutgw/mkt_interface/ShDbfOrderHelper.h"

#include "simutgw/stgw_config/sys_function.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "simutgw/work_manage/ProcBase_Order_Valid.h"
#include "simutgw/work_manage/Task_ValidOrder.h"
#include "simutgw/work_manage/Task_ABStockMatch.h"
#include "simutgw/work_manage/Task_Etf_Match.h"
#include "simutgw/work_manage/Task_CancelOrder.h"
#include "simutgw/work_manage/Task_ShRule_Match.h"
#include "simutgw/work_manage/Task_SzRule_Match.h"

/*
生成待验证消息

Return :
0 -- 成功
-1 -- 失败
*/
int GenTaskHelper::GenTask_Valid(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrderMsg)
{
	static const string ftag("GenTaskHelper::GenTask_Valid() ");

	int iRes = 0;

	if (nullptr == in_ptrOrderMsg)
	{
		EzLog::e(ftag, "nullprt");
		return -1;
	}

	// Gen task
	Task_ValidOrder* validTask = new Task_ValidOrder(simutgw::g_uidTaskGen.GetId());
	validTask->SetOrderMsg(in_ptrOrderMsg);

	std::shared_ptr<TaskPriorityBase> task(dynamic_cast<TaskPriorityBase*>(validTask));
	//iRes = simutgw::g_mtskPool_valid.AssignTaskWithKey(task);
	iRes = simutgw::g_mtskPool_valid.AssignTaskInMini(task);
	if (0 != iRes)
	{
		string sDebug("AssignTask error, Clordid=");
		sDebug += in_ptrOrderMsg->strClordid;
		EzLog::e(ftag, sDebug);

		return -1;
	}

	return 0;
}

/*
生成待成交消息

Return :
0 -- 成功
-1 -- 失败
*/
int GenTaskHelper::GenTask_Match(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrderMsg)
{
	static const string ftag("GenTaskHelper::GenTask_Match() ");

	int iRes = 0;

	if (nullptr == in_ptrOrderMsg)
	{
		EzLog::e(ftag, "nullptr!");

		return -1;
	}

	// Gen task
	void* vTask = nullptr;

	if (0 != in_ptrOrderMsg->tradePolicy.ui64RuleId)
	{
		// 已定义了成交规则
		if (nullptr != in_ptrOrderMsg->tradePolicy.ptrRule_Sh)
		{
			// 
			Task_ShRule_Match* shRuleTask = new Task_ShRule_Match(simutgw::g_uidTaskGen.GetId());
			shRuleTask->SetOrderMsg(in_ptrOrderMsg);
			vTask = dynamic_cast<void*>(shRuleTask);
		}
		else if (nullptr != in_ptrOrderMsg->tradePolicy.ptrRule_Sz)
		{
			Task_SzRule_Match* szRuleTask = new Task_SzRule_Match(simutgw::g_uidTaskGen.GetId());
			szRuleTask->SetOrderMsg(in_ptrOrderMsg);
			vTask = dynamic_cast<void*>(szRuleTask);
		}
		else
		{
			string sDebug("error order cliordid=");
			sDebug += in_ptrOrderMsg->strClordid;
			sDebug += "nullptr Rule, ruleId=";
			string sItoa;
			sof_string::itostr(in_ptrOrderMsg->tradePolicy.ui64RuleId, sItoa);
			sDebug += sItoa;
			EzLog::e(ftag, sDebug);

			return -1;
		}
	}
	else
	{

		switch (in_ptrOrderMsg->iTrade_type)
		{

		case simutgw::TADE_TYPE::cancel:
		{
			Task_CancelOrder* cancelTask = new Task_CancelOrder(simutgw::g_uidTaskGen.GetId());
			cancelTask->SetOrderMsg(in_ptrOrderMsg);

			vTask = dynamic_cast<void*>(cancelTask);
		}
		break;
		case simutgw::TADE_TYPE::a_trade:
			// A股
		case simutgw::TADE_TYPE::b_trade:
			// B股
		case simutgw::TADE_TYPE::margin_cash:
			// 融资交易，融资买入和卖券还款
		case simutgw::TADE_TYPE::margin_stock:
			// 融券交易，融券卖出和买券还券
		case simutgw::TADE_TYPE::etf_buy:
			// ETF买入		
		case simutgw::TADE_TYPE::etf_sell:
			// ETF卖出
		{
			Task_ABStockMatch* matchTask = new Task_ABStockMatch(simutgw::g_uidTaskGen.GetId());
			matchTask->SetOrderMsg(in_ptrOrderMsg);

			vTask = dynamic_cast<void*>(matchTask);
		}
		break;

		case simutgw::TADE_TYPE::etf_crt:
			// ETF申购			
		case simutgw::TADE_TYPE::etf_rdp:
			// ETF赎回
		{
			Task_Etf_Match* matchTask = new Task_Etf_Match(simutgw::g_uidTaskGen.GetId());
			matchTask->SetOrderMsg(in_ptrOrderMsg);

			vTask = dynamic_cast<void*>(matchTask);
		}
		break;

		default:
			string sItoA;

			string sDebug("error Trade_type=");
			sDebug += sof_string::itostr(in_ptrOrderMsg->iTrade_type, sItoA);
			sDebug += " Clordid=";
			sDebug += in_ptrOrderMsg->strClordid;
			EzLog::e(ftag, sDebug);
			return -1;

			break;
		}
	}
	std::shared_ptr<TaskPriorityBase> task((TaskPriorityBase*)vTask);
	iRes = simutgw::g_mtskPool_match_cancel.AssignTaskWithKey(task);
	if (0 != iRes)
	{
		string sDebug("AssignTask error, Clordid=");
		sDebug += in_ptrOrderMsg->strClordid;
		EzLog::e(ftag, sDebug);

		return -1;
	}

	return 0;
}
