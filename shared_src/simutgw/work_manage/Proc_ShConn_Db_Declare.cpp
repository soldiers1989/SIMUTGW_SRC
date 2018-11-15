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
�����Ϻ�����Ϣ

Return :
0 -- �����ɹ�
-1 -- ����ʧ��
*/
int Proc_ShConn_Db_Declare::ProcShMessage()
{
	static const string ftag("Proc_ShConn_Db_Declare::ProcShMessage() ");

	//---------------ȡ�Ϻ�ί��--------------------
	vector <std::shared_ptr<struct simutgw::OrderMessage>> vecOrder;

	int iRes = ShConn_DbfOrder::BatchGetSHOrder(vecOrder);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetSHOrder() faild";

		return -1;
	}

	if (vecOrder.size() == 0)
	{
		// ��ί��
		simutgw::Simutgw_Sleep();
	}

	for (size_t st = 0; st < vecOrder.size(); ++st)
	{
		//// �ж�trade_type
		//iRes = StockOrderHelper::GetOrderTradeType(vecOrder[st]);
		//if (0 == iRes)
		{
			// д����Ϣ���뻺��
			simutgw::g_inMsg_buffer.PushBack(vecOrder[st]);
		}

		// �����ڲ�����
		simutgw::g_counter.GetSh_InnerCounter()->Inc_Received();
	}

	return 0;
}