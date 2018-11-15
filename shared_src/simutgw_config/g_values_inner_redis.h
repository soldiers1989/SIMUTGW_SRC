#ifndef __G_VALUES_INNER_REDIS_H__
#define __G_VALUES_INNER_REDIS_H__

#include <stdint.h>
#include "config/g_values_inner_IdGen.h"

#include "tool_redis/RedisConnectionPool.h"

#include "tool_redis/Redis3_0Cnn_dll.h"

namespace simutgw
{	
	// Redis连接池，因为要用到数据库配置g_strRedis_HostName等，所以声明在其之后
	extern RedisConnectionPool<Redis3_0Cnn_dll> g_redisPool;
}

#endif