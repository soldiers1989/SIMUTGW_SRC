#ifndef __TGW_REDIS_HELPER_H__
#define __TGW_REDIS_HELPER_H__

#include <string>

#ifdef _MSC_VER
#include <Winsock2.h>
#else
#include <dlfcn.h>
#endif

#include "tool_json/RapidJsonHelper_tgw.h"
#include "tool_redis/Redis3_0Cnn_dll.h"

using namespace std;

/*
操作simutgw的Redis插件代理类
*/
class Tgw_RedisHelper
{
	//
	// Members
	//
public:

	// Redis返回OK
	static const string g_RplyRes_OK;

	// 8小时过期时间
	static const string g_Expire_8Hour;

	/* function pointers */
	typedef redisContext_dll* (*pfDll_redisConnectWithTimeout)(const char *ip, int port, const timeval tv);

	typedef void(*pfDll_redisFree)(redisContext_dll *c);

	typedef void* (*pfDLL_redisCmd) (redisContext_dll *c, const char *cmd);
	typedef void* (*pfDLL_redisCommandArgv)(redisContext_dll *c, int argc, const char **argv, const size_t *argvlen);

	typedef void(*pfDLL_freeReplyObject)(void *reply);

	// hiredis functions
	static pfDll_redisConnectWithTimeout F_Dll_redisConnectWithTimeout;
	static pfDll_redisFree F_Dll_redisFree;
	static pfDLL_redisCmd F_DLL_redisCmd;
	static pfDLL_redisCommandArgv F_DLL_redisCommandArgv;
	static pfDLL_freeReplyObject F_DLL_freeReplyObject;

	//
	// Functions
	//
private:
	Tgw_RedisHelper(void);
	virtual ~Tgw_RedisHelper(void);

#ifdef _MSC_VER
	static HINSTANCE m_hDLL;
#else
	static void* m_hDLL;
#endif

public:
	/*
	Load hiredis wrapper dll.
	*/
	static int LoadHiredisLibrary(void);

	/*
	Unload hiredis wrapper dll.
	*/
	static void FreeHiredisLibrary(void);

	/*
	执行Redis Command
	Params :
	const std::string strCmd :
	redis command string.

	long long* out_pllRes :
	返回的long long值.

	std::string* out_pstrRes :
	返回的string值.

	std::vector<string>* out_pvectArray :
	返回的数组值.

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
	static RedisReply RunCmd(const std::string& in_strRedisCmd,
		long long* out_pllRes, std::string* out_pstrRes, int* out_piStrLen,
		std::vector<StgwRedisReply>* out_pvectArray, size_t* out_pElemSize);

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
	static RedisReply RunCmdArgv(const vector<string>& vctArgs,
		long long* out_pllRes, std::string* out_pstrRes, int* out_piStrLen,
		std::vector<StgwRedisReply>* out_pvectArray, size_t* out_pElemSize);

	/*
	判断Redis的返回结果
	Params:
	const RedisReply in_emReplyType :
	enum RedisReply
	{
	RedisReply_error = -2, // REDIS_REPLY_ERROR 6
	RedisReply_nil = -1, // REDIS_REPLY_NIL 4
	RedisReply_string = 1, // REDIS_REPLY_STRING 1
	RedisReply_array = 2, // REDIS_REPLY_ARRAY 2
	RedisReply_integer = 3, // REDIS_REPLY_INTEGER 3
	RedisReply_status = 5 // REDIS_REPLY_STATUS 5
	};

	Return :
	true -- 操作成功
	false -- 操作失败
	*/
	static bool IsRedisCmdSuccess(const RedisReply in_emReplyType,
		const long long in_llRes, const string& in_strRedisRes);

	/*
	查看redis的队列深度
	Return:
	>=0 -- 成功
	-1 -- 失败
	*/
	static int GetLength(const string& in_strQueueName);

};

#endif