#include "Epoll_Socket_Connection.h"

#include <sys/socket.h>

#include "linux_socket_base/LinuxNetUtil.h"

Epoll_Socket_Connection::Epoll_Socket_Connection(const int socketfd,
	const uint64_t cid, const struct ConnectionInformation& cinfo, const int epollfd)
	: m_socketfd(socketfd), m_id(cid), m_cinfo(cinfo), m_epollfd(epollfd),
	m_bIsEpoutSet(false), 
	m_scl(keywords::channel = "Epoll_Socket_Connection"),
	m_ui64NextSendSeq(0)
{

}

Epoll_Socket_Connection::~Epoll_Socket_Connection()
{
	BOOST_LOG_SEV(m_scl, trivial::info) << "object dismember id=" << m_id << ", socketfd=" << m_socketfd
		<< ", ip=" << m_cinfo.strRemoteIpAddress
		<< ", port=" << m_cinfo.remotePortNumber;

	if (0 < m_socketfd)
	{
		shutdown(m_socketfd, SHUT_RDWR);
		close(m_socketfd);
	}
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
int Epoll_Socket_Connection::AddSend(std::shared_ptr<std::vector<uint8_t>>& in_pSendData, int& out_errno, int& out_cid, int& out_sockfd)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex_sendQue);

	m_sendQueue.AddSend(in_pSendData);

	// ���û�����ù�out event
	if (!m_bIsEpoutSet)
	{
		int iErr = 0;
		
		int iRes = LinuxNetUtil::Mod_EpollWatch_Send(m_epollfd, m_socketfd, iErr);
		if (0 != iRes)
		{
			out_errno = iErr;
			out_cid = m_id;
			out_sockfd = m_socketfd;
			return -1;
		}

		m_bIsEpoutSet = true;
	}

	return 0;
}

int Epoll_Socket_Connection::LoopSend(uint64_t& out_byteTransferred, int& out_errno, int& out_cid, int& out_sockfd)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex_sendQue);

	uint64_t uiTotal_ByteTransferred = 0;

	std::shared_ptr<std::vector<uint8_t>> pSendData;
	int iRes = 0;
	int iErr = 0;

	// ѭ������ֱ����ջ�����
	do
	{
		iRes = m_sendQueue.GetSend(pSendData);
		if (1 == iRes)
		{
			// �д���������
			uint64_t uiByteTransferred = 0;

			int iResSend = LinuxNetUtil::SendData(m_socketfd, *pSendData, uiByteTransferred, iErr);
			if (0 == iResSend)
			{
				// д�뷢�ͻ������ɹ�
				uiTotal_ByteTransferred += uiByteTransferred;
			}
			else if (1 == iResSend)
			{
				// ���������Ȳ���
				return 1;
			}
			else
			{
				out_errno = iErr;
				out_cid = m_id;
				out_sockfd = m_socketfd;
				return -1;
			}
		}
		else
		{
			// �޴���������
			// ȡ��out event
			iRes = LinuxNetUtil::Mod_EpollWatch_CancellSend(m_epollfd, m_socketfd, iErr);
			if (0 != iRes)
			{
				out_errno = iErr;
				out_cid = m_id;
				out_sockfd = m_socketfd;
				return -1;
			}

			m_bIsEpoutSet = false;
			break;
		}
	} while (0 < iRes);

	out_byteTransferred = uiTotal_ByteTransferred;

	return 0;
}