#include "g_client_values.h"

namespace simutgw
{
	namespace client
	{
#ifdef _MSC_VER
		// ע���¼�event
		// �Զ���λ����ʼ״̬δ����
		HANDLE g_registerEvent = CreateEvent(NULL, false, false, NULL);

		// ��ȡ����event
		HANDLE g_getParamEvent = CreateEvent(NULL, false, false, NULL);
#else
		// ע���¼�event
		pthread_mutex_t g_mutex_registerEvent(PTHREAD_MUTEX_INITIALIZER);
		pthread_cond_t g_cond_registerEvent(PTHREAD_COND_INITIALIZER);

		// ��ȡ����event
		pthread_mutex_t g_mutex_getParamEvent(PTHREAD_MUTEX_INITIALIZER);
		pthread_cond_t g_cond_getParamEvent(PTHREAD_COND_INITIALIZER);
#endif

	}
}