#ifndef __APPID_120_ORDER_VALIDE_H__
#define __APPID_120_ORDER_VALIDE_H__

#include "order/define_order_msg.h"

/*
业务消息检查
处理ApplID为120的业务
Etf交易
*/
class AppId_120_OrderValide
{
	//
	// member
	//
private:

	//
	// function
	//
public:
	virtual ~AppId_120_OrderValide( void );

	/*
	交易前检查函数
	Return:
	0 -- 合法
	-1 -- 不合法
	*/
	static int ValidateOrder( std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg );

protected:


private:
	// 禁止使用默认构造函数
	AppId_120_OrderValide( void );
};

#endif