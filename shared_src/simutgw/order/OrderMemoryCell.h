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
	使用原始订单，从撤单队列中找到并删除该撤单数据

	Return :
	0 -- 已找到并删除
	1 -- 未找到
	*/
	int FindCancellOrderAndDelete(const std::shared_ptr<struct simutgw::OrderMessage>& in_obj,
		std::shared_ptr<struct simutgw::OrderMessage>& out_selfObj);

	/*
	使用撤单，从下单队列中找到并删除该原始订单

	Return :
	0 -- 已找到并删除
	1 -- 未找到
	*/
	int FindOrigOrderAndDelete(const std::shared_ptr<struct simutgw::OrderMessage>& in_cancelPtr,
		std::shared_ptr<struct simutgw::OrderMessage>& out_origPtr);

	/*
	获取内存中的数据概况

	Param :


	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int GetSurveyofStore( std::string& out_strSurvey );

	/*
	获取内存中的数据详情

	Param :
	const string& in_strClOrdId : 待查询的订单号

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int GetOrderDetail(const string& in_strClOrdId,
		std::string& out_strDetail  );


};

#endif