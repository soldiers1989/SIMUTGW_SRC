#include "EpollClient_EventHandler.h"

#include "util/EzLog.h"

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"

#include "tool_net/PacketAssembler.h"

#include "socket_conn_manage_base/ConnectionInformation.h"

using namespace std;

EpollClient_EventHandler::EpollClient_EventHandler(void)
	:m_scl(keywords::channel = "EpollClient_EventHandler"), m_pServiceOwner(nullptr)
{
}

EpollClient_EventHandler::~EpollClient_EventHandler(void)
{
}

void EpollClient_EventHandler::OnNewConnection(uint64_t cid, struct ConnectionInformation const & ci, std::shared_ptr<Epoll_Socket_Connection>& cConn)
{
	static const string ftag("EpollClient_EventHandler::OnNewConnection() ");

	BOOST_LOG_SEV(m_scl, trivial::info) << ftag
		<< "new id=" << cid << ", ip=" << ci.strRemoteIpAddress
		<< ", port=" << ci.remotePortNumber;
}
