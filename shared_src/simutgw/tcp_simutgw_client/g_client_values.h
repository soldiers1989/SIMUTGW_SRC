#ifndef __G_CLIENT_VALUES_H__
#define __G_CLIENT_VALUES_H__

#ifdef _MSC_VER
#include "windows.h"
#else
#include <pthread.h>
#endif


namespace simutgw
{
	namespace client
	{
#ifdef _MSC_VER
		// 注册事件event
		extern HANDLE g_registerEvent;

		// 获取参数event
		extern HANDLE g_getParamEvent;
#else
		// 注册事件event
		extern pthread_mutex_t g_mutex_registerEvent;
		extern pthread_cond_t g_cond_registerEvent;

		// 获取参数event
		extern pthread_mutex_t g_mutex_getParamEvent;
		extern pthread_cond_t g_cond_getParamEvent;
#endif
	}
}

#endif