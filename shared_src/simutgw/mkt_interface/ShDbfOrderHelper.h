#ifndef __SH_DBF_ORDER_ORDER_HELPER_H__
#define __SH_DBF_ORDER_ORDER_HELPER_H__

#include "tool_json/RapidJsonHelper_tgw.h"

#include "order/define_order_msg.h"

#include "tool_odbc/OTLConn40240.h"

/*
处理上海sqlserver接口类
从ashare_ordwth取下单数据，回写status字段，落地到本地mysql数据库
写ashare_cjhb表，转换本地order_report表字段到ashare_cjhb表字段
*/
class ShDbfOrderHelper
{
	/*
	member
	*/
private:

	/*
	function
	*/

public:
	virtual ~ShDbfOrderHelper( void );

	/*
	上海委托数据转换成FIX协议格式
	*/
	static int SHOrderToFix(OTLConn40240& otlConn, otl_stream& in_stream,
		std::vector<std::shared_ptr<struct simutgw::OrderMessage>> &io_vecOrder,
		const std::string &strSessionid, const struct TradePolicyCell& in_policy );

private:
	// 禁止使用默认构造函数
	ShDbfOrderHelper( void );


	/*
	检查上海委托是否是支持的业务类型

	Return:
	0 -- 支持
	-1 -- 不支持
	*/
	static int Validate_SH_Order( std::shared_ptr<struct simutgw::OrderMessage> &shOrder );

};

#endif