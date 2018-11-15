#ifndef __SIMUTGW_TCP_SERVER_H__
#define __SIMUTGW_TCP_SERVER_H__

#include <string>
#include <stdint.h>
#include <memory>

#ifdef _MSC_VER
#include "simutgw/win_iocp_simutgw_server/EchoHandler.h"
#include "simutgw/win_iocp_simutgw_server/SocketIOCPServer.h"
#else
#include "linux_epoll_server/EpollServer_Core.h"
#include "simutgw/linux_tcp_simutgw_server/LnxTcpHandler_Simutgw_Server.h"
#endif


class SimutgwTcpServer
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

#ifdef _MSC_VER
	// 处理线程数
	DWORD m_dNumOfThreads;

	std::shared_ptr<SocketIOCPServer> m_socketServer;

	std::shared_ptr<IocpEventHandler> m_pIocpHandler;
#else
	unsigned long m_dNumOfThreads;
	unsigned long m_dNumOfListenBacklogs;

	std::shared_ptr<EpollServer_Core> m_socketServer;

	std::shared_ptr<LnxTcpHandler_Simutgw_Server> m_pIocpHandler;
#endif
	// listen ip
	std::string m_strIp;
	// listen端口
	u_short m_usPort;

	//
	// Functions
	//
public:
	SimutgwTcpServer(const std::string& strIp, const u_short usPort);
	virtual ~SimutgwTcpServer(void);

#ifdef _MSC_VER
	std::shared_ptr<SocketIOCPServer> GetServerCore(void)
	{
		return m_socketServer;
	}
#else
	std::shared_ptr<EpollServer_Core> GetServerCore(void)
	{
		return m_socketServer;
	}
#endif

	int StartServer(void);

	//!***************************************************************************
	//! @details
	//! Destructor
	//! Closes down the IO completion port, as well as ending all worker thread.
	//! All active connections will be shutdown abortively, and all outstanding
	//! sends are discarded. OnDisconnect callback will not be invoked if there
	//! are remaining connection in the IOCP server.
	//!
	//! For graceful shutdown, it is the user's responsibility to ensure that
	//! all connections are closed gracefully prior to destroying the IOCP server.
	//!
	//! OnServerClosed callback will be invoked when the server exits.
	//!
	//!***************************************************************************
	int StopServer(void);

	/*
	Send data to client id by cid

	@param uint64_t cid : cid of the client
	@param const int iMsgType : message type
	@param const std::string &in_data : message

	@return
	*/
	void Send(uint64_t cid, const int iMsgType, const std::string &in_data);

	//!***************************************************************************
	//! @details
	//! Shutdown certain operation on the socket.
	//!
	//! @param[in,out] cid
	//! The id of the connection to shut down.
	//!
	//! @param[in,out] how
	//! A flag that describes what types of operation will no longer be
	//! allowed. Possible values for this flag are listed in the Winsock2.h
	//! header file.
	//!
	//! SD_RECEIVE - Shutdown receive operations.
	//! SD_SEND - Shutdown send operations.
	//! SD_BOTH - Shutdown both send and receive operations.
	//!
	//! @throw
	//! CIocpException if connection no longer exists.
	//!
	//! CWin32Exception if the IOCP server failed to post this data to the
	//! IO Completion port.
	//!
	//! @post
	//! If the function completes successfully, the specified operation
	//! on the socket will no longer be allowed.
	//!
	//! @remark
	//! Shutdown should not be called on the same connection simultaneously from
	//! different threads. Otherwise, the post condition is undefined.
	//!
	//!***************************************************************************
	void Shutdown(uint64_t cid, int how);

	//!***************************************************************************
	//! @details
	//! Fully disconnect from a connected client. Once all outstanding sends
	//! are completed, a corresponding OnDisconnect callback will be invoked.
	//!
	//! @throw
	//! CIocpException if connection no longer exists.
	//!
	//! @param[in,out] cid
	//! The connection to disconnect from.
	//!
	//! @post
	//! If this function completes successfully, the socket will no longer
	//! be capable of sending or receiving new data (queued data prior to
	//! disconnect will be sent).
	//!
	//! @remark
	//! If the server is initiating the disconnect, it is recommended to call
	//! Shutdown(cid, SD_BOTH) and wait for OnClientDisconnected() callback prior
	//! to calling Disconnect(). Otherwise, it is possible OnReceiveData() callback
	//! to be called simultaneously with OnDisconnect() callback. After all, the
	//! client may simultaneously send data to this server during the
	//! Disconnect() call. In such scenario, you need to provide mutexes and
	//! additional logic to ignore the last sets of packets.
	//!
	//! Shutdown should not be called on the same connection simultaneously from
	//! different threads. Otherwise, the post condition is undefined.
	//!
	//!***************************************************************************
	void Disconnect(uint64_t cid);

protected:

private:
	SimutgwTcpServer(void);
};

#endif
