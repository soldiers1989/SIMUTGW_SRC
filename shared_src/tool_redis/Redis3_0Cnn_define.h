#ifndef __REDIS_3_0_CNN_DEFINE_H__
#define __REDIS_3_0_CNN_DEFINE_H__

#ifdef _MSC_VER
#include <Windows.h>
#else

#endif

#include <string>
#include <errno.h>
#include <vector>
#include <ctime>
#include <map>
#include <stdint.h>

#include "boost/shared_ptr.hpp"

struct StgwRedisReply
{
	int type;
	std::string str;
	long long integer;
};

enum RedisReply
{
	RedisReply_error = -2, // REDIS_REPLY_ERROR 6
	RedisReply_nil = -1, // REDIS_REPLY_NIL 4
	RedisReply_string = 1, // REDIS_REPLY_STRING 1
	RedisReply_array = 2, // REDIS_REPLY_ARRAY 2
	RedisReply_integer = 3, // REDIS_REPLY_INTEGER 3		
	RedisReply_status = 5 // REDIS_REPLY_STATUS 5		
};

/* Context for a connection to Redis */
typedef struct redisContext_dll
{
	int err; /* Error flags, 0 when there is no error */
	char errstr[128]; /* String representation of error when applicable */
	int fd;
	int flags;
	char *obuf; /* Write buffer */
	void *reader; /* Protocol reader */
} redisContext_dll;

/*
redis的储存结构
*/
struct RedisContextPack_dll
{
	// ip:port 的key
	std::string strKey;
	// ip
	std::string strIP;
	// port
	unsigned int uiPort;
	// last alive time
	time_t tLastActiveTime;
	// redis context redisContext*
	redisContext_dll* contextRedis;
};

/* This is the reply object returned by redisCommand() */
typedef struct redisReply_dll
{
	int type; /* REDIS_REPLY_* */
	int64_t integer; /* The integer when type is REDIS_REPLY_INTEGER */
	int len; /* Length of string */
	char *str; /* Used for both REDIS_REPLY_ERROR and REDIS_REPLY_STRING */
	size_t elements; /* number of elements, for REDIS_REPLY_ARRAY */
	struct redisReply_dll **element; /* elements vector for REDIS_REPLY_ARRAY */
} redisReply_dll;

namespace simutgw
{
	static const int redis_REDIS_REPLY_STRING = 1;
	static const int redis_REDIS_REPLY_ARRAY = 2;
	static const int redis_REDIS_REPLY_INTEGER = 3;
	static const int redis_REDIS_REPLY_NIL = 4;
	static const int redis_REDIS_REPLY_STATUS = 5;
	static const int redis_REDIS_REPLY_ERROR = 6;

	// Redis返回OK
	static const std::string g_RplyRes_OK = "OK";

	// connection time out
	static const long redis_clTimeOut = 2000;
};

#endif