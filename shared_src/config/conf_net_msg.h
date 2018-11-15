#ifndef __CONF_NET_MSG_H__
#define __CONF_NET_MSG_H__

#include <vector>
#include <string>
#include "boost/thread/mutex.hpp"

#include "tool_net/NetPackage.h"

namespace simutgw
{
	using namespace std;

	enum NET_MSG_TYPE
	{
		// Heartbeat ������Ϣ
		Heartbeat = 0,
		// Logon ��¼��Ϣ
		Logon = 10,
		// TestRequest ����������Ϣ
		TestRequest = 1,
		// Resend �ط�������Ϣ
		Resend = 2,
		// Reject �Ự�ܾ���Ϣ
		Reject = 3,
		// SeqReset ���������Ϣ
		SeqReset = 4,
		// Logout ע����Ϣ
		Logout = 5,
		// Execution ִ��
		Order = 7,
		// Execution Report ִ�б���
		Report = 8
	};

	/*
	socket��¼��Ϣ
	*/
	struct SocketUser
	{
		// 
		string strMsgtype;

		string strVersion;

		// �˺�
		string strAccountid;

		// ����
		string strPwd;

		//
		string strText;
	};

	/*
	socket��¼�û�����Ϣ
	*/
	struct SockerUserInfo
	{
		// �˺�
		string strAccountid;

		// ��¼��־
		bool bLogon;

		// ��һ��������Ϣ���
		uint64_t ui64NextSendSeq;

		SockerUserInfo()
		{
			bLogon = false;
			ui64NextSendSeq = 0;
		}
	};

	/*
	socket�û�����Ϣ����
	������
	δ�ְ����ֺ��ѷְ�����
	*/
	struct SocketUserMessage
	{
		boost::mutex msgMutex;

		// δ�ְ�����
		std::vector<uint8_t> vecRevBuffer;

		// �ѷְ�����
		std::vector<std::shared_ptr<simutgw::NET_PACKAGE>> vecRevDatas;
	};

	/*
		�յ���������Ϣ
	*/
	struct SocketMsgInfo
	{
		uint64_t ui64MsgType;
		uint64_t ui64ApplID;
		uint64_t ui64SeqNum;
		uint64_t ui64Add_Balance;
		uint64_t ui64Add_HK_Balance;
		uint64_t ui64Add_US_Balance;
		uint64_t ui64Add_Quantity;
		std::string strMsgType;
		std::string strAccount;
		std::string strTradeGroup;
		std::string strSeat;
		std::string strTrader;
		std::string strAdd_Balance;
		std::string strAdd_HK_Balance;
		std::string strAdd_US_Balance;
		std::string strFileType;
		std::string strFilePath;

		std::string strSecurityID;
		std::string strQuantity;

		std::string strCmdValue;
	};
}

#endif