#include "ClientSocket.h"

#include <string>

#include "mstcpip.h"

#include "util/EzLog.h"
#include "tool_net/PacketAssembler.h"

using namespace std;

ClientSocket::ClientSocket(std::shared_ptr<IocpEventHandler> pIocpHandler)
	:m_pIocpHandler(pIocpHandler), m_usServer_port(0),
	m_pIocpdata(std::shared_ptr<IocpData_Client>(new IocpData_Client())),
	m_dNumOfThreads(0), m_ui64MyCid(0)
{
}

ClientSocket::~ClientSocket()
{
	CleanUp();
}


/*
��ʼ��������IOCP�̳߳�

Param :
void

Return :
0 -- ����ɹ�
-1 -- ����ʧ��
*/
int ClientSocket::Init(void)
{
	static const string ftag("ClientSocket::Init() ");

	try
	{
		int iResult = 0;

		m_dNumOfThreads = 10;

		//----------------------------------------
		// Initialize winsock
		iResult = WinSockUtil::InitializeWinsock();
		if ( 0 != iResult )
		{
			CleanUp();
			return -1;
		}

		// create IoCompletionPort
		iResult = IOCPUtil::InitializeIocp(m_dNumOfThreads, m_pIocpdata->m_hCompletePort);
		if ( 0 != iResult )
		{
			CleanUp();
			return -1;
		}

		//----------------------------------------
		// Start process threadpool
		iResult = InitializeThreadPool(m_dNumOfThreads);
		if ( 0 != iResult )
		{
			CleanUp();
			return -1;
		}

		// wait a little bit.
		Sleep(10);

	}
	catch ( exception& e )
	{
		EzLog::ex(ftag, e);

		CleanUp();
		return -1;
	}
	catch ( ... )
	{
		string strTran;
		string strDebug = "unkown exception with error: ";
		strDebug += sof_string::itostr((uint64_t) GetLastError(), strTran);

		EzLog::e(ftag, strDebug);

		CleanUp();
		return -1;
	}
	return 0;
}


/*
��ʼ���̳߳�

Param :
DWORD numThread : �߳�����

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int ClientSocket::InitializeThreadPool(DWORD numThread)
{
	static const string ftag("ClientSocket::InitializeThreadPool() ");

	if ( 0 == numThread )
	{
		numThread = 4;
	}

	m_pIocpThreadPool = std::shared_ptr<IOCP_ThreadPool<IocpData_Client, IOCP_Thread_Client>>(
		new IOCP_ThreadPool<IocpData_Client, IOCP_Thread_Client>(numThread, m_pIocpdata));
	if ( NULL == m_pIocpThreadPool )
	{
		string strTran;
		string strDebug = "new IOCP_ThreadPool() with error: ";
		strDebug += sof_string::itostr((uint64_t) GetLastError(), strTran);
		EzLog::e(ftag, strDebug);

		return -1;
	}

	int iRes = m_pIocpThreadPool->InitPool();
	if ( 0 != iRes )
	{
		EzLog::e(ftag, "failed");
		return -1;
	}

	iRes = m_pIocpThreadPool->StartPool();
	if ( 0 != iRes )
	{
		EzLog::e(ftag, "failed");
		return -1;
	}

	return 0;
}

/*
�ر�������Դ

Param :
void

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int ClientSocket::CleanUp(void)
{
	static const string ftag("ClientSocket::CleanUp() ");

	try
	{
		//! @remark
		//! See Network Programming for Microsoft Windows, SE Chapter 5
		//! for the following shutdown strategy.

		// Close all socket handles to flush out all pending overlapped
		// I/O operation.
		m_pIocpdata->m_connectionManager.CloseAllConnections();

		// Give out a NULL completion status to help unblock all worker
		// threads. This is retract all I/O request made to the threads, and
		// it may not be a graceful shutdown. It is the user's job to
		// graceful shutdown all connection before shutting down the server.
		unsigned int uiThreadNum = m_pIocpThreadPool->GetCurrentThreadNum();
		for ( unsigned int i = 0; i < uiThreadNum; ++i )
		{
			//Help threads get out of blocking - GetQueuedCompletionStatus()
			PostQueuedCompletionStatus(
				m_pIocpdata->m_hCompletePort,
				0,
				(DWORD)
				NULL,
				NULL);
		}

		//! @remark
		//! Some background information from Windows via CC++ by Mr. Richter.
		//!
		//! Before Windows Vista, when a thread issued an I/O request against 
		//! a device associated with a completion port, it was mandatory that 
		//! the thread remain alive until the request completed; otherwise, 
		//! Windows canceled any outstanding requests made by the thread. With 
		//! Windows Vista, this is no longer necessary: threads can now issue 
		//! requests and terminate; the request will still be processed and 
		//! the result will be queued to the completion port.
		m_pIocpThreadPool->StopPool_WaitAllFinish();

		if ( INVALID_SOCKET != m_pIocpdata->m_connectSocket )
		{
			closesocket(m_pIocpdata->m_connectSocket);
			m_pIocpdata->m_connectSocket = INVALID_SOCKET;
		}

		if ( INVALID_HANDLE_VALUE != m_pIocpdata->m_hCompletePort )
		{
			CloseHandle(m_pIocpdata->m_hCompletePort);
			m_pIocpdata->m_hCompletePort = INVALID_HANDLE_VALUE;
		}

		if ( NULL != m_pIocpdata->m_pIocpHandler )
		{
			m_pIocpdata->m_pIocpHandler->OnServerClose(0);
			m_pIocpdata->m_pIocpHandler.reset();
		}

		return 0;
	}
	catch ( exception& e )
	{
		EzLog::ex(ftag, e);
		return -1;
	}
	catch ( ... )
	{
		string strTran;
		string strDebug = "unkown exception with error: ";
		strDebug += sof_string::itostr((uint64_t) GetLastError(), strTran);

		EzLog::e(ftag, strDebug);
		return -1;
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
int ClientSocket::Connect(const string& in_ip, const u_short in_port)
{
	static const string ftag("ClientSocket::Connect() ");

	try
	{
		m_strServerIp = in_ip;
		m_usServer_port = in_port;
		//----------------------------------------
		// Declare and initialize variables

		int iResult = 0;

		// Create a client socket
		iResult = WinSock_Client::CreateClientSocket(in_ip, in_port, m_pIocpdata->m_connectSocket);
		if ( 0 != iResult )
		{
			CleanUp();
			return -1;
		}

		m_pIocpdata->m_pIocpHandler = m_pIocpHandler;

		iResult = SetKeepAlive();
		if ( 0 != iResult )
		{
			CleanUp();
			return -1;
		}

		// 
		struct ConnectionInformation cinfo = WinSockUtil::GetConnectionInformation(m_pIocpdata->m_connectSocket);

		m_ui64MyCid = m_pIocpdata->GetNextId();
		std::shared_ptr<Connection> c(new Connection(
			m_pIocpdata->m_connectSocket,
			m_ui64MyCid,
			m_pIocpdata->m_rcvBufferSize
			));

		// ���ջ�����
		int nRecvBuf = 256 * 1024;//����Ϊ32K
		int iSetSocket = setsockopt(m_pIocpdata->m_connectSocket, SOL_SOCKET, SO_RCVBUF, (const char*) &nRecvBuf, sizeof(int));
		if ( 0 != iSetSocket )
		{
			int i = 0;
		}
		//���ͻ�����
		int nSendBuf = 256 * 1024;//����Ϊ32K
		iSetSocket = setsockopt(m_pIocpdata->m_connectSocket, SOL_SOCKET, SO_SNDBUF, (const char*) &nSendBuf, sizeof(int));
		if ( 0 != iSetSocket )
		{
			int i = 0;
		}

		m_pIocpdata->m_connectionManager.AddConnection(c);

		// Associate the connect socket with the completion port
		iResult = IOCPUtil::RegisterIocpHandle((HANDLE) c->m_socket,
			( std::shared_ptr<IocpDataBase> )m_pIocpdata);
		if ( 0 != iResult )
		{
			CleanUp();
			return -1;
		}

		if ( NULL != m_pIocpdata->m_pIocpHandler )
		{
			m_pIocpdata->m_pIocpHandler->OnNewConnection(c->m_id, cinfo);
		}

		int lasterror = WinSockUtil::PostRecv(c->m_rcvContext);

		// Failed to post a queue a receive context. It is likely that the
		// connection is already terminated at this point (by user or client).
		// In such case, just remove the connection.
		if ( WSA_IO_PENDING != lasterror )
		{
			if ( true == c->CloseRcvContext() )
			{
				WinSockUtil::PostDisconnect(( std::shared_ptr<IocpDataBase> )m_pIocpdata, *c);
			}
		}

		return 0;
	}
	catch ( exception& e )
	{
		EzLog::ex(ftag, e);

		CleanUp();
		return -1;
	}
	catch ( ... )
	{
		string strTran;
		string strDebug = "unkown exception with error: ";
		strDebug += sof_string::itostr((uint64_t) GetLastError(), strTran);

		EzLog::e(ftag, strDebug);

		CleanUp();
		return -1;
	}
}

//!***************************************************************************
//! @details
//! Shutdown certain operation on the socket.
//!
//! check class SocketServiceBase::Shutdown()
//!***************************************************************************
void ClientSocket::Shutdown(uint64_t cid, int how)
{
	static const string ftag("ClientSocket::Shutdown() ");

	std::shared_ptr<Connection> connection =
		m_pIocpdata->m_connectionManager.GetConnection(cid);

	if ( NULL == connection )
	{
		string strTran;
		string strDebug = "Connection does not exist cid=";
		strDebug += sof_string::itostr(cid, strTran);
		EzLog::e(ftag, strDebug);
		return;
	}

	::shutdown(connection->m_socket, how);
}

//!***************************************************************************
//! @details
//! Fully disconnect from a connected client. Once all outstanding sends
//! are completed, a corresponding OnDisconnect callback will be invoked.
//!
//! check class SocketServiceBase::Disconnect()
//!***************************************************************************
void ClientSocket::Disconnect(void)
{
	static const string ftag("ClientSocket::Disconnect() ");

	std::shared_ptr<Connection> connection =
		m_pIocpdata->m_connectionManager.GetConnection(m_ui64MyCid);

	if ( NULL == connection )
	{
		string strTran;
		string strDebug = "Connection does not exist cid=";
		strDebug += sof_string::itostr(m_ui64MyCid, strTran);
		EzLog::e(ftag, strDebug);
		return;
	}

	Shutdown(m_ui64MyCid, SD_BOTH);

	::InterlockedIncrement(&connection->m_disconnectPending);

	// Disconnect context is special (hacked) because it is not
	// tied to a connection. During graceful shutdown, it is very
	// difficult to determine when exactly is a good time to 
	// remove the connection. For example, a disconnect context 
	// might have already been sent by the IOCP thread send handler, 
	// and you wouldn't know it unless mutex are used. To keep it as 
	// lock-free as possible, this disconnect context may be redundant.
	// The disconnect handler will gracefully reject the redundant 
	// disconnect context.
	WinSockUtil::PostDisconnect(( std::shared_ptr<IocpDataBase> )m_pIocpdata, *connection);
}

/*
��server��������

Param :
std::shared_ptr<std::vector<uint8_t>>& pdata : �Ѵ���ô�����unsigned char������

Return :
void
*/
void ClientSocket::Send(std::shared_ptr<std::vector<uint8_t>>& pdata)
{
	static const string ftag("SocketIOCPServer::Send() ");

	if ( nullptr == pdata )
	{
		return;
	}

	std::shared_ptr<Connection> connection =
		m_pIocpdata->m_connectionManager.GetConnection(m_ui64MyCid);

	if ( NULL == connection )
	{
		string strTran;
		string strDebug = "Connection does not exist cid=";
		strDebug += sof_string::itostr(m_ui64MyCid, strTran);
		EzLog::e(ftag, strDebug);
		return;
	}

	std::shared_ptr<IocpContext> sendContext =
		connection->CreateSendContext();

	// Take over user's data here and post it to the completion port.
	sendContext->m_data.swap(*pdata);
	sendContext->ResetWsaBuf();

	int lastError = WinSockUtil::PostSend(*sendContext);
	if ( WSA_IO_PENDING != lastError )
	{
		connection->m_sendQueue.RemoveSendContext(sendContext.get());

		// Undo the swap here before throwing. This way, the user's
		// data is untouched and they may proceed to recover.
		pdata->swap(sendContext->m_data);

		// ����errorcode ѡ���Ƿ�����
		// WSAENETRESET The connection has been broken due to keep - alive activity detecting a failure while the operation was in progress.
		if ( WSAENETRESET == lastError )
		{
			int iRes = ReConnect();
			if ( 0 != iRes )
			{
				EzLog::e(ftag, "�������ӷ����ʧ��");
			}
		}
		else
		{
			string strTran;
			string strDebug = "send to cid=";
			strDebug += sof_string::itostr(m_ui64MyCid, strTran);
			strDebug += " failed with error: ";
			strDebug += sof_string::itostr(lastError, strTran);
			EzLog::e(ftag, strDebug);
		}
	}
}

/*
����keepalive
@Return
0 -- �ɹ�
-1 -- ʧ��
*/
int ClientSocket::SetKeepAlive()
{
	static const string ftag("SocketIOCPServer::SetKeepAlive() ");

	std::string strTran;
	// ����keepalive
	bool bKeepAlive = TRUE;
	int iResult = setsockopt(m_pIocpdata->m_connectSocket, SOL_SOCKET, SO_KEEPALIVE,
		(const char*) &bKeepAlive, sizeof(bKeepAlive));
	if ( iResult == SOCKET_ERROR )
	{
		std::string strDebug("set keepalive failed with error:");
		strDebug += sof_string::itostr(WSAGetLastError(), strTran);
		EzLog::e(ftag, strDebug);

		return -1;

	}

	//On Windows Vista and later, the number of keep-alive probes (data retransmissions) is set to 10 and cannot be changed.
	// set keepalive param
	tcp_keepalive alivein;
	tcp_keepalive aliveout;
	// �೤ʱ�䣨ms��û�����ݾͿ�ʼsend������ 
	alivein.keepalivetime = 5 * 1000;
	// ÿ���೤ʱ�䣨ms��sendһ��������
	alivein.keepaliveinterval = 1 * 1000;
	alivein.onoff = TRUE;
	DWORD dw = 0;
	iResult = WSAIoctl(m_pIocpdata->m_connectSocket, SIO_KEEPALIVE_VALS, &alivein, sizeof(alivein),
		&aliveout, sizeof(aliveout), &dw, NULL, NULL);
	if ( iResult == SOCKET_ERROR )
	{
		std::string strDebug("set keepalive parameter failed with error:");
		strDebug += sof_string::itostr(WSAGetLastError(), strTran);
		EzLog::e(ftag, strDebug);

		return -1;

	}

	return 0;
}

/*
��server���½���socket tcp����

Return :
0 -- ���ӳɹ�
-1 -- ����ʧ��
*/
int ClientSocket::ReConnect()
{
	static const string ftag("ClientSocket::ReConnect() ");

	try
	{
		//----------------------------------------
		// Declare and initialize variables

		int iResult = 0;

		// Create a client socket
		iResult = WinSock_Client::CreateClientSocket(m_strServerIp, m_usServer_port, m_pIocpdata->m_connectSocket);
		if ( 0 != iResult )
		{
			CleanUp();
			return -1;
		}

		m_pIocpdata->m_pIocpHandler = m_pIocpHandler;

		iResult = SetKeepAlive();
		if ( 0 != iResult )
		{
			CleanUp();
			return -1;
		}

		// Remove 
		m_pIocpdata->m_connectionManager.RemoveConnection(m_ui64MyCid);

		// 
		struct ConnectionInformation cinfo = WinSockUtil::GetConnectionInformation(m_pIocpdata->m_connectSocket);

		m_ui64MyCid = m_pIocpdata->GetNextId();
		std::shared_ptr<Connection> c(new Connection(
			m_pIocpdata->m_connectSocket,
			m_ui64MyCid,
			m_pIocpdata->m_rcvBufferSize
			));

		m_pIocpdata->m_connectionManager.AddConnection(c);

		// Associate the connect socket with the completion port
		iResult = IOCPUtil::RegisterIocpHandle((HANDLE) c->m_socket,
			( std::shared_ptr<IocpDataBase> )m_pIocpdata);
		if ( 0 != iResult )
		{
			CleanUp();
			return -1;
		}

		if ( NULL != m_pIocpdata->m_pIocpHandler )
		{
			m_pIocpdata->m_pIocpHandler->OnNewConnection(c->m_id, cinfo);
		}

		int lasterror = WinSockUtil::PostRecv(c->m_rcvContext);

		// Failed to post a queue a receive context. It is likely that the
		// connection is already terminated at this point (by user or client).
		// In such case, just remove the connection.
		if ( WSA_IO_PENDING != lasterror )
		{
			if ( true == c->CloseRcvContext() )
			{
				WinSockUtil::PostDisconnect(( std::shared_ptr<IocpDataBase> )m_pIocpdata, *c);
			}
		}

		return 0;
	}
	catch ( exception& e )
	{
		EzLog::ex(ftag, e);

		CleanUp();
		return -1;
	}
	catch ( ... )
	{
		string strTran;
		string strDebug = "unkown exception with error: ";
		strDebug += sof_string::itostr((uint64_t) GetLastError(), strTran);

		EzLog::e(ftag, strDebug);

		CleanUp();
		return -1;
	}
}