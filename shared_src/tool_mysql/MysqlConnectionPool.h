#ifndef __MYSQL_CONNECTION_POOL_H__
#define __MYSQL_CONNECTION_POOL_H__

#include <memory>

#include "boost/thread/thread.hpp"
#include "boost/thread/mutex.hpp"

#include "tool_mysql/MySqlCnnC602.h"
#include "util/EzLog.h"

class MysqlConnectionPool
{
	//
	// Members
	//
private:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// ���ӵĹ���ʱ�䣬��λ����
	static const time_t CONNECTION_EXPIRE_GAP;

	// ��ֹ��γ�ʼ��
	bool m_bIsInited;

	// ����host
	std::string m_strSqlHost;

	// �ʺ�
	std::string m_strSqlUsername;
	// ����
	std::string m_strSqlPassword;

	// port
	unsigned int m_uiSqlPort;

	// catalog
	std::string m_strSqlCatalog;

	//��ǰ�ѽ��������ݿ���������
	int m_iCurSize;
	//���ӳ��ж����������ݿ�������
	int m_iMaxSize;

	//���ӳص���������
	list<std::shared_ptr<MySqlCnnC602>> m_connList;

	// ������
	boost::mutex m_mutexlock;
	
	//
	// Functions
	//
public:
	// ���ӳصĹ��캯��
	MysqlConnectionPool();

	virtual ~MysqlConnectionPool(void);
	
	// ���ӳ�����
	int SetConnection(const std::string& hostName, const std::string& userName, const std::string& password,
		unsigned int port, const std::string& catalog, int maxSize);

	int Init(void);

	// �õ��������ӳش�С
	size_t GetAvailablePoolSize(void);

	// �����ӳ��л��һ������
	std::shared_ptr<MySqlCnnC602> GetConnection();

	/*
	�������ݿ�����
	���棺�޷��Կ��ⲿ�ͷ����Ӽ����������ӵ����⣬���ܻ���ɼ�������
	*/
	void ReleaseConnection(std::shared_ptr<MySqlCnnC602> sp);


protected:
	// ��ʼ�����ӳأ����������������һ����������
	void InitConnection(const int iInitialSize);

	// ��������
	void AddConn(const int iSize);

	// ��������,����һ��Connection
	std::shared_ptr<MySqlCnnC602> CreateConnection();

	/*
	����Ĭ�ϵ����ݿ�

	Return :
	bool
	true -- ���óɹ�
	false -- ����ʧ��
	*/
	bool SetDefaultCatalog(std::shared_ptr<MySqlCnnC602>& in_sp);

	// �������ӳ�,����Ҫ���������ӳص�������
	void DestoryConnPool(void);

	// ����һ������
	void DestoryConnection(std::shared_ptr<MySqlCnnC602> sp);

private:

};

#endif
