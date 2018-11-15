#ifndef __EPOLL_CONNECTION_MANAGER_H__
#define __EPOLL_CONNECTION_MANAGER_H__

#include <map>
#include <memory>

#include "boost/thread/mutex.hpp"

#include "Epoll_Socket_Connection.h"

#include "util/EzLog.h"

/*
Connection Manager.
*/
class Epoll_ConnectionManager
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	typedef std::map<uint64_t, std::shared_ptr<Epoll_Socket_Connection>> EpollConnMap_t;

	EpollConnMap_t m_connMap;

	boost::mutex m_mutex;

	//
	// Functions
	//
public:
	Epoll_ConnectionManager(void);
	virtual ~Epoll_ConnectionManager(void);

	void AddConnection(std::shared_ptr<Epoll_Socket_Connection> client);

	bool RemoveConnection(uint64_t clientId);

	std::shared_ptr<Epoll_Socket_Connection> GetConnection(uint64_t clientId);

	void CloseAllConnections();

	int GetAllConnectionIds(std::vector<uint64_t>& out_vIds);

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
	int Send(uint64_t cid, std::shared_ptr<std::vector<uint8_t>>& in_data, int& out_errno, int& out_cid, int& out_sockfd);
};

#endif