#include "OrderMemoryStoreFactory.h"

#include "tool_string/Tgw_StringUtil.h"

OrderMemoryStoreFactory::OrderMemoryStoreFactory(void)
{
}

OrderMemoryStoreFactory::~OrderMemoryStoreFactory(void)
{
}

// ��������
int OrderMemoryStoreFactory::InsertOrder(simutgw::ORDER_TYPE enumType,
										 std::shared_ptr<struct simutgw::OrderMessage> ptrObj)
{
	static const string ftag("OrderMemoryStoreFactory::InsertOrder()");

	int iRes = 0;
	switch (enumType)
	{
	case simutgw::ordtype_buy:
		iRes = m_storeBuyOrder.PushBack(ptrObj);
		break;

	case simutgw::ordtype_sell:
		iRes = m_storeSellOrder.PushBack(ptrObj);
		break;

	case simutgw::ordtype_error:
		iRes = m_storeErrorOrder.PushBack(ptrObj);
		break;

	case simutgw::ordtype_cancel:
		iRes = m_storeCancellOrder.PushBack(ptrObj);
		break;

	default:
		return -1;
	}

	return iRes;
}

// ȡ���������
size_t OrderMemoryStoreFactory::GetOrderDepth(simutgw::ORDER_TYPE enumType)
{
	static const string ftag("OrderMemoryStoreFactory::GetOrderDepth()");

	size_t iRes = 0;
	switch (enumType)
	{
	case simutgw::ordtype_buy:
		iRes = m_storeBuyOrder.GetSize();
		break;

	case simutgw::ordtype_sell:
		iRes = m_storeSellOrder.GetSize();
		break;

	case simutgw::ordtype_error:
		iRes = m_storeErrorOrder.GetSize();
		break;

	case simutgw::ordtype_cancel:
		iRes = m_storeCancellOrder.GetSize();
		break;

	default:
		return -1;
	}

	return iRes;
}

/*
ȡ������

Return :
0 -- �ɹ�ȡ��
1 -- ������
*/
int OrderMemoryStoreFactory::GetOrder(simutgw::ORDER_TYPE enumType,
									  std::shared_ptr<struct simutgw::OrderMessage>& ptrObj)
{
	static const string ftag("OrderMemoryStoreFactory::GetOrder()");

	int iRes = 0;
	switch (enumType)
	{
	case simutgw::ordtype_buy:
		iRes = m_storeBuyOrder.PopFront(ptrObj);
		break;

	case simutgw::ordtype_sell:
		iRes = m_storeSellOrder.PopFront(ptrObj);
		break;

	case simutgw::ordtype_error:
		iRes = m_storeErrorOrder.PopFront(ptrObj);
		break;

	case simutgw::ordtype_cancel:
		iRes = m_storeCancellOrder.PopFront(ptrObj);
		break;

	default:
		return -1;
	}

	return iRes;
}

/*
�ҳ���������������Ƿ��ǳ���

Return :
0 -- �ҳ������������ǳ���
1 -- ������
2 -- �ҳ������������ǳ���
-1 -- �Ƿ��Ĳ���
*/
int OrderMemoryStoreFactory::GetBSOrderAndCheckIfCancell(simutgw::ORDER_TYPE enumType,
									  std::shared_ptr<struct simutgw::OrderMessage>& out_ptrBSObj,
									  std::shared_ptr<struct simutgw::OrderMessage>& out_ptrCancellObj)
{
	static const string ftag("OrderMemoryStoreFactory::FindAndDelete()");

	int iRes = 0;
	switch (enumType)
	{
	case simutgw::ordtype_buy:
		iRes = m_storeBuyOrder.PopFront(out_ptrBSObj);
		break;

	case simutgw::ordtype_sell:
		iRes = m_storeSellOrder.PopFront(out_ptrBSObj);
		break;

	default:
		return -1;
	}

	if( 0 != iRes )
	{
		return iRes;
	}

	iRes = m_storeCancellOrder.FindCancellOrderAndDelete(out_ptrBSObj, out_ptrCancellObj);
	if ( 1 == iRes )
	{
		// 1 -- δ�ҵ�
		return 0;
	}
	else if( 0 == iRes )
	{
		// 0 -- ���ҵ���ɾ��
		return 2;
	}

	return iRes;
}

/*
�ҳ�������ԭʼ����

Return :
0 -- �ɹ�
1 -- δ�ҵ�
-1 -- �Ƿ��Ĳ���
*/
int OrderMemoryStoreFactory::GetOrigOrder(simutgw::ORDER_TYPE enumType,
				 std::shared_ptr<struct simutgw::OrderMessage>& in_ptrCancel,
				 std::shared_ptr<struct simutgw::OrderMessage>& out_ptrOrig)
{
	static const string strTag("OrderMemoryStoreFactory::GetDbUserData()");

	if (enumType != simutgw::ordtype_cancel)
	{
		EzLog::e(strTag, "Error enumType");

		return -1;
	}
	
	int iRes = m_storeCancellOrder.FindOrigOrderAndDelete(in_ptrCancel, out_ptrOrig);

	return iRes;
}

/*
��ȡ�ڴ��е����ݸſ�

Param :


Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int OrderMemoryStoreFactory::GetSurveyofStore( std::string& out_strSurvey )
{
	static const string ftag("OrderMemoryStoreFactory::GetSurveyofStore()");

	int iRes = 0;

	out_strSurvey = "\nBuy { ";
	// simutgw::ordtype_buy:
	std::string strStoreSurTmp("");
	iRes = m_storeBuyOrder.GetSurveyofStore(strStoreSurTmp);
	if( 0 != iRes )
	{
		EzLog::e(ftag, "GetSurveyofStore storeBuyOrder Error!");
		return -1;
	}

	out_strSurvey += strStoreSurTmp;
	out_strSurvey += " }\nSell { ";

	// simutgw::ordtype_sell:
	strStoreSurTmp = "";
	iRes = m_storeSellOrder.GetSurveyofStore(strStoreSurTmp);
	if( 0 != iRes )
	{
		EzLog::e(ftag, "GetSurveyofStore storeSellOrder Error!");
		return -1;
	}

	out_strSurvey += strStoreSurTmp;
	out_strSurvey += " }\nError { ";

	// simutgw::ordtype_error:
	strStoreSurTmp = "";
	iRes = m_storeErrorOrder.GetSurveyofStore(strStoreSurTmp);
	if( 0 != iRes )
	{
		EzLog::e(ftag, "GetSurveyofStore storeErrorOrder Error!");
		return -1;
	}

	out_strSurvey += strStoreSurTmp;
	out_strSurvey += " }\nCancel { ";

	// simutgw::ordtype_cancel:
	strStoreSurTmp = "";
	iRes = m_storeCancellOrder.GetSurveyofStore(strStoreSurTmp);
	if( 0 != iRes )
	{
		EzLog::e(ftag, "GetSurveyofStore storeCancellOrder Error!");
		return -1;
	}

	out_strSurvey += strStoreSurTmp;
	out_strSurvey += " }\n";

	return 0;
}

/*
��ȡ�ڴ��е���������

Param :
const string& in_strClOrdId : ����ѯ�Ķ�����

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int OrderMemoryStoreFactory::GetOrderDetail(simutgw::ORDER_TYPE enumType,
							const string& in_strClOrdId,
							std::string& out_strDetail  )
{
	static const string ftag("OrderMemoryStoreFactory::GetOrderDetail()");

	int iRes = 0;
	switch (enumType)
	{
	case simutgw::ordtype_buy:
		iRes = m_storeBuyOrder.GetOrderDetail(in_strClOrdId, out_strDetail);
		break;

	case simutgw::ordtype_sell:
		iRes = m_storeSellOrder.GetOrderDetail(in_strClOrdId, out_strDetail);
		break;

	case simutgw::ordtype_error:
		iRes = m_storeErrorOrder.GetOrderDetail(in_strClOrdId, out_strDetail);
		break;

	case simutgw::ordtype_cancel:
		iRes = m_storeCancellOrder.GetOrderDetail(in_strClOrdId, out_strDetail);
		break;

	default:
		return -1;
	}

	return iRes;
}