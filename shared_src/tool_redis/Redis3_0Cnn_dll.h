#ifndef __REDIS_3_0_CNN_DLL_H__
#define __REDIS_3_0_CNN_DLL_H__

#ifdef _MSC_VER
#include <Windows.h>
#include <WinSock2.h>
#else

#endif

#include <string>
#include <errno.h>
#include <vector>
#include <ctime>
#include <map>
#include <memory>

#include "Redis3_0Cnn_define.h"

class Redis3_0Cnn_dll
{
	//
	// Members
	//

protected:

	long m_lTimeout_Millisecond;

	// 当前的redis连接 key
	std::shared_ptr<RedisContextPack_dll> m_ptrCurrentContext;

	// redis cluster时连接集群
	// key是<ip:port>
	std::map<std::string, std::shared_ptr<RedisContextPack_dll>> m_mapRedisContexts;

	// 是否使用string.c_str()
	bool m_bShareMem;

	// 上次使用的时间戳 单位：秒
	time_t m_timeLastUseTime;

	//
	// Functions
	//
public:
	Redis3_0Cnn_dll();
	virtual ~Redis3_0Cnn_dll();

	inline void UpdateUseTime(void)
	{
		time(&m_timeLastUseTime);
	}

	inline time_t GetLastUseTime(void)
	{
		return m_timeLastUseTime;
	}

	/*
	连接Redis Server
	Return :
	0 -- 连接成功
	-1 -- 连接失败
	*/
	int Connect( const std::string& strSvrIp, const unsigned int uiPort,
		const long in_lTimeout_Millisecond );

	/*
	断开连接
	Return :
	*/
	void Disconnect(void);

	/*
	执行Redis Command
	Params :
	const std::string strCmd :
	redis command string.

	long long* out_pllRes :
	返回的long long值.

	std::string* out_pstrRes :
	返回的string值.

	int* out_piStrLen :
	Length of string.

	std::vector<string>* out_pvectArray :
	返回的数组值.

	size_t* out_pElemSize :
	number of elements, for REDIS_REPLY_ARRAY.

	Return :
	返回的数据类型
	enum RedisReply
	{
	RedisReply_error = -2, // REDIS_REPLY_ERROR 6
	RedisReply_nil = -1, // REDIS_REPLY_NIL 4
	RedisReply_string = 1, // REDIS_REPLY_STRING 1
	RedisReply_array = 2, // REDIS_REPLY_ARRAY 2
	RedisReply_integer = 3, // REDIS_REPLY_INTEGER 3
	RedisReply_status = 5 // REDIS_REPLY_STATUS 5
	};
	*/
	RedisReply Cmd( const std::string& strCmd,
		long long* out_pllRes, std::string* out_pstrRes, int* out_piStrLen,
		std::vector<StgwRedisReply>* out_pvectArray, size_t* out_pElemSize );

	/*
	执行Redis Command
	Params :
	const vector<string>& vctArgs :
	redis command vector.

	long long* out_pllRes :
	返回的long long值.

	std::string* out_pstrRes :
	返回的string值.

	int* out_piStrLen :
	Length of string.

	std::vector<string>* out_pvectArray :
	返回的数组值.

	size_t* out_pElemSize :
	number of elements, for REDIS_REPLY_ARRAY.

	Return :
	返回的数据类型
	enum RedisReply
	{
	RedisReply_error = -2, // REDIS_REPLY_ERROR 6
	RedisReply_nil = -1, // REDIS_REPLY_NIL 4
	RedisReply_string = 1, // REDIS_REPLY_STRING 1
	RedisReply_array = 2, // REDIS_REPLY_ARRAY 2
	RedisReply_integer = 3, // REDIS_REPLY_INTEGER 3
	RedisReply_status = 5 // REDIS_REPLY_STATUS 5
	};
	*/
	RedisReply CmdArgv( const std::vector<std::string>& vctArgs,
		long long* out_pllRes, std::string* out_pstrRes, int* out_piStrLen,
		std::vector<StgwRedisReply>* out_pvectArray, size_t* out_pElemSize );

protected:
	/*
	连接Redis Server
	Return :
	0 -- 连接成功
	-1 -- 连接失败
	*/
	int OpenNewConnect( const std::string& strSvrIp, const unsigned int uiPort,
		const long in_lTimeout_Millisecond, std::shared_ptr<RedisContextPack_dll>& out_ptrContext );

	/*
	释放连接
	*/
	void FreeContext( std::shared_ptr<RedisContextPack_dll>& in_ptrContext );

	/*
	释放连接
	*/
	void CleanContext( void );

	/*
	添加新连接进连接库
	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int AddConnection( std::shared_ptr<RedisContextPack_dll>& in_ptrContext );

	/*
	从连接库获取连接
	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int GetConnection( const std::string& in_key, std::shared_ptr<RedisContextPack_dll>& out_ptrContext );

	/*
	执行Redis redisCommandArgv
	Params :

	Return :
	redisReply*
	*/
	redisReply_dll* Send_redisCommandArgv( const std::vector<std::string>& vctArgs );

	/*
	check if redis cluster moved
	Return :
	0 -- is cluster moved
	-1 -- not cluster moved
	*/
	int CheckIsClusterMoved( const std::string& strErrMsg,
		std::string& out_strIpPort, std::string& out_strIp, unsigned int& out_uiPort );

	/*
	redis cluster moved, reconnect to another node.

	Return :
	0 -- reconnect success
	-1 -- reconnect failed
	*/
	int ClusterMovedReconnect( const std::string& in_strIpPort,
		const std::string& in_strIp, const unsigned int& in_uiPort,
		std::shared_ptr<RedisContextPack_dll>& out_ptrContext );
};

#endif