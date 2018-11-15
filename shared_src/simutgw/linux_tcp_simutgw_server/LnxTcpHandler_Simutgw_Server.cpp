#include "LnxTcpHandler_Simutgw_Server.h"

#include <stdexcept>

#include "tool_net/PacketAssembler.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"

#include "linux_epoll_server/EpollServer_Core.h"
#include "simutgw/tcp_simutgw_server/SimutgwTcpServer.h"

string LnxTcpHandler_Simutgw_Server::m_strPing("Ping");
string LnxTcpHandler_Simutgw_Server::m_strPong("Pong");

LnxTcpHandler_Simutgw_Server::LnxTcpHandler_Simutgw_Server(SimutgwTcpServer* pServer)
	:m_scl(keywords::channel = "LnxTcpHandler_Simutgw_Server")
{
	if ( nullptr == pServer )
	{
		throw std::invalid_argument("received nullptr server hanlder value");
	}
	m_pServer = pServer;
}

LnxTcpHandler_Simutgw_Server::~LnxTcpHandler_Simutgw_Server(void)
{
}

void LnxTcpHandler_Simutgw_Server::OnNewConnection(uint64_t cid, struct ConnectionInformation const & ci, std::shared_ptr<Epoll_Socket_Connection>& cConn)
{
	static const string ftag("LnxTcpHandler_Simutgw_Server::OnNewConnection() ");

	BOOST_LOG_SEV(m_scl, trivial::info) << ftag
		<< "new id=" << cid << ", ip=" << ci.strRemoteIpAddress
		<< ", port=" << ci.remotePortNumber;
}

void LnxTcpHandler_Simutgw_Server::OnReceiveData(uint64_t cid, std::vector<uint8_t> const &data)
{
	static const string ftag("LnxTcpHandler_Simutgw_Server::OnReceiveData() ");

	std::shared_ptr<EpollServer_Core> pServer = m_pServer->GetServerCore();
	if (nullptr == pServer)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag
			<< "socket server not started";
		return;
	}

	std::shared_ptr<Epoll_Socket_Connection> pClientConn = pServer->m_connectionManager.GetConnection(cid);
	if (nullptr == pClientConn)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag
			<< "connection manage couldn't find id=" << cid;
		return;
	}

	// 已分包部分
	std::vector<std::shared_ptr<simutgw::NET_PACKAGE>> vecRevDatas;

	// 添加到主buffer,并再次分包
	m_handlerMsg.AppendBuffer(cid, data, vecRevDatas);

	string strReport;
	uint64_t ui64ReportIndex = 0;
	for (size_t i = 0; i < vecRevDatas.size(); ++i)
	{
		/* 先取ReportIndex */

		if (nullptr != pClientConn)
		{
			pClientConn->GetSeq(cid, ui64ReportIndex);
		}
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << " rcv_data=" << vecRevDatas[i]->data;

		m_pServer->Send(cid, 1, "hello");
		// BOOST_LOG_SEV(m_scl, trivial::info) << ftag	<< " rept_data=" << strReport;
	}
}

void LnxTcpHandler_Simutgw_Server::OnSentData(uint64_t cid, uint64_t byteTransferred)
{
	static const string ftag("LnxTcpHandler_Simutgw_Server::OnSentData() ");

	BOOST_LOG_SEV(m_scl, trivial::info) << ftag
		<< " id=" << cid << " byteTransferred=" << byteTransferred;
}

void LnxTcpHandler_Simutgw_Server::OnClientDisconnect(uint64_t cid, int errorcode)
{
	static const string ftag("LnxTcpHandler_Simutgw_Server::OnClientDisconnect() ");

	// 从 message buffer 删除
	m_handlerMsg.RemoveId(cid);
}

void LnxTcpHandler_Simutgw_Server::OnDisconnect(uint64_t cid, int errorcode)
{
	static const string ftag("LnxTcpHandler_Simutgw_Server::OnDisconnect() ");

	// 从 message buffer 删除
	m_handlerMsg.RemoveId(cid);
}

void LnxTcpHandler_Simutgw_Server::OnServerClose(int errorCode)
{
	// static const string ftag("LnxTcpHandler_Simutgw_Server::OnServerClose() ");
}

void LnxTcpHandler_Simutgw_Server::OnServerError(int errorCode)
{
	BOOST_LOG_SEV(m_scl, trivial::error) << "OnServerError() " << " error=" << errorCode
		<< " serror=" << strerror(errorCode);
}
