#ifndef __G_VALUES_INNER_REDIS_H__
#define __G_VALUES_INNER_REDIS_H__

#include <stdint.h>
#include "config/g_values_inner_IdGen.h"

#include "tool_redis/RedisConnectionPool.h"

#include "tool_redis/Redis3_0Cnn_dll.h"

namespace simutgw
{	
	// Redis���ӳأ���ΪҪ�õ����ݿ�����g_strRedis_HostName�ȣ�������������֮��
	extern RedisConnectionPool<Redis3_0Cnn_dll> g_redisPool;
}

#endif