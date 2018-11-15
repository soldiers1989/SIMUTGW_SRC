#include "LnxTcpHandler_Simutgw_Client.h"

#include "tool_net/PacketAssembler.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "tool_string/StringEncodeUtil.h"

#include "linux_epoll_client/EpollClient_Core.h"

#include "simutgw/tcp_simutgw_client/Clienter.h"

string LnxTcpHandler_Simutgw_Client::m_strPing("Ping");
string LnxTcpHandler_Simutgw_Client::m_strPong("Pong");

LnxTcpHandler_Simutgw_Client::LnxTcpHandler_Simutgw_Client(Clienter* pclienter)
	:m_scl(keywords::channel = "LnxTcpHandler_Simutgw_Client")
{
	m_pClienter = pclienter;
}

LnxTcpHandler_Simutgw_Client::~LnxTcpHandler_Simutgw_Client(void)
{
}

void LnxTcpHandler_Simutgw_Client::OnNewConnection(uint64_t cid, struct ConnectionInformation const & ci, std::shared_ptr<Epoll_Socket_Connection>& cConn)
{
	static const string ftag("LnxTcpHandler_Simutgw_Client::OnNewConnection() ");

	BOOST_LOG_SEV(m_scl, trivial::info) << ftag
		<< "new id=" << cid << ", ip=" << ci.strRemoteIpAddress
		<< ", port=" << ci.remotePortNumber;

	m_pClienter->SetConnectStatus( true );
}

void LnxTcpHandler_Simutgw_Client::OnReceiveData(uint64_t cid, std::vector<uint8_t> const &data)
{
	static const string ftag("LnxTcpHandler_Simutgw_Client::OnReceiveData() ");

	// 已分包部分
	std::vector<std::shared_ptr<simutgw::NET_PACKAGE>> vecRevDatas;

	// 添加到主buffer,并再次分包
	m_handlerMsg.AppendBuffer(cid, data, vecRevDatas);

	string strReport;
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

void LnxTcpHandler_Simutgw_Client::OnSentData(uint64_t cid, uint64_t byteTransferred)
{
	static const string ftag("LnxTcpHandler_Simutgw_Client::OnSentData() ");

	BOOST_LOG_SEV(m_scl, trivial::info) << ftag
		<< " id=" << cid << " byteTransferred=" << byteTransferred;
}

void LnxTcpHandler_Simutgw_Client::OnClientDisconnect(uint64_t cid, int errorcode)
{
	static const string ftag("LnxTcpHandler_Simutgw_Client::OnClientDisconnect() ");

	m_pClienter->SetConnectStatus( false );

	// 从 message buffer 删除
	m_handlerMsg.RemoveId(cid);
}

void LnxTcpHandler_Simutgw_Client::OnDisconnect(uint64_t cid, int errorcode)
{
	static const string ftag("LnxTcpHandler_Simutgw_Client::OnDisconnect() ");

	m_pClienter->SetConnectStatus( false );

	// 从 message buffer 删除
	m_handlerMsg.RemoveId(cid);
}

void LnxTcpHandler_Simutgw_Client::OnServerClose(int errorCode)
{
	// static const string ftag("LnxTcpHandler_Simutgw_Client::OnServerClose() ");
}

void LnxTcpHandler_Simutgw_Client::OnServerError(int errorCode)
{
	BOOST_LOG_SEV(m_scl, trivial::error) << "OnServerError() " << " error=" << errorCode
		<< " serror=" << strerror(errorCode);
}
