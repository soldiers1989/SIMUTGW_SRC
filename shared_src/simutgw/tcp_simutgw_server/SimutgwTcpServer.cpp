#include "SimutgwTcpServer.h"

#include "tool_net/PacketAssembler.h"

SimutgwTcpServer::SimutgwTcpServer(const string& strIp, const u_short usPort)
	:m_scl(keywords::channel = "SimutgwTcpServer"),
	m_strIp(strIp), m_usPort(usPort)
{
	m_dNumOfThreads = 2;

#ifdef _MSC_VER
	m_pIocpHandler = std::shared_ptr<IocpEventHandler>(new EchoHandler());
#else
	m_dNumOfListenBacklogs = 100;
	m_pIocpHandler = std::shared_ptr<LnxTcpHandler_Simutgw_Server>(new LnxTcpHandler_Simutgw_Server(this));
#endif
}

SimutgwTcpServer::~SimutgwTcpServer(void)
{
	StopServer();
}

int SimutgwTcpServer::StartServer(void)
{
	static const string ftag("SimutgwTcpServer::Init() ");

#ifdef _MSC_VER
	m_socketServer = std::shared_ptr<SocketIOCPServer>(
		new SocketIOCPServer(m_strIp, m_usPort, m_pIocpHandler));
#else
	m_socketServer = std::shared_ptr<EpollServer_Core>(
		new EpollServer_Core(2, m_pIocpHandler));
#endif

	if ( nullptr == m_socketServer )
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << " not enough space to new SocketServer()";
		return -1;
	}
	int iRes = 0;

#ifdef _MSC_VER
	iRes = m_socketServer->Init(4);
#else
	iRes = m_socketServer->StartServer(m_usPort, m_dNumOfListenBacklogs);
#endif

	return iRes;
}

int SimutgwTcpServer::StopServer(void)
{
	static const string ftag("SimutgwTcpServer::StopServer() ");

	if ( nullptr != m_socketServer )
	{
#ifdef _MSC_VER
		m_socketServer->CleanUp();
#else
		m_socketServer->StopServer();
#endif

	}

	return 0;
}

/*
Send data to client id by cid

@param uint64_t cid : cid of the client
@param const int iMsgType : message type
@param const std::string &in_data : message

@return
*/
void SimutgwTcpServer::Send(uint64_t cid, const int iMsgType, const std::string &in_data)
{
	static const string ftag("SimutgwTcpServer::Send() ");

	if ( nullptr == m_socketServer )
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << " no SocketIOCPServer()";
		return;

	}

	shared_ptr<vector<uint8_t>> ptrVectNetData(new vector<uint8_t>());

	int iRes = PacketAssembler::LocalPackageToNetBuffer(iMsgType, in_data, nullptr, *ptrVectNetData);
	if ( 0 != iRes )
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag
			<< "LocalPackageToNet failed with err=" << iRes << ", msg=" << in_data;
		return;
	}

	m_socketServer->Send(cid, ptrVectNetData);
}

void SimutgwTcpServer::Shutdown(uint64_t cid, int how)
{
	static const string ftag("SimutgwTcpServer::Shutdown() ");

	if ( nullptr == m_socketServer )
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << " no SocketIOCPServer()";
		return;
	}

	m_socketServer->Shutdown(cid, how);
}

void SimutgwTcpServer::Disconnect(uint64_t cid)
{
	static const string ftag("SimutgwTcpServer::Disconnect() ");

	if ( nullptr == m_socketServer )
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << " no SocketIOCPServer()";
		return;
	}

	m_socketServer->Disconnect(cid);
}