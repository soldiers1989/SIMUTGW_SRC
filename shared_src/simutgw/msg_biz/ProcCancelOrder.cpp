#include "ProcCancelOrder.h"

#include "simutgw/order/OrderMemoryStoreFactory.h"
#include "simutgw/db_oper/RecordNewOrderHelper.h"

#include "tool_redis/Tgw_RedisHelper.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"

#include "order/StockOrderHelper.h"

#include "config/conf_mysql_table.h"
#include "simutgw/stgw_config/g_values_inner.h"

ProcCancelOrder::ProcCancelOrder( void )
{
}

ProcCancelOrder::~ProcCancelOrder( void )
{

}

/*
�����ʳ���������������û�г�������
*/
int ProcCancelOrder::ProcSingleCancelOrder_Without_Request( std::shared_ptr<struct simutgw::OrderMessage>& in_cancelPtr )
{
	static const string strTag( "ProcCancelOrder::ProcCancelSuccess()" );

	std::shared_ptr<struct simutgw::OrderMessage> cancelPtr( new struct simutgw::OrderMessage );
	cancelPtr = in_cancelPtr;

	cancelPtr->strIsProc = "1";

	cancelPtr->enMatchType = simutgw::CancelMatch;

	cancelPtr->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

	cancelPtr->strExecType = simutgw::STEPMSG_EXECTYPE_CANCEL;

	cancelPtr->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_CANCEL;

	cancelPtr->strClordid = in_cancelPtr->strClordid;

	//cancelPtr->strOrigClordid = in_cancelOrder->strOrigClordid;

	cancelPtr->strLastPx = "0.0000";

	//Tgw_StringUtil::String2UInt64_strtoui64(in_cancelOrder->strLeavesQty, in_cancelOrder->ui64Orderqty_unmatched);
	Tgw_StringUtil::String2UInt64_strtoui64( cancelPtr->strOrderqty_origin, cancelPtr->ui64Orderqty_origin );

	//uint64_t uiValue;
	//uiValue = in_origPtr->ui64Orderqty_origin - in_origPtr->ui64Orderqty_unmatched;
	//cancelPtr->strLastQty = sof_string::itostr(uiValue, cancelPtr->strLastQty);

	cancelPtr->strLastQty = in_cancelPtr->strLeavesQty;

	cancelPtr->strCumQty = cancelPtr->strLastQty;

	cancelPtr->strLeavesQty = in_cancelPtr->strLeavesQty;

	cancelPtr->strCashorderqty = "0";

	// �����ڲ�������
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//Ψһִ��id
	cancelPtr->strExecID = strTransId;
	//Ψһ����id
	cancelPtr->strOrderID = strTransId;

	// д��ر�����
	int iRes = simutgw::g_outMsg_buffer.PushBack( cancelPtr );

	return iRes;
}