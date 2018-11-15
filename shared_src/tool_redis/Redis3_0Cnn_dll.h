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

	// ��ǰ��redis���� key
	std::shared_ptr<RedisContextPack_dll> m_ptrCurrentContext;

	// redis clusterʱ���Ӽ�Ⱥ
	// key��<ip:port>
	std::map<std::string, std::shared_ptr<RedisContextPack_dll>> m_mapRedisContexts;

	// �Ƿ�ʹ��string.c_str()
	bool m_bShareMem;

	// �ϴ�ʹ�õ�ʱ��� ��λ����
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
	����Redis Server
	Return :
	0 -- ���ӳɹ�
	-1 -- ����ʧ��
	*/
	int Connect( const std::string& strSvrIp, const unsigned int uiPort,
		const long in_lTimeout_Millisecond );

	/*
	�Ͽ�����
	Return :
	*/
	void Disconnect(void);

	/*
	ִ��Redis Command
	Params :
	const std::string strCmd :
	redis command string.

	long long* out_pllRes :
	���ص�long longֵ.

	std::string* out_pstrRes :
	���ص�stringֵ.

	int* out_piStrLen :
	Length of string.

	std::vector<string>* out_pvectArray :
	���ص�����ֵ.

	size_t* out_pElemSize :
	number of elements, for REDIS_REPLY_ARRAY.

	Return :
	���ص���������
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
	ִ��Redis Command
	Params :
	const vector<string>& vctArgs :
	redis command vector.

	long long* out_pllRes :
	���ص�long longֵ.

	std::string* out_pstrRes :
	���ص�stringֵ.

	int* out_piStrLen :
	Length of string.

	std::vector<string>* out_pvectArray :
	���ص�����ֵ.

	size_t* out_pElemSize :
	number of elements, for REDIS_REPLY_ARRAY.

	Return :
	���ص���������
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
	����Redis Server
	Return :
	0 -- ���ӳɹ�
	-1 -- ����ʧ��
	*/
	int OpenNewConnect( const std::string& strSvrIp, const unsigned int uiPort,
		const long in_lTimeout_Millisecond, std::shared_ptr<RedisContextPack_dll>& out_ptrContext );

	/*
	�ͷ�����
	*/
	void FreeContext( std::shared_ptr<RedisContextPack_dll>& in_ptrContext );

	/*
	�ͷ�����
	*/
	void CleanContext( void );

	/*
	��������ӽ����ӿ�
	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int AddConnection( std::shared_ptr<RedisContextPack_dll>& in_ptrContext );

	/*
	�����ӿ��ȡ����
	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int GetConnection( const std::string& in_key, std::shared_ptr<RedisContextPack_dll>& out_ptrContext );

	/*
	ִ��Redis redisCommandArgv
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