#include "tool_mysql/MysqlConnectionPool.h"

// 连接的过期时间，单位：秒
const time_t MysqlConnectionPool::CONNECTION_EXPIRE_GAP = 60 * 60;

// 连接池的构造函数
MysqlConnectionPool::MysqlConnectionPool()
	:m_scl(keywords::channel = "MysqlConnectionPool")
{
	m_bIsInited = false;
}

MysqlConnectionPool::~MysqlConnectionPool(void)
{
	DestoryConnPool();
}

// 连接池设置
int MysqlConnectionPool::SetConnection(const std::string& hostName, const std::string& userName, const std::string& password,
	unsigned int port, const std::string& catalog, int maxSize)
{
	static const string ftag("MysqlConnectionPool::SetConnection() ");

	m_strSqlHost = hostName;
	m_strSqlUsername = userName;
	m_strSqlPassword = password;
	m_uiSqlPort = port;
	m_strSqlCatalog = catalog;

	m_iMaxSize = maxSize;

	return 0;
}

// 得到可用连接池大小
size_t MysqlConnectionPool::GetAvailablePoolSize(void)
{
	return m_connList.size();
}

int MysqlConnectionPool::Init(void)
{
	static const string ftag("MysqlConnectionPool::Init() ");

	if ( m_bIsInited )
	{
		return 0;
	}

	int iRes = MySqlCnnC602_Guard::MysqlLibraryInit();
	if ( 0 != iRes )
	{
		EzLog::e("MysqlConnectionPool()", "MysqlLibraryInit failed");
		perror("MysqlLibraryInit failed");
	}

	if ( m_strSqlHost.empty() || m_strSqlUsername.empty() || m_strSqlPassword.empty() || 0 == m_uiSqlPort )
	{
		EzLog::e(ftag, "connection params unlegal!");

		return -1;
	}

	InitConnection(m_iMaxSize / 2);

	m_bIsInited = true;

	return 0;
}

// 初始化连接池，创建最大连接数的一半连接数量
void MysqlConnectionPool::InitConnection(const int iInitialSize)
{
	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	AddConn(iInitialSize);
}

//增加连接
void MysqlConnectionPool::AddConn(const int iSize)
{
	static const string ftag("MysqlConnectionPool::AddConn() ");

	for ( int i = 0; i < iSize; ++i )
	{
		// 创建连接
		std::shared_ptr<MySqlCnnC602> sp = CreateConnection();

		if ( NULL == sp )
		{
			EzLog::e(ftag, "mysql connect failed");
		}
		else
		{
			m_connList.push_back(sp);
			++m_iCurSize;
		}
	}
}

// 创建连接,返回一个Connection
std::shared_ptr<MySqlCnnC602> MysqlConnectionPool::CreateConnection(void)
{
	static const string ftag("MysqlConnectionPool::CreateConnection() ");

	std::shared_ptr<MySqlCnnC602> sp;

	try
	{
		// 创建连接
		sp = std::shared_ptr<MySqlCnnC602>(new MySqlCnnC602());

		if ( NULL == sp )
		{
			EzLog::e(ftag, "no memory, new failed");

			return sp;
		}

		int iRes = sp->Connect(m_strSqlHost, m_strSqlUsername,
			m_strSqlPassword, m_uiSqlPort, m_strSqlCatalog);

		if ( 0 != iRes )
		{
			EzLog::e(ftag, "mysql connect failed");

			sp.reset();
		}
		else
		{
			// set default CataLog
			if ( !m_strSqlCatalog.empty() )
			{
				bool bres = SetDefaultCatalog(sp);
				if ( !bres )
				{
					EzLog::e(ftag, "mysql SetDefaultCatalog failed");
					sp.reset();
				}
			}
		}

		if ( nullptr != sp )
		{
			sp->UpdateUseTime();
		}

		return sp;
	}
	catch ( exception &e )
	{
		EzLog::ex(ftag, e);

		return sp;
	}
}

/*
设置默认的数据库

Return :
bool
true -- 设置成功
false -- 设置失败
*/
bool MysqlConnectionPool::SetDefaultCatalog(std::shared_ptr<MySqlCnnC602>& in_sp)
{
	static const string ftag("MysqlConnectionPool::SetDefaultCatalog() ");

	std::string strQuery("USE `");
	strQuery += m_strSqlCatalog;

	strQuery += "`;";

	if ( nullptr == in_sp )
	{
		//mysql连接为NULL

		EzLog::e(ftag, "Incoming Connection is NULL");

		return false;
	}

	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	int iRes = in_sp->Query(strQuery, &pResultSet, ulAffectedRows);
	if ( 2 == iRes )
	{
	}
	else
	{
		std::string strItoa;
		std::string strDebug("运行[");
		strDebug += strQuery;
		strDebug += "]得到Res=";
		strDebug += sof_string::itostr(iRes, strItoa);

		EzLog::e(ftag, strDebug);

		return false;
	}

	return true;
}

//在连接池中获得一个连接
std::shared_ptr<MySqlCnnC602> MysqlConnectionPool::GetConnection(void)
{
	static const string ftag("MysqlConnectionPool::GetConnection() ");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	std::shared_ptr<MySqlCnnC602> sp;

	if ( 0 < m_connList.size() )
	{
		// 连接池容器中还有连接
		// 得到顶层元素
		sp = m_connList.front();
		// 删除顶层元素
		m_connList.pop_front();

		if ( NULL == sp || sp->IsClosed() )
		{
			// 如果连接已经被关闭，删除后重新建立一个
			sp.reset();

			sp = CreateConnection();

			// 如果连接为空，则创建连接出错
			if ( NULL == sp )
			{
				--m_iCurSize;
				EzLog::e(ftag, "mysql connect failed");
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
			if ( NULL == sp )
			{
				EzLog::e(ftag, "mysql connect failed");
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
void MysqlConnectionPool::ReleaseConnection(std::shared_ptr<MySqlCnnC602> sp)
{
	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	if ( NULL == sp )
	{
		// 警告：无法对抗外部释放连接及返还空连接的问题，可能会造成计数紊乱
		--m_iCurSize;
		return;
	}

	//回滚操作，预防事务未提交的情况
	sp->RollBack();

	sp->UpdateUseTime();

	m_connList.push_back(sp);
}

//销毁连接池,首先要先销毁连接池的中连接
void MysqlConnectionPool::DestoryConnPool(void)
{
	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	list<std::shared_ptr<MySqlCnnC602>>::iterator itcon;

	for ( itcon = m_connList.begin(); itcon != m_connList.end(); ++itcon )
	{
		DestoryConnection(*itcon); //销毁连接池中的连接
	}

	m_iCurSize = 0;

	//清空连接池中的连接
	m_connList.clear();
}

//销毁一个连接
void MysqlConnectionPool::DestoryConnection(std::shared_ptr<MySqlCnnC602> sp)
{
	if ( NULL == sp )
	{
	}
	else
	{
		sp->Close();
	}

	return;
}
