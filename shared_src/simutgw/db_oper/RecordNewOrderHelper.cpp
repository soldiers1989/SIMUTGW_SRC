#include "RecordNewOrderHelper.h"
#include "simutgw/db_oper/TaskRecordOrder.h"

#include "order/StockOrderHelper.h"
#include "simutgw/order/OrderRepeatChecker.h"

#include "simutgw/msg_biz/ProcCancelOrder.h"

#include "tool_redis/Tgw_RedisHelper.h"

#include "config/conf_mysql_table.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "simutgw/work_manage/GenTaskHelper.h"


RecordNewOrderHelper::RecordNewOrderHelper( void )
{
}

RecordNewOrderHelper::~RecordNewOrderHelper( void )
{
}

// 将下单消息写入到数据库，包括流水表、买表和卖表，并加入待成交任务
int RecordNewOrderHelper::RecordInOrderToDb_Match( std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg )
{
	static const string ftag( "RecordNewOrderHelper::RecordInOrderToDb() " );

	{
		string strDebug( "Record clordid[" );
		strDebug += in_OrderMsg->strClordid;
		EzLog::d(ftag, strDebug);
	}

	int iRes = 0;

	if (0 != in_OrderMsg->tradePolicy.ui64RuleId)
	{
		// 已定义了成交规则
		// 不是废单模式
		// 处理申报确认
		// 写入回报队列
		std::shared_ptr<struct simutgw::OrderMessage> ptrConfirm(new struct simutgw::OrderMessage(*in_OrderMsg));
		iRes = simutgw::g_outMsg_buffer.PushBack(ptrConfirm);
	}
	else
	{
		if (simutgw::SysRunMode::NormalMode == in_OrderMsg->tradePolicy.iRunMode)
		{
			// 3 -- 普通模式 记录数据库
			TaskRecordOrder* task = new TaskRecordOrder(simutgw::g_uidTaskGen.GetId());
			task->SetOrder(in_OrderMsg);

			std::shared_ptr<TaskBase> base(dynamic_cast<TaskBase*>(task));
			simutgw::g_asyncDbwriter.AssignTask(base);
		}

		if (0 == in_OrderMsg->strMsgType.compare("D"))
		{
			if (in_OrderMsg->tradePolicy.iMatchMode != simutgw::SysMatchMode::SimulErrMatch
				&& 0 != in_OrderMsg->strApplID.compare("120"))
			{
				// 不是废单模式
				// 处理申报确认
				// 写入回报队列
				std::shared_ptr<struct simutgw::OrderMessage> ptrConfirm(new struct simutgw::OrderMessage(*in_OrderMsg));
				Tgw_StringUtil::iLiToStr(ptrConfirm->ui64mOrderPrice, ptrConfirm->strOrderPrice, 4);
				ptrConfirm->strExecType = simutgw::STEPMSG_EXECTYPE_NEW;
				ptrConfirm->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_NEW;
				iRes = simutgw::g_outMsg_buffer.PushBack(ptrConfirm);
			}
		}
		else
		{
			//nothing
		}
	}	

	// 记录
	OrderRepeatChecker::RecordOrderInRedis(in_OrderMsg);

	// 生成待成交/撤单任务
	iRes = GenTaskHelper::GenTask_Match( in_OrderMsg );
	if ( iRes < 0 )
	{
		EzLog::e( ftag, "GenTask_Match error" );
		return -1;
	}

	return iRes;
}
