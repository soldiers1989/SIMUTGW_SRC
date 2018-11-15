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
��֤��������
Return:
0 -- �ɹ�
1 -- ʧ��
*/
int Task_ValidOrder::ValidateOrder(void)
{
	static const string ftag("Task_ValidOrder::ValidateOrder() ");

	int iRes = 0;

	if (0 == m_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		// ����Ƿ��ص�
		if (simutgw::SysRunMode::PressureMode != m_orderMsg->tradePolicy.iRunMode)
		{
			// 1 -- ѹ��ģʽ ѹ��ģʽ������ص�
			bool bIsRepeat = OrderRepeatChecker::CheckIfRepeatOrder(m_orderMsg);
			if (bIsRepeat)
			{
				std::string strRepeat("ί��");
				strRepeat += m_orderMsg->strClordid;
				strRepeat += "�ص�";
				BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << strRepeat;
				// �ص�
				iRes = StgwFixReport::Send_SZ_RepeatRejectMsg(m_orderMsg);

				return 1;
			}
		}

		// ����
		iRes = SzConn_StepOrder::Valide_Record_Order(m_orderMsg);
		if (iRes < 0)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Clordid=" << m_orderMsg->strClordid << ", Valide_Record_Order faild";

			return -1;
		}
		else
		{
			// ����
			simutgw::g_counter.GetSz_InnerCounter()->Inc_Confirm();
		}
	}
	else if (0 == m_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// �Ϻ�
		// ����Ƿ��ص�
		if (simutgw::SysRunMode::PressureMode != m_orderMsg->tradePolicy.iRunMode)
		{
			// 1 -- ѹ��ģʽ ѹ��ģʽ������ص�
			bool bIsRepeat = OrderRepeatChecker::CheckIfRepeatOrder(m_orderMsg);
			if (bIsRepeat)
			{
				// �ص�
				std::string strRepeat("ί��[");
				strRepeat += m_orderMsg->strClordid;
				strRepeat += "]�ص�";
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
			// ����
			simutgw::g_counter.GetSh_InnerCounter()->Inc_Confirm();
		}
	}

	return 0;
}
