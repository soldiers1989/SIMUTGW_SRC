#include "Epoll_ConnectionManager.h"

#include <sys/socket.h>

#include "util/EzLog.h"

#include "tool_net/PacketAssembler.h"

using namespace std;

Epoll_ConnectionManager::Epoll_ConnectionManager(void)
	:m_scl(keywords::channel = "Epoll_ConnectionManager")
{
}

Epoll_ConnectionManager::~Epoll_ConnectionManager(void)
{
}

void Epoll_ConnectionManager::AddConnection(std::shared_ptr<Epoll_Socket_Connection> client)
{
	static const string ftag("AddConnection() ");

	boost::unique_lock<boost::mutex> Locker(m_mutex);

	std::pair<EpollConnMap_t::iterator, bool> ret = m_connMap.insert(
		std::make_pair(client->m_id, client));

	bool inserted = ret.second;
	if (inserted)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << " id=" << client->m_id << " info " << client->m_cinfo.ToString();
	}
}

bool Epoll_ConnectionManager::RemoveConnection(uint64_t clientId)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	if (0 < m_connMap.erase(clientId))
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::shared_ptr<Epoll_Socket_Connection> Epoll_ConnectionManager::GetConnection(uint64_t clientId)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	EpollConnMap_t::iterator itr = m_connMap.find(clientId);

	if (m_connMap.end() != itr)
	{
		return itr->second;
	}

	return shared_ptr<Epoll_Socket_Connection>();
}

void Epoll_ConnectionManager::CloseAllConnections()
{
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	EpollConnMap_t::iterator itr = m_connMap.begin();
	while (m_connMap.end() != itr)
	{
		shutdown(itr->second->m_socketfd, SHUT_RDWR);
		close(itr->second->m_socketfd);
		itr->second->m_socketfd = -1;
		++itr;
	}

	m_connMap.clear();
}

int Epoll_ConnectionManager::GetAllConnectionIds(std::vector<uint64_t>& out_vIds)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	EpollConnMap_t::iterator itr = m_connMap.begin();
	while (m_connMap.end() != itr)
	{
		out_vIds.push_back(itr->first);
		++itr;
	}

	return 0;
}

/*
�����������ݼ��뷢�Ͷ��еĶ�β����֪ͨepoll����socket epollout�¼�

@param [in] uint64_t cid : client connection id
@param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : ����������
@param [out] int& out_errno  : �����errno
@param [out] int& out_cid : �����client connection id
@param [out] int& out_sockfd : �����client socket fd

@return 0 : �ɹ�
@return -1 : ʧ�ܣ�������п�����epoll handleʧЧ������socketfdʧЧ(��ʱ��Ҫ�رտͻ�������)
*/
int Epoll_ConnectionManager::Send(uint64_t cid, std::shared_ptr<std::vector<uint8_t>>& in_data, int& out_errno, int& out_cid, int& out_sockfd)
{
	static const string ftag("Epoll_ConnectionManager::Send() ");
		
	std::shared_ptr<Epoll_Socket_Connection> pClientConn = GetConnection(cid);
	if (nullptr == pClientConn)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag
			<< "connection manage couldn't find id=" << cid;
		return -1;
	}

	int iRes = pClientConn->AddSend(in_data, out_errno, out_cid, out_sockfd);
	if (0 != iRes)
	{		
		return -1;
	}

	return 0;
}
