#ifndef __CONF_NET_H__
#define __CONF_NET_H__

#include <memory>

#ifdef _MSC_VER
#include "simutgw/win_iocp_simutgw_server/EchoHandler.h"
#include "simutgw/win_iocp_simutgw_server/SocketIOCPServer.h"
#else
#include "linux_epoll_server/EpollServer_Core.h"
#include "simutgw/linux_tcp_simutgw_server/LnxTcpHandler_Simutgw_Server.h"
#endif

#include "simutgw/tcp_simutgw_server/SimutgwTcpServer.h"
#include "simutgw/tcp_simutgw_client/Clienter.h"

namespace simutgw
{
	// tcp server
	extern std::shared_ptr<SimutgwTcpServer> g_SocketIOCPServer;

	// socket¿Í»§¶Ë
	extern std::shared_ptr<Clienter> g_SocketClient;

}

#endif