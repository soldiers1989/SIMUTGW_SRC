#ifndef __EPOLL_SOCKET_CONNECTION_H__
#define __EPOLL_SOCKET_CONNECTION_H__

#include <string>
#include <stdint.h>
#include <sys/socket.h>
#include <memory.h>
#include <vector>

#include "boost/thread/mutex.hpp"

#include "config/conf_net_msg.h"

#include "util/EzLog.h"
#include "socket_conn_manage_base/ConnectionInformation.h"

#include "Epoll_SendQueue.h"

/*
epoll ��ص�socket connection��
*/
class Epoll_Socket_Connection
{
	//
	// Members
	//
public:
	// socket file handle
	int m_socketfd;

	// id for this connection
	uint64_t m_id;

	// conn info
	struct ConnectionInformation m_cinfo;

	// epoll file handle for this socket have been hooked
	int m_epollfd;

	// mutex for NextSendSeq
	boost::mutex m_mutex_NextSendSeq;

	// mutex for message Send queue
	boost::mutex m_mutex_sendQue;

	// is epoll out event set, true : set, false : not set
	bool m_bIsEpoutSet;

	// ������Ϣ����
	Epoll_SendQueue m_sendQueue;

	// ������Ϣ����
	//struct simutgw::SocketUserMessage m_rcvSocketMsg;

protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// ��һ��������Ϣ���
	uint64_t m_ui64NextSendSeq;

	//
	// Functions
	//
public:
	Epoll_Socket_Connection(const int socketfd,
		const uint64_t cid, const struct ConnectionInformation& cinfo, const int epollfd);

	virtual ~Epoll_Socket_Connection();

	/*
	ȡ��һ��������Ϣ���
	*/
	bool GetSeq(uint64_t clientId, uint64_t& out_ui64Seq)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutex_NextSendSeq);

		out_ui64Seq = ++m_ui64NextSendSeq;

		return true;
	}

	/*
	�����������ݼ��뷢�Ͷ��еĶ�β����֪ͨepoll����socket epollout�¼�

	@param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : ����������
	@param [out] int& out_errno  : �����errno
	@param [out] int& out_cid : �����client connection id
	@param [out] int& out_sockfd : �����client socket fd

	@return 0 : �ɹ�
	@return -1 : ʧ�ܣ�������п�����epoll handleʧЧ������socketfdʧЧ(��ʱ��Ҫ�رտͻ�������)
	*/
	int AddSend(std::shared_ptr<std::vector<uint8_t>>& in_pSendData, int& out_errno, int& out_cid, int& out_sockfd);

	int LoopSend(uint64_t& out_byteTransferred, int& out_errno, int& out_cid, int& out_sockfd);

protected:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	Epoll_Socket_Connection();

};

#endif