#include "tool_mysql/MysqlConnectionPool.h"

// ���ӵĹ���ʱ�䣬��λ����
const time_t MysqlConnectionPool::CONNECTION_EXPIRE_GAP = 60 * 60;

// ���ӳصĹ��캯��
MysqlConnectionPool::MysqlConnectionPool()
	:m_scl(keywords::channel = "MysqlConnectionPool")
{
	m_bIsInited = false;
}

MysqlConnectionPool::~MysqlConnectionPool(void)
{
	DestoryConnPool();
}

// ���ӳ�����
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

// �õ��������ӳش�С
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

// ��ʼ�����ӳأ����������������һ����������
void MysqlConnectionPool::InitConnection(const int iInitialSize)
{
	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	AddConn(iInitialSize);
}

//��������
void MysqlConnectionPool::AddConn(const int iSize)
{
	static const string ftag("MysqlConnectionPool::AddConn() ");

	for ( int i = 0; i < iSize; ++i )
	{
		// ��������
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

// ��������,����һ��Connection
std::shared_ptr<MySqlCnnC602> MysqlConnectionPool::CreateConnection(void)
{
	static const string ftag("MysqlConnectionPool::CreateConnection() ");

	std::shared_ptr<MySqlCnnC602> sp;

	try
	{
		// ��������
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
����Ĭ�ϵ����ݿ�

Return :
bool
true -- ���óɹ�
false -- ����ʧ��
*/
bool MysqlConnectionPool::SetDefaultCatalog(std::shared_ptr<MySqlCnnC602>& in_sp)
{
	static const string ftag("MysqlConnectionPool::SetDefaultCatalog() ");

	std::string strQuery("USE `");
	strQuery += m_strSqlCatalog;

	strQuery += "`;";

	if ( nullptr == in_sp )
	{
		//mysql����ΪNULL

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
		std::string strDebug("����[");
		strDebug += strQuery;
		strDebug += "]�õ�Res=";
		strDebug += sof_string::itostr(iRes, strItoa);

		EzLog::e(ftag, strDebug);

		return false;
	}

	return true;
}

//�����ӳ��л��һ������
std::shared_ptr<MySqlCnnC602> MysqlConnectionPool::GetConnection(void)
{
	static const string ftag("MysqlConnectionPool::GetConnection() ");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	std::shared_ptr<MySqlCnnC602> sp;

	if ( 0 < m_connList.size() )
	{
		// ���ӳ������л�������
		// �õ�����Ԫ��
		sp = m_connList.front();
		// ɾ������Ԫ��
		m_connList.pop_front();

		if ( NULL == sp || sp->IsClosed() )
		{
			// ��������Ѿ����رգ�ɾ�������½���һ��
			sp.reset();

			sp = CreateConnection();

			// �������Ϊ�գ��򴴽����ӳ���
			if ( NULL == sp )
			{
				--m_iCurSize;
				EzLog::e(ftag, "mysql connect failed");
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
			if ( NULL == sp )
			{
				EzLog::e(ftag, "mysql connect failed");
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
void MysqlConnectionPool::ReleaseConnection(std::shared_ptr<MySqlCnnC602> sp)
{
	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	if ( NULL == sp )
	{
		// ���棺�޷��Կ��ⲿ�ͷ����Ӽ����������ӵ����⣬���ܻ���ɼ�������
		--m_iCurSize;
		return;
	}

	//�ع�������Ԥ������δ�ύ�����
	sp->RollBack();

	sp->UpdateUseTime();

	m_connList.push_back(sp);
}

//�������ӳ�,����Ҫ���������ӳص�������
void MysqlConnectionPool::DestoryConnPool(void)
{
	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	list<std::shared_ptr<MySqlCnnC602>>::iterator itcon;

	for ( itcon = m_connList.begin(); itcon != m_connList.end(); ++itcon )
	{
		DestoryConnection(*itcon); //�������ӳ��е�����
	}

	m_iCurSize = 0;

	//������ӳ��е�����
	m_connList.clear();
}

//����һ������
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
