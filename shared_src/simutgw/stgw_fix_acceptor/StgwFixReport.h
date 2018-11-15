#ifndef __STGW_FIX_REPORT_H__
#define __STGW_FIX_REPORT_H__

#include <memory>

#include "StgwApplication.h"

#include "order/define_order_msg.h"

#include "util/EzLog.h"

// 取fix回报消息类
class StgwFixReport
{
	//
	// member
	//
protected:
	static src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	//
	// function
	//
public:
	StgwFixReport();
	virtual ~StgwFixReport();

	/*
	处理深圳回报业务
	Return:
	0 -- 成功
	-1 -- 失败
	1 -- 无回报
	*/
	static int Send_SZReport();

	/*
	发送深圳重复单业务拒绝消息
	*/
	static int Send_SZ_RepeatRejectMsg(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

private:
	/*
	处理一条回报

	Reutrn:
	0 -- 成功
	-1 -- 失败
	*/
	static int ProcSingleReport(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	处理深圳撤单

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int ProcSZCancelOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	处理深圳错误订单

	Return:
	0 -- 成功
	-1 -- 失败
	1 -- 无回报
	*/
	static int ProcSZErrorOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	处理深圳确认

	Return:
	0 -- 成功
	-1 -- 失败
	1 -- 无回报
	*/
	static int ProcSZConfirmOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	处理一条委托成功的执行报告回报,msgtype = 8
	*/
	static int ProcMsgType_8_Report(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	处理一条委托成功的执行报告回报,msgtype = 8
	按JSON配置规则

	Reutrn:
	0 -- 成功
	-1 -- 失败
	*/
	static int ProcMsgType_8_Confirm_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	处理一条委托成功的执行报告回报,msgtype = 8
	按JSON配置规则

	Reutrn:
	0 -- 成功
	1 -- 无回复消息
	-1 -- 失败
	*/
	static int ProcMsgType_8_Report_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& out_vctFixReport);

	// 增加回报NoPartyIds重复组
	static int AddNoPartyIds(FIX::Message& fixReport, const std::string& strSeat,
		const std::string& strAccount, const std::string& strMarket_branchid);

	// 增加回报NoSecurity重复组
	static int AddNoSecurity(FIX::Message& fixReport,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport);

	/*
	处理一条撤单失败成功的执行报告回报, msgtype = 9
	*/
	static int ProcMsgType_9_Report(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);
};

#endif