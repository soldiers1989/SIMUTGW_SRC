#ifndef __CLIENTER_H__
#define __CLIENTER_H__

#include <string>

#include "boost/thread/mutex.hpp"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#ifdef _MSC_VER
#include "simutgw/win_iocp_simutgw_client/ClientHandler.h"
#include "simutgw/win_iocp_simutgw_client/ClientSocket.h"
#else
#include "linux_epoll_client/EpollClient_Core.h"
#include "simutgw/linux_tcp_simutgw_client/LnxTcpHandler_Simutgw_Client.h"
#endif

#include "ClientReconnectGapper.h"

/*
�ͻ���ҵ�����
*/
class Clienter : public SocketServiceBase
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	struct SendBuffer
	{
		uint64_t SendSeq;
		int Response;
		std::string Msg;
		std::string respText;
	};

	// 
#ifdef _MSC_VER
	std::shared_ptr<IocpEventHandler> m_handler;
	std::shared_ptr<ClientSocket> m_clientSocket;
#else
	std::shared_ptr<EpollClient_EventHandler> m_handler;
	std::shared_ptr<EpollClient_Core> m_clientSocket;
#endif

	// server ip
	std::string m_strServerIp;
	// server port
	u_short m_uiServerPort;

	// �Ƿ�������
	// true -- ����
	// flase -- ����
	bool m_bIsConnected;

	ClientReconnectGapper reconnGapper;

	// mutex for client reconnect.
	boost::mutex m_mutexReconnect;

	uint64_t m_ui64MsgSeq;
	// mutex for message sequence.
	boost::mutex m_mutexSeq;

	// mutex for response feed back.
	boost::mutex m_mutexResp;

	// �ѷ�����Ϣ�б�
	map<uint64_t, struct SendBuffer> m_mapSendBuffer;

	// �յ�������Ϣ����
	uint64_t m_ui64ResponseReceived;

	// �յ�����Ϣ �ɹ�����
	uint64_t m_ui64ResponseSuccess;
	// �յ�����Ϣ ʧ������
	uint64_t m_ui64ResponseFailed;

	//
	// Functions
	//
public:
	Clienter();
	virtual ~Clienter();

	/*
	��ȡ������Ϣ��sequence
	*/
	uint64_t GetMsgSeq(void)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutexSeq);

		return m_ui64MsgSeq++;
	}

	/*
	���ÿͻ��˵�¼״̬

	@param bool bIsConnected : �Ƿ�������
	true -- ����
	false -- ����
	*/
	void SetConnectStatus(bool bIsConnected)
	{
		m_bIsConnected = bIsConnected;

		if (!m_bIsConnected)
		{
			reconnGapper.RecordDisconnectTime();
		}
	}

	/*
	��server����socket tcp����

	Param :
	const string& in_ip : Server Ip address.
	const u_short in_port : Server Listen port.

	Return :
	0 -- ���ӳɹ�
	-1 -- ����ʧ��
	*/
	int Connect(const string& in_ip, const u_short in_port);

	/*
	��server���½���socket tcp����

	@param :

	@return :
	0 -- ���ӳɹ�
	-1 -- ����ʧ��
	*/
	int Reconnect(void);

	//!***************************************************************************
	//! @details
	//! Fully disconnect from a connected client. Once all outstanding sends
	//! are completed, a corresponding OnDisconnect callback will be invoked.
	//!
	//! check class SocketServiceBase::Disconnect()
	//!***************************************************************************
	virtual void Disconnect(uint64_t cid);

	//!***************************************************************************
	//! @details
	//! Shutdown certain operation on the socket.
	//!
	//! check class SocketServiceBase::Shutdown()
	//!***************************************************************************
	virtual void Shutdown(uint64_t cid, int how);

	/*
	send message to server

	@param const std::string msgKeyValue : message key value
	const std::string* in_pstrOrigseqnum : �ر���Ϣ�� ԭʼ���
	@param rapidjson::Document& docValue : value message
	@param trivial::severity_level in_logLvl : log level
	@param bool bNeedLog : �Ƿ���Ҫ����־
	*/
	int SendMsgToServer(const std::string& msgKeyValue, const std::string* in_pstrOrigseqnum,
		rapidjson::Document& in_docValue, trivial::severity_level in_logLvl, bool bNeedLog = true);

	/*
	��Serverע��
	@Return
	0 -- ע��ɹ�
	-1 -- ע��ʧ��
	*/
	int SendMsg_RegisterToServer();

	/*
	��Server��ȡ��������
	@Return
	0 -- ��ȡ�ɹ�
	-1 -- ��ȡʧ��
	*/
	int SendMsg_GetParamFromServer();

	/*
	��Server��ȡMatchRule

	@Return
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int SendMsg_GetMatchRuleContentFromServer(std::vector<uint64_t>& in_vctNeedFetchRule_Sh,
		std::vector<uint64_t>& in_vctNeedFetchRule_Sz);

	/*
	�����server�յ�����Ϣ

	@param
	const std::string& in_cstrMsg ҵ����Ϣ

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcessReceivedData(const std::string& in_cstrMsg);

protected:
	/*
	����ע��ر���Ϣ
	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcReqMsg_RegisterReport(rapidjson::Value& in_jsonvalue);

	/*
	�����ȡ�����ر���Ϣ
	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcReqMsg_ParamReport(rapidjson::Value& in_jsonvalue);

	/*
	������Ϣ ������������
	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcReqMsg_EngineStateCheck(const std::string& in_cstrOrigseqnum,
		rapidjson::Value& in_jsonvalue);

	/*
	��������������Ϣ
	@param
	const std::string& in_cstrOrigseqnum ��Ϣ���
	rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcReqMsg_RestartRequest(const std::string& in_cstrOrigseqnum,
		rapidjson::Value& in_jsonvalue);

	/*
	�������ģʽ������Ϣ
	@param
	const std::string& in_cstrOrigseqnum ��Ϣ���
	rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcReqMsg_SwitchModeRequest(const std::string& in_cstrOrigseqnum,
		rapidjson::Value& in_jsonvalue);

	/*
	���� ͨ�����Ըı����� ��Ϣ
	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcReqMsg_ChangeLinkStrategy(const std::string& in_cstrOrigseqnum,
		rapidjson::Value& in_jsonvalue);

	/*
	��ȡ ͨ�����Ըı�����

	@param rapidjson::Value& in_jsonvalue : json��Ϣ�е�value����
	@param rapidjson::Document& io_docResp : �ر�����
	@param rapidjson::Document::AllocatorType& in_allocResp: �ر�����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int Get_ChangeLinkStrategy(rapidjson::Value& in_jsonvalue, rapidjson::Document& io_docResp,
		rapidjson::Document::AllocatorType& in_allocResp);

	/*
	��ȡ ͨ���ͳɽ��������õİ󶨹�ϵ

	@param rapidjson::Value& in_jsonvalue : json��Ϣ�е�value����
	@param rapidjson::Document& io_docResp : �ر�����
	@param rapidjson::Document::AllocatorType& in_allocResp: �ر�����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int Get_LinkMatchRuleRelations(rapidjson::Value& in_jsonvalue, rapidjson::Document& io_docResp,
		rapidjson::Document::AllocatorType& in_allocResp);

	/*
	ȡϵͳ������Ϣ

	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�sysConfig����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int GetSysConfig(rapidjson::Value& in_jsonvalue);

	/*
	ȡ���ڽӿڲ���

	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�szConn����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int GetSzConnConfig(rapidjson::Value& in_jsonvalue);

	/*
	ȡ�Ϻ��ӿڲ���

	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�shConn����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int GetShConnConfig(rapidjson::Value& in_jsonvalue);

	/*
	���� Etf��Ϣ ��Ϣ
	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcReqMsg_Etfinfo(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue);

	/*
	���� �ɽ����øı� ��Ϣ
	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcReqMsg_MatchRule(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue);

	/*
	���� ͨ����Ӧ�ɽ��������øı����� ��Ϣ
	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcReqMsg_ChangeLinkRules(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue);

	/*
	���� ���������ļ� ��Ϣ
	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcReqMsg_SettleAccounts(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue);

	/*
	���� �������� ��Ϣ
	@param
	rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

	@Return
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int ProcReqMsg_DayEnd(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue);
};

#endif