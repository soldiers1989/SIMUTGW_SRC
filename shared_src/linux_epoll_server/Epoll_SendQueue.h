#ifndef __EPOLL_SEND_QUEUE_H__
#define __EPOLL_SEND_QUEUE_H__

#include <stdint.h>
#include <memory.h>
#include <vector>
#include <list>

#include "boost/thread/mutex.hpp"

/*
���͵Ļ���
*/
class Epoll_SendQueue
{
	//
	// Members
	//
protected:
	std::list<std::shared_ptr<std::vector<uint8_t>>> m_dataNeedSend;

	boost::mutex m_mutex;

	//
	// Functions
	//
public:
	Epoll_SendQueue(void);
	virtual ~Epoll_SendQueue(void);

	/*
	�����������ݼ��뷢�Ͷ��еĶ�β

	@param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : ����������

	@return
	*/
	void AddSend(std::shared_ptr<std::vector<uint8_t>>& in_pSendData);

	/*
	�ɷ��Ͷ��ж��׻�ȡ����������

	@param [out] std::shared_ptr<std::vector<uint8_t>>& out_pSendData : ����������

	@return 0 : ��ȡ�˴���������
	@return 1 : �޴���������
	*/
	int GetSend(std::shared_ptr<std::vector<uint8_t>>& out_pSendData);

	/*
	���򻺳������������Ĵ��������ݼ��뷢�Ͷ��еĶ���

	@param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : ����������

	@return
	*/
	void FailSendRestore(std::shared_ptr<std::vector<uint8_t>>& in_pSendData);

	/*
	������д���������

	@param

	@return
	*/
	void CloseAllSends(void);

	/*
	��ȡ��������������

	@param

	@return
	*/
	size_t NumOutstandingSends(void);

};

#endif