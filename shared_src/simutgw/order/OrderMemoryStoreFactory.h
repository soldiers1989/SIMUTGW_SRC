#ifndef __ORDER_MEMORY_STORE_HELPER_H__
#define __ORDER_MEMORY_STORE_HELPER_H__

#include <memory>

#include "OrderMemoryCell.h"
#include "order/define_order_msg.h"


/*
内存订单数据存储类
*/
class OrderMemoryStoreFactory
{
	//
	// Members
	//
protected:
	// 买单
	OrderMemoryCell m_storeBuyOrder;

	// 卖单
	OrderMemoryCell m_storeSellOrder;

	// 错单
	OrderMemoryCell m_storeErrorOrder;

	// 撤单
	OrderMemoryCell m_storeCancellOrder;

	//
	// Functions
	//
public:
	OrderMemoryStoreFactory(void);
	virtual ~OrderMemoryStoreFactory(void);

	// 插入数据
	int InsertOrder(simutgw::ORDER_TYPE enumType,
		std::shared_ptr<struct simutgw::OrderMessage> ptrObj);

	// 取出队列深度
	size_t GetOrderDepth(simutgw::ORDER_TYPE enumType);

	/*
	取出数据

	Return :
	0 -- 成功取出
	1 -- 无数据
	*/
	int GetOrder(simutgw::ORDER_TYPE enumType,
		std::shared_ptr<struct simutgw::OrderMessage>& ptrObj);

	/*
	找出买卖单，并检查是否是撤单

	Return :
	0 -- 找出买卖单并不是撤单
	1 -- 无数据
	2 -- 找出买卖单但是是撤单
	-1 -- 非法的参数
	*/
	int GetBSOrderAndCheckIfCancell(simutgw::ORDER_TYPE enumType,
		std::shared_ptr<struct simutgw::OrderMessage>& out_ptrBSObj,
		std::shared_ptr<struct simutgw::OrderMessage>& out_ptrCancellObj);

	/*
	找出撤单的原始订单

	Return :
	0 -- 成功
	1 -- 未找到
	-1 -- 非法的参数
	*/
	int GetOrigOrder(simutgw::ORDER_TYPE enumType,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrCancel,
		std::shared_ptr<struct simutgw::OrderMessage>& out_ptrOrig);

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
	int GetOrderDetail(simutgw::ORDER_TYPE enumType,
		const string& in_strClOrdId,
		std::string& out_strDetail  );
};

#endif