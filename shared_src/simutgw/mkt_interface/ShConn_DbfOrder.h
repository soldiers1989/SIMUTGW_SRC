#ifndef __SH_CONN_DBF_ORDER_H__
#define __SH_CONN_DBF_ORDER_H__

#include "tool_json/RapidJsonHelper_tgw.h"

#include "order/define_order_msg.h"

/*
处理上海sqlserver接口类
从ashare_ordwth取下单数据，回写status字段，落地到本地mysql数据库
写ashare_cjhb表，转换本地order_report表字段到ashare_cjhb表字段
*/
class ShConn_DbfOrder
{
	/*
	member
	*/
private:

	/*
	function
	*/
public:
	virtual ~ShConn_DbfOrder( void );

	/*
	从ashare_ordwth取委托
	*/
	static int BatchGetSHOrder(std::vector<std::shared_ptr<struct simutgw::OrderMessage>> &io_vecOrder);


	// 校验下单消息，记录入数据库
	static int Valide_Record_Order( std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg );

	/*
	修改ordwth表的status状态为P
	*/
	static int SetOrdwthStatus( const std::string& in_strName,
		const std::string &in_strRec_num, const std::string &in_strStatus );

	/*
	修改ordwth表的一批记录的status状态为P
	*/
	static int Set_MutilOrd_Status( const std::string& in_strShConnName,
		const vector<std::string> &in_VecRec_num );

private:
	// 禁止使用默认构造函数
	ShConn_DbfOrder( void );

	/*
	取上海成交编号
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int GetSHCjbh( const std::string& in_strShConnName );

	/*
	取上海确认编号
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int GetSHRec_Num( const std::string& in_strShConnName );

	/*
	取一个上海接口的委托
	*/
	static int GetOneConnSHOrder( const std::string& in_strShConnName,
		std::vector<std::shared_ptr<struct simutgw::OrderMessage>> &io_vecOrder );
};

#endif