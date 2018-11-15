#include "ClientHandler.h"

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

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/StringEncodeUtil.h"
#include "util/EzLog.h"
#include "tool_json/RapidJsonHelper_tgw.h"
#include "simutgw/tcp_simutgw_client/Clienter.h"

#include "simutgw/tcp_simutgw_client/ClientMessageParser.h"
#include "simutgw/tcp_simutgw_client/config_client_msgkey.h"
#include "simutgw/tcp_simutgw_client/g_client_values.h"

#include "simutgw_config/g_values_sys_run_config.h"

using namespace std;

ClientHandler::ClientHandler(Clienter* pclienter) 
	:m_scl(keywords::channel = "ClientHandler")
{
	m_pClienter = pclienter;
	IocpEventHandler::SetServiceOwner(pclienter);
}

ClientHandler::~ClientHandler(void)
{
}

void ClientHandler::OnNewConnection(uint64_t cid, struct ConnectionInformation const & c)
{
	static const string ftag("ClientHandler::OnNewConnection() ");

	string strTran;
	string strDebug = "new id=";
	strDebug += sof_string::itostr(cid, strTran);
	EzLog::i(ftag, strDebug);

	m_pClienter->SetConnectStatus( true );
}

void ClientHandler::OnReceiveData(uint64_t cid, std::vector<uint8_t> const &data)
{
	static const string ftag("ClientHandler::OnReceiveData() ");

	if (false)
	{
		string strTran;
		string strDebug;
		sof_string::itostr(cid, strTran);

		strDebug = "id=[";
		strDebug += strTran;

		strDebug += "],size=[";
		strDebug += sof_string::itostr(data.size(), strTran);
		strDebug += "],data=[";

		std::vector<uint8_t>::const_iterator it;
		string strMsgReceived;
		for (it = data.begin(); it != data.end(); ++it)
		{
			strMsgReceived += *it;
		}

		strDebug += strMsgReceived;
		strDebug += "]";

		EzLog::i(ftag, strDebug);
	}

	// 已分包部分
	std::vector<std::shared_ptr<simutgw::NET_PACKAGE>> vecRevDatas;

	m_handlerMsg.AppendBuffer(cid, data, vecRevDatas);	

	for (size_t i = 0; i < vecRevDatas.size(); ++i)
	{
		string strGbk;
		StringEncodeUtil::UtfToGbk(vecRevDatas[i]->data, strGbk);

		if (true)
		{
			string strTran;
			string strDebug;
			sof_string::itostr(cid, strTran);

			strDebug = "id=[";
			strDebug += strTran;
			strDebug += "],UtfToGbk data=[";
			strDebug += strGbk;
			strDebug += "]";

			EzLog::i(ftag, strDebug);
		}

		// 处理业务消息
		m_pClienter->ProcessReceivedData(strGbk);
	}
}

void ClientHandler::OnSentData(uint64_t cid, uint64_t byteTransferred)
{
	// static const string ftag("ClientHandler::OnSentData() ");

	// BOOST_LOG_SEV(m_scl, trivial::info) << ftag	<< " id=" << cid << " byteTransferred=" << byteTransferred;
}

void ClientHandler::OnClientDisconnect(uint64_t cid, int errorcode)
{
	static const string ftag("ClientHandler::OnClientDisconnect() ");

	string strTran;
	string strDebug = "id=";
	strDebug += sof_string::itostr(cid, strTran);
	EzLog::i(ftag, strDebug);

	try
	{
		GetServiceOwner().Shutdown(cid, SD_SEND);

		GetServiceOwner().Disconnect(cid);
	}
	catch (exception const & e)
	{
		EzLog::ex(ftag, e);
		return;
	}

	m_pClienter->SetConnectStatus( false );

	// 从 message buffer 删除
	m_handlerMsg.RemoveId(cid);
}

void ClientHandler::OnDisconnect(uint64_t cid, int errorcode)
{
	static const string ftag("ClientHandler::OnDisconnect() ");

	string strTran;
	string strDebug = "id=";
	strDebug += sof_string::itostr(cid, strTran);
	strDebug += " errorcode=";
	strDebug += sof_string::itostr(errorcode, strTran);
	EzLog::i(ftag, strDebug);

	m_pClienter->SetConnectStatus( false );

	// 从 message buffer 删除
	m_handlerMsg.RemoveId(cid);
}

void ClientHandler::OnServerClose(int errorCode)
{
	// OnServerClose时一般是程序退出时，因为是多线程程序，可能会引用到已经释放的资源
}

void ClientHandler::OnServerError(int errorCode)
{
	static const string ftag("ClientHandler::OnServerError() ");

	string strTran;
	string strDebug = "errorCode=";
	strDebug += sof_string::itostr(errorCode, strTran);
	EzLog::i(ftag, strDebug);
}