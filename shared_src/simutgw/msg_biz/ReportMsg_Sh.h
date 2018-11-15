#ifndef __REPORT_MSG_SH_H__
#define __REPORT_MSG_SH_H__

#include <memory>

#include "order/define_order_msg.h"

#include "util/EzLog.h"

/*
	处理从回报队列中读出的回报类，
	主要是更新表和返回上海sql串
	*/
class ReportMsg_Sh
{
	/*
	member
	*/


	/*
	function
	*/
public:
	ReportMsg_Sh();
	virtual ~ReportMsg_Sh();

protected:
	static src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

private:
	/*
	处理一条回报

	Reutrn:
	0 -- 成功
	-1 -- 失败
	*/
	static int ProcSingleReport(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);


	//----------------------------------上海begin-------------------------------------
	/*
	FIX协议格式数据转换成FIX协议格式上海确认，返回一个sql串

	Reutrn:
	0 -- 成功
	-1 -- 失败
	*/
	static int FixToSHConfirm(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	/*
	FIX协议格式数据 按JSON配置规则 转换成FIX协议格式上海确认，返回一个sql串

	Reutrn:
	0 -- 成功
	-1 -- 失败
	*/
	static int FixToSHConfirm_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	/*
	FIX协议格式数据转换成FIX协议格式上海回报，返回一个sql串

	Reutrn:
	0 -- 成功
	-1 -- 失败
	*/
	static int FixToSHReport(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	/*
	FIX协议格式数据 按JSON配置规则 转换成FIX协议格式上海回报，返回一个sql串

	Reutrn:
	0 -- 成功
	-1 -- 失败
	*/
	static int FixToSHReport_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	//----------------------------------上海end-------------------------------------

	/*
	处理上海撤单

	@param std::string& out_strSql_confirm : 写入确认表的数据

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int ProcSHCancelOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	/*
	处理上海错误订单

	Return:
	0 -- 成功
	-1 -- 失败
	1 -- 无回报
	*/
	static int ProcSHErrorOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	/*
	处理上海确认

	Return:
	0 -- 成功
	-1 -- 失败
	1 -- 无回报
	*/
	static int ProcSHConfirmOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

public:
	/*
		处理上海回报业务
		Return:
		0 -- 成功
		-1 -- 失败
		1 -- 无回报
		*/
	static int Get_SHReport(map<string, vector<string>>& out_mapUpdate);
};

#endif