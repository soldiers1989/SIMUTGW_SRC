#include "g_client_values.h"

namespace simutgw
{
	namespace client
	{
#ifdef _MSC_VER
		// 注册事件event
		// 自动复位，初始状态未触发
		HANDLE g_registerEvent = CreateEvent(NULL, false, false, NULL);

		// 获取参数event
		HANDLE g_getParamEvent = CreateEvent(NULL, false, false, NULL);
#else
		// 注册事件event
		pthread_mutex_t g_mutex_registerEvent(PTHREAD_MUTEX_INITIALIZER);
		pthread_cond_t g_cond_registerEvent(PTHREAD_COND_INITIALIZER);

		// 获取参数event
		pthread_mutex_t g_mutex_getParamEvent(PTHREAD_MUTEX_INITIALIZER);
		pthread_cond_t g_cond_getParamEvent(PTHREAD_COND_INITIALIZER);
#endif

	}
}