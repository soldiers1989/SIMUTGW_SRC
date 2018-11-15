#include "TaskRecordComponentMatch.h"

#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/db_oper/RecordMatchTableHelper.h"

TaskRecordComponentMatch::TaskRecordComponentMatch(const unsigned int uiId) :TaskBase(uiId)
{
	m_ptrReport.reset();
	m_ptrFrozeComponent.reset();
}


TaskRecordComponentMatch::~TaskRecordComponentMatch()
{
}

int TaskRecordComponentMatch::TaskProc(void)
{
	RecordCompnentMatchInfo();
	return 0;
}

/*
etf����ɷֹɼ�¼����ˮ��
*/
int TaskRecordComponentMatch::RecordCompnentMatchInfo()
{
	static const string strTag("RecordMatch::InsertMatchTable() ");

	if (NULL == m_ptrReport || NULL == m_ptrFrozeComponent)
	{
		EzLog::e(strTag, "ptrReport or ptrFrozeComponent is NULL");
		return -1;
	}


	//��mysql���ӳ�ȡ����
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (NULL == mysqlConn)
	{
		//ȡ����mysql����ΪNULL

		//�黹����
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		EzLog::e(strTag, "Get Connection is NULL");

		return -1;
	}

	mysqlConn->StartTransaction();

	//struct simutgw::OrderMessage stNewOrder = *m_ptrReport;
	std::shared_ptr<struct simutgw::OrderMessage> ptrNewOrder(new struct simutgw::OrderMessage(*m_ptrReport));

	// ���ȯ����
	ptrNewOrder->strStockID2 = m_ptrFrozeComponent->strSecurityID;
	ptrNewOrder->strOrderPrice = "0";

	string strTrans;
	uint64_t ui64Trans = m_ptrFrozeComponent->ui64act_pch_count + m_ptrFrozeComponent->ui64avl_count
		+ m_ptrFrozeComponent->ui64rdp_count;
	sof_string::itostr(ui64Trans, strTrans);
	ptrNewOrder->strOrderqty_origin = strTrans;
	ptrNewOrder->strLastQty = strTrans;
	ptrNewOrder->strLastPx = "0";
	ptrNewOrder->strCashorderqty = "0";
	ptrNewOrder->strCumQty = strTrans;
	ptrNewOrder->strLeavesQty = "0";

	int iRes = RecordMatchTableHelper::RecordMatchInfo(ptrNewOrder, mysqlConn);

	if (0 == iRes)
	{
		mysqlConn->Commit();
	}
	else
	{
		mysqlConn->RollBack();
	}
	//�黹����
	simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

	return iRes;
}