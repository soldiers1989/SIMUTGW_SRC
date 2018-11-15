#include "Proc_Distribute_InMsg.h"

#include <memory>

#include "simutgw/mkt_interface/ShDbfOrderHelper.h"
#include "simutgw/msg_biz/ProcMemoryReport.h"

#include "simutgw/stgw_config/sys_function.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "simutgw/work_manage/ProcBase_Order_Valid.h"

#include "simutgw/work_manage/GenTaskHelper.h"

Proc_Distribute_InMsg::Proc_Distribute_InMsg( void )
	:m_scl( keywords::channel = "Proc_Distribute_InMsg" )
{
}

Proc_Distribute_InMsg::~Proc_Distribute_InMsg( void )
{
}

int Proc_Distribute_InMsg::TaskProc( void )
{
	static const string ftag( "Proc_Distribute_InMsg::TaskProc() " );

	int iRes = ProcInMessage();

	return iRes;
}

/*
处理下单消息缓存

Return :
0 -- 处理成功
-1 -- 处理失败
*/
int Proc_Distribute_InMsg::ProcInMessage()
{
	static const string ftag( "Proc_Distribute_InMsg::ProcInMessage() " );

	int iRes = 0;

	std::shared_ptr<struct simutgw::OrderMessage> ptrOrderMsg( new struct simutgw::OrderMessage );

	iRes = simutgw::g_inMsg_buffer.PopFront( ptrOrderMsg );

	if ( 0 == iRes )
	{
		if ( nullptr == ptrOrderMsg )
		{
			BOOST_LOG_SEV( m_scl, trivial::error ) << ftag << "nullptr!";

			return -1;
		}

		// 生成流水线任务
		iRes = GenTaskHelper::GenTask_Valid( ptrOrderMsg );
		if ( 0 != iRes )
		{
			string sDebug( "GenTask_Valid error, Clordid=" );
			sDebug += ptrOrderMsg->strClordid;
			BOOST_LOG_SEV( m_scl, trivial::error ) << ftag << sDebug;

			return -1;
		}
	}
	else if ( 1 == iRes )
	{
		// 无回报
		simutgw::Simutgw_Sleep();
	}
	else
	{
		BOOST_LOG_SEV( m_scl, trivial::error ) << ftag << "PopFront() faild";

		return -1;
	}

	return 0;
}
