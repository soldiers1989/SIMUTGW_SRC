#include "StgwMsgHelper.h"

#include "simutgw/stgw_fix_acceptor/SZ_STEP_OrderHelper.h"
#include "simutgw/stgw_fix_acceptor/StgwMockerResponse.h"

#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/stgw_config/g_values_biz.h"
#include "simutgw_config/g_values_sys_run_config.h"

#include "util/EzLog.h"

using namespace FIX;

StgwMsgHelper::StgwMsgHelper()
{
}


StgwMsgHelper::~StgwMsgHelper()
{
}

// ��¼
int StgwMsgHelper::ProcLogOn(const FIX::SessionID& sessionID)
{
	static const std::string ftag("StgwMsgHelper::ProcLogOn() ");

	std::string strSenderID = sessionID.getTargetCompID();
	std::string strTargetID = sessionID.getSenderCompID();
	simutgw::g_mapSzConns[strSenderID].LogOn();
	simutgw::g_mapSzConns[strSenderID].SetSenderID(strSenderID);
	simutgw::g_mapSzConns[strSenderID].SetTargetID(strTargetID);

	// Add PartitionNo
	std::shared_ptr<std::map<int, uint64_t>> ptrMap = simutgw::g_mapSzConns[strSenderID].GetPartitionsMap();

	(*ptrMap)[1] = 0;
	(*ptrMap)[2] = 0;

	// �������Ӽ�����
	// ʹ�� TargetCompID Ϊ��ʶ
	simutgw::g_counter.AddSz_LinkCounter(strSenderID);

	string strInfo("client=");
	strInfo += strSenderID;
	strInfo += " logon!";
	EzLog::i(ftag, strInfo);

	return 0;
}

// �ǳ�
int StgwMsgHelper::ProcLogOut(const FIX::SessionID& sessionID)
{
	static const std::string ftag("StgwMsgHelper::ProcLogOut() ");

	string strSenderID = sessionID.getTargetCompID();
	// ��������
	if (simutgw::g_mapSzConns.end() != simutgw::g_mapSzConns.find(strSenderID))
	{
		simutgw::g_mapSzConns[strSenderID].LogOut();

		string strInfo("client=");
		strInfo += strSenderID;
		strInfo += " logout!";
		EzLog::i(ftag, strInfo);

		return 0;
	}

	return -1;
}

// admin��Ϣ
int StgwMsgHelper::ProcAdminMsg(const FIX::Message& message,
	const FIX::SessionID& sessionID, FIX::Message& report)
{
	static const std::string ftag("StgwMsgHelper::ProcAdminMsg() ");

	bool bhas_msgtype = message.getHeader().isSetField(FIX::FIELD::MsgType);
	if (!bhas_msgtype)
	{
		EzLog::e(ftag, "[msgtype] ��ʧ");
		return -1;
	}

	const std::string strMsgType = message.getHeader().getField(FIELD::MsgType);

	if (strMsgType == MsgType_Heartbeat)
	{
		// ����
	}
	else if (strMsgType == MsgType_Logout)
	{
		//�ǳ�
	}
	else if (strMsgType == MsgType_Logon)
	{
		//��¼
	}
	else
	{
		EzLog::i(ftag, strMsgType);
	}

	return 0;
}

// app��Ϣ
int StgwMsgHelper::ProcAppMsg(const FIX::Message& message,
	const FIX::SessionID& sessionID, FIX::Message& report)
{
	static const std::string ftag("StgwMsgHelper::ProcAppMsg() ");

	bool bhas_msgtype = message.getHeader().isSetField(FIX::FIELD::MsgType);
	if (!bhas_msgtype)
	{
		EzLog::e(ftag, "[msgtype] ��ʧ");
		return -1;
	}

	const std::string strMsgType = message.getHeader().getField(FIELD::MsgType);

	/*
	std::string strApplId;
	if (message.isSetField(FIELD::ApplID))
	{
	strApplId = message.getField(FIELD::ApplID);
	}

	if (simutgw::MiniMode == simutgw::g_iRunMode &&
	simutgw::g_bEnable_Json_Mocker &&
	0 != strMsgType.compare(FIX::MsgType_ReportSynchronization))
	{
	// ����ģʽ������json�ļ����ûر�
	// ��������json�ļ����лر�
	return StgwMockerResponse::Response(message);
	}
	*/
	if (strMsgType == MsgType_ReportSynchronization)
	{
		// �ر�ͬ����Ϣ
		ReportSynchronization(message);
	}
	else if (0 == strMsgType.compare(FIX::MsgType_NewOrderSingle) ||
		0 == strMsgType.compare(FIX::MsgType_OrderCancelRequest))
	{
		// �¶��� �� ����
		std::shared_ptr<struct simutgw::OrderMessage> ptrorder(new struct simutgw::OrderMessage);
		int iRes = SZ_STEP_OrderHelper::NewOrder2Struct(message, ptrorder);
		if (0 != iRes)
		{
			EzLog::e(ftag, "msg to struct ת��ʧ��");
			return -1;
		}

		simutgw::g_inMsg_buffer.PushBack(ptrorder);

		// ��¼��·����
		simutgw::g_counter.IncSz_Link_Receive(sessionID.getTargetCompID());

		// �����ڲ�����
		simutgw::g_counter.GetSz_InnerCounter()->Inc_Received();

	}
	else
	{
		string strError("δ֪��MsgType[");
		strError += strMsgType;
		strError += "]";

		EzLog::e(ftag, strError);

		return -1;
	}

	return 0;
}

/*
�ر� ƽ̨״̬��Ϣ Platform State Info
*/
int StgwMsgHelper::Get_PlatformStateInfo(const FIX::SessionID& sessionID, FIX::Message& report)
{
	static const std::string ftag("StgwMsgHelper::Get_PlatformStateInfo() ");

	FIX::MsgType fix_msgtype(FIX::MsgType_PlatformStateInfo);

	//
	report.getHeader().setField(fix_msgtype);

	string strSenderCompID = sessionID.getTargetCompID();
	string strTargetCompID = sessionID.getSenderCompID();	

	size_t stFind = strSenderCompID.find("applid_120");
	// PlatformID
	if (std::string::npos != stFind)
	{
		// 3=�ǽ��״���ƽ̨
		report.setField(FIELD::PlatformID, "3");

		string sDebug("PlatformStateInfo Is 3 to SenderCompID=");
		sDebug += strSenderCompID;
		sDebug += ", TargetCompID=";
		sDebug += strTargetCompID;
		EzLog::i(ftag, sDebug);
	}
	else
	{
		// 1=�ֻ����о��۽���ƽ̨
		report.setField(FIELD::PlatformID, "1");

		string sDebug("PlatformStateInfo Is 1 to SenderCompID=");
		sDebug += strSenderCompID;
		sDebug += ", TargetCompID=";
		sDebug += strTargetCompID;
		EzLog::i(ftag, sDebug);
	}

	//PlatformStatus
	report.setField(FIELD::PlatformStatus, "2");

	return 0;
}

/*
�ر� ƽ̨��Ϣ��Ϣ Platform Info
*/
int StgwMsgHelper::Get_PlatformInfo(const FIX::SessionID& sessionID, FIX::Message& report)
{
	static const std::string ftag("StgwMsgHelper::Get_PlatformInfo() ");

	FIX::MsgType fix_msgtype(FIX::MsgType_PlatformInfo);

	//
	report.getHeader().setField(fix_msgtype);

	string strSenderCompID = sessionID.getTargetCompID();
	string strTargetCompID = sessionID.getSenderCompID();

	size_t stFind = strSenderCompID.find("applid_120");
	// PlatformID
	if (std::string::npos != stFind)
	{
		// 3=�ǽ��״���ƽ̨
		report.setField(FIELD::PlatformID, "3");
	}
	else
	{
		// 1=�ֻ����о��۽���ƽ̨
		report.setField(FIELD::PlatformID, "1");
	}

	std::shared_ptr<std::map<int, uint64_t>> prtMapPati;

	// ��������
	if (simutgw::g_mapSzConns.end() != simutgw::g_mapSzConns.find(strSenderCompID))
	{
		prtMapPati = simutgw::g_mapSzConns[strSenderCompID].GetPartitionsMap();
	}
	else
	{
		string sDebug("No connection in PlatformInfo to SenderCompID=");
		sDebug += strSenderCompID;
		sDebug += ", TargetCompID=";
		sDebug += strTargetCompID;

		EzLog::e(ftag, sDebug);

		return -1;
	}

	if (nullptr == prtMapPati)
	{
		string sDebug("nullptr in PlatformInfo to SenderCompID=");
		sDebug += strSenderCompID;
		sDebug += ", TargetCompID=";
		sDebug += strTargetCompID;

		EzLog::e(ftag, sDebug);

		return -1;
	}

	FIX::FieldMap& reportMap = static_cast<FIX::FieldMap&>(report);
	
	string strItoa;
	std::map<int, uint64_t>::iterator it = prtMapPati->begin();
	for (; prtMapPati->end() != it; ++it)
	{
		FIX::Group group(FIX::FIELD::NoPartitions, FIX::FIELD::PartitionNo);
		sof_string::itostr(it->first, strItoa);
		group.setField(FIX::FIELD::PartitionNo, strItoa);
		reportMap.addGroup(FIX::FIELD::NoPartitions, group);
	}

	//PlatformStatus
	report.setField(FIELD::PlatformStatus, "2");

	return 0;
}

/*
�ر�ͬ����Ϣ
*/
int StgwMsgHelper::ReportSynchronization(const FIX::Message& message)
{
	static const std::string ftag("StgwMsgHelper::ReportSynchronization() ");

	uint64_t ui64Num = 0;
	std::string strReportIndex;
	if (simutgw::g_bSZ_Step_ver110)
	{
		const FIX::FieldMap& messageMap = static_cast<const FIX::FieldMap&>(message);

		// ����STEP�ر���Ver1.10
		if (!messageMap.hasGroup(FIX::FIELD::NoPartitions))
		{
			// û���ظ���
			string strDebug("no group NoPartitions");
			EzLog::e(ftag, strDebug);
			return -1;
		}

		// �ظ���Ԫ�ظ���
		size_t num = messageMap.groupCount(FIX::FIELD::NoPartitions);

		std::string strPartitionNo;
		for (int i = 1; i <= num; ++i)
		{
			FIX::FieldMap group;
			messageMap.getGroup(i, FIX::FIELD::NoPartitions, group);
			if (group.isEmpty())
			{
				continue;
			}

			if (group.isSetField(FIX::FIELD::PartitionNo))
			{
				strPartitionNo = group.getField(FIX::FIELD::PartitionNo);
			}

			if (group.isSetField(FIX::FIELD::ReportIndex))
			{
				strReportIndex = group.getField(FIX::FIELD::ReportIndex);
			}
		
		}
	}
	else
	{
		if (!message.isSetField(FIX::FIELD::ReportIndex))
		{
			// ȱ���ֶ�
			string strDebug("no field ReportIndex");
			EzLog::e(ftag, strDebug);
			return -1;
		}

		strReportIndex = message.getField(FIX::FIELD::ReportIndex);
	}

	int iRes = Tgw_StringUtil::String2UInt64_strtoui64(strReportIndex, ui64Num);
	if (iRes < 0)
	{
		//ת��ʧ��
		string strDebug("ReportIndex to int failed");
		EzLog::e(ftag, strDebug);

		return -1;
	}

	if (!message.getHeader().isSetField(FIX::FIELD::TargetCompID))
	{
		// ȱ���ֶ�
		string strDebug("no field TargetCompID");
		EzLog::e(ftag, strDebug);

		return -1;
	}

	string strSenderCompID = message.getHeader().getField(FIX::FIELD::SenderCompID);
	// ��������
	if (simutgw::g_mapSzConns.end() != simutgw::g_mapSzConns.find(strSenderCompID))
	{

		simutgw::g_mapSzConns[strSenderCompID].SetRptIdx(ui64Num);
	}

	string strReciveLog("Recived SZ Report synchronization msg, client=");
	strReciveLog += strSenderCompID;

	EzLog::i(ftag, strReciveLog);

	return 0;
}
