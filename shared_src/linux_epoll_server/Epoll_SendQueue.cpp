#include "Epoll_SendQueue.h"

Epoll_SendQueue::Epoll_SendQueue(void)
{

}

Epoll_SendQueue::~Epoll_SendQueue(void)
{

}

/*
�����������ݼ��뷢�Ͷ��еĶ�β

@param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : ����������

@return
*/
void Epoll_SendQueue::AddSend(std::shared_ptr<std::vector<uint8_t>>& in_pSendData)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	m_dataNeedSend.push_back(in_pSendData);	
}

/*
�ɷ��Ͷ��ж��׻�ȡ����������

@param [out] std::shared_ptr<std::vector<uint8_t>>& out_pSendData : ����������

@return 1 : ��ȡ�˴���������
@return 0 : �޴���������
*/
int Epoll_SendQueue::GetSend(std::shared_ptr<std::vector<uint8_t>>& out_pSendData)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	if (0 == m_dataNeedSend.size())
	{
		return 0;
	}

	out_pSendData = m_dataNeedSend.front();
	m_dataNeedSend.pop_front();

	return 1;
}

/*
���򻺳������������Ĵ��������ݼ��뷢�Ͷ��еĶ���

@param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : ����������

@return
*/
void Epoll_SendQueue::FailSendRestore(std::shared_ptr<std::vector<uint8_t>>& in_pSendData)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	m_dataNeedSend.push_front(in_pSendData);
}

/*
������д���������

@param

@return
*/
void  Epoll_SendQueue::CloseAllSends(void)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	m_dataNeedSend.clear();
}

/*
��ȡ��������������

@param

@return
*/
size_t  Epoll_SendQueue::NumOutstandingSends(void)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	return m_dataNeedSend.size();
}