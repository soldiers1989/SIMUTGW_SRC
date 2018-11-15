#include "Proc_SzConn_Step.h"

#include "simutgw/mkt_interface/ShDbfOrderHelper.h"

#include "simutgw/stgw_config/sys_function.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "order/StockOrderHelper.h"

#include "simutgw/work_manage/ProcBase_Order_Valid.h"

#include "simutgw/stgw_fix_acceptor/StgwFixReport.h"

Proc_SzConn_Step::Proc_SzConn_Step(void)
	:m_scl(keywords::channel = "Proc_SzConn_Step")
{
}

Proc_SzConn_Step::~Proc_SzConn_Step(void)
{
}

int Proc_SzConn_Step::TaskProc(void)
{
	static const string fTag("Proc_SzConn_Step::TaskProc() ");

	int iRes = ProcSzMessage();

	return iRes;
}

/*
处理深圳的消息

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int Proc_SzConn_Step::ProcSzMessage()
{
	static const string ftag("Proc_SzConn_Step::ProcSzMessage() ");

	int iRes = 0;

	//---------------处理回报--------------------
	iRes = StgwFixReport::Send_SZReport();
	if (iRes < 0)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ProcSZReport() faild";

		return -1;
	}
	else if (1 == iRes)
	{
		// 无回报
		simutgw::Simutgw_Sleep();
	}

	return 0;
}
