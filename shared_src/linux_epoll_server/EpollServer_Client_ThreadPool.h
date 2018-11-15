#ifndef __EPOLL_CLIENT_THREADPOOL_H__
#define __EPOLL_CLIENT_THREADPOOL_H__

#include <vector>
#include <stdint.h>

#include <memory>

#include "boost/thread/mutex.hpp"

#include "thread_pool_base/ThreadPoolBase.h"
#include "EpollServer_Client_Thread.h"

class EpollServer_Core;

class EpollServer_Client_ThreadPool :
	public ThreadPoolBase<EpollServer_Client_Thread>
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	std::shared_ptr<EpollServer_EventHandler> m_pEventHandler;

	//
	// Functions
	//
public:
	EpollServer_Client_ThreadPool(const unsigned int uiNum, std::shared_ptr<EpollServer_EventHandler> pEventHandler)
		: ThreadPoolBase(uiNum), m_scl(keywords::channel = "EpollServer_Client_ThreadPool"), m_pEventHandler(pEventHandler)
	{
	}

	virtual ~EpollServer_Client_ThreadPool(void)
	{
	}

	/*
	�����̳߳ض���

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	virtual int InitPool(void)
	{
		static const std::string ftag("EpollServer_Client_ThreadPool::InitPool() ");

		m_pool_state = ThreadPool_Conf::STOPPED;

		unsigned int i = 0;
		for (i = 0; i < m_uiThreadMaxNums; ++i)
		{
			std::shared_ptr<EpollServer_Client_Thread> ptrThread =
				std::shared_ptr<EpollServer_Client_Thread>(new EpollServer_Client_Thread(i, m_pEventHandler));
			if (NULL == ptrThread)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new ThreadBase() failed, NULL";

				return -1;
			}

			m_hdThreads.push_back(ptrThread);

			++m_uiCurrThreadNums;
		}

		return 0;
	}

	/*
	�����û�

	@param const int in_clientfd : accepted client socket fd
	@param const string& in_remoteAddr : remote Internet address
	@param const uint16_t in_remotePort : remote Port number
	@param int& out_epollfd : socketfd�ҹ���epoll file handle

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int AddNewClient(const int in_clientfd, const string& in_remoteAddr, const uint16_t in_remotePort, int& out_epollfd)
	{
		static const std::string ftag("EpollServer_Client_ThreadPool::AddNewClient() ");

		int iRes = 0;
		int i = 0;

		// ����������߳̾��
		std::vector< std::shared_ptr<EpollServer_Client_Thread> >::iterator itFewerTask;
		// ����������߳�������
		uint64_t uiFewerTaskNum = 0;

		// �Ȳ����Ƿ���Ը���������ʷ����ֱ�Ӳ���
		std::vector< std::shared_ptr<EpollServer_Client_Thread> >::iterator it = m_hdThreads.begin();
		for (i = 0, it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it, ++i)
		{
			uint64_t uiTaskNum = 0;
			uiTaskNum = (*it)->GetCurrentClientNums();

			if (0 == i)
			{
				// �״����⴦��
				uiFewerTaskNum = uiTaskNum;
				itFewerTask = it;
			}
			else
			{
				if (uiTaskNum < uiFewerTaskNum)
				{
					// ���̱߳�֮ǰ������������
					uiFewerTaskNum = uiTaskNum;
					itFewerTask = it;
				}
			}

			continue;
		}

		// ���������ٶ�����
		iRes = (*itFewerTask)->AddNewClient(in_clientfd, out_epollfd);
		if (0 == iRes)
		{
			return 0;
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "AddIfHaveTask() failed, res=" << iRes;
			return -1;
		}
	}
};
#endif
