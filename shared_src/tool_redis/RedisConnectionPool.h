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

	// 连接的过期时间，单位：秒
	static const time_t CONNECTION_EXPIRE_GAP = 60 * 60;

	// 防止多次初始化
	bool m_bIsInited;

	// 连接host
	std::string m_strRedisHost;

	// port
	unsigned int m_uiRedisPort;

	// redis是否需要密码验证
	bool m_bRequirePass;

	// 密码
	std::string m_strRedisPassword;

	//当前已建立的数据库连接数量
	int m_iCurSize;
	//连接池中定义的最大数据库连接数
	int m_iMaxSize;

	//连接池的容器队列
	std::list<std::shared_ptr<T>> m_connList;

	// 锁对象
	boost::mutex m_mutexlock;


	//static ConnectionPool *pool;//连接池对象


	//
	// Functions
	//
public:
	// 连接池的构造函数
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

	// 连接池设置
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

		// 连接host
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

	// 得到可用连接池大小
	size_t GetAvailablePoolSize(void)
	{
		return m_connList.size();
	}

	// 在连接池中获得一个连接
	std::shared_ptr<T> GetConnection()
	{
		static const std::string ftag("RedisConnectionPool::GetConnection() ");

		boost::unique_lock<boost::mutex> Locker(m_mutexlock);

		std::shared_ptr<T> sp;

		if ( 0 < m_connList.size() )
		{
			// 连接池容器中还有连接
			// 得到顶层元素
			sp = m_connList.front();
			// 删除顶层元素
			m_connList.pop_front();

			if ( nullptr == sp )
			{
				// 如果连接已经被关闭，删除后重新建立一个
				sp.reset();

				sp = CreateConnection();

				// 如果连接为空，则创建连接出错
				if ( nullptr == sp )
				{
					--m_iCurSize;
					EzLog::e(ftag, "redis connect failed");
				}
			}
			else
			{
				// 判断连接是否已超时
				time_t tNow = time(NULL);
				time_t tLinkLast = sp->GetLastUseTime();

				time_t tGap = tNow - tLinkLast;
				if ( CONNECTION_EXPIRE_GAP < tGap )
				{
					// error
					BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << " connection time out, reconnect.";

					// 该连接已过期，删除后重新建立一个
					sp.reset();

					sp = CreateConnection();

					// 如果连接为空，则创建连接出错
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
				// 还可以创建新的连接
				sp = CreateConnection();
				// 如果连接为空，则创建连接出错
				if ( nullptr == sp )
				{
					EzLog::e(ftag, "redis connect failed");
				}
				else
				{
					// 增加现有数量
					++m_iCurSize;
				}

				return sp;
			}
			else
			{
				// 建立的连接数已经达到maxSize
				sp.reset();
				return sp;
			}
		}

	}

	/*
	回收数据库连接
	警告：无法对抗外部释放连接及返还空连接的问题，可能会造成计数紊乱
	*/
	void ReleaseConnection(std::shared_ptr<T> sp)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutexlock);

		if ( NULL == sp )
		{
			// 警告：无法对抗外部释放连接及返还空连接的问题，可能会造成计数紊乱
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
	// 初始化连接池，创建最大连接数的一半连接数量
	void InitConnection(const int iInitialSize)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutexlock);

		AddConn(iInitialSize);
	}

	// 增加连接
	void AddConn(const int iSize)
	{
		static const std::string ftag("RedisConnectionPool::AddConn() ");

		for ( int i = 0; i < iSize; ++i )
		{
			// 创建连接
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

	// 创建连接,返回一个Connection
	std::shared_ptr<T> CreateConnection()
	{
		static const std::string ftag("RedisConnectionPool::CreateConnection() ");

		std::shared_ptr<T> sp;

		try
		{
			// 创建连接
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

	// 销毁连接池,首先要先销毁连接池的中连接
	void DestoryConnPool(void)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutexlock);

		typename std::list<std::shared_ptr<T>>::iterator itcon;

		for ( itcon = m_connList.begin(); itcon != m_connList.end(); ++itcon )
		{
			DestoryConnection(*itcon); //销毁连接池中的连接
		}

		m_iCurSize = 0;

		//清空连接池中的连接
		m_connList.clear();

		m_bIsInited = false;
	}

	// 销毁一个连接
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
