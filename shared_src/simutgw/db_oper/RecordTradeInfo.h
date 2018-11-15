#ifndef __RECORD_TRADE_INFO_H__
#define __RECORD_TRADE_INFO_H__

#include "order/define_order_msg.h"
#include "tool_mysql/MySqlCnnC602.h"

class RecordTradeInfo
{
public:
	RecordTradeInfo();
	virtual ~RecordTradeInfo();

	/*
		function
	*/
public:
	/*
	向数据库中写入成交的数据
	成交
	Param :

	Return :
	0 -- 写入成功
	非0 -- 写入失败
	*/
	static int WriteTransInfoInDb(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport);

	/*
	向数据库中写入成交的数据
	撤单成功
	Param :

	Return :
	0 -- 写入成功
	非0 -- 写入失败
	*/
	static int WriteTransInfoInDb_CancelSuccess(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport);

	/*
	向数据库中写入成交的数据
	撤单失败
	Param :

	Return :
	0 -- 写入成功
	非0 -- 写入失败
	*/
	static int WriteTransInfoInDb_CancelFail(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport);

	/*
	向数据库中写入成交的数据
	废单
	Param :

	Return :
	0 -- 写入成功
	非0 -- 写入失败
	*/
	static int WriteTransInfoInDb_Error(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport);

	/*
	更新order_record表字段
	Param :

	Return :
	0 -- 写入成功
	非0 -- 写入失败
	*/
	static int UpdateRecordTable(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);
};

#endif