#include "Task_ValidOrder.h"

#include "order/StockOrderHelper.h"
#include "simutgw/order/OrderRepeatChecker.h"

#include "simutgw/stgw_fix_acceptor/StgwFixReport.h"

#include "simutgw/db_oper/RecordNewOrderHelper.h"

#include "tool_redis/Tgw_RedisHelper.h"

#include "simutgw/stgw_config/sys_function.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "simutgw/mkt_interface/SzConn_StepOrder.h"
#include "simutgw/mkt_interface/ShConn_DbfOrder.h"

/*
验证订单数据
Return:
0 -- 成功
1 -- 失败
*/
int Task_ValidOrder::ValidateOrder(void)
{
	static const string ftag("Task_ValidOrder::ValidateOrder() ");

	int iRes = 0;

	if (0 == m_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		// 检查是否重单
		if (simutgw::SysRunMode::PressureMode != m_orderMsg->tradePolicy.iRunMode)
		{
			// 1 -- 压力模式 压力模式不检查重单
			bool bIsRepeat = OrderRepeatChecker::CheckIfRepeatOrder(m_orderMsg);
			if (bIsRepeat)
			{
				std::string strRepeat("委托");
				strRepeat += m_orderMsg->strClordid;
				strRepeat += "重单";
				BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << strRepeat;
				// 重单
				iRes = StgwFixReport::Send_SZ_RepeatRejectMsg(m_orderMsg);

				return 1;
			}
		}

		// 深圳
		iRes = SzConn_StepOrder::Valide_Record_Order(m_orderMsg);
		if (iRes < 0)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Clordid=" << m_orderMsg->strClordid << ", Valide_Record_Order faild";

			return -1;
		}
		else
		{
			// 计数
			simutgw::g_counter.GetSz_InnerCounter()->Inc_Confirm();
		}
	}
	else if (0 == m_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// 上海
		// 检查是否重单
		if (simutgw::SysRunMode::PressureMode != m_orderMsg->tradePolicy.iRunMode)
		{
			// 1 -- 压力模式 压力模式不检查重单
			bool bIsRepeat = OrderRepeatChecker::CheckIfRepeatOrder(m_orderMsg);
			if (bIsRepeat)
			{
				// 重单
				std::string strRepeat("委托[");
				strRepeat += m_orderMsg->strClordid;
				strRepeat += "]重单";
				BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << strRepeat;
				return 1;
			}
		}

		iRes = ShConn_DbfOrder::Valide_Record_Order(m_orderMsg);
		if (iRes < 0)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Clordid=" << m_orderMsg->strClordid << ", Valide_Record_Order faild";

			return -1;
		}
		else
		{
			// 计数
			simutgw::g_counter.GetSh_InnerCounter()->Inc_Confirm();
		}
	}

	return 0;
}
