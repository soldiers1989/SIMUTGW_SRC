#ifndef __EPOLL_SERVER_CLIENT_THREAD_H__
#define __EPOLL_SERVER_CLIENT_THREAD_H__

#ifdef _MSC_VER
#error "this file is for linux only, doesn't support Windows!"
#else

#endif

#include <atomic>
#include <errno.h>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "thread_pool_base/ThreadBase.h"
#include "linux_epoll_server/EpollServer_EventHandler.h"

#include "util/EzLog.h"

/*
TCP Server 用来和客户端进行通信处理的线程
*/
class EpollServer_Client_Thread :
	public ThreadBase
{
	//
	// Members
	//
public:
	// max epoll size
	static const int MAX_EPOLL_EVENTS;

	// max epoll client events
	static const int MAX_CLIENT_EVENTS;

protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	int m_epollfd;

	atomic<unsigned long long> m_aullClientNums;

	std::shared_ptr<EpollServer_EventHandler> m_pEventHandler;

	//
	// Functions
	//
public:
	EpollServer_Client_Thread(const uint64_t uiId, std::shared_ptr<EpollServer_EventHandler> pEventHandler);
	virtual ~EpollServer_Client_Thread();

	/*
	启动线程

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	virtual int StartThread();

	int AddNewClient(const int in_sockfd, int& out_epollfd);

	uint64_t GetCurrentClientNums(void) const;

protected:
	static void* Run(void* pParam);

private:
	// 禁止使用默认构造函数
	EpollServer_Client_Thread();
};

#endif // __EPOLL_SERVER_LISTEN_THREAD_H__
