#include "LinuxNetUtil.h"

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "tool_string/sof_string.h"
#include "util/EzLog.h"

namespace LinuxNetUtil
{
	using namespace std;

	int SetNonblocking(const int in_socketfd)
	{
		int old_option = fcntl(in_socketfd, F_GETFL);
		if (0 > old_option)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("fcntl F_GETFL error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e("SetNonblocking", sDebug);
			return -1;
		}

		int new_option = old_option | O_NONBLOCK;
		int iRes = fcntl(in_socketfd, F_SETFL, new_option);
		if (0 > iRes)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("fcntl F_SETFL error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e("SetNonblocking", sDebug);
			return -1;
		}

		return 0;
	}

	int SetKeepAlive(const int in_socketfd, int& out_errno)
	{
		static const string ftag("SetKeepAlive() ");

		// 开启keepalive属性
		int keepAlive = 1;
		// 如该连接在60秒内没有任何数据往来,则进行探测
		int keepIdle = 60;
		// 探测时发包的时间间隔为5 秒
		int keepInterval = 5;
		// 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
		int keepCount = 3;

		int iRes = setsockopt(in_socketfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
		if (0 != iRes)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("setsockopt error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(in_socketfd);

			out_errno = iErr;

			return -1;
		}

		iRes = (in_socketfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
		if (0 > iRes)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("setsockopt error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(in_socketfd);

			out_errno = iErr;

			return -1;
		}

		iRes = setsockopt(in_socketfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
		if (0 != iRes)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("setsockopt error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(in_socketfd);

			out_errno = iErr;

			return -1;
		}

		iRes = setsockopt(in_socketfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
		if (0 != iRes)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("setsockopt error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(in_socketfd);

			out_errno = iErr;

			return -1;
		}

		return 0;
	}

	int CreateListenSocket(const unsigned int in_port, const int in_backlog)
	{
		static const string ftag("CreateListenSocket() ");

		// AF_INET IPv4 Internet protocols
		// SOCK_STREAM Tcp socket
		// protocol ip 0 IP	internet protocol, pseudo protocol number
		int listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if (-1 == listenfd)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("create socket error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);
			return -1;
		}

		//设置地址可重用
		int reuse = 1;
		int iRes = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
		if (0 == iRes)
		{
			//cout << "setsockopt SO_REUSEADDR" << endl;
		}
		else
		{
			int iErr = errno;
			string sItoa;
			string sDebug("setsockopt error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(listenfd);

			return -1;
		}

		int recvbuf = 0;
		int recvbuf_len = 0;

		recvbuf_len = sizeof(recvbuf);
		//获取 接受缓冲区大小
		iRes = getsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, &recvbuf, (socklen_t*)&recvbuf_len);
		if (0 == iRes)
		{
			cout << "the receive buffer size is " << recvbuf << endl;
		}
		else
		{
			int iErr = errno;
			string sItoa;
			string sDebug("getsockopt error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(listenfd);

			return -1;
		}

		recvbuf_len = sizeof(recvbuf);
		//获取 发送缓冲区大小
		iRes = getsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, &recvbuf, (socklen_t*)&recvbuf_len);
		if (0 == iRes)
		{
			cout << "the send buffer size is " << recvbuf << endl;
		}
		else
		{
			int iErr = errno;
			string sItoa;
			string sDebug("getsockopt error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(listenfd);

			return -1;
		}

		// set non-blocking
		iRes = SetNonblocking(listenfd);
		if (0 != iRes)
		{
			string sDebug("SetNonblocking error");
			EzLog::e(ftag, sDebug);

			close(listenfd);

			return -1;
		}

		struct sockaddr_in servaddr;

		memset(&servaddr, 0, sizeof(servaddr));
		// AF_INET IPv4 Internet protocols
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(in_port);

		iRes = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
		if (-1 == iRes)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("bind socket in port=");
			sDebug += sof_string::itostr(in_port, sItoa);
			sDebug += " error=";
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(listenfd);

			return -1;
		}

		/*
		The backlog argument provides a hint to the  implementation  which  the
		implementation shall use to limit the number of outstanding connections
		in the socket's listen queue. Implementations may  impose  a  limit  on
		backlog  and  silently  reduce  the specified value. Normally, a larger
		backlog argument value shall result in a larger or equal length of  the
		listen  queue.   Implementations  shall support values of backlog up to
		SOMAXCONN, defined in <sys/socket.h>.
		*/
		iRes = listen(listenfd, in_backlog);
		if (-1 == iRes)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("listen socket in port=");
			sDebug += sof_string::itostr(in_port, sItoa);
			sDebug += " error=";
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(listenfd);

			return -1;
		}

		{
			string sItoa;
			string sDebug("listen socket in port=");
			sDebug += sof_string::itostr(in_port, sItoa);
			sDebug += " success";
			EzLog::i(ftag, sDebug);
		}
		return listenfd;
	}

	int CreateConnectSocket(const string& in_serverIp, const unsigned int in_port)
	{
		static const string ftag("CreateConnectSocket() ");

		int iRes = 0;

		// AF_INET IPv4 Internet protocols
		// SOCK_STREAM Tcp socket
		// protocol ip 0 IP	internet protocol, pseudo protocol number
		int clientfd = socket(AF_INET, SOCK_STREAM, 0);
		if (-1 == clientfd)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("create socket error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);
			return -1;
		}

		int recvbuf = 0;
		int recvbuf_len = 0;

		recvbuf_len = sizeof(recvbuf);
		//获取 接受缓冲区大小
		iRes = getsockopt(clientfd, SOL_SOCKET, SO_RCVBUF, &recvbuf, (socklen_t*)&recvbuf_len);
		if (0 == iRes)
		{
			cout << "the receive buffer size is " << recvbuf << endl;
		}
		else
		{
			int iErr = errno;
			string sItoa;
			string sDebug("getsockopt error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(clientfd);

			return -1;
		}

		recvbuf_len = sizeof(recvbuf);
		//获取 发送缓冲区大小
		iRes = getsockopt(clientfd, SOL_SOCKET, SO_SNDBUF, &recvbuf, (socklen_t*)&recvbuf_len);
		if (0 == iRes)
		{
			cout << "the send buffer size is " << recvbuf << endl;
		}
		else
		{
			int iErr = errno;
			string sItoa;
			string sDebug("getsockopt error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(clientfd);

			return -1;
		}

		int iErr_KeepAlive = 0;
		iRes = SetKeepAlive(clientfd, iErr_KeepAlive);
		if (0 == iRes)
		{

		}
		else
		{
			string sItoa;
			string sDebug("SetKeepAlive error=");
			sDebug += sof_string::itostr(iErr_KeepAlive, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr_KeepAlive);
			EzLog::e(ftag, sDebug);

			close(clientfd);

			return -1;
		}

		struct sockaddr_in servaddr;

		memset(&servaddr, 0, sizeof(servaddr));
		// AF_INET IPv4 Internet protocols
		servaddr.sin_family = AF_INET;
		inet_pton(AF_INET, in_serverIp.c_str(), &servaddr.sin_addr);
		servaddr.sin_port = htons(in_port);

		iRes = connect(clientfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
		if (-1 == iRes)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("connect socket in ip=");
			sDebug += in_serverIp;
			sDebug += " port=";
			sDebug += sof_string::itostr(in_port, sItoa);
			sDebug += " error=";
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(clientfd);

			return -1;
		}
		// set non-blocking
		iRes = SetNonblocking(clientfd);
		if (0 != iRes)
		{
			string sDebug("SetNonblocking error");
			EzLog::e(ftag, sDebug);

			close(clientfd);

			return -1;
		}

		{
			string sItoa;
			string sDebug("connect socket in ip=");
			sDebug += in_serverIp;
			sDebug += " port=";
			sDebug += sof_string::itostr(in_port, sItoa);
			sDebug += " success";
			EzLog::i(ftag, sDebug);
		}

		return clientfd;
	}

	/*
	Accept client Connection

	@param const int in_listenfd : listen socket fd
	@param int& out_clientfd : accepted client socket fd
	@param string& out_remoteAddr : remote Internet address
	@param uint16_t& out_remotePort : remote Port number
	*/
	int AcceptConn(const int in_listenfd, int& out_clientfd, string& out_remoteAddr, uint16_t& out_remotePort)
	{
		static const string ftag("AcceptConn() ");

		struct sockaddr_in clientaddr;
		socklen_t clilen = sizeof(struct sockaddr_in);
		memset(&clientaddr, 0, clilen);

		int connfd = accept(in_listenfd, (struct sockaddr *)&clientaddr, &clilen);
		if (0 > connfd)
		{
			int iErr = errno;
			string sItoa;
			string sDebug("accept socket in error=");
			sDebug += sof_string::itostr(iErr, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr);
			EzLog::e(ftag, sDebug);

			close(connfd);

			return -1;
		}

		out_remoteAddr = inet_ntoa(clientaddr.sin_addr);
		out_remotePort = clientaddr.sin_port;
		// cout << "accapt a connection from ip=" << strClientAddr
		// << " port=" << out_remote << " fd=" << connfd << endl;

		int iRes = SetNonblocking(connfd);
		if (0 != iRes)
		{
			string sDebug("SetNonblocking error ip=");
			sDebug += out_remoteAddr;
			EzLog::e(ftag, sDebug);

			close(connfd);
			return -1;
		}

		int iErr_KeepAlive = 0;
		iRes = SetKeepAlive(connfd, iErr_KeepAlive);
		if (0 == iRes)
		{

		}
		else
		{
			string sItoa;
			string sDebug("SetKeepAlive error=");
			sDebug += sof_string::itostr(iErr_KeepAlive, sItoa);
			sDebug += " serror=";
			sDebug += strerror(iErr_KeepAlive);
			EzLog::e(ftag, sDebug);

			close(connfd);

			return -1;
		}

		out_clientfd = connfd;
		return 0;
	}

	/*
	读取数据

	@param const int sockFd : socket file handler
	@param std::vector<uint8_t>& out_recvData : 接收到的数据
	@param int& out_errno : 出错时的错误码

	@return 0 : 读取正常
	@return -1 : 读取失败，错误原因查看返回的errno，一般情况需关闭sockfd
	*/
	int RecvData(const int sockFd, std::vector<uint8_t>& out_recvData, int& out_errno)
	{
		int ret = 0;
		int recvLen = 0;
		// 数据接受 buf
		const int ciRecvBuffLen = 1024L * 10;
		char recvBuf[ciRecvBuffLen];

		while (ret >= 0)
		{
			memset(recvBuf, 0, ciRecvBuffLen);

			//recv数据
			// These  calls  return  the  number  of bytes received, or -1 if an error occurred.
			ret = recv(sockFd, (char *)recvBuf, ciRecvBuffLen, 0);
			if (0 == ret)
			{
				recvLen = 0;

				// 当recv()返回值小于等于0时，还需要判断 errno是否等于 EINTR，
				// 如果errno == EINTR 则说明recv函数是由于程序接收到信号后返回的，socket连接还是正常的，不应close掉socket连接。
				int iErr = errno;
				if (EINTR == iErr || 0 == iErr)
				{
					return 0;
				}
				else
				{
					cout << "fd=" << sockFd << "0==ret errno=" << iErr << " serror=" << strerror(iErr)
						<< endl;
					out_errno = iErr;

					return -1;
				}
			}
			else if (0 > ret)
			{
				int iErr = errno;
				// the socket is nonblocking  (see  fcntl(2)),
				// in  which case the value - 1 is returned and the external variable errno
				// is set to EAGAIN or EWOULDBLOCK.
				if (EAGAIN == iErr || EWOULDBLOCK == iErr)
				{
					return 0;
				}
				else
				{
					cout << "fd=" << sockFd << "0>ret errno=" << iErr << " serror=" << strerror(iErr)
						<< endl;
					out_errno = iErr;

					return -1;
				}
			}
			else
			{
				// 数据接受正常
				recvLen = ret;
				for (int i = 0; i < recvLen; ++i)
				{
					out_recvData.push_back(recvBuf[i]);
				}

				/*
				{
				recvBuf[ret] = '\0';

				string ss = recvBuf;
				cout << "data is " << ss << endl;
				}
				*/
			}
		}

		return 0;
	}

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
	int SendData(const int sockFd, std::vector<uint8_t>& in_sendData, uint64_t& out_byteTransferred, int& out_errno)
	{
		out_byteTransferred = 0;

		char* pbuf = reinterpret_cast<char *>(&in_sendData[0]);
		size_t len = static_cast<size_t>(in_sendData.size() * sizeof(in_sendData[0]));

		int ret = send(sockFd, pbuf, len, 0);
		if (0 > ret)
		{
			/*
			Upon successful completion, send() shall return  the  number  of  bytes
			sent.  Otherwise,  -1  shall  be returned and errno set to indicate the
			error.
			If space is not
			available at the sending socket to hold the message to be  transmitted,
			and  the  socket file descriptor does have O_NONBLOCK set, send() shall
			fail.
			*/
			int iErr = errno;
			// the socket is nonblocking  (see  fcntl(2)),
			// in  which case the value - 1 is returned and the external variable errno
			// is set to EAGAIN or EWOULDBLOCK.
			if (EAGAIN == iErr || EWOULDBLOCK == iErr)
			{
				return 1;
			}
			else
			{
				cout << "fd=" << sockFd << " errno=" << iErr << " serror=" << strerror(iErr)
					<< endl;
				out_errno = iErr;

				return -1;
			}
		}

		out_byteTransferred = ret;
		return 0;
	}

	int Add_EpollWatch_NewClient(const int in_epollfd, const int in_sockfd, int& out_errno)
	{
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLRDHUP;
		event.data.fd = in_sockfd;

		//add Event
		int iRes = epoll_ctl(in_epollfd, EPOLL_CTL_ADD, in_sockfd, &event);
		if (0 > iRes)
		{
			out_errno = errno;
			/*
			uint32_t uiEvents = event.events;
			cout << "error event=" << uiEvents
			<< "epoll add fail fd=" << in_sockfd << " error=" << iErr
			<< " serror=" << strerror(iErr) << endl;
			*/
			return -1;
		}

		return 0;
	}

	int Mod_EpollWatch_Send(const int in_epollfd, const int in_sockfd, int& out_errno)
	{
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLRDHUP | EPOLLOUT;
		event.data.fd = in_sockfd;

		// mod Event
		int iRes = epoll_ctl(in_epollfd, EPOLL_CTL_MOD, in_sockfd, &event);
		if (0 > iRes)
		{
			out_errno = errno;
			/*
			uint32_t uiEvents = event.events;
			cout << "error event=" << uiEvents
			<< "epoll add fail fd=" << in_sockfd << " error=" << iErr
			<< " serror=" << strerror(iErr) << endl;
			*/
			return -1;
		}

		return 0;
	}

	int Mod_EpollWatch_CancellSend(const int in_epollfd, const int in_sockfd, int& out_errno)
	{
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLRDHUP;
		event.data.fd = in_sockfd;

		// mod Event
		int iRes = epoll_ctl(in_epollfd, EPOLL_CTL_MOD, in_sockfd, &event);
		if (0 > iRes)
		{
			out_errno = errno;
			/*
			uint32_t uiEvents = event.events;
			cout << "error event=" << uiEvents
			<< "epoll add fail fd=" << in_sockfd << " error=" << iErr
			<< " serror=" << strerror(iErr) << endl;
			*/
			return -1;
		}

		return 0;
	}
}
