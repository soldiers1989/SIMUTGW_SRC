#ifndef __APPID_010_ORDER_VALIDE_H__
#define __APPID_010_ORDER_VALIDE_H__

#include "order/define_order_msg.h"

/*
业务消息检查
	处理ApplID为010的业务
	现货（股票，基金，债券等）集中竞价交易申报
*/
class AppId_010_OrderValide 
{
	//
	// member
	//
protected:

	//
	// function
	//
public:	
	virtual ~AppId_010_OrderValide();

	/*
	交易前检查函数
	Return:
	0 -- 合法
	-1 -- 不合法
	*/
	static int ValidateOrder( std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg );
	
private:
	// 禁止使用默认构造函数
	AppId_010_OrderValide( void );

	/*
	交易前检查函数
	做一些基本检查
	Return:
	0 -- 合法
	-1 -- 不合法
	*/
	static int BasicValidate( std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg );

	/*
	取得申报委托的类型

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int GetOrder_DEL_TYPE( std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg );
};

#endif