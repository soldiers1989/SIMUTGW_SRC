#ifndef __REDIS_CONNECTION_POOL_H__
#define __REDIS_CONNECTION_POOL_H__

#include "boost/shared_ptr.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/mutex.hpp"

#include "Redis3_0Cnn_define.h"

#include "util/EzLog.h"

template <typename T>
class RedisConnectionPool
{
	//
	// Members
	//
private:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// ���ӵĹ���ʱ�䣬��λ����
	static const time_t CONNECTION_EXPIRE_GAP = 60 * 60;

	// ��ֹ��γ�ʼ��
	bool m_bIsInited;

	// ����host
	std::string m_strRedisHost;

	// port
	unsigned int m_uiRedisPort;

	// redis�Ƿ���Ҫ������֤
	bool m_bRequirePass;

	// ����
	std::string m_strRedisPassword;

	//��ǰ�ѽ��������ݿ���������
	int m_iCurSize;
	//���ӳ��ж����������ݿ�������
	int m_iMaxSize;

	//���ӳص���������
	std::list<std::shared_ptr<T>> m_connList;

	// ������
	boost::mutex m_mutexlock;


	//static ConnectionPool *pool;//���ӳض���


	//
	// Functions
	//
public:
	// ���ӳصĹ��캯��
	RedisConnectionPool()
		: m_scl(keywords::channel = "RedisConnectionPool"),
		m_bIsInited(false), m_strRedisHost(""), m_uiRedisPort(0),
		m_strRedisPassword(""), m_bRequirePass(false), m_iCurSize(0), m_iMaxSize(0)
	{
	}

	virtual ~RedisConnectionPool(void)
	{
		DestoryConnPool();
	}

	// ���ӳ�����
	int SetConnection(const std::string& hostName, unsigned int port, int maxSize,
		bool bRequirepass = false, const std::string& password = "")
	{
		static const std::string ftag("RedisConnectionPool::SetConnection() ");

		m_strRedisHost = hostName;
		m_uiRedisPort = port;

		m_bRequirePass = bRequirepass;
		m_strRedisPassword = password;


		m_iCurSize = 0;
		m_iMaxSize = maxSize;

		return 0;
	}

	int Init(void)
	{
		static const std::string ftag("RedisConnectionPool::Init() ");

		if ( m_bIsInited )
		{
			return 0;
		}

		// ����host
		if ( m_strRedisHost.empty() || 0 == m_uiRedisPort )
		{
			EzLog::e(ftag, "connection params unlegal!");

			return -1;
		}

		InitConnection(m_iMaxSize / 2);

		m_bIsInited = true;

		return 0;
	}

	void Stop(void)
	{
		DestoryConnPool();
	}

	// �õ��������ӳش�С
	size_t GetAvailablePoolSize(void)
	{
		return m_connList.size();
	}

	// �����ӳ��л��һ������
	std::shared_ptr<T> GetConnection()
	{
		static const std::string ftag("RedisConnectionPool::GetConnection() ");

		boost::unique_lock<boost::mutex> Locker(m_mutexlock);

		std::shared_ptr<T> sp;

		if ( 0 < m_connList.size() )
		{
			// ���ӳ������л�������
			// �õ�����Ԫ��
			sp = m_connList.front();
			// ɾ������Ԫ��
			m_connList.pop_front();

			if ( nullptr == sp )
			{
				// ��������Ѿ����رգ�ɾ�������½���һ��
				sp.reset();

				sp = CreateConnection();

				// �������Ϊ�գ��򴴽����ӳ���
				if ( nullptr == sp )
				{
					--m_iCurSize;
					EzLog::e(ftag, "redis connect failed");
				}
			}
			else
			{
				// �ж������Ƿ��ѳ�ʱ
				time_t tNow = time(NULL);
				time_t tLinkLast = sp->GetLastUseTime();

				time_t tGap = tNow - tLinkLast;
				if ( CONNECTION_EXPIRE_GAP < tGap )
				{
					// error
					BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << " connection time out, reconnect.";

					// �������ѹ��ڣ�ɾ�������½���һ��
					sp.reset();

					sp = CreateConnection();

					// �������Ϊ�գ��򴴽����ӳ���
					if ( NULL == sp )
					{
						--m_iCurSize;
						EzLog::e(ftag, "mysql connect failed");
					}
				}

			}

			return sp;
		}
		else // if( 0 < m_connList.size() )
		{
			if ( m_iCurSize < m_iMaxSize )
			{
				// �����Դ����µ�����
				sp = CreateConnection();
				// �������Ϊ�գ��򴴽����ӳ���
				if ( nullptr == sp )
				{
					EzLog::e(ftag, "redis connect failed");
				}
				else
				{
					// ������������
					++m_iCurSize;
				}

				return sp;
			}
			else
			{
				// �������������Ѿ��ﵽmaxSize
				sp.reset();
				return sp;
			}
		}

	}

	/*
	�������ݿ�����
	���棺�޷��Կ��ⲿ�ͷ����Ӽ����������ӵ����⣬���ܻ���ɼ�������
	*/
	void ReleaseConnection(std::shared_ptr<T> sp)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutexlock);

		if ( NULL == sp )
		{
			// ���棺�޷��Կ��ⲿ�ͷ����Ӽ����������ӵ����⣬���ܻ���ɼ�������
			--m_iCurSize;
			return;
		}

		if ( nullptr != sp )
		{
			sp->UpdateUseTime();
		}

		m_connList.push_back(sp);
	}


protected:
	// ��ʼ�����ӳأ����������������һ����������
	void InitConnection(const int iInitialSize)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutexlock);

		AddConn(iInitialSize);
	}

	// ��������
	void AddConn(const int iSize)
	{
		static const std::string ftag("RedisConnectionPool::AddConn() ");

		for ( int i = 0; i < iSize; ++i )
		{
			// ��������
			std::shared_ptr<T> sp = CreateConnection();

			if ( nullptr == sp )
			{
				EzLog::e(ftag, "redis connect failed");
			}
			else
			{
				m_connList.push_back(sp);
				++m_iCurSize;
			}
		}
	}

	// ��������,����һ��Connection
	std::shared_ptr<T> CreateConnection()
	{
		static const std::string ftag("RedisConnectionPool::CreateConnection() ");

		std::shared_ptr<T> sp;

		try
		{
			// ��������
			sp = std::shared_ptr<T>(new T());

			if ( nullptr == sp )
			{
				EzLog::e(ftag, "no memory, new failed");

				return sp;
			}

			int iRes = sp->Connect(m_strRedisHost, m_uiRedisPort, simutgw::redis_clTimeOut);

			if ( 0 != iRes )
			{
				EzLog::e(ftag, "Redis connect failed");

				sp.reset();
			}

			if ( m_bRequirePass )
			{
				std::string strCmd = "AUTH ";
				strCmd += m_strRedisPassword;
				std::string strRedisRes;
				RedisReply redisRes = sp->Cmd(strCmd, nullptr, &strRedisRes, nullptr, nullptr, nullptr);

				if ( ( RedisReply_status != redisRes ) || ( 0 != simutgw::g_RplyRes_OK.compare(strRedisRes) ) )
				{
					std::string strDebug("redis AUTH failed:");
					strDebug += m_strRedisPassword;
					EzLog::e(ftag, strDebug);

					sp.reset();
				}

			}

			if ( nullptr != sp )
			{
				sp->UpdateUseTime();
			}

			return sp;
		}
		catch ( std::exception &e )
		{
			EzLog::ex(ftag, e);

			return sp;
		}
	}

	// �������ӳ�,����Ҫ���������ӳص�������
	void DestoryConnPool(void)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutexlock);

		typename std::list<std::shared_ptr<T>>::iterator itcon;

		for ( itcon = m_connList.begin(); itcon != m_connList.end(); ++itcon )
		{
			DestoryConnection(*itcon); //�������ӳ��е�����
		}

		m_iCurSize = 0;

		//������ӳ��е�����
		m_connList.clear();

		m_bIsInited = false;
	}

	// ����һ������
	void DestoryConnection(std::shared_ptr<T> sp)
	{
		if ( nullptr == sp )
		{
		}
		else
		{
			sp->Disconnect();
		}

		return;
	}

private:

};


#endif
