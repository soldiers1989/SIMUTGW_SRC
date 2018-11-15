#include "Proc_ShConn_Db_Declare.h"

#include <memory>

#include "simutgw/mkt_interface/ShDbfOrderHelper.h"
#include "simutgw/mkt_interface/ShConn_DbfOrder.h"

#include "simutgw/stgw_config/sys_function.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "order/StockOrderHelper.h"

#include "util/TimeDuration.h"

Proc_ShConn_Db_Declare::Proc_ShConn_Db_Declare(void)
	:m_scl(keywords::channel = "Proc_ShConn_Db_Declare")
{
}

Proc_ShConn_Db_Declare::~Proc_ShConn_Db_Declare(void)
{
}

int Proc_ShConn_Db_Declare::TaskProc(void)
{
	static const string fTag("Proc_ShConn_Db_Declare::TaskProc() ");

	int iRes = ProcShMessage();

	return iRes;
}

/*
处理上海的消息

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int Proc_ShConn_Db_Declare::ProcShMessage()
{
	static const string ftag("Proc_ShConn_Db_Declare::ProcShMessage() ");

	//---------------取上海委托--------------------
	vector <std::shared_ptr<struct simutgw::OrderMessage>> vecOrder;

	int iRes = ShConn_DbfOrder::BatchGetSHOrder(vecOrder);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetSHOrder() faild";

		return -1;
	}

	if (vecOrder.size() == 0)
	{
		// 无委托
		simutgw::Simutgw_Sleep();
	}

	for (size_t st = 0; st < vecOrder.size(); ++st)
	{
		//// 判断trade_type
		//iRes = StockOrderHelper::GetOrderTradeType(vecOrder[st]);
		//if (0 == iRes)
		{
			// 写入消息进入缓存
			simutgw::g_inMsg_buffer.PushBack(vecOrder[st]);
		}

		// 增加内部计数
		simutgw::g_counter.GetSh_InnerCounter()->Inc_Received();
	}

	return 0;
}