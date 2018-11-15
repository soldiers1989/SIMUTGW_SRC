#ifndef __ORDER_MEMORY_STORE_HELPER_H__
#define __ORDER_MEMORY_STORE_HELPER_H__

#include <memory>

#include "OrderMemoryCell.h"
#include "order/define_order_msg.h"


/*
�ڴ涩�����ݴ洢��
*/
class OrderMemoryStoreFactory
{
	//
	// Members
	//
protected:
	// ��
	OrderMemoryCell m_storeBuyOrder;

	// ����
	OrderMemoryCell m_storeSellOrder;

	// ��
	OrderMemoryCell m_storeErrorOrder;

	// ����
	OrderMemoryCell m_storeCancellOrder;

	//
	// Functions
	//
public:
	OrderMemoryStoreFactory(void);
	virtual ~OrderMemoryStoreFactory(void);

	// ��������
	int InsertOrder(simutgw::ORDER_TYPE enumType,
		std::shared_ptr<struct simutgw::OrderMessage> ptrObj);

	// ȡ���������
	size_t GetOrderDepth(simutgw::ORDER_TYPE enumType);

	/*
	ȡ������

	Return :
	0 -- �ɹ�ȡ��
	1 -- ������
	*/
	int GetOrder(simutgw::ORDER_TYPE enumType,
		std::shared_ptr<struct simutgw::OrderMessage>& ptrObj);

	/*
	�ҳ���������������Ƿ��ǳ���

	Return :
	0 -- �ҳ������������ǳ���
	1 -- ������
	2 -- �ҳ������������ǳ���
	-1 -- �Ƿ��Ĳ���
	*/
	int GetBSOrderAndCheckIfCancell(simutgw::ORDER_TYPE enumType,
		std::shared_ptr<struct simutgw::OrderMessage>& out_ptrBSObj,
		std::shared_ptr<struct simutgw::OrderMessage>& out_ptrCancellObj);

	/*
	�ҳ�������ԭʼ����

	Return :
	0 -- �ɹ�
	1 -- δ�ҵ�
	-1 -- �Ƿ��Ĳ���
	*/
	int GetOrigOrder(simutgw::ORDER_TYPE enumType,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrCancel,
		std::shared_ptr<struct simutgw::OrderMessage>& out_ptrOrig);

	/*
	��ȡ�ڴ��е����ݸſ�

	Param :


	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int GetSurveyofStore( std::string& out_strSurvey );

	/*
	��ȡ�ڴ��е���������

	Param :
	const string& in_strClOrdId : ����ѯ�Ķ�����

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int GetOrderDetail(simutgw::ORDER_TYPE enumType,
		const string& in_strClOrdId,
		std::string& out_strDetail  );
};

#endif