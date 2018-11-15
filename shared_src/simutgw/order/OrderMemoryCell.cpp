#include "OrderMemoryCell.h"

#include "order/StockOrderHelper.h"

OrderMemoryCell::OrderMemoryCell(void)
{
}

OrderMemoryCell::~OrderMemoryCell(void)
{
}

/*
使用原始订单，从撤单队列中找到并删除该撤单数据

Return :
0 -- 已找到并删除
1 -- 未找到
*/
int OrderMemoryCell::FindCancellOrderAndDelete(const std::shared_ptr<struct simutgw::OrderMessage>& in_obj,
											   std::shared_ptr<struct simutgw::OrderMessage>& out_selfObj)
{
	static const string ftag("OrderMemoryCell::FindAndDelete()");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	std::deque<std::shared_ptr<struct simutgw::OrderMessage>>::iterator itStore;

	for( itStore = m_listStore.begin(); itStore != m_listStore.end(); ++itStore )
	{
		if( 0 == (*itStore)->strOrigClordid.compare(in_obj->strClordid)
			&& 0 == (*itStore)->strMarket.compare(in_obj->strMarket)
			&& 0 == (*itStore)->strSessionId.compare(in_obj->strSessionId) )
		{
			out_selfObj = (*itStore);

			m_listStore.erase(itStore);

			return 0;
		}
	}

	return 1;
}

/*
使用撤单，从下单队列中找到并删除该原始订单

Return :
0 -- 已找到并删除
1 -- 未找到
*/
int OrderMemoryCell::FindOrigOrderAndDelete(const std::shared_ptr<struct simutgw::OrderMessage>& in_cancelPtr,
						   std::shared_ptr<struct simutgw::OrderMessage>& out_origPtr)
{
	static const string ftag("OrderMemoryCell::FindOrigOrderAndDelete()");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	std::deque<std::shared_ptr<struct simutgw::OrderMessage>>::iterator itStore;

	for( itStore = m_listStore.begin(); itStore != m_listStore.end(); ++itStore )
	{
		if( 0 == (*itStore)->strClordid.compare(in_cancelPtr->strOrigClordid)
			&& 0 == (*itStore)->strMarket.compare(in_cancelPtr->strMarket)
			&& 0 == (*itStore)->strSessionId.compare(in_cancelPtr->strSessionId) )
		{
			out_origPtr = (*itStore);

			m_listStore.erase(itStore);

			return 0;
		}
	}

	return 1;
}

/*
获取内存中的数据概况

Param :


Return :
0 -- 成功
-1 -- 失败
*/
int OrderMemoryCell::GetSurveyofStore( std::string& out_strSurvey )
{
	static const string ftag("OrderMemoryCell::GetSurveyofStore()");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	string strTran;
	out_strSurvey = "size=";
	out_strSurvey += sof_string::itostr(m_listStore.size() , strTran);

	out_strSurvey += " [";

	std::deque<std::shared_ptr<struct simutgw::OrderMessage>>::const_iterator itStore;

	for( itStore = m_listStore.begin(); itStore != m_listStore.end(); ++itStore )
	{
		out_strSurvey += (*itStore)->strClordid;
		out_strSurvey += ", ";
	}

	out_strSurvey += "]";

	return 0;
}

/*
获取内存中的数据详情

Param :
const string& in_strClOrdId : 待查询的订单号

Return :
0 -- 成功
-1 -- 失败
*/
int OrderMemoryCell::GetOrderDetail(const string& in_strClOrdId,
				   std::string& out_strDetail  )
{
	static const string ftag("OrderMemoryCell::GetOrderDetail()");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	string strSingleOrder;
	out_strDetail = "";

	std::deque<std::shared_ptr<struct simutgw::OrderMessage>>::const_iterator itStore;

	for( itStore = m_listStore.begin(); itStore != m_listStore.end(); ++itStore )
	{
		if( 0 == (*itStore)->strClordid.compare(in_strClOrdId) )
		{
			out_strDetail += StockOrderHelper::OrderToScreenOut(*itStore, strSingleOrder);
		}
	}

	return 0;
}