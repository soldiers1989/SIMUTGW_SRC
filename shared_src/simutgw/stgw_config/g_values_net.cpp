#include "g_values_net.h"

namespace simutgw
{
	// tcp server
	std::shared_ptr<SimutgwTcpServer> g_SocketIOCPServer;

	// socket¿Í»§¶Ë
	std::shared_ptr<Clienter> g_SocketClient;
}