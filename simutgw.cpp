// simutgw.cpp : 定义控制台应用程序的入口点。
//

#include <stdio.h>

#include "simutgw/work_manage/SystemInit.h"

#include "simutgw/stgw_config/g_values_net.h"
#include "simutgw/stgw_config/sys_function.h"

volatile bool g_bRun = true;

#ifdef _MSC_VER
bool ctrlhandler(DWORD fdwctrltype)
{
	switch (fdwctrltype)
	{
		// handle the ctrl-c signal. 
	case CTRL_C_EVENT:
		EzLog::i("ctrlhandler() ", "ctrl-c event");
		break;

		// ctrl-close: confirm that the user wants to exit. 
	case CTRL_CLOSE_EVENT:
		EzLog::i("ctrlhandler() ", "ctrl-close event");
		break;

		// pass other signals to the next handler. 
	case CTRL_BREAK_EVENT:
		EzLog::i("ctrlhandler() ", "ctrl-break event");
		break;

	case CTRL_LOGOFF_EVENT:
		EzLog::i("ctrlhandler() ", "ctrl-logoff event");
		break;

	case CTRL_SHUTDOWN_EVENT:
		EzLog::i("ctrlhandler() ", "ctrl-shutdown event");
		break;

	default:
		break;
	}

	g_bRun = false;
	//simutgw::SimuTgwSelfExit();

	//exit(0);
	return true;
}
#else

#endif


int main(int argc, char* argv[])
{
	try
	{
		// 配置初始化
		int iRes = SystemInit::Simutgw_ConfigInit();
		if (0 != iRes)
		{
			return -1;
		}

#ifdef _MSC_VER
		// 控制台添加或删除应用程序处理函数列表;
		if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrlhandler, true))
		{
			EzLog::i("main() ", "the control handler is installed.");
		}
		else
		{
			EzLog::e("main() ", "the control handler failed to install.");
			return -1;
		}
#else

#endif

		while (g_bRun)
		{
#ifdef _MSC_VER
			Sleep(1);
#else
			usleep(1 * 1000);
#endif
		}

		// 先关闭内部handle
		simutgw::SimuTgwSelfExit();
	}
	catch (exception& e)
	{
		std::cout << "main exception" << e.what() << std::endl;
	}
	catch (...)
	{
#ifdef _MSC_VER
		std::cout << "unkonw exception errno=" << GetLastError << std::endl;
#else
		std::cout << "unkonw exception errno=" << errno << std::endl;
#endif
	}

	// Sleep for others to quit
#ifdef _MSC_VER
	Sleep(15000);
#else
	usleep(15000 * 1000);
#endif

	return 0;
}

