#ifndef __EPOLL_SERVER_EVENT_HANDLER_H__
#define __EPOLL_SERVER_EVENT_HANDLER_H__

#include <stdint.h>
#include <vector>
#include <string>

#include "socket_conn_manage_base/ConnectionInformation.h"
#include "socket_conn_manage_base/EventBaseHandler.h"
#include "socket_conn_manage_base/SocketServiceBase.h"
#include "linux_epoll_server/Epoll_Socket_Connection.h"
#include "linux_epoll_server/Epoll_ConnectionManager.h"

#include "util/EzLog.h"

class EpollServer_Core;

/*
消息回调句柄类.
*/
class EpollServer_EventHandler : public EventBaseHandler
{
	//
	// Members
	//
public:

protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	EpollServer_Core *m_pServiceOwner;

	//
	// Functions
	//
public:
	EpollServer_EventHandler(void);
	virtual ~EpollServer_EventHandler(void);

	void SetServiceOwner(EpollServer_Core* hServer)
	{
		m_pServiceOwner = hServer;
	}

	EpollServer_Core* GetServiceOwner(void)
	{
		return m_pServiceOwner;
	}

	//!***************************************************************************
	//! @details
	//! This callback is invoked asynchronously when a new connection is accepted
	//! by the IOCP server. 
	//!
	//! @param[in] cid
	//! A unique Id that represents the connection. 
	//! This is the unique key that may be used for bookkeeping, and will remain
	//! valid until OnDisconnect is called.
	//!
	//! @param[in] c
	//! Information regarding the endpoints of the connection.
	//!
	//! @remark
	//! This callback is invoked through the context of an IOCP thread, which
	//! may or may not be your main thread's context.
	//!
	//!***************************************************************************
	virtual void OnNewConnection(uint64_t cid, struct ConnectionInformation const & ci, std::shared_ptr<Epoll_Socket_Connection>& cConn);

protected:

private:
};

#endif