#ifndef __ORDER_MEMORY_CELL_H__
#define __ORDER_MEMORY_CELL_H__

#include <memory>

#include "order/define_order_msg.h"
#include "cache/MemoryStoreCell.h"

class OrderMemoryCell : public MemoryStoreCell<std::shared_ptr<struct simutgw::OrderMessage>>
{
	//
	// Members
	//
protected:

	//
	// Functions
	//
public:
	OrderMemoryCell(void);

	virtual ~OrderMemoryCell(void);

	/*
	ʹ��ԭʼ�������ӳ����������ҵ���ɾ���ó�������

	Return :
	0 -- ���ҵ���ɾ��
	1 -- δ�ҵ�
	*/
	int FindCancellOrderAndDelete(const std::shared_ptr<struct simutgw::OrderMessage>& in_obj,
		std::shared_ptr<struct simutgw::OrderMessage>& out_selfObj);

	/*
	ʹ�ó��������µ��������ҵ���ɾ����ԭʼ����

	Return :
	0 -- ���ҵ���ɾ��
	1 -- δ�ҵ�
	*/
	int FindOrigOrderAndDelete(const std::shared_ptr<struct simutgw::OrderMessage>& in_cancelPtr,
		std::shared_ptr<struct simutgw::OrderMessage>& out_origPtr);

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
	int GetOrderDetail(const string& in_strClOrdId,
		std::string& out_strDetail  );


};

#endif