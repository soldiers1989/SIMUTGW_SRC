#include "sys_function_base.h"

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "simutgw_config/g_values_sys_run_config.h"

using namespace std;

namespace simutgw
{
	/*
	�����˳�
	*/
	void ErrorExit( const int exitcode )
	{
#ifdef _MSC_VER
		Sleep( 10000 );
#else
		usleep(10000 * 1000);
#endif

		exit( exitcode );
	}

	// ��׼sleep
	void Simutgw_Sleep( void )
	{
		if ( simutgw::g_bHighPfm )
		{
			// ������ģʽ
#ifdef _MSC_VER
			Sleep( simutgw::g_iHighPfmSleepTime );
#else
			usleep(simutgw::g_iHighPfmSleepTime * 1000);
#endif

		}
		else
		{
			// �Ǹ�����ģʽ
#ifdef _MSC_VER
			Sleep( simutgw::g_iUnHighPfmSleepTime );
#else
			usleep(simutgw::g_iUnHighPfmSleepTime * 1000);
#endif

		}
	}
}