#include "Task_ShRule_Match.h"

#include <memory>

#include "simutgw/order/OrderMemoryStoreFactory.h"
#include "order/StockOrderHelper.h"
#include "simutgw/db_oper/RecordNewOrderHelper.h"

#include "simutgw/db_oper/DbUserInfoAsset.h"

#include "simutgw/msg_biz/ProcCancelOrder.h"

#include "quotation/MarketInfoHelper.h"

#include "util/SystemCounter.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/sys_function.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "etf/ETFHelper.h"
#include "cache/UserStockHelper.h"

#include "GenTaskHelper.h"

/*
���׳ɽ�����
Return:
0 -- �ɽ�
-1 -- ʧ��
*/
int Task_ShRule_Match::MatchOrder()
{
	static const string ftag("Task_ShRule_Match::MatchOrder() ");
	
	m_orderMsg->enMatchType = simutgw::MatchAll;

	// д��ر�����
	simutgw::g_outMsg_buffer.PushBack(m_orderMsg);
	
	return 0;
}
