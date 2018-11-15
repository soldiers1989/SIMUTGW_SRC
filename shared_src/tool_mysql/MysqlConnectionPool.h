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

	// 连接的过期时间，单位：秒
	static const time_t CONNECTION_EXPIRE_GAP;

	// 防止多次初始化
	bool m_bIsInited;

	// 连接host
	std::string m_strSqlHost;

	// 帐号
	std::string m_strSqlUsername;
	// 密码
	std::string m_strSqlPassword;

	// port
	unsigned int m_uiSqlPort;

	// catalog
	std::string m_strSqlCatalog;

	//当前已建立的数据库连接数量
	int m_iCurSize;
	//连接池中定义的最大数据库连接数
	int m_iMaxSize;

	//连接池的容器队列
	list<std::shared_ptr<MySqlCnnC602>> m_connList;

	// 锁对象
	boost::mutex m_mutexlock;
	
	//
	// Functions
	//
public:
	// 连接池的构造函数
	MysqlConnectionPool();

	virtual ~MysqlConnectionPool(void);
	
	// 连接池设置
	int SetConnection(const std::string& hostName, const std::string& userName, const std::string& password,
		unsigned int port, const std::string& catalog, int maxSize);

	int Init(void);

	// 得到可用连接池大小
	size_t GetAvailablePoolSize(void);

	// 在连接池中获得一个连接
	std::shared_ptr<MySqlCnnC602> GetConnection();

	/*
	回收数据库连接
	警告：无法对抗外部释放连接及返还空连接的问题，可能会造成计数紊乱
	*/
	void ReleaseConnection(std::shared_ptr<MySqlCnnC602> sp);


protected:
	// 初始化连接池，创建最大连接数的一半连接数量
	void InitConnection(const int iInitialSize);

	// 增加连接
	void AddConn(const int iSize);

	// 创建连接,返回一个Connection
	std::shared_ptr<MySqlCnnC602> CreateConnection();

	/*
	设置默认的数据库

	Return :
	bool
	true -- 设置成功
	false -- 设置失败
	*/
	bool SetDefaultCatalog(std::shared_ptr<MySqlCnnC602>& in_sp);

	// 销毁连接池,首先要先销毁连接池的中连接
	void DestoryConnPool(void);

	// 销毁一个连接
	void DestoryConnection(std::shared_ptr<MySqlCnnC602> sp);

private:

};

#endif
