#ifndef __STGW_MSG_HELPER_H__
#define __STGW_MSG_HELPER_H__

#include "simutgw/stgw_fix_acceptor/StgwApplication.h"

/*
fix消息Helper
*/
class StgwMsgHelper
{
	//
	// member
	//

	//
	// function
	//
public:
	StgwMsgHelper();
	virtual ~StgwMsgHelper();

	// 登录
	static int ProcLogOn(const FIX::SessionID& sessionID);
	// 登出
	static int ProcLogOut(const FIX::SessionID& sessionID);
	// admin消息
	static int ProcAdminMsg(const FIX::Message& message,
		const FIX::SessionID& sessionID, FIX::Message& report);
	// app消息
	static int ProcAppMsg(const FIX::Message& message,
		const FIX::SessionID& sessionID, FIX::Message& report);

	/*
	回报 平台状态消息 Platform State Info
	*/
	static int Get_PlatformStateInfo(const FIX::SessionID& sessionID, FIX::Message& report);

	/*
	回报 平台信息消息 Platform Info
	*/
	static int Get_PlatformInfo(const FIX::SessionID& sessionID, FIX::Message& report);

private:
	/*
	回报同步消息
	*/
	static int ReportSynchronization(const FIX::Message& message);
};

#endif