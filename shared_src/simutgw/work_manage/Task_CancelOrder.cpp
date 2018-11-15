#include "Task_CancelOrder.h"

#include <memory>

#include "Task_ABStockMatch.h"

#include "util/SystemCounter.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/sys_function.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "cache/UserStockHelper.h"


/*
处理单笔撤单，有撤单请求
*/
int Task_CancelOrder::ProcSingleCancelOrder()
{
	static const string ftag( "Task_CancelOrder::ProcSingleCancelOrder() " );

	std::shared_ptr<TaskPriorityBase> origOrderTask( new Task_ABStockMatch( 0 ) );

	int iRes = 0;

	if ( nullptr == m_pTaskqueue )
	{
		// 无队列句柄，严重错误
		string sdebug( "m_pTaskqueue nullptr, clordid=" );
		sdebug += m_strClordid;

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sdebug;

		// 无原始订单，已经被处理
		iRes = ProcCancelFail_OrigIsProc();
	}
	else
	{
		iRes = m_pTaskqueue->DeleteIfHaveTask( m_iKey, m_orderMsg->strOrigClordid, origOrderTask );

		if ( 1 == iRes )
		{
			// 无原始订单，已经被处理
			iRes = ProcCancelFail_OrigIsProc();
		}
		else
		{
			// nothing 
			// 撤单成功
			iRes = ProcCancelSuccess( origOrderTask );

			// 内存解冻
			if ( 0 == iRes )
			{
				std::shared_ptr<Task_ABStockMatch> task( dynamic_pointer_cast<Task_ABStockMatch>( origOrderTask ) );
				std::shared_ptr<struct simutgw::OrderMessage> origPtr( new struct simutgw::OrderMessage() );
				task->GetOrderMsg( origPtr );
				if (origPtr->tradePolicy.bCheck_Assets)
				{
					iRes = UserStockHelper::UpdateAfterCancel(origPtr);
				}				
			}
		}
	}

	return iRes;
}

/*
撤单失败
*/
int Task_CancelOrder::ProcCancelFail_OrigNotFound()
{
	static const string strTag( "Task_CancelOrder::ProcCancelFail_OrigNotFound()" );

	m_orderMsg->strIsProc = "1";

	m_orderMsg->enMatchType = simutgw::CancelMatch;

	m_orderMsg->strOrdStatus = "8";

	m_orderMsg->strCxlRejReason = "20097";

	// 写入回报队列
	int iRes = simutgw::g_outMsg_buffer.PushBack( m_orderMsg );

	return iRes;
}

/*
撤单失败
*/
int Task_CancelOrder::ProcCancelFail_OrigIsProc()
{
	static const string strTag( "Task_CancelOrder::ProcCancelFail_OrigNotFound()" );

	m_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_CANCELREJECT;

	m_orderMsg->enMatchType = simutgw::CancelMatch;

	m_orderMsg->strOrdStatus = "2";

	m_orderMsg->strCxlRejReason = "20096";

	// 写入回报队列
	int iRes = simutgw::g_outMsg_buffer.PushBack( m_orderMsg );

	return iRes;
}

/*
撤单成功
*/
int Task_CancelOrder::ProcCancelSuccess( std::shared_ptr<TaskPriorityBase>& origOrderTask )
{
	static const string strTag( "Task_CancelOrder::ProcCancelSuccess()" );

	std::shared_ptr<Task_ABStockMatch> task( dynamic_pointer_cast<Task_ABStockMatch>( origOrderTask ) );
	std::shared_ptr<struct simutgw::OrderMessage> cancelPtr( new struct simutgw::OrderMessage() );
	int iRes = task->GetOrderMsg( cancelPtr );
	if ( -1 == iRes )
	{
		ProcCancelFail_OrigNotFound();
	}

	cancelPtr->strIsProc = "1";

	cancelPtr->enMatchType = simutgw::CancelMatch;

	cancelPtr->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

	cancelPtr->strExecType = simutgw::STEPMSG_EXECTYPE_CANCEL;

	cancelPtr->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_CANCEL;

	cancelPtr->strOrigClordid = m_orderMsg->strOrigClordid;
	cancelPtr->strClordid = m_orderMsg->strClordid;

	// 撤单价格为0
	cancelPtr->strLastPx = "0.0000";
	Tgw_StringUtil::String2UInt64_strtoui64( cancelPtr->strLastPx, cancelPtr->ui64mPrice_matched );

	Tgw_StringUtil::String2UInt64_strtoui64( cancelPtr->strLeavesQty, cancelPtr->ui64Orderqty_unmatched );
	Tgw_StringUtil::String2UInt64_strtoui64( cancelPtr->strOrderqty_origin, cancelPtr->ui64Orderqty_origin );

	// 撤单数量
	cancelPtr->strLastQty = cancelPtr->strLeavesQty;
	Tgw_StringUtil::String2UInt64_strtoui64( cancelPtr->strLastQty, cancelPtr->ui64Orderqty_matched );

	// 剩余数量为0
	cancelPtr->strLeavesQty = "0";

	// 撤单时累计成交量是成交数量不是撤单数量
	// 成交数量 = 总量 - 剩余量
	sof_string::itostr( ( cancelPtr->ui64Orderqty_origin - cancelPtr->ui64Orderqty_unmatched ), cancelPtr->strCumQty );

	// 生成内部订单号
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//唯一执行id
	cancelPtr->strExecID = strTransId;
	//唯一订单id
	cancelPtr->strOrderID = strTransId;

	// 写入回报队列
	iRes = simutgw::g_outMsg_buffer.PushBack( cancelPtr );

	return iRes;
}

/*
处理单笔撤单，主动撤单，没有撤单请求
*/
int Task_CancelOrder::ProcSingleCancelOrder_Without_Request()
{
	static const string strTag( "Task_CancelOrder::ProcCancelSuccess()" );

	m_orderMsg->strIsProc = "1";

	m_orderMsg->enMatchType = simutgw::CancelMatch;

	m_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

	m_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_CANCEL;

	m_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_CANCEL;

	//cancelPtr->strOrigClordid = in_cancelOrder->strOrigClordid;

	m_orderMsg->strLastPx = "0.0000";

	//Tgw_StringUtil::String2UInt64_strtoui64(in_cancelOrder->strLeavesQty, in_cancelOrder->ui64Orderqty_unmatched);
	Tgw_StringUtil::String2UInt64_strtoui64( m_orderMsg->strOrderqty_origin, m_orderMsg->ui64Orderqty_origin );

	//uint64_t uiValue;
	//uiValue = in_origPtr->ui64Orderqty_origin - in_origPtr->ui64Orderqty_unmatched;
	//cancelPtr->strLastQty = sof_string::itostr(uiValue, cancelPtr->strLastQty);

	m_orderMsg->strLastQty = m_orderMsg->strLeavesQty;

	m_orderMsg->strCumQty = m_orderMsg->strLastQty;

	m_orderMsg->strLeavesQty = m_orderMsg->strLeavesQty;

	m_orderMsg->strCashorderqty = "0";

	// 生成内部订单号
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//唯一执行id
	m_orderMsg->strExecID = strTransId;
	//唯一订单id
	m_orderMsg->strOrderID = strTransId;

	// 写入回报队列
	int iRes = simutgw::g_outMsg_buffer.PushBack( m_orderMsg );

	return iRes;
}