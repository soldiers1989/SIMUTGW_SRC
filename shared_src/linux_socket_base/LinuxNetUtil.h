#ifndef __LINUX_NET_UTIL_H__
#define __LINUX_NET_UTIL_H__

#ifdef _MSC_VER
#error "this file is for linux only, doesn't support Windows!"
#else

#endif

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "util/EzLog.h"

namespace LinuxNetUtil
{
	int SetNonblocking(const int in_socketfd);

	int CreateListenSocket(const unsigned int in_port, const int in_backlog);

	int CreateConnectSocket(const string& in_serverIp, const unsigned int in_port);

	/*
	Accept client Connection

	@param const int in_listenfd : listen socket fd
	@param int& out_clientfd : accepted client socket fd
	@param string& out_remoteAddr : remote Internet address
	@param uint16_t& out_remotePort : remote Port number
	*/
	int AcceptConn(const int in_listenfd, int& out_clientfd, string& out_remoteAddr, uint16_t& out_remotePort);

	/*
	读取数据

	@param const int sockFd : socket file handler
	@param std::vector<uint8_t>& out_recvData : 接收到的数据
	@param int& out_errno : 出错时的错误码

	@return 0 : 读取正常
	@return -1 : 读取失败，错误原因查看返回的errno，一般情况需关闭sockfd
	*/
	int RecvData(const int sockFd, std::vector<uint8_t>& out_recvData, int& out_errno);

	/*
	发送数据

	@param [in] const int sockFd : socket file handler
	@param [in] std::vector<uint8_t>& in_sendData : 待发送的数据
	@param [out] size_t& out_byteTransferred : 发送的数据长度
	@param [out] int& out_errno : 出错时的错误码

	@return 0 : 发送数据已写入缓冲区
	@return 1 : 缓冲区长度不足
	@return -1 : 失败，错误原因查看返回的errno，一般情况需关闭sockfd
	*/
	int SendData(const int sockFd, std::vector<uint8_t>& in_sendData, uint64_t& out_byteTransferred, int& out_errno);

	int Add_EpollWatch_NewClient(const int in_epollfd, const int in_sockfd, int& out_errno);

	int Mod_EpollWatch_Send(const int in_epollfd, const int in_sockfd, int& out_errno);

	int Mod_EpollWatch_CancellSend(const int in_epollfd, const int in_sockfd, int& out_errno);
}

#endif // __LINUX_NET_UTIL_H__
