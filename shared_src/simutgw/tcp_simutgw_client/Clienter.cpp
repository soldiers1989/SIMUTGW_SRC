#include "Clienter.h"

#include <iostream>
#include <stdlib.h>
#include <stdint.h>

#include "config_client_msgkey.h"
#include "g_client_values.h"

#include "util/FileHandler.h"
#include "tool_file/FileOperHelper.h"
#include "tool_json/RapidJsonHelper_tgw.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "tool_net/PacketAssembler.h"
#include "tool_libcurl/CurlHttpMultiFileUpload.h"

#include "simutgw_config/define_version.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw_config/config_define.h"
#include "simutgw/stgw_config/g_values_biz.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/stgw_config/sys_function.h"
#include "simutgw/work_manage/SystemInit.h"

using namespace std;

Clienter::Clienter()
	:m_scl(keywords::channel = "Clienter"),
	m_strServerIp(""), m_uiServerPort(0), m_bIsConnected(false),
	m_ui64MsgSeq(1), m_ui64ResponseReceived(0),
	m_ui64ResponseSuccess(0), m_ui64ResponseFailed(0)
{
#ifdef _MSC_VER
	m_handler = std::shared_ptr<IocpEventHandler>(new ClientHandler(this));

	m_clientSocket = std::shared_ptr<ClientSocket>(new ClientSocket(m_handler));
#else
	LnxTcpHandler_Simutgw_Client* ptrH = new LnxTcpHandler_Simutgw_Client(this);
	m_handler = shared_ptr<EpollClient_EventHandler>((EpollClient_EventHandler*)ptrH);

	m_clientSocket = std::shared_ptr<EpollClient_Core>(new EpollClient_Core(m_handler));
#endif
}

Clienter::~Clienter()
{
	// 
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
int Clienter::Connect(const string& in_ip, const u_short in_port)
{
	int iRes = m_clientSocket->Init();
	if (0 != iRes)
	{
		return -1;
	}

	m_strServerIp = in_ip;
	m_uiServerPort = in_port;

	iRes = m_clientSocket->Connect(in_ip, in_port);
	if (0 != iRes)
	{
		return -1;
	}

	return 0;
}

/*
��server���½���socket tcp����

@param :

@return :
0 -- ���ӳɹ�
-1 -- ����ʧ��
*/
int Clienter::Reconnect(void)
{
	static const string ftag("Clienter::Reconnect() ");
	BOOST_LOG_SEV(m_scl, trivial::info) << ftag << " perform reconnect to " << m_strServerIp << ":" << m_uiServerPort;

	int iRes = m_clientSocket->Connect(m_strServerIp, m_uiServerPort);
	if (0 != iRes)
	{
		return -1;
	}

	return 0;
}

//!***************************************************************************
//! @details
//! Fully disconnect from a connected client. Once all outstanding sends
//! are completed, a corresponding OnDisconnect callback will be invoked.
//!
//! check class SocketServiceBase::Disconnect()
//!***************************************************************************
void Clienter::Disconnect(uint64_t cid)
{
	// static const string ftag("Clienter::Disconnect() ");

	m_clientSocket->Disconnect();
}

//!***************************************************************************
//! @details
//! Shutdown certain operation on the socket.
//!
//! check class SocketServiceBase::Shutdown()
//!***************************************************************************
void Clienter::Shutdown(uint64_t cid, int how)
{
	// static const string ftag("Clienter::Shutdown() ");

	m_clientSocket->Disconnect();
}

/*
send message to server

@param const std::string msgKeyValue : message key value
const std::string* in_pstrOrigseqnum : �ر���Ϣ�� ԭʼ���
@param rapidjson::Document& docValue : value message
@param trivial::severity_level in_logLvl : log level
@param bool bNeedLog : �Ƿ���Ҫ����־
*/
int Clienter::SendMsgToServer(const std::string& msgKeyValue, const std::string* in_pstrOrigseqnum,
	rapidjson::Document& in_docValue, trivial::severity_level in_logLvl, bool bNeedLog)
{
	static const string ftag("SendMsgToServer() ");

	static const char cstrEngineIdKey[] = "engineId";

	// �Ƿ��Ѷ��ߣ���Ҫ����
	if (!m_bIsConnected)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutexReconnect);
		bool bRes = reconnGapper.CanReconnect();
		if (!bRes)
		{
			return 1;
		}

		int iRes = Reconnect();
		if (0 != iRes)
		{
			return -1;
		}
	}

	// ���ע����Ϣ������
	string strSendMsg;
	std::shared_ptr<rapidjson::Document> ptrDocSendTmp(new rapidjson::Document(rapidjson::kObjectType));
	if (nullptr == ptrDocSendTmp)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "allocate failed!";
		return -1;
	}
	rapidjson::Document::AllocatorType& allocSendTmp = ptrDocSendTmp->GetAllocator();

	uint64_t ui64SendSeq = GetMsgSeq();
	// msgseq
	ptrDocSendTmp->AddMember(simutgw::client::cstrMsgSeq, ui64SendSeq, allocSendTmp);

	// origmsgseq
	if (nullptr != in_pstrOrigseqnum)
	{
		ptrDocSendTmp->AddMember(simutgw::client::cstrOrigMsgSeq,
			rapidjson::Value(in_pstrOrigseqnum->c_str(), allocSendTmp), allocSendTmp);
	}

	// key
	ptrDocSendTmp->AddMember(simutgw::client::cstrMsgKey, rapidjson::Value(msgKeyValue.c_str(), allocSendTmp), allocSendTmp);

	// engine id
	ptrDocSendTmp->AddMember(cstrEngineIdKey,
		rapidjson::Value(simutgw::g_strWeb_id.c_str(), allocSendTmp), allocSendTmp);

	// timestamp
	ptrDocSendTmp->AddMember("timestamp", (long)TimeStringUtil::GetTimeStamp(), allocSendTmp);

	// value
	ptrDocSendTmp->AddMember(simutgw::client::cstrMsgValue, in_docValue, allocSendTmp);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	ptrDocSendTmp->Accept(writer);
	strSendMsg = buffer.GetString();

	shared_ptr<vector<uint8_t>> pvectNetData(new vector<uint8_t>());

	int iRes = PacketAssembler::LocalPackageToNetBuffer(simutgw::NET_MSG_TYPE::Order, strSendMsg, nullptr, *pvectNetData);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag
			<< "LocalPackageToNet failed with err=" << iRes << ", msg=" << strSendMsg;
		return -1;
	}

	m_clientSocket->Send(pvectNetData);

	if (bNeedLog)
	{
		BOOST_LOG_SEV(m_scl, in_logLvl) << ftag << strSendMsg;
	}

	return 0;
}

/*
��Serverע��
@Return
0 -- ע��ɹ�
-1 -- ע��ʧ��
*/
int Clienter::SendMsg_RegisterToServer()
{
	static const string ftag("Clienter::SendMsg_RegisterToServer() ");

	// ���ע����Ϣ������
	string strSendMsg;
	std::shared_ptr<rapidjson::Document> ptrDocSendTmp(new rapidjson::Document(rapidjson::kObjectType));
	if (nullptr == ptrDocSendTmp)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "allocate failed!";
		return -1;
	}
	rapidjson::Document::AllocatorType& allocSendTmp = ptrDocSendTmp->GetAllocator();

	uint64_t ui64SendSeq = GetMsgSeq();
	// msgseq
	ptrDocSendTmp->AddMember(simutgw::client::cstrMsgSeq, ui64SendSeq, allocSendTmp);

	// key
	ptrDocSendTmp->AddMember(simutgw::client::cstrMsgKey, simutgw::client::cstrRegisterKey, allocSendTmp);

	// version number.
	ptrDocSendTmp->AddMember(simutgw::client::cstrVersionNumKey, rapidjson::Value(simutgw::g_strSystemVersion.c_str(), allocSendTmp), allocSendTmp);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	ptrDocSendTmp->Accept(writer);
	strSendMsg = buffer.GetString();

	shared_ptr<vector<uint8_t>> pvectNetData(new vector<uint8_t>());

	int iRes = PacketAssembler::LocalPackageToNetBuffer(simutgw::NET_MSG_TYPE::Order, strSendMsg, nullptr, *pvectNetData);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag
			<< "LocalPackageToNet failed with err=" << iRes << ", msg=" << strSendMsg;
		return -1;
	}

	m_clientSocket->Send(pvectNetData);

	BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strSendMsg;

#ifdef _MSC_VER
	// �ȴ�ע��ɹ���Ϣ����
	// 600s
	DWORD dTimeout = 600 * 1000;
	// �ȴ�ע���¼�event
	DWORD dRes = WaitForSingleObject(simutgw::client::g_registerEvent, dTimeout);
	if (WAIT_OBJECT_0 == dRes)
	{
		// �ɹ�
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "ע��ɹ�";
	}
	else if (WAIT_TIMEOUT == dRes)
	{
		// ��ʱ
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ע�ᳬʱ";
		return -1;
	}
	else if (WAIT_ABANDONED == dRes)
	{
		// error
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ע�����";
		return -1;
	}
	else
	{
		// error
		return -1;
	}
#else
	// �ȴ�ע��ɹ���Ϣ����
	// 600s

	// �ȴ�ע���¼�event
	//�ȴ��߳��źţ���ʱ		
	pthread_mutex_lock(&simutgw::client::g_mutex_registerEvent);

	// The  pthread_cond_timedwait()   function   shall   be   equivalent   to
	// pthread_cond_wait(),  except  that an error is returned if the absolute
	// time specified by abstime  passes  (that  is,  system  time  equals  or
	// exceeds  abstime) before the condition cond is signaled or broadcasted,
	// or if the absolute time specified by abstime has already been passed at
	// the time of the call.
	struct timeval now;
	struct timespec tmRestrict;

	gettimeofday(&now, NULL);

	tmRestrict.tv_sec = now.tv_sec + 600;
	tmRestrict.tv_nsec = now.tv_usec * 1000;
	int iCondTimeWaitRes = pthread_cond_timedwait(&simutgw::client::g_cond_registerEvent, &simutgw::client::g_mutex_registerEvent, &tmRestrict);

	pthread_mutex_unlock(&simutgw::client::g_mutex_registerEvent);

	if (0 == iCondTimeWaitRes)
	{
		// �ɹ�
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "ע��ɹ�";
	}
	else if (ETIMEDOUT == iCondTimeWaitRes)
	{
		// ��ʱ
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ע�ᳬʱ";
		return -1;
	}
	else if (EINVAL == iCondTimeWaitRes)
	{
		// error
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ע����� EINVAL";
		return -1;
	}
	else
	{
		// error
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << " errres=" << iCondTimeWaitRes << " strerror=" << strerror(iCondTimeWaitRes);

		return -1;
	}

#endif

	return 0;
}

/*
��Server��ȡ��������
@Return
0 -- ��ȡ�ɹ�
-1 -- ��ȡʧ��
*/
int Clienter::SendMsg_GetParamFromServer()
{
	static const string ftag("Clienter::GetParamFromServer() ");
	static const string cstrGetParamKey("getEngineLinkParam");
	static const string cstrEngineIdKey("engineId");

	// ���������Ϣ������
	string strSendMsg;
	std::shared_ptr<rapidjson::Document> ptrDocSendTmp(new rapidjson::Document(rapidjson::kObjectType));
	if (nullptr == ptrDocSendTmp)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "allocate failed!";
		return -1;
	}
	rapidjson::Document::AllocatorType& allocSendTmp = ptrDocSendTmp->GetAllocator();

	uint64_t ui64SendSeq = GetMsgSeq();
	// msgseq
	ptrDocSendTmp->AddMember(simutgw::client::cstrMsgSeq, ui64SendSeq, allocSendTmp);

	// key
	ptrDocSendTmp->AddMember(simutgw::client::cstrMsgKey, rapidjson::Value(cstrGetParamKey.c_str(), allocSendTmp), allocSendTmp);

	//
	// value
	rapidjson::Value val;
	val.SetObject();
	val.AddMember(rapidjson::Value(cstrEngineIdKey.c_str(), allocSendTmp),
		rapidjson::Value(simutgw::g_strWeb_id.c_str(), allocSendTmp), allocSendTmp);
	// version number.
	val.AddMember(simutgw::client::cstrVersionNumKey, rapidjson::Value(simutgw::g_strSystemVersion.c_str(), allocSendTmp), allocSendTmp);

	ptrDocSendTmp->AddMember(simutgw::client::cstrMsgValue, val, allocSendTmp);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	ptrDocSendTmp->Accept(writer);
	strSendMsg = buffer.GetString();

	shared_ptr<vector<uint8_t>> pvectNetData(new vector<uint8_t>());

	int iRes = PacketAssembler::LocalPackageToNetBuffer(simutgw::NET_MSG_TYPE::Order, strSendMsg, nullptr, *pvectNetData);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag
			<< "LocalPackageToNet failed with err=" << iRes << ", msg=" << strSendMsg;
		return -1;
	}

	m_clientSocket->Send(pvectNetData);

	BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strSendMsg;


#ifdef _MSC_VER
	// �ȴ���ȡ��������
	// 600s
	DWORD dTimeout = 600 * 1000;
	DWORD dRes = WaitForSingleObject(simutgw::client::g_getParamEvent, dTimeout);
	if (WAIT_OBJECT_0 == dRes)
	{
		// �ɹ�
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "��ȡ�����ɹ�";
	}
	else if (WAIT_TIMEOUT == dRes)
	{
		// ��ʱ
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "��ȡ������ʱ";
		return -1;
	}
	else if (WAIT_ABANDONED == dRes)
	{
		// error
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "��ȡ��������";
		return -1;
	}
	else
	{
		// error
		return -1;
	}
#else
	// �ȴ���ȡ��������
	// 600s

	//�ȴ��߳��źţ���ʱ		
	pthread_mutex_lock(&simutgw::client::g_mutex_getParamEvent);

	// The  pthread_cond_timedwait()   function   shall   be   equivalent   to
	// pthread_cond_wait(),  except  that an error is returned if the absolute
	// time specified by abstime  passes  (that  is,  system  time  equals  or
	// exceeds  abstime) before the condition cond is signaled or broadcasted,
	// or if the absolute time specified by abstime has already been passed at
	// the time of the call.
	struct timeval now;
	struct timespec tmRestrict;

	gettimeofday(&now, NULL);

	tmRestrict.tv_sec = now.tv_sec + 600;
	tmRestrict.tv_nsec = now.tv_usec * 1000;
	int iCondTimeWaitRes = pthread_cond_timedwait(&simutgw::client::g_cond_getParamEvent, &simutgw::client::g_mutex_getParamEvent, &tmRestrict);

	pthread_mutex_unlock(&simutgw::client::g_mutex_getParamEvent);

	if (0 == iCondTimeWaitRes)
	{
		// �ɹ�
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "��ȡ�����ɹ�";
	}
	else if (ETIMEDOUT == iCondTimeWaitRes)
	{
		// ��ʱ
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "��ȡ������ʱ";
		return -1;
	}
	else if (EINVAL == iCondTimeWaitRes)
	{
		// error
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "��ȡ�������� EINVAL";
		return -1;
	}
	else
	{
		// error
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << " errres=" << iCondTimeWaitRes << " strerror=" << strerror(iCondTimeWaitRes);

		return -1;
	}
#endif

	return 0;
}

/*
��Server��ȡMatchRule

@Return
0 -- �ɹ�
-1 -- ʧ��
*/
int Clienter::SendMsg_GetMatchRuleContentFromServer(
	std::vector<uint64_t>& in_vctNeedFetchRule_Sh,
	std::vector<uint64_t>& in_vctNeedFetchRule_Sz)
{
	static const string ftag("Clienter::SendMsg_GetMatchRuleContentFromServer() ");

	static const char cstrKey_Rules_sh[] = "sh_rules";
	static const char cstrKey_Rules_sz[] = "sz_rules";

	if (in_vctNeedFetchRule_Sh.empty() && in_vctNeedFetchRule_Sz.empty())
	{
		return 0;
	}

	// ���������Ϣ������
	rapidjson::Document docSendTmp(rapidjson::kObjectType);

	rapidjson::Document::AllocatorType& allocSendTmp = docSendTmp.GetAllocator();

	// ruleid array
	rapidjson::Value valRuleIdArray_sh(rapidjson::kArrayType);
	rapidjson::Value valRuleIdArray_sz(rapidjson::kArrayType);

	string strItoa;
	size_t i = 0;
	for (; i < in_vctNeedFetchRule_Sh.size(); ++i)
	{
		sof_string::itostr(in_vctNeedFetchRule_Sh[i], strItoa);
		valRuleIdArray_sh.PushBack(rapidjson::Value(strItoa.c_str(), allocSendTmp), allocSendTmp);
	}

	for (i = 0; i < in_vctNeedFetchRule_Sz.size(); ++i)
	{
		sof_string::itostr(in_vctNeedFetchRule_Sz[i], strItoa);
		valRuleIdArray_sz.PushBack(rapidjson::Value(strItoa.c_str(), allocSendTmp), allocSendTmp);
	}

	docSendTmp.AddMember(cstrKey_Rules_sh, valRuleIdArray_sh, allocSendTmp);
	docSendTmp.AddMember(cstrKey_Rules_sz, valRuleIdArray_sz, allocSendTmp);

	int iRes = SendMsgToServer(simutgw::client::cstrKey_GetMatchRuleContent, nullptr, docSendTmp, trivial::info);
	if (0 != iRes)
	{
		return -1;
	}

	return 0;
}

/*
�����server�յ�����Ϣ

@param
const std::string& in_cstrMsg ҵ����Ϣ

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcessReceivedData(const std::string& in_cstrMsg)
{
	static const string ftag("Clienter::ProcessReceivedData() ");

	EzLog::i(ftag, in_cstrMsg);

	// json parse
	rapidjson::Document docData;

	if (docData.Parse<0>(in_cstrMsg.c_str()).HasParseError() || docData.IsNull())
	{
		//������Ϣʧ��
		string strDebug = "Parse raw string to Json failed,string=[";
		strDebug += in_cstrMsg;
		strDebug += "]";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	// ����Ƿ����msgseq
	if (!docData.HasMember(simutgw::client::cstrMsgSeq) ||
		!docData[simutgw::client::cstrMsgKey].IsString())
	{
		string strDebug = "�ַ����ֶ�key��ʽ����ȷ,string=[";
		strDebug += in_cstrMsg;
		strDebug += "]";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	// ����Ƿ����key
	if (!docData.HasMember(simutgw::client::cstrMsgKey) ||
		!docData[simutgw::client::cstrMsgKey].IsString())
	{
		string strDebug = "�ַ����ֶ�key��ʽ����ȷ,string=[";
		strDebug += in_cstrMsg;
		strDebug += "]";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	// ����Ƿ����value
	if (!docData.HasMember(simutgw::client::cstrMsgValue) ||
		!(docData[simutgw::client::cstrMsgValue].IsObject() || docData[simutgw::client::cstrMsgValue].IsArray()))
	{
		string strDebug = "�ַ����ֶ�value��ʽ����ȷ,string=[";
		strDebug += in_cstrMsg;
		strDebug += "]";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	// ȡ�ֶ���
	std::string strKeyName = docData[simutgw::client::cstrMsgKey].GetString();
	std::string strSeqNum = docData[simutgw::client::cstrMsgSeq].GetString();
	rapidjson::Value& val = docData[simutgw::client::cstrMsgValue];
	if (0 == strKeyName.compare(simutgw::client::cstrRegisterKey))
	{
		// ע��ر�
		return ProcReqMsg_RegisterReport(val);
	}
	else if (0 == strKeyName.compare(simutgw::client::cstrGetParamKey))
	{
		// ȡ�����ر�
		return ProcReqMsg_ParamReport(val);
	}
	else if (0 == strKeyName.compare(simutgw::client::cstrEngineStateCheckKey))
	{
		// ����˵���������
		return ProcReqMsg_EngineStateCheck(strSeqNum, val);
	}
	else if (0 == strKeyName.compare(simutgw::client::cstrRestartKey))
	{
		// ��������
		return ProcReqMsg_RestartRequest(strSeqNum, val);
	}
	else if (0 == strKeyName.compare(simutgw::client::cstrSwitchModeKey))
	{
		// ģʽ�л�����
		return ProcReqMsg_SwitchModeRequest(strSeqNum, val);
	}
	else if (0 == strKeyName.compare(simutgw::client::cstrKey_LinkStrategy))
	{
		// ͨ�����Ըı�����
		return ProcReqMsg_ChangeLinkStrategy(strSeqNum, val);
	}
	else if (0 == strKeyName.compare(simutgw::client::cstrKey_Etfinfo))
	{
		// ETF��Ϣ�ļ�
		return ProcReqMsg_Etfinfo(strSeqNum, val);
	}
	else if (0 == strKeyName.compare(simutgw::client::cstrKey_MatchRule))
	{
		// �ɽ��������øı�����
		return ProcReqMsg_MatchRule(strSeqNum, val);
	}
	else if (0 == strKeyName.compare(simutgw::client::cstrKey_ChangeLinkRules))
	{
		// ͨ����Ӧ�ɽ��������øı�����
		return ProcReqMsg_ChangeLinkRules(strSeqNum, val);
	}
	else if (0 == strKeyName.compare(simutgw::client::cstrKey_SettleAccounts))
	{
		// ���������ļ�
		return ProcReqMsg_SettleAccounts(strSeqNum, val);
	}
	else if (0 == strKeyName.compare(simutgw::client::cstrKey_DayEnd))
	{
		// ��������
		return ProcReqMsg_DayEnd(strSeqNum, val);
	}
	else
	{
		string strDebug("δ֧�ֵ���Ϣkey=");
		strDebug += strKeyName;
		BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << strDebug;
	}

	return 0;
}

/*
����ע��ر���Ϣ
@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcReqMsg_RegisterReport(rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::ProcReqMsg_RegisterReport() ");

	if (!in_jsonvalue.HasMember(simutgw::client::cstrEngineIdKey) ||
		!in_jsonvalue[simutgw::client::cstrEngineIdKey].IsString())
	{
		string strDebug = "ע��ر��ַ����ֶ�engineId��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	simutgw::g_strWeb_id = in_jsonvalue[simutgw::client::cstrEngineIdKey].GetString();

#ifdef _MSC_VER
	// ����ע���¼�event
	SetEvent(simutgw::client::g_registerEvent);
#else
	// ����ע���¼�event
	pthread_cond_signal(&simutgw::client::g_cond_registerEvent);
#endif	

	return 0;
}

/*
�����ȡ�����ر���Ϣ
@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcReqMsg_ParamReport(rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::ProcReqMsg_ParamReport() ");

	static const char cstrsysConfig[] = "sysConfig";
	static const char cstrszConn[] = "szConn";
	static const char cstrshConn[] = "shConn";
	static const char cstrStrategy[] = "strategy";
	static const char cstrMatchRules[] = "matchRules";

	// ����ʽ
	if (!in_jsonvalue.HasMember(cstrsysConfig) ||
		!in_jsonvalue[cstrsysConfig].IsObject())
	{
		string strDebug = "ȡ�λر��ֶ�sysConfig��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	// ����ʽ
	if (!in_jsonvalue.HasMember(cstrszConn) ||
		!in_jsonvalue[cstrszConn].IsArray())
	{
		string strDebug = "ȡ�λر��ֶ�szConn��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	// ����ʽ
	if (!in_jsonvalue.HasMember(cstrshConn) ||
		!in_jsonvalue[cstrshConn].IsArray())
	{
		string strDebug = "ȡ�λر��ֶ�shConn��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	// ����ʽ
	if (!in_jsonvalue.HasMember(cstrStrategy) ||
		!in_jsonvalue[cstrStrategy].IsArray())
	{
		string strDebug = "ȡ�λر��ֶ�Strategy��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	// ����ʽ
	if (!in_jsonvalue.HasMember(cstrMatchRules) ||
		!in_jsonvalue[cstrMatchRules].IsArray())
	{
		string strDebug = "ȡ�λر��ֶ�MatchRules��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	int iRes = 0;
	// ϵͳ����
	iRes = GetSysConfig(in_jsonvalue[cstrsysConfig]);
	if (0 != iRes)
	{
		return -1;
	}

	// �������Ӳ���
	iRes = GetSzConnConfig(in_jsonvalue[cstrszConn]);
	if (0 != iRes)
	{
		return -1;
	}

	// �Ϻ����Ӳ���
	iRes = GetShConnConfig(in_jsonvalue[cstrshConn]);
	if (0 != iRes)
	{
		return -1;
	}

	// �ر�����
	rapidjson::Document docRespTmp(rapidjson::kArrayType);
	rapidjson::Document::AllocatorType& allocRespTmp = docRespTmp.GetAllocator();

	iRes = Get_ChangeLinkStrategy(in_jsonvalue[cstrStrategy], docRespTmp, allocRespTmp);
	if (0 == iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "ģʽ�л��ɹ�";
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ģʽ�л�ʧ��";
	}

	iRes = Get_LinkMatchRuleRelations(in_jsonvalue[cstrMatchRules], docRespTmp, allocRespTmp);
	if (0 == iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "MatchRules�ɹ�";
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "MatchRulesʧ��";
	}

#ifdef _MSC_VER
	// ������ȡ����event
	SetEvent(simutgw::client::g_getParamEvent);
#else
	// ������ȡ����event
	pthread_cond_signal(&simutgw::client::g_cond_getParamEvent);
#endif


	return 0;
}

/*
������Ϣ ������������
@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcReqMsg_EngineStateCheck(const std::string& in_cstrOrigseqnum,
	rapidjson::Value& in_jsonvalue)
{
	// static const string ftag("Clienter::ProcReqMsg_EngineStateCheck() ");

	return 0;
}

/*
��������������Ϣ
@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcReqMsg_RestartRequest(const std::string& in_cstrOrigseqnum,
	rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::ProcReqMsg_RestartRequest() ");
	static const char cstrRestartTypeKey[] = "restart_type";

	// value content
	// response
	static const char cstrStatusKey[] = "status";
	static const char cstrTextKey[] = "text";

	// �ر�
	rapidjson::Document docRespTmp(rapidjson::kObjectType);
	rapidjson::Document::AllocatorType& allocRespTmp = docRespTmp.GetAllocator();

	// ����״̬
	int iStatus = 0;
	// ����
	// �Ƿ�Ϊobject
	if (!in_jsonvalue.IsArray())
	{
		string strDebug = "���������ֶ�value��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
		iStatus = 1;
	}

	for (rapidjson::SizeType i = 0; i < 1; ++i)
	{
		rapidjson::Value & vEle = in_jsonvalue[i];
		if (!vEle.IsObject())
		{
			string strDebug = "����ģʽ�����ֶ�value��ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
			iStatus = 1;
			break;
		}

		// ����Ƿ����engineId
		if (!vEle.HasMember(simutgw::client::cstrEngineIdKey) ||
			!vEle[simutgw::client::cstrEngineIdKey].IsString())
		{
			string strDebug = "���������ֶ�value-engineId��ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
			iStatus = 1;
			break;
		}

		// ����Ƿ��뱾����ͬ
		std::string strRequestWebId = vEle[simutgw::client::cstrEngineIdKey].GetString();
		if (0 != simutgw::g_strWeb_id.compare(strRequestWebId))
		{
			string strDebug = "engineId�뵱ǰ���治һ��";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
			iStatus = 1;
			break;
		}

		// ����Ƿ����restart_type
		if (!vEle.HasMember(cstrRestartTypeKey) ||
			!vEle[cstrRestartTypeKey].IsString())
		{
			string strDebug = "���������ֶ�value-restart_type��ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
			iStatus = 1;
			break;
		}

		{
			std::string strDebug("��������=");
			strDebug += vEle[cstrRestartTypeKey].GetString();
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strDebug;
		}
	}

	// engine id
	docRespTmp.AddMember(simutgw::client::cstrEngineIdKey,
		rapidjson::Value(simutgw::g_strWeb_id.c_str(), allocRespTmp), allocRespTmp);

	docRespTmp.AddMember(cstrStatusKey, iStatus, allocRespTmp);

	// �ر�
	SendMsgToServer(simutgw::client::cstrRestartKey, &in_cstrOrigseqnum, docRespTmp, trivial::info);

	if (0 != iStatus)
	{
		return -1;
	}

#ifdef _MSC_VER
	Sleep(10);
#else		
	usleep(10 * 1000L);
#endif

	// �ȹر��ڲ�handle
	simutgw::SimuTgwSelfExit_remoterestart();

	// ���������ű�
	system("python exe_restart.py");

	if (1 == iStatus)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "�����ɹ�";
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "����ʧ��";

		return -1;
	}

	return 0;
}

/*
�������ģʽ������Ϣ
@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcReqMsg_SwitchModeRequest(const std::string& in_cstrOrigseqnum,
	rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::ProcReqMsg_SwitchModeRequest() ");
	static char cstrEnginModelKey[] = "engineModel";
	static char cstrMarketKey[] = "market";
	static char cstrValidation[] = "validation";
	static char cstrMethodKey[] = "method";
	static char cstrDealNumKey[] = "dealNumber";

	// value content
	// response
	static const char cstrStatusKey[] = "status";
	static const char cstrTextKey[] = "text";

	// �ر�
	rapidjson::Document docRespTmp(rapidjson::kObjectType);
	rapidjson::Document::AllocatorType& allocRespTmp = docRespTmp.GetAllocator();

	string strRspText("");

	// ����״̬
	int iStatus = 0;

	// ����
	// �Ƿ�Ϊobject
	if (!in_jsonvalue.IsArray())
	{
		string strDebug = "����ģʽ�����ֶ�value��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		iStatus = 1;
		strRspText = strDebug;
	}

	if (0 == iStatus)
	{
		for (rapidjson::SizeType i = 0; i < in_jsonvalue.Size(); ++i)
		{
			rapidjson::Value & vEle = in_jsonvalue[i];
			if (!vEle.IsObject())
			{
				string strDebug = "����ģʽ�����ֶ�value��ʽ����ȷ";
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
				iStatus = 1;
				strRspText = strDebug;

				break;
			}

			// ����Ƿ����engineId
			if (0 == iStatus &&
				(!vEle.HasMember(simutgw::client::cstrEngineIdKey) ||
				!vEle[simutgw::client::cstrEngineIdKey].IsString()))
			{
				string strDebug = "����ģʽ�����ֶ�value-engineId��ʽ����ȷ";
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
				iStatus = 1;
				strRspText = strDebug;
				break;
			}

			// ����Ƿ��뱾����ͬ
			std::string strRequestWebId = vEle[simutgw::client::cstrEngineIdKey].GetString();
			if (0 != simutgw::g_strWeb_id.compare(strRequestWebId))
			{
				string strDebug = "engineId�뵱ǰ���治һ��";
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
				iStatus = 1;
				strRspText = strDebug;
				break;
			}

			// ����Ƿ����engine_model
			if (0 == iStatus &&
				(!vEle.HasMember(cstrEnginModelKey) ||
				!vEle[cstrEnginModelKey].IsString()))
			{
				string strDebug = "����ģʽ�����ֶ�value-engine_model��ʽ����ȷ";
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
				iStatus = 1;
				strRspText = strDebug;
				break;
			}

			// ����Ƿ����market
			if (0 == iStatus &&
				(!vEle.HasMember(cstrMarketKey) ||
				!vEle[cstrMarketKey].IsString()))
			{
				string strDebug = "����ģʽ�����ֶ�value-market��ʽ����ȷ";
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
				iStatus = 1;
				strRspText = strDebug;
				break;
			}

			// ����Ƿ����validation
			if (0 == iStatus &&
				(!vEle.HasMember(cstrValidation) ||
				!vEle[cstrValidation].IsString()))
			{
				string strDebug = "����ģʽ�����ֶ�value-validation��ʽ����ȷ";
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
				iStatus = 1;
				strRspText = strDebug;
				break;
			}

			// ����Ƿ����method
			if (0 == iStatus &&
				(!vEle.HasMember(cstrMethodKey) ||
				!vEle[cstrMethodKey].IsString()))
			{
				string strDebug = "����ģʽ�����ֶ�value-method��ʽ����ȷ";
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
				iStatus = 1;
				strRspText = strDebug;
				break;
			}

			// ����Ƿ����dealNumber
			if (0 == iStatus && (!vEle.HasMember(cstrDealNumKey) ||
				!vEle[cstrDealNumKey].IsInt()))
			{
				string strDebug = "����ģʽ�����ֶ�value-dealNumber��ʽ����ȷ";
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
				iStatus = 1;
				strRspText = strDebug;
				break;
			}

			// ����ģʽ
			string strTrans;
			int iValue = 0;

			//"market" : "0",//�Ƿ������飺0 �У�1, ��
			strTrans = vEle[cstrMarketKey].GetString();
			Tgw_StringUtil::String2Int_atoi(strTrans, iValue);
			if (0 == iValue)
			{
				simutgw::g_bEnable_Quotation_Task = true;
			}
			else if (1 == iValue)
			{
				simutgw::g_bEnable_Quotation_Task = false;
			}
			else
			{
				string strDebug = "����ģʽ�����ֶ�value-marketֵ����ȷ";
				iStatus = 1;
				strRspText = strDebug;
			}

			//"validation" : "0",// �Ƿ���� �� 0 �� ��1 ��
			strTrans = vEle[cstrValidation].GetString();
			Tgw_StringUtil::String2Int_atoi(strTrans, iValue);
			if (0 == iValue && simutgw::g_iRunMode == simutgw::SysRunMode::NormalMode)
			{
				// ������ģʽ�������
				simutgw::g_bEnable_Check_Assets = true;
			}
			else if (1 == iValue)
			{
				simutgw::g_bEnable_Quotation_Task = false;
			}
			else
			{
				string strDebug = "����ģʽ�����ֶ�value-validationֵ����ȷ";
				iStatus = 1;
				strRspText = strDebug;
			}

			//"method" : �ɽ�ģʽ��0:ʵ��ģ��;1:����ģ��-ȫ���ɽ�;2:����ģ��-�ֱʳɽ�;3:����ģ��-�ҵ�(�ɳ���);4:����ģ��-��;5:����ģ��-���ֳɽ�
			//ʵ��ģ������ģʽ��0:����ξ���;1:��һ��һ�۸�;2:����ɽ���
			strTrans = vEle[cstrMethodKey].GetString();
			Tgw_StringUtil::String2Int_atoi(strTrans, iValue);
			if (simutgw::g_bEnable_Quotation_Task)
			{
				// ʵ��ģ������ģʽ
				if (0 == iValue)
				{
					simutgw::g_Quotation_Type = simutgw::QuotationType::AveragePrice;
				}
				else if (1 == iValue)
				{
					simutgw::g_Quotation_Type = simutgw::QuotationType::SellBuyPrice;
				}
				else if (2 == iValue)
				{
					simutgw::g_Quotation_Type = simutgw::QuotationType::RecentMatchPrice;
				}
				else
				{
					string strDebug = "����ģʽ�����ֶ�value-methodֵ����ȷ";
					iStatus = 1;
					strRspText = strDebug;
				}
			}
			else
			{
				// �ɽ�ģʽ
				if (0 == iValue)
				{
					simutgw::g_iMatchMode = simutgw::SysMatchMode::EnAbleQuta;
				}
				else if (1 == iValue)
				{
					simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchAll;
				}
				else if (2 == iValue)
				{
					simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchByDivide;
				}
				else if (3 == iValue)
				{
					simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulNotMatch;
				}
				else if (4 == iValue)
				{
					simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulErrMatch;
				}
				else if (5 == iValue)
				{
					simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchPart;
				}
				else
				{
					string strDebug = "����ģʽ�����ֶ�value-methodֵ����ȷ";
					iStatus = 1;
					strRspText = strDebug;
				}
			}

			// �ֱʳɽ����������Ϊ2
			iValue = vEle[cstrDealNumKey].GetInt();
			if (iValue <= 2)
			{
				iValue = 2;
			}

			simutgw::g_ui32_Part_Match_Num = iValue;
		}
	}

	// �ر�
	docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strRspText.c_str(), allocRespTmp), allocRespTmp);
	docRespTmp.AddMember(cstrStatusKey, iStatus, allocRespTmp);

	// �ر�
	SendMsgToServer(simutgw::client::cstrSwitchModeKey, &in_cstrOrigseqnum, docRespTmp, trivial::info);

	if (0 == iStatus)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "ģʽ�л��ɹ�";
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ģʽ�л�ʧ��";
	}
	return 0;
}


/*
���� ͨ�����Ըı����� ��Ϣ
@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcReqMsg_ChangeLinkStrategy(const std::string& in_cstrOrigseqnum,
	rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::ProcReqMsg_ChangeLinkStrategy() ");

	// �ر�����
	rapidjson::Document docRespTmp(rapidjson::kArrayType);
	rapidjson::Document::AllocatorType& allocRespTmp = docRespTmp.GetAllocator();

	int iRes = Get_ChangeLinkStrategy(in_jsonvalue, docRespTmp, allocRespTmp);
	if (0 == iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "ģʽ�л��ɹ�";
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ģʽ�л�ʧ��";
	}

	// �ر�
	SendMsgToServer(simutgw::client::cstrKey_LinkStrategy, &in_cstrOrigseqnum, docRespTmp, trivial::info);

	return 0;
}

/*
��ȡ ͨ�����Ըı�����

@param rapidjson::Value& in_jsonvalue : json��Ϣ�е�value����
@param rapidjson::Document& io_docResp : �ر�����
@param rapidjson::Document::AllocatorType& in_allocResp: �ر�����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::Get_ChangeLinkStrategy(rapidjson::Value& in_jsonvalue, rapidjson::Document& io_docResp,
	rapidjson::Document::AllocatorType& in_allocResp)
{
	static const string ftag("Clienter::Get_ChangeLinkStrategy() ");

	// value content
	// ͨ��ID��UUID��
	static const char cstrLinkId[] = "linkId";
	// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�
	static const char cstrLinkFlag[] = "linkFlag";
	// ϯλ
	static const char cstrSeatNumber[] = "seatNumber";
	// ����ģʽ��0 ѹ��ģʽ��1 ����ģʽ��2����ģʽ
	static const char cstrEnginModelKey[] = "engineModel";
	// �Ƿ���� �� 1 �� ��0 ��
	static const char cstrValidation[] = "validation";
	// �ɽ���ʽ
	static const char cstrMethodKey[] = "method";
	// �ɽ�����
	static const char cstrDealNumKey[] = "dealNumber";

	// response
	static const char cstrStatusKey[] = "status";
	static const char cstrTextKey[] = "text";

	// ����״̬
	int iAllStatus = 0;

	string strScreenOut;

	try
	{
		// ����
		// �Ƿ�Ϊobject
		if (!in_jsonvalue.IsArray())
		{
			string strDebug = "�ֶ�value��ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
			iAllStatus = 1;
		}

		if (0 == iAllStatus)
		{
			for (rapidjson::SizeType i = 0; i < in_jsonvalue.Size(); ++i)
			{
				// ѭ��ǰ����
				int iStatus = 0;

				rapidjson::Value vResp(rapidjson::kObjectType);

				rapidjson::Value & vEle = in_jsonvalue[i];
				if (!vEle.IsObject())
				{
					string strDebug = "�ֶ�value��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
					iStatus = 1;

					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), in_allocResp), in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
				}

				//
				// ����Ƿ����engineId
				if (0 == iStatus &&
					(!vEle.HasMember(simutgw::client::cstrEngineIdKey) ||
					!vEle[simutgw::client::cstrEngineIdKey].IsString()))
				{
					string strDebug = "�ֶ�value-engineId��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
					iStatus = 1;

					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), in_allocResp), in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
				}

				//
				// ����Ƿ��뱾����ͬ
				string strRequestWebId = vEle[simutgw::client::cstrEngineIdKey].GetString();
				if (0 != simutgw::g_strWeb_id.compare(strRequestWebId))
				{
					string strDebug = "engineId�뵱ǰ���治һ��";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
					iStatus = 1;

					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), in_allocResp), in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
				}

				//
				// ����Ƿ����LinkId
				if (0 == iStatus &&
					(!vEle.HasMember(cstrLinkId) ||
					!vEle[cstrLinkId].IsString()))
				{
					string strDebug = "�ֶ�value-linkName��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
					iStatus = 1;

					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), in_allocResp), in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
				}

				// linkid
				string strLinkId = vEle[cstrLinkId].GetString();

				// screen out
				strScreenOut = "LinkId=";
				strScreenOut += strLinkId;

				//
				// ����Ƿ����linkFlag
				if (0 == iStatus &&
					(!vEle.HasMember(cstrLinkFlag) ||
					!vEle[cstrLinkFlag].IsString()))
				{
					string strDebug = "�ֶ�value-linkFlag��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
					iStatus = 1;

					vResp.AddMember(cstrLinkId, rapidjson::Value(strLinkId.c_str(), in_allocResp), in_allocResp);
					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), in_allocResp), in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
				}

				//
				// ����Ƿ����Seat
				if (0 == iStatus &&
					(!vEle.HasMember(cstrSeatNumber) ||
					!vEle[cstrSeatNumber].IsString()))
				{
					string strDebug = "�ֶ�value-engine_model��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
					iStatus = 1;

					vResp.AddMember(cstrLinkId, rapidjson::Value(strLinkId.c_str(), in_allocResp), in_allocResp);
					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), in_allocResp), in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
				}

				//
				// ����Ƿ����validation
				if (0 == iStatus &&
					(!vEle.HasMember(cstrValidation) ||
					!vEle[cstrValidation].IsString()))
				{
					string strDebug = "�ֶ�value-validation��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
					iStatus = 1;

					vResp.AddMember(cstrLinkId, rapidjson::Value(strLinkId.c_str(), in_allocResp), in_allocResp);
					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), in_allocResp), in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
				}

				//
				// ����Ƿ����method
				if (0 == iStatus &&
					(!vEle.HasMember(cstrMethodKey) ||
					!vEle[cstrMethodKey].IsString()))
				{
					string strDebug = "�ֶ�value-method��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
					iStatus = 1;

					vResp.AddMember(cstrLinkId, rapidjson::Value(strLinkId.c_str(), in_allocResp), in_allocResp);
					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), in_allocResp), in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
				}

				//
				// ����Ƿ����dealNumber
				if (0 == iStatus && (!vEle.HasMember(cstrDealNumKey) ||
					!vEle[cstrDealNumKey].IsString()))
				{
					string strDebug = "�ֶ�value-dealNumber��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
					iStatus = 1;

					vResp.AddMember(cstrLinkId, rapidjson::Value(strLinkId.c_str(), in_allocResp), in_allocResp);
					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), in_allocResp), in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
				}

				// Get values

				// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�
				string strLinkFlag = vEle[cstrLinkFlag].GetString();
				int iLinkFlag = 0;
				Tgw_StringUtil::String2Int_atoi(strLinkFlag, iLinkFlag);
				if (0 == iLinkFlag)
				{
					// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�
					// screen out
					strScreenOut += ", Shenzhen";
				}
				else if (1 == iLinkFlag)
				{
					// screen out
					strScreenOut += ", Shanghai";
				}

				// ϯλ
				string strSeatNumber = vEle[cstrSeatNumber].GetString();
				// screen out
				strScreenOut += ", SeatNumber=";
				strScreenOut += strSeatNumber;

				// �Ƿ���� �� 1 �� ��0 ��
				string strValidation = vEle[cstrValidation].GetString();
				int iValidation = 0;
				Tgw_StringUtil::String2Int_atoi(strValidation, iValidation);
				if (0 == iValidation)
				{
					// screen out
					strScreenOut += ", Valid=false�����, ";
				}
				else
				{
					// screen out
					strScreenOut += ", Valid=true���, ";
				}

				// �ɽ���ʽ 
				string strMethod = vEle[cstrMethodKey].GetString();
				int iMethod = 0;
				Tgw_StringUtil::String2Int_atoi(strMethod, iMethod);

				// �ɽ�����
				string strDealNumber = vEle[cstrDealNumKey].GetString();
				int iDealNumber = 0;
				Tgw_StringUtil::String2Int_atoi(strDealNumber, iDealNumber);
				// screen out
				strScreenOut += "DealNumber=";
				strScreenOut += strDealNumber;

				//
				// judge
				if (strLinkId.empty())
				{
					string strDebug = "�ֶ�value-LinkId��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
					iStatus = 1;

					vResp.AddMember(cstrLinkId, rapidjson::Value(strLinkId.c_str(), in_allocResp), in_allocResp);
					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), in_allocResp), in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
				}

				bool bIsCheckAssert = { iValidation != 0 };

				enum simutgw::SysRunMode emRunmode = simutgw::g_iRunMode;
				enum simutgw::SysMatchMode emMatchMode = simutgw::SysMatchMode::SimulMatchAll;
				enum simutgw::QuotationType emquotType = simutgw::QuotationType::AveragePrice;

				// �ɽ�ģʽ
				if (0 == iMethod)
				{
					// ��00�� �� �ɽ�ģʽ����0:ʵ��ģ�� ʵ��ģ������ģʽ����0:����ξ���
					emMatchMode = simutgw::SysMatchMode::EnAbleQuta;
					emquotType = simutgw::QuotationType::AveragePrice;

					// screen out
					strScreenOut += ", MatchMode=EnAbleQutaʵ��ģ��, QuotationType=AveragePrice����ξ���, ";
				}
				else if (1 == iMethod)
				{
					// ��01�� �� �ɽ�ģʽ����0:ʵ��ģ�� ʵ��ģ������ģʽ����1:��һ��һ�۸�
					emMatchMode = simutgw::SysMatchMode::EnAbleQuta;
					emquotType = simutgw::QuotationType::SellBuyPrice;

					// screen out
					strScreenOut += ", MatchMode=EnAbleQutaʵ��ģ��, QuotationType=SellBuyPrice��һ��һ�۸�, ";
				}
				else if (2 == iMethod)
				{
					// ��02�� �� �ɽ�ģʽ����0:ʵ��ģ�� ʵ��ģ������ģʽ����2:����ɽ���
					emMatchMode = simutgw::SysMatchMode::EnAbleQuta;
					emquotType = simutgw::QuotationType::RecentMatchPrice;

					// screen out
					strScreenOut += ", MatchMode=EnAbleQutaʵ��ģ��, QuotationType=RecentMatchPrice����ɽ���, ";
				}
				else if (11 == iMethod)
				{
					// ��11�� ���ɽ�ģʽ����1:����ģ��-ȫ���ɽ�
					emMatchMode = simutgw::SysMatchMode::SimulMatchAll;

					// screen out
					strScreenOut += ", MatchMode=SimulMatchAll����ģ��-ȫ���ɽ�, ";
				}
				else if (12 == iMethod)
				{
					// ��12�� ���ɽ�ģʽ����2:����ģ��-�ֱʳɽ�
					emMatchMode = simutgw::SysMatchMode::SimulMatchByDivide;

					// screen out
					strScreenOut += ", MatchMode=SimulMatchByDivide����ģ��-�ֱʳɽ�, ";
				}
				else if (13 == iMethod)
				{
					// ��13�� ���ɽ�ģʽ����3:����ģ��-�ҵ�(�ɳ���)
					emMatchMode = simutgw::SysMatchMode::SimulNotMatch;

					// screen out
					strScreenOut += ", MatchMode=SimulNotMatch����ģ��-�ҵ�(�ɳ���), ";
				}
				else if (14 == iMethod)
				{
					// ��14�� ���ɽ�ģʽ����4:����ģ��-��
					emMatchMode = simutgw::SysMatchMode::SimulErrMatch;

					// screen out
					strScreenOut += ", MatchMode=SimulErrMatch����ģ��-��, ";
				}
				else if (15 == iMethod)
				{
					// ��15�� ���ɽ�ģʽ����5:����ģ��-���ֳɽ�
					emMatchMode = simutgw::SysMatchMode::SimulMatchPart;

					// screen out
					strScreenOut += ", MatchMode=SimulMatchPart����ģ��-���ֳɽ�, ";
				}
				else
				{
					string strDebug = "�ֶ�value-method��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
					iStatus = 1;

					vResp.AddMember(cstrLinkId, rapidjson::Value(strLinkId.c_str(), in_allocResp), in_allocResp);
					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), in_allocResp), in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
				}

				if (iDealNumber <= 1)
				{
					iDealNumber = 2;
				}

				int iRes = 0;
				switch (iLinkFlag)
				{
					// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�
				case 0:
					if (strSeatNumber.empty())
					{
						iRes = simutgw::g_tradePolicy.Set_Sz(strLinkId, emRunmode, bIsCheckAssert, emMatchMode, emquotType, iDealNumber);
						if (0 == iRes)
						{
							BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "�л�ģʽ�ɹ�:" << strScreenOut;
						}
						else
						{
							BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�л�ģʽʧ��:" << strScreenOut;
						}
					}
					else
					{
						iRes = simutgw::g_tradePolicy.Set_Sz(strLinkId, strSeatNumber, emRunmode, bIsCheckAssert, emMatchMode, emquotType, iDealNumber);
						if (0 == iRes)
						{
							BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "�л�ģʽ�ɹ�:" << strScreenOut;
						}
						else
						{
							BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�л�ģʽʧ��:" << strScreenOut;
						}
					}

					break;

				case 1:
					iRes = simutgw::g_tradePolicy.Set_Sh(strLinkId, emRunmode, bIsCheckAssert, emMatchMode, emquotType, iDealNumber);
					if (0 == iRes)
					{
						BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "�л�ģʽ�ɹ�:" << strScreenOut;
					}
					else
					{
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�л�ģʽʧ��:" << strScreenOut;
					}
					break;

				default:

					vResp.AddMember(cstrLinkId, rapidjson::Value(strLinkId.c_str(), in_allocResp), in_allocResp);
					vResp.AddMember(cstrStatusKey, 1, in_allocResp);
					vResp.AddMember(cstrTextKey, "in_allocResp error", in_allocResp);
					io_docResp.PushBack(vResp, in_allocResp);
					continue;
					break;
				}

				vResp.AddMember(cstrLinkId, rapidjson::Value(strLinkId.c_str(), in_allocResp), in_allocResp);
				vResp.AddMember(cstrStatusKey, iRes, in_allocResp);
				io_docResp.PushBack(vResp, in_allocResp);
				continue;
			}
		}
	}
	catch (exception& e)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << e.what();
		iAllStatus = 1;
	}

	if (0 == iAllStatus)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "ģʽ�л��ɹ�";
		return 0;
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ģʽ�л�ʧ��";
		return -1;
	}
}

/*
��ȡ ͨ���ͳɽ��������õİ󶨹�ϵ

@param rapidjson::Value& in_jsonvalue : json��Ϣ�е�value����
@param rapidjson::Document& io_docResp : �ر�����
@param rapidjson::Document::AllocatorType& in_allocResp: �ر�����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::Get_LinkMatchRuleRelations(rapidjson::Value& in_jsonvalue, rapidjson::Document& io_docResp,
	rapidjson::Document::AllocatorType& in_allocResp)
{
	static const string ftag("Clienter::Get_LinkMatchRuleRelations() ");

	/*
	"matchRules":[
	{
	"linkid":"1",
	"linkFlag":"0",
	"rules":[
	{"ruleid":"1","timestamp":1578646788},
	{"ruleid":"2","timestamp":1578646788}
	]
	},
	{
	"linkid":"2",
	"linkFlag":"1",
	"rules":[
	{"ruleid":"1","timestamp":1578646788},
	{"ruleid":"2","timestamp":1578646788}
	]
	}
	]
	*/

	// value content
	// ͨ��ID��UUID��
	static const char cstrLinkId[] = "linkid";
	// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�
	static const char cstrLinkFlag[] = "linkFlag";
	// ͨ���µĳɽ�����
	static const char cstrRules[] = "rules";

	static const char cstrRuleId[] = "ruleid";
	static const char cstrTimestamp[] = "timestamp";

	// response
	static const char cstrStatusKey[] = "status";
	static const char cstrTextKey[] = "text";

	// ����״̬
	int iAllStatus = 0;

	string strScreenOut;

	try
	{
		// ����
		// ����ʽ
		if (!in_jsonvalue.IsArray())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "��Ϣ���岻��ȷ";
			return -1;
		}

		for (rapidjson::SizeType i = 0; i < in_jsonvalue.Size(); ++i)
		{
			rapidjson::Value& elem = in_jsonvalue[i];
			// "linkId"
			if (!elem.HasMember(cstrLinkId) ||
				!elem[cstrLinkId].IsString())
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�ֶ�linkId��ʽ����ȷ";
				return -1;
			}

			string strName = elem[cstrLinkId].GetString();

			// "linkFlag"
			if (!elem.HasMember(cstrLinkFlag) ||
				!elem[cstrLinkFlag].IsString())
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�ֶ�linkFlag��ʽ����ȷ";
				return -1;
			}

			string strLinkFlag = elem[cstrLinkFlag].GetString();

			// "rules"
			if (!elem.HasMember(cstrRules) ||
				!elem[cstrRules].IsArray())
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�ֶ�rules��ʽ����ȷ";
				return -1;
			}

			string strRuleId("");
			uint64_t ui64Timestamp = 0;
			uint64_t ui64RuleId = 0;
			for (rapidjson::SizeType j = 0; j < elem[cstrRules].Size(); ++j)
			{
				rapidjson::Value& elemRules = elem[cstrRules][j];

				if (!elemRules.HasMember(cstrRuleId) || !elemRules[cstrRuleId].IsString()
					|| !elemRules.HasMember(cstrTimestamp) || !elemRules[cstrTimestamp].IsInt())
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�ֶ�rules��ʽ����ȷ";
					return -1;
				}

				strRuleId = elemRules[cstrRuleId].GetString();
				ui64Timestamp = elemRules[cstrTimestamp].GetInt();
				ui64RuleId = 0;
				int iRes = Tgw_StringUtil::String2UInt64_strtoui64(strRuleId, ui64RuleId);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ȡ���Ϻ������ֶ�rules���ݲ���ȷ";
					return -1;
				}

				// rules
				if (0 == strLinkFlag.compare("0"))
				{
					// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�

					bool bFindSzMapName = false;
					string strSzTargetCompID("");

					// ���ڽӿں�Web���õĶ�Ӧ��ϵ���� �Զ�ID TargetCompID ΪKey��strConnId��Value��
					for (std::map<std::string, struct Connection_webConfig>::iterator it = simutgw::g_mapSzConn_webConfig.begin();
						simutgw::g_mapSzConn_webConfig.end() != it; ++it)
					{
						bFindSzMapName = it->second.HasWebLinkid(strName);
						if (bFindSzMapName)
						{
							// �ҵ� Value����ConnId�ģ��Զ�ID TargetCompID ΪKey
							strSzTargetCompID = it->first;
							break;
						}
					}

					if (bFindSzMapName)
					{
						// ��¼���ڽӿں�Web���õĶ�Ӧ��ϵ
						// �� �Զ�ID TargetCompID ΪKey��strConnId��Value��
						simutgw::g_mapSzConn_webConfig[strSzTargetCompID].mapLinkRules.insert(std::pair<uint64_t, uint64_t>(ui64RuleId, ui64Timestamp));
					}
					else
					{
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Shenzhen webConfig δ�ҵ����� Linkid=" << strName;

						return -1;
					}

				}
				else if (0 == strLinkFlag.compare("1"))
				{
					// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�

					// ��¼�Ϻ��ӿں�Web���õĶ�Ӧ��ϵ
					simutgw::g_mapShConn_webConfig[strName].mapLinkRules.insert(std::pair<uint64_t, uint64_t>(ui64RuleId, ui64Timestamp));
				}
			}
		}

		return 0;
	}
	catch (exception& e)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << e.what();

		return -1;
	}
}

/*
ȡϵͳ������Ϣ

@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�sysConfig����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::GetSysConfig(rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::GetSysConfig() ");

	static const char cstrsz_link_enable[] = "sz_link_enable";
	static const char cstrsh_link_enable[] = "sh_link_enable";
	static const char cstrrun_mode[] = "run_mode";
	static const char cstrenable_check_assets[] = "enable_check_assets";
	static const char cstrmatch_mode[] = "match_mode";
	static const char cstrpart_match_num[] = "part_match_num";

	std::string strTrans;
	int iValue = 0;
	// ���
	if (!in_jsonvalue.HasMember(cstrsz_link_enable) ||
		!in_jsonvalue[cstrsz_link_enable].IsString())
	{
		string strDebug = "ȡ��ϵͳ�����ֶ�sz_link_enable��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -1;
	}
	strTrans = in_jsonvalue[cstrsz_link_enable].GetString();
	Tgw_StringUtil::String2Int_atoi(strTrans, iValue);

	// �Ƿ�����������Ϣ����
	if (iValue >= 1)
	{
		simutgw::g_bEnable_Sz_Msg_Task = true;
	}
	else
	{
		simutgw::g_bEnable_Sz_Msg_Task = false;
	}

	// ���
	if (!in_jsonvalue.HasMember(cstrsh_link_enable) ||
		!in_jsonvalue[cstrsh_link_enable].IsString())
	{
		string strDebug = "ȡ��ϵͳ�����ֶ�sh_link_enable��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -1;
	}
	strTrans = in_jsonvalue[cstrsh_link_enable].GetString();
	Tgw_StringUtil::String2Int_atoi(strTrans, iValue);

	// �Ƿ������Ϻ���Ϣ����
	if (iValue >= 1)
	{
		simutgw::g_bEnable_Sh_Msg_Task = true;
	}
	else
	{
		simutgw::g_bEnable_Sh_Msg_Task = false;
	}

	// ���
	if (!in_jsonvalue.HasMember(cstrrun_mode) ||
		!in_jsonvalue[cstrrun_mode].IsString())
	{
		string strDebug = "ȡ��ϵͳ�����ֶ�run_mode��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -1;
	}
	strTrans = in_jsonvalue[cstrrun_mode].GetString();
	Tgw_StringUtil::String2Int_atoi(strTrans, iValue);

	// ����ģʽ
	Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.run_mode"), iValue);
	string strRunModeType("��ǰ����ģʽ--");
	if (1 == iValue)
	{
		// 1 -- ѹ��ģʽ;
		simutgw::g_iRunMode = simutgw::SysRunMode::PressureMode;
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strRunModeType << "ѹ��ģʽ";
	}
	else if (2 == iValue)
	{
		// 2 -- ����ģʽ
		simutgw::g_iRunMode = simutgw::SysRunMode::MiniMode;
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strRunModeType << "����ģʽ";
	}
	else if (3 == iValue)
	{
		// 3 -- ��ͨģʽ
		simutgw::g_iRunMode = simutgw::SysRunMode::NormalMode;
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strRunModeType << "��ͨģʽ";
	}
	else
	{
		// 3 -- ��ͨģʽ
		simutgw::g_iRunMode = simutgw::SysRunMode::NormalMode;
	}

	// ���
	if (!in_jsonvalue.HasMember(cstrenable_check_assets) ||
		!in_jsonvalue[cstrenable_check_assets].IsString())
	{
		string strDebug = "ȡ��ϵͳ�����ֶ�enable_check_assets��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -1;
	}
	strTrans = in_jsonvalue[cstrenable_check_assets].GetString();
	Tgw_StringUtil::String2Int_atoi(strTrans, iValue);

	// �Ƿ������������
	if (iValue >= 1 && simutgw::g_iRunMode == simutgw::SysRunMode::NormalMode)
	{
		// ��ͨģʽ��֧�����
		simutgw::g_bEnable_Check_Assets = true;
	}
	else
	{
		// ����ģʽ��֧��
		simutgw::g_bEnable_Check_Assets = false;
	}

	// ���
	if (!in_jsonvalue.HasMember(cstrmatch_mode) ||
		!in_jsonvalue[cstrmatch_mode].IsString())
	{
		string strDebug = "ȡ��ϵͳ�����ֶ�match_mode��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -1;
	}
	strTrans = in_jsonvalue[cstrmatch_mode].GetString();
	Tgw_StringUtil::String2Int_atoi(strTrans, iValue);

	// �ɽ�ģʽ
	string strMatchModeType("��ǰ�ɽ�ģʽ--");
	if (0 == iValue)
	{
		// ��00�� �� �ɽ�ģʽ����0:ʵ��ģ�� ʵ��ģ������ģʽ����0:����ξ���
		simutgw::g_iMatchMode = simutgw::SysMatchMode::EnAbleQuta;
		simutgw::g_Quotation_Type = simutgw::QuotationType::AveragePrice;
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strMatchModeType << "ʵ��ģ��-����";
	}
	else if (1 == iValue)
	{
		// ��01�� �� �ɽ�ģʽ����0:ʵ��ģ�� ʵ��ģ������ģʽ����1:��һ��һ�۸�
		simutgw::g_iMatchMode = simutgw::SysMatchMode::EnAbleQuta;
		simutgw::g_Quotation_Type = simutgw::QuotationType::SellBuyPrice;
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strMatchModeType << "ʵ��ģ��-��һ��һ��";
	}
	else if (2 == iValue)
	{
		// ��02�� �� �ɽ�ģʽ����0:ʵ��ģ�� ʵ��ģ������ģʽ����2:����ɽ���
		simutgw::g_iMatchMode = simutgw::SysMatchMode::EnAbleQuta;
		simutgw::g_Quotation_Type = simutgw::QuotationType::RecentMatchPrice;
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strMatchModeType << "ʵ��ģ��-����ɽ���";
	}
	else if (11 == iValue)
	{
		// ��11�� ���ɽ�ģʽ����1:����ģ��-ȫ���ɽ�
		simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchAll;
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strMatchModeType << "ģ�����-ȫ���ɽ�";
	}
	else if (12 == iValue)
	{
		// ��12�� ���ɽ�ģʽ����2:����ģ��-�ֱʳɽ�
		simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchByDivide;
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strMatchModeType << "ģ�����-�ֱʳɽ�";
	}
	else if (13 == iValue)
	{
		// ��13�� ���ɽ�ģʽ����3:����ģ��-�ҵ�(�ɳ���)
		simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulNotMatch;
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strMatchModeType << "ģ�����-�ҵ�(�ɳ���)";
	}
	else if (14 == iValue)
	{
		// ��14�� ���ɽ�ģʽ����4:����ģ��-��
		simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulErrMatch;
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strMatchModeType << "ģ�����-��";
	}
	else if (15 == iValue)
	{
		// ��15�� ���ɽ�ģʽ����5:����ģ��-���ֳɽ�
		simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchPart;
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << strMatchModeType << "ģ�����-���ֳɽ�";
	}
	else
	{
		string strError("�ɽ�ģʽδ֪matchmode=[");
		strError += simutgw::g_ptConfig.get<std::string>("system.match_mode");
		strError += "]";

		// �ص�Ĭ��
		simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchAll;

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strError;
	}

	// ���
	if (!in_jsonvalue.HasMember(cstrpart_match_num) ||
		!in_jsonvalue[cstrpart_match_num].IsString())
	{
		string strDebug = "ȡ��ϵͳ�����ֶ�part_match_num��ʽ����ȷ";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -1;
	}
	strTrans = in_jsonvalue[cstrpart_match_num].GetString();
	Tgw_StringUtil::String2Int_atoi(strTrans, iValue);

	// �ֱʳɽ�����
	if (iValue >= 2)
	{
		simutgw::g_ui32_Part_Match_Num = iValue;
	}
	else
	{
		// �ֱ�����Ϊ2��
		simutgw::g_ui32_Part_Match_Num = 2;
	}

	return 0;
}

/*
ȡ���ڽӿڲ���

@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�szConn����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::GetSzConnConfig(rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::GetSzConnConfig() ");
	static const string cstrname("name");
	static const string cstrenable("enable");

	// in_jsonvalue��һ��array
	std::string strSessionConfig;
	static const std::string cstrSession("[SESSION]\n");
	//static const std::string cstrEqual("=");
	//static const std::string cstrNewLine("\n");
	std::string strJsName;

	/*
	"szConn":[
	{
	"linkid":"715a9a6f9c844610b5f099359ae83dd8",
	"targetCompId":"192.168.60.76_8019",
	"socketAcceptPort":"8019",
	"senderCompId":"tgw_1",
	"seatNumber":""
	},
	{
	"linkid":"fe70a0bcbf1c41e9bc01a099095e642b",
	"targetCompId":"192.168.60.76_8020",
	"socketAcceptPort":"8020",
	"senderCompId":"applid_120_tgw_2",
	"seatNumber":""
	},
	{
	"linkid":"99c9a4bdf6654f71bb52ca3df692510b",
	"targetCompId":"192.168.60.76_8021",
	"socketAcceptPort":"8021",
	"senderCompId":"applid_120_tgw_3",
	"seatNumber":""
	},
	{
	"linkid":"d7eb73176b59442cabf721e32e00ada9",
	"targetCompId":"192.168.60.76_8022",
	"socketAcceptPort":"8022",
	"senderCompId":"applid_120_testdb22",
	"seatNumber":""
	}]
	*/

	// web linkid
	static const std::string cstr_Name_WebLinkid = "linkid";
	// web targetCompId
	static const std::string cstr_Name_targetCompId = "targetCompId";
	// web targetCompId_port
	static const std::string cstr_Name_socketAcceptPort = "socketAcceptPort";
	// web senderCompId
	static const std::string cstr_Name_senderCompId = "senderCompId";
	// ����ر���
	static const std::string cstr_Name_SettleGroupName = "settleGroup";

	std::string strWebLinkid;
	std::string strTargetCompId;
	std::string strSocketAcceptPort;
	string strSenderCompId;
	// ����ر���
	std::string strSettleGroupName;

	for (rapidjson::Value::ValueIterator it = in_jsonvalue.Begin();
		it != in_jsonvalue.End(); ++it)
	{
		if (!it->IsObject())
		{
			string strDebug = "ȡ���������Ӹ�ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			return -1;
		}

		strWebLinkid = "";
		strTargetCompId = "";
		strSocketAcceptPort = "";
		strSenderCompId = "";
		strSettleGroupName = "";

		//
		// get param
		for (rapidjson::Value::MemberIterator memIt = it->MemberBegin();
			memIt != it->MemberEnd(); ++memIt)
		{
			// name
			strJsName = memIt->name.GetString();

			// add links to map
			if (0 == cstr_Name_WebLinkid.compare(strJsName))
			{
				// web linkid
				strWebLinkid = memIt->value.GetString();
			}
			else if (0 == cstr_Name_targetCompId.compare(strJsName))
			{
				// web targetCompId
				strTargetCompId = memIt->value.GetString();
			}
			else if (0 == cstr_Name_socketAcceptPort.compare(strJsName))
			{
				// web targetCompId_port
				strSocketAcceptPort = memIt->value.GetString();
			}
			else if (0 == cstr_Name_senderCompId.compare(strJsName))
			{
				// web senderCompId
				strSenderCompId = memIt->value.GetString();
			}
			else if (0 == cstr_Name_SettleGroupName.compare(strJsName))
			{
				strSettleGroupName = memIt->value.GetString();
			}
		}

		/*
		;���ڽӿ�����
		[SESSION]
		;�����˿�
		SocketAcceptPort=8020
		;����ID
		SenderCompID=applid_120_tgw_2
		;�Զ�ID
		TargetCompID=192.168.60.76_8020

		==================================
		web
		"linkid":"715a9a6f9c844610b5f099359ae83dd8",
		"targetCompId" : "192.168.60.76",
		"socketAcceptPort" : "8019",
		"senderCompId" : "tgw_1",
		*/

		// Build session
		// ;�����˿�
		strSessionConfig += cstrSession;
		strSessionConfig += "SocketAcceptPort=";
		strSessionConfig += strSocketAcceptPort;
		// ;����ID
		strSessionConfig += "\nSenderCompID=";
		strSessionConfig += strSenderCompId;
		// ;�Զ�ID
		strSessionConfig += "\nTargetCompID=";
		strSessionConfig += strTargetCompId;

		strSessionConfig += "\n";

		// ��¼���ڽӿں�Web���õĶ�Ӧ��ϵ
		if (!strWebLinkid.empty())
		{
			struct Connection_webConfig stWebConf(strWebLinkid, strSettleGroupName);
			simutgw::g_mapSzConn_webConfig[strTargetCompId] = stWebConf;
		}
	}

	if (strSessionConfig.empty())
	{
		simutgw::g_strSzConfig = "";

		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "no Shenzhen STEP link config";

		return 0;
	}

	string strDay = boost::gregorian::to_iso_string(boost::gregorian::day_clock::local_day());
	simutgw::g_strSzConfig =
		"[DEFAULT]\n"
		"ConnectionType=acceptor\n"
		"StartTime=00:00:00\n"
		"EndTime=00:00:00\n"
		"ResetOnLogon=N\n"
		"FileStorePath=./log/store/";
	simutgw::g_strSzConfig += strDay;
	simutgw::g_strSzConfig += "\nFileLogPath=./log/log/";
	simutgw::g_strSzConfig += strDay;

	if (simutgw::g_bSZ_Step_ver110)
	{
		// ����STEP�ر���Ver1.10
		// ��ƽ̨��Ϣ��Ϣ Platform Info
		simutgw::g_strSzConfig += "\nTransportDataDictionary=./config/FIXT11.xml\n"
			"AppDataDictionary=./config/FIX50SP2.xml\n"
			"BeginString=FIXT.1.1\n"
			"HeartBtInt=30\n"
			"ReconnectInterval=5\n"
			"DefaultApplVerID=FIX.5.0SP2\n"
			"DefaultCstmApplVerID=STEP1.2.0_SZ_1.10\n"
			"CheckLatency=N\n"
			"PersistMessages=N\n";
	}
	else
	{
		simutgw::g_strSzConfig += "\nTransportDataDictionary=./config/FIXT11_beforev110.xml\n"
			"AppDataDictionary=./config/FIX50SP2_beforev110.xml\n"
			"BeginString=FIXT.1.1\n"
			"HeartBtInt=30\n"
			"ReconnectInterval=5\n"
			"DefaultApplVerID=FIX.5.0SP2\n"
			"DefaultCstmApplVerID=STEP1.2.0_SZ_1.00\n"
			"CheckLatency=N\n"
			"PersistMessages=N\n";
	}

	simutgw::g_strSzConfig += strSessionConfig;

	return 0;
}

/*
ȡ�Ϻ��ӿڲ���

@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�shConn����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::GetShConnConfig(rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::GetShConnConfig() ");

	/*
	"shConn":
	[{
	"linkid":"e5669e3b847e4e4a822cb09e410a69fc",
	"enable":"1",
	"serverip":"10.81.1.31",
	"serverport":"1433",
	"database":"Ashare_OIW",
	"uid":"sa",
	"pwd":"888888"
	}]
	*/
	static const char cstrname[] = "linkid";

	static const char cstrServerip[] = "serverip";
	static const char cstrServerport[] = "serverport";
	static const char cstrDatabase[] = "database";
	static const char cstrUid[] = "uid";
	static const char cstrPwd[] = "pwd";

	// ����ر���
	static const char cstrSettleGroupName[] = "settleGroup";

	// in_jsonvalue��һ��array

	std::string strTrans;
	for (rapidjson::Value::ValueIterator it = in_jsonvalue.Begin();
		it != in_jsonvalue.End(); ++it)
	{
		if (!it->IsObject())
		{
			string strDebug = "ȡ���Ϻ����Ӹ�ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			return -1;
		}

		// name
		if (!it->HasMember(cstrname) ||
			!(*it)[cstrname].IsString())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ȡ���Ϻ������ֶ�name��ʽ����ȷ";

			return -1;
		}
		std::string strName = (*it)[cstrname].GetString();

		// serverip
		if (!it->HasMember(cstrServerip) ||
			!(*it)[cstrServerip].IsString())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ȡ���Ϻ������ֶ�serverip��ʽ����ȷ";

			return -1;
		}
		std::string strServerip = (*it)[cstrServerip].GetString();

		// serverport
		if (!it->HasMember(cstrServerport) ||
			!(*it)[cstrServerport].IsString())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ȡ���Ϻ������ֶ�serverport��ʽ����ȷ";

			return -1;
		}
		std::string strServerport = (*it)[cstrServerport].GetString();

		// database
		if (!it->HasMember(cstrDatabase) ||
			!(*it)[cstrDatabase].IsString())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ȡ���Ϻ������ֶ�database��ʽ����ȷ";

			return -1;
		}
		std::string strDatabase = (*it)[cstrDatabase].GetString();

		// uid
		if (!it->HasMember(cstrUid) ||
			!(*it)[cstrUid].IsString())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ȡ���Ϻ������ֶ�uid��ʽ����ȷ";

			return -1;
		}
		std::string strUid = (*it)[cstrUid].GetString();

		// pwd
		if (!it->HasMember(cstrPwd) ||
			!(*it)[cstrPwd].IsString())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ȡ���Ϻ������ֶ�pwd��ʽ����ȷ";

			return -1;
		}
		std::string strPwd = (*it)[cstrPwd].GetString();

		// ����ر���
		std::string strSettleGroupName("");
		if (it->HasMember(cstrSettleGroupName) &&
			(*it)[cstrSettleGroupName].IsString())
		{
			strSettleGroupName = (*it)[cstrSettleGroupName].GetString();
		}

#ifdef _MSC_VER
		// "connection":"Driver={SQL Server};Server={10.81.1.31,1433};Connection Timeout=3;Network=DBMSSOCN;Database=Ashare_OIW;Uid=sa;Pwd=888888"
		std::string strConn = "Driver={SQL Server};Server={";
		strConn += strServerip;
		strConn += ",";
		strConn += strServerport;
		strConn += "};Connection Timeout=3;Database=";
		strConn += strDatabase;
		strConn += ";Uid=";
		strConn += strUid;
		strConn += ";Pwd=";
		strConn += strPwd;
		strConn += ";";
#else
		// "Driver=/usr/local/lib/libtdsodbc.so;Host=10.81.1.18;Port=1433;tds version = 7.4;Server=10.81.1.18;Database=Ashare_OIW;Uid=sa;Pwd=888888"
		std::string strConn = "Driver=/usr/local/lib/libtdsodbc.so;Host=";
		strConn += strServerip;
		strConn += ";Port=";
		strConn += strServerport;
		strConn += ";tds version=7.4;Server=";
		strConn += strServerip;
		strConn += ";Database=";
		strConn += strDatabase;
		strConn += ";Uid=";
		strConn += strUid;
		strConn += ";Pwd=";
		strConn += strPwd;
		strConn += ";";
#endif			
		// ��¼�Ϻ��ӿں�Web���õĶ�Ӧ��ϵ
		struct Connection_webConfig stWebConf(strName, strSettleGroupName);

		simutgw::g_mapShConns[strName].SetName(strName);
		simutgw::g_mapShConns[strName].SetConnection(strConn);

		// ��¼�Ϻ��ӿں�Web���õĶ�Ӧ��ϵ		
		simutgw::g_mapShConn_webConfig[strName] = stWebConf;

		// ����sh���Ӽ�����
		simutgw::g_counter.AddSh_LinkCounter(strName);

		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "add sh db link name=" << strName;
	}

	return 0;
}

/*
���� Etf��Ϣ ��Ϣ
@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcReqMsg_Etfinfo(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::ProcReqMsg_Etfinfo() ");

	// value content
	// response
	static const char cstrStatusKey[] = "status";
	static const char cstrTextKey[] = "text";

	// ����״̬
	int iStatus = 0;

	// �ر�����
	rapidjson::Document docRespTmp(rapidjson::kObjectType);
	rapidjson::Document::AllocatorType& allocRespTmp = docRespTmp.GetAllocator();

	try
	{
		// ����
		// �Ƿ�ΪArray
		if (!in_jsonvalue.IsObject())
		{
			string strDebug = "�ֶ�value��ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			docRespTmp.AddMember(cstrStatusKey, 1, allocRespTmp);
			docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
			iStatus = 1;
		}

		if (0 == iStatus)
		{
			rapidjson::Value & vEle = in_jsonvalue;
			//
			// ����Ƿ����engineId
			if (!vEle.HasMember(simutgw::client::cstrEngineIdKey) ||
				!vEle[simutgw::client::cstrEngineIdKey].IsString())
			{
				string strDebug = "�ֶ�value-engineId��ʽ����ȷ";
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

				docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
				iStatus = 1;
			}

			if (0 == iStatus)
			{
				//
				// ����Ƿ��뱾����ͬ
				string strRequestWebId = vEle[simutgw::client::cstrEngineIdKey].GetString();
				if (0 != simutgw::g_strWeb_id.compare(strRequestWebId))
				{
					string strDebug = "engineId�뵱ǰ���治һ��";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

					docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
					iStatus = 1;
				}
			}

			if (0 == iStatus)
			{
				//
				// ETF��Ϣ����
				string strErr;
				int iRes = simutgw::g_etfContainer.InsertEtf_FromWebControl(vEle, strErr);
				if (0 != iRes)
				{
					string strDebug = strErr;
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strErr;

					docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strErr.c_str(), allocRespTmp), allocRespTmp);
					iStatus = 1;
				}
			}
		}
	}
	catch (exception& e)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << e.what();
		iStatus = 1;
	}
	// engine id
	docRespTmp.AddMember(simutgw::client::cstrEngineIdKey,
		rapidjson::Value(simutgw::g_strWeb_id.c_str(), allocRespTmp), allocRespTmp);

	// Status
	docRespTmp.AddMember(cstrStatusKey, iStatus, allocRespTmp);

	// �ر�
	SendMsgToServer(simutgw::client::cstrKey_Etfinfo, &in_cstrOrigseqnum, docRespTmp, trivial::info);

	if (0 == iStatus)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "ETF��Ϣ����ɹ�";
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ETF��Ϣ����ʧ��";
	}

	return 0;
}

/*
���� �ɽ����øı� ��Ϣ
@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcReqMsg_MatchRule(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::ProcReqMsg_MatchRule() ");

	// value content
	// response
	static const char cstrStatusKey[] = "status";
	static const char cstrTextKey[] = "text";

	static const char cstrRules[] = "rules";
	static const char cstrRuleFlag[] = "ruleFlag";

	// ����״̬
	int iStatus = 0;

	// �ر�����
	rapidjson::Document docRespTmp(rapidjson::kObjectType);
	rapidjson::Document::AllocatorType& allocRespTmp = docRespTmp.GetAllocator();

	try
	{
		// ����
		// �Ƿ�ΪArray
		if (!in_jsonvalue.IsObject()
			|| !in_jsonvalue.HasMember(cstrRules)
			|| !in_jsonvalue[cstrRules].IsArray())
		{
			string strDebug = "�ֶ�value��ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			docRespTmp.AddMember(cstrStatusKey, 1, allocRespTmp);
			docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
			iStatus = 1;
		}

		if (0 == iStatus)
		{
			//
			// �ɽ����øı����
			int iRes = 0;
			std::string strRuleFlag;
			for (rapidjson::SizeType i = 0; i < in_jsonvalue[cstrRules].Size(); ++i)
			{
				rapidjson::Value& elem = in_jsonvalue[cstrRules][i];
				if (!elem.IsObject()
					|| !elem.HasMember(cstrRuleFlag)
					|| !elem[cstrRuleFlag].IsString())
				{
					string strDebug = "�ֶ�ruleFlag��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

					docRespTmp.AddMember(cstrStatusKey, 1, allocRespTmp);
					docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
					iStatus = 1;
					break;
				}

				// insert by rule type.
				strRuleFlag = elem[cstrRuleFlag].GetString();

				if (0 == strRuleFlag.compare("0"))
				{
					// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�
					iRes = simutgw::g_matchRule.InsertSzRule(elem);
					if (0 != iRes)
					{
						string strDebug = "�ɽ����øı�failed";
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

						docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
						iStatus = 1;
						break;
					}
				}
				else if (0 == strRuleFlag.compare("1"))
				{
					// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�
					iRes = simutgw::g_matchRule.InsertShRule(elem);
					if (0 != iRes)
					{
						string strDebug = "�ɽ����øı�failed";
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

						docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
						iStatus = 1;
						break;
					}
				}
				else
				{
					string strDebug = "�ֶ�ruleFlag��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

					docRespTmp.AddMember(cstrStatusKey, 1, allocRespTmp);
					docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
					iStatus = 1;
					break;
				}
			}
		}
	}
	catch (exception& e)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << e.what();
		iStatus = 1;
	}

	// engine id
	docRespTmp.AddMember(simutgw::client::cstrEngineIdKey,
		rapidjson::Value(simutgw::g_strWeb_id.c_str(), allocRespTmp), allocRespTmp);

	// Status
	docRespTmp.AddMember(cstrStatusKey, iStatus, allocRespTmp);

	// �ر�
	SendMsgToServer(simutgw::client::cstrKey_MatchRule, &in_cstrOrigseqnum, docRespTmp, trivial::info);

	if (0 == iStatus)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "�ɽ����øı�ɹ�";
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�ɽ����øı�ʧ��";
	}

	return 0;
}

/*
���� ͨ����Ӧ�ɽ��������øı����� ��Ϣ
@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcReqMsg_ChangeLinkRules(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::ProcReqMsg_ChangeLinkRules() ");

	/*{
	"key":"changeLinkRules",
	"msgseq":"2",
	"value":{
	"engineId":"201807181559531",
	"linkid":"1",
	"linkFlag":"1",
	"rules":[
	"1",
	"2"
	]
	}
	}*/
	// value content
	// ͨ��ID��UUID��
	static const char cstrLinkId[] = "linkid";
	// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�
	static const char cstrLinkFlag[] = "linkFlag";
	// ͨ���µĳɽ�����
	static const char cstrRules[] = "rules";
	static const char cstrRuleId[] = "ruleid";
	static const char cstrTimestamp[] = "timestamp";

	// response
	static const char cstrStatusKey[] = "status";
	static const char cstrTextKey[] = "text";

	// ����״̬
	int iStatus = 0;

	// �ر�����
	rapidjson::Document docRespTmp(rapidjson::kObjectType);
	rapidjson::Document::AllocatorType& allocRespTmp = docRespTmp.GetAllocator();

	try
	{
		// ����
		// �Ƿ�ΪObject
		if (!in_jsonvalue.IsArray())
		{
			string strDebug = "�ֶ�value !IsArray()";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			docRespTmp.AddMember(cstrStatusKey, 1, allocRespTmp);
			docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
			iStatus = 1;
		}

		if (0 == iStatus)
		{
			for (rapidjson::SizeType i = 0; i < in_jsonvalue.Size(); ++i)
			{
				if (1 == iStatus)
				{
					break;
				}

				rapidjson::Value& elemI = in_jsonvalue[i];

				//
				// ����Ƿ����engineId
				if (!elemI.HasMember(simutgw::client::cstrEngineIdKey) ||
					!elemI[simutgw::client::cstrEngineIdKey].IsString())
				{
					string strDebug = "�ֶ�value-engineId��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

					docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
					iStatus = 1;
					break;
				}
				else
				{
					//
					// ����Ƿ��뱾����ͬ
					string strRequestWebId = elemI[simutgw::client::cstrEngineIdKey].GetString();
					if (0 != simutgw::g_strWeb_id.compare(strRequestWebId))
					{
						string strDebug = "engineId�뵱ǰ���治һ��";
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

						docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
						iStatus = 1;
						break;
					}
				}

				// "linkId"
				if (!elemI.HasMember(cstrLinkId) ||
					!elemI[cstrLinkId].IsString())
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�ֶ�linkId��ʽ����ȷ";
					iStatus = 1;
					break;
				}

				// "linkFlag"
				if (!elemI.HasMember(cstrLinkFlag) ||
					!elemI[cstrLinkFlag].IsString())
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�ֶ�linkFlag��ʽ����ȷ";
					iStatus = 1;
					break;
				}

				// "rules"
				if (!elemI.HasMember(cstrRules) ||
					!elemI[cstrRules].IsArray())
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�ֶ�rules��ʽ����ȷ";
					iStatus = 1;
					break;
				}

				string strName = elemI[cstrLinkId].GetString();
				string strLinkFlag = elemI[cstrLinkFlag].GetString();
				std::map<std::string, struct Connection_webConfig>::iterator itConnWebconf;

				// clean rules first
				if (0 == strLinkFlag.compare("0"))
				{
					// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�

					// ��¼���ڽӿں�Web���õĶ�Ӧ��ϵ
					itConnWebconf = simutgw::g_mapSzConn_webConfig.find(strName);
					if (simutgw::g_mapSzConn_webConfig.end() != itConnWebconf)
					{
						itConnWebconf->second.mapLinkRules.clear();
					}
				}
				else if (0 == strLinkFlag.compare("1"))
				{
					// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�

					// ��¼�Ϻ��ӿں�Web���õĶ�Ӧ��ϵ
					itConnWebconf = simutgw::g_mapShConn_webConfig.find(strName);
					if (simutgw::g_mapShConn_webConfig.end() != itConnWebconf)
					{
						itConnWebconf->second.mapLinkRules.clear();
					}
				}

				string strRuleId("");
				uint64_t ui64Timestamp = 0;
				uint64_t ui64RuleId = 0;
				for (rapidjson::SizeType j = 0; j < elemI[cstrRules].Size(); ++j)
				{
					rapidjson::Value& elemJ = elemI[cstrRules][j];
					if (!elemJ.IsObject()
						|| !elemJ.HasMember(cstrRuleId)
						|| !elemJ[cstrRuleId].IsString()
						|| !elemJ.HasMember(cstrTimestamp)
						|| !elemJ[cstrTimestamp].IsInt())
					{
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�ֶ�rules��ʽ����ȷ";
						iStatus = 1;
						break;
					}

					strRuleId = elemJ[cstrRuleId].GetString();
					ui64Timestamp = elemJ[cstrTimestamp].GetInt();
					ui64RuleId = 0;
					int iRes = Tgw_StringUtil::String2UInt64_strtoui64(strRuleId, ui64RuleId);
					if (0 != iRes)
					{
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ȡ�������ֶ�rules���ݲ���ȷ";
						iStatus = 1;
						break;
					}

					// rules
					if (0 == strLinkFlag.compare("0"))
					{
						// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�

						// ��¼�Ϻ��ӿں�Web���õĶ�Ӧ��ϵ
						simutgw::g_mapSzConn_webConfig[strName].mapLinkRules.insert(std::pair<uint64_t, uint64_t>(ui64RuleId, ui64Timestamp));
					}
					else if (0 == strLinkFlag.compare("1"))
					{
						// ���ӱ�ʶ��0 ���ڣ�1�Ϻ�

						// ��¼�Ϻ��ӿں�Web���õĶ�Ӧ��ϵ
						simutgw::g_mapShConn_webConfig[strName].mapLinkRules.insert(std::pair<uint64_t, uint64_t>(ui64RuleId, ui64Timestamp));
					}
				}
			}
		}
	}
	catch (exception& e)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << e.what();
		iStatus = 1;
	}

	// engine id
	docRespTmp.AddMember(simutgw::client::cstrEngineIdKey,
		rapidjson::Value(simutgw::g_strWeb_id.c_str(), allocRespTmp), allocRespTmp);

	// Status
	docRespTmp.AddMember(cstrStatusKey, iStatus, allocRespTmp);

	// �ر�
	SendMsgToServer(simutgw::client::cstrKey_ChangeLinkRules, &in_cstrOrigseqnum, docRespTmp, trivial::info);

	if (0 == iStatus)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "�ɽ����øı�ɹ�";

		SystemInit::Step8_CheckLinkRuleSync();
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�ɽ����øı�ʧ��";
	}

	return 0;
}

/*
���� ���������ļ� ��Ϣ
@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcReqMsg_SettleAccounts(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::ProcReqMsg_SettleAccounts() ");

	// value content
	static const char cstrHttpUpLinkKey[] = "fileServerAddress";
	static const char cstrEngineIdKey[] = "engineId";
	static const char cstrKey_SettleGroups[] = "settleGroup";

	// response
	static const char cstrStatusKey[] = "status";
	static const char cstrTextKey[] = "text";

	static const char cstrUpContentKey[] = "upContent";
	static const char cstrUpErrorKey[] = "upError";

	// ����״̬
	int iStatus = 0;

	// �ر�
	rapidjson::Document docRespTmp(rapidjson::kObjectType);
	rapidjson::Document::AllocatorType& allocRespTmp = docRespTmp.GetAllocator();

	try
	{
		// ����
		// �Ƿ�ΪArray
		if (!in_jsonvalue.IsArray())
		{
			string strDebug = "�ֶ�value��ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			docRespTmp.AddMember(cstrStatusKey, 1, allocRespTmp);
			docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
			iStatus = 1;
		}
		else if (1 > in_jsonvalue.Size())
		{
			string strDebug = "�ֶ�value��ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			docRespTmp.AddMember(cstrStatusKey, 1, allocRespTmp);
			docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
			iStatus = 1;
		}


		if (0 == iStatus)
		{
			rapidjson::Value & vEle = in_jsonvalue[0];
			if (!vEle.IsObject())
			{
				string strDebug = "����ģʽ�����ֶ�value��ʽ����ȷ";
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

				docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
				iStatus = 1;
			}

			if (0 == iStatus)
			{
				//
				// ����Ƿ����engineId
				if (!vEle.HasMember(simutgw::client::cstrEngineIdKey) ||
					!vEle[simutgw::client::cstrEngineIdKey].IsString())
				{
					string strDebug = "�ֶ�value-engineId��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

					docRespTmp.AddMember(cstrStatusKey, 1, allocRespTmp);
					docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
					iStatus = 1;
				}
			}

			if (0 == iStatus)
			{
				//
				// ����Ƿ��뱾����ͬ
				string strRequestWebId = vEle[simutgw::client::cstrEngineIdKey].GetString();
				if (0 != simutgw::g_strWeb_id.compare(strRequestWebId))
				{
					string strDebug = "engineId�뵱ǰ���治һ��";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

					docRespTmp.AddMember(cstrStatusKey, 1, allocRespTmp);
					docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
					iStatus = 1;
				}
			}

			// http �ϴ�����
			string strHttpUpLink("");
			// http response
			string strHttpResp("");
			// http error
			string strHttpError("");

			vector<string> vctSettleGroups;

			if (0 == iStatus)
			{
				//
				// ����Ƿ���� httpuplink
				if (!vEle.HasMember(cstrHttpUpLinkKey) ||
					!vEle[cstrHttpUpLinkKey].IsString())
				{
					// �����ϴ�����
					strHttpUpLink = "";
				}
				else
				{
					// ���ϴ�����
					strHttpUpLink = vEle[cstrHttpUpLinkKey].GetString();
				}
			}

			if (0 == iStatus)
			{
				//
				// ����Ƿ���� SettleGroups
				if (!vEle.HasMember(cstrKey_SettleGroups) ||
					!vEle[cstrKey_SettleGroups].IsArray())
				{
					// ����
				}
				else
				{
					// ��SettleGroups
					for (rapidjson::SizeType i = 0; i < vEle[cstrKey_SettleGroups].Size(); ++i)
					{
						rapidjson::Value & vEleArray = vEle[cstrKey_SettleGroups][i];
						if (vEleArray.IsString())
						{
							string sGroup = vEleArray.GetString();
							vctSettleGroups.push_back(sGroup);
						}
					}
				}
			}

			if (0 == iStatus)
			{
				//
				// �������
				// �����ļ�·��
				string strDay;
				string strSettlePath;

				int iRes = simutgw::Simutgw_Settle(vctSettleGroups, strDay, strSettlePath);
				if (0 != iRes)
				{
					string strDebug = "�������failed";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

					docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
					iStatus = 1;
				}

				// engine id
				docRespTmp.AddMember(simutgw::client::cstrEngineIdKey,
					rapidjson::Value(simutgw::g_strWeb_id.c_str(), allocRespTmp), allocRespTmp);

				// Status
				docRespTmp.AddMember(cstrStatusKey, iStatus, allocRespTmp);

				// �ر�
				SendMsgToServer(simutgw::client::cstrKey_SettleAccounts, &in_cstrOrigseqnum, docRespTmp, trivial::info);

				if (0 == iStatus)
				{
					BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "��������ɹ�";
				}
				else
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�������ʧ��";
				}

				// ������ϴ����Ӳ��ϴ�
				if (!strHttpUpLink.empty())
				{
					// �г����������ļ�
					std::vector<std::string> vectFiles;
					FileOperHelper::ListFilesInPath(strSettlePath, vectFiles);

					if (0 != vectFiles.size())
					{
						// �ϴ������Ϣ
						rapidjson::Document docRespTmp_up(rapidjson::kObjectType);
						rapidjson::Document::AllocatorType& allocRespTmp_up = docRespTmp_up.GetAllocator();

						// engine id
						docRespTmp_up.AddMember(cstrEngineIdKey,
							rapidjson::Value(simutgw::g_strWeb_id.c_str(), allocRespTmp_up), allocRespTmp_up);

						//
						CurlHttpMultiFileUpload httpUper;
						iRes = httpUper.UpLoadFiles(strHttpUpLink, simutgw::g_strWeb_id, strDay, vectFiles, strHttpResp, strHttpError);
						if (0 == iRes)
						{
							// �ϴ��ɹ�
							// Status
							docRespTmp_up.AddMember(cstrStatusKey, 0, allocRespTmp_up);

							// upContent
							docRespTmp_up.AddMember(cstrUpContentKey,
								rapidjson::Value(strHttpResp.c_str(), allocRespTmp_up), allocRespTmp_up);

							BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "�����ļ��ϴ��ɹ�";
						}
						else
						{
							// �ϴ�ʧ��
							// Status
							docRespTmp_up.AddMember(cstrStatusKey, 1, allocRespTmp_up);

							// upError
							docRespTmp_up.AddMember(cstrUpErrorKey,
								rapidjson::Value(strHttpError.c_str(), allocRespTmp_up), allocRespTmp_up);

							BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�����ļ��ϴ�ʧ�� " << strHttpError;
						}

						// �ϴ������Ϣ
						SendMsgToServer(simutgw::client::cstrKey_SettleFileUpload, nullptr, docRespTmp_up, trivial::info);
					}
					else
					{
						//�������ļ�

					}
				}
			}
		}
	}
	catch (exception& e)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << e.what();
		iStatus = 1;
	}

	return 0;
}

/*
���� �������� ��Ϣ
@param
rapidjson::Value& in_jsonvalue json��Ϣ�е�value����

@Return
0 -- ����ɹ�
-1 -- ʧ��
*/
int Clienter::ProcReqMsg_DayEnd(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue)
{
	static const string ftag("Clienter::ProcReqMsg_DayEnd() ");

	// value content
	// response
	static const char cstrStatusKey[] = "status";
	static const char cstrTextKey[] = "text";

	// ����״̬
	int iStatus = 0;

	// �ر�����
	rapidjson::Document docRespTmp(rapidjson::kObjectType);
	rapidjson::Document::AllocatorType& allocRespTmp = docRespTmp.GetAllocator();

	try
	{
		// ����
		// �Ƿ�ΪArray
		if (!in_jsonvalue.IsArray())
		{
			string strDebug = "�ֶ�value��ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			docRespTmp.AddMember(cstrStatusKey, 1, allocRespTmp);
			docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
			iStatus = 1;
		}
		else if (1 > in_jsonvalue.Size())
		{
			string strDebug = "�ֶ�value��ʽ����ȷ";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			docRespTmp.AddMember(cstrStatusKey, 1, allocRespTmp);
			docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
			iStatus = 1;
		}

		if (0 == iStatus)
		{
			rapidjson::Value & vEle = in_jsonvalue[0];
			if (!vEle.IsObject())
			{
				string strDebug = "����ģʽ�����ֶ�value��ʽ����ȷ";
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

				docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
				iStatus = 1;
			}

			if (0 == iStatus)
			{
				//
				// ����Ƿ����engineId
				if (!vEle.HasMember(simutgw::client::cstrEngineIdKey) ||
					!vEle[simutgw::client::cstrEngineIdKey].IsString())
				{
					string strDebug = "�ֶ�value-engineId��ʽ����ȷ";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

					docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
					iStatus = 1;
				}
			}

			if (0 == iStatus)
			{
				//
				// ����Ƿ��뱾����ͬ
				string strRequestWebId = vEle[simutgw::client::cstrEngineIdKey].GetString();
				if (0 != simutgw::g_strWeb_id.compare(strRequestWebId))
				{
					string strDebug = "engineId�뵱ǰ���治һ��";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

					docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
					iStatus = 1;
				}
			}

			if (0 == iStatus)
			{
				//
				// �������
				int iRes = simutgw::Simutgw_DayEnd();
				if (0 != iRes)
				{
					string strDebug = "�������̲���failed";
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

					docRespTmp.AddMember(cstrTextKey, rapidjson::Value(strDebug.c_str(), allocRespTmp), allocRespTmp);
					iStatus = 1;
				}
			}
		}
	}
	catch (exception& e)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << e.what();
		iStatus = 1;
	}
	// engine id
	docRespTmp.AddMember(simutgw::client::cstrEngineIdKey,
		rapidjson::Value(simutgw::g_strWeb_id.c_str(), allocRespTmp), allocRespTmp);

	// Status
	docRespTmp.AddMember(cstrStatusKey, iStatus, allocRespTmp);

	// �ر�
	SendMsgToServer(simutgw::client::cstrKey_DayEnd, &in_cstrOrigseqnum, docRespTmp, trivial::info);

	if (0 == iStatus)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "�������̲����ɹ�";
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�������̲���ʧ��";
	}

	return 0;
}