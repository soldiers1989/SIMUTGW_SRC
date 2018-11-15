#ifndef __GEN_TASK_HELPER_H__
#define __GEN_TASK_HELPER_H__

#include <memory>

/*
生成task的类
*/
#include "simutgw_flowwork/FlowWorkBase.h"

#include "order/define_order_msg.h"

class GenTaskHelper
{
	//
	// member
	//
private:

	//
	// function
	//
public:
	virtual ~GenTaskHelper( void );

	/*
	生成待验证消息

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	static int GenTask_Valid( std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrderMsg );

	/*
	生成待成交消息

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	static int GenTask_Match( std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrderMsg );

private:
	// 禁止使用默认构造函数
	GenTaskHelper( void );
};

#endif