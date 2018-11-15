#ifndef __STGW_MSG_HELPER_H__
#define __STGW_MSG_HELPER_H__

#include "simutgw/stgw_fix_acceptor/StgwApplication.h"

/*
fix��ϢHelper
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

	// ��¼
	static int ProcLogOn(const FIX::SessionID& sessionID);
	// �ǳ�
	static int ProcLogOut(const FIX::SessionID& sessionID);
	// admin��Ϣ
	static int ProcAdminMsg(const FIX::Message& message,
		const FIX::SessionID& sessionID, FIX::Message& report);
	// app��Ϣ
	static int ProcAppMsg(const FIX::Message& message,
		const FIX::SessionID& sessionID, FIX::Message& report);

	/*
	�ر� ƽ̨״̬��Ϣ Platform State Info
	*/
	static int Get_PlatformStateInfo(const FIX::SessionID& sessionID, FIX::Message& report);

	/*
	�ر� ƽ̨��Ϣ��Ϣ Platform Info
	*/
	static int Get_PlatformInfo(const FIX::SessionID& sessionID, FIX::Message& report);

private:
	/*
	�ر�ͬ����Ϣ
	*/
	static int ReportSynchronization(const FIX::Message& message);
};

#endif