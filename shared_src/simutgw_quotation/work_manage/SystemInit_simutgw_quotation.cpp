#include "SystemInit_simutgw_quotation.h"

#include "boost/date_time.hpp"

#include "config/sys_function_base.h"

#include "config/conf_fix.h"
#include "config/conf_mysql_table.h"

#include "simutgw_config/define_version.h"
#include "simutgw_config/config_define.h"
#include "simutgw_config/g_values_sys_run_config.h"

#include "simutgw_quotation/config/g_values_inner_subp_quot.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_redis/Tgw_RedisHelper.h"
#include "tool_file/FileOperHelper.h"

#include "util/FileHandler.h"
#include "util/EzLog.h"

#include "boost/property_tree/ptree.hpp"    
#include "boost/property_tree/ini_parser.hpp"    

/*
读取模式文件，验证运行模式

@param int type: 类型，暂保留

@return:
0 -- 有模式授权文件
-1 -- 无模式授权文件
*/
typedef int(*pfDll_CheckRunMode)(const int type);

/*
使用动态库检查运行模式

return:
0 -- 有模式文件
-1 -- 无模式文件
*/
int SystemInit_simutgw_quotation::CheckRunMode_Dll()
{
	static const string ftag("SystemInit_simutgw_quotation::CheckRunMode_Dll() ");

	try
	{
		//加载动态链接库hiredis_wrapper.dll文件
		HINSTANCE hAuthDLL = LoadLibrary("dll_AuthCheck.dll");
		if (NULL == hAuthDLL)
		{
			DWORD dw = GetLastError();
			std::string strTran;
			std::string strDebug("couldn't load dll_AuthCheck.dll, error=");
			strDebug += sof_string::itostr((UINT64)dw, strTran);

			char szCurrDir[260];
			GetCurrentDirectory(260, szCurrDir);
			szCurrDir[260 - 1] = '\0';

			strDebug += ", current directory=[";
			strDebug += szCurrDir;
			strDebug += "]";

			EzLog::e(ftag, strDebug);

			return -1;
		}

		// Load functions addresses.
		//
		pfDll_CheckRunMode F_Dll_CheckRunMode = (pfDll_CheckRunMode)GetProcAddress(hAuthDLL, "Dll_CheckRunMode");
		if (NULL == F_Dll_CheckRunMode)
		{
			DWORD dw = GetLastError();
			std::string strTran;
			std::string strDebug("couldn't load Dll_CheckRunMode() in dll_AuthCheck.dll, error=");
			strDebug += sof_string::itostr((UINT64)dw, strTran);
			EzLog::e(ftag, strDebug);

			//卸载dll文件；
			FreeLibrary(hAuthDLL);
			return -1;
		}

		int iRes = F_Dll_CheckRunMode(0);
		if (0 == iRes)
		{
			EzLog::i(ftag, "Mode file good");
		}

		//卸载dll文件；
		if (NULL != hAuthDLL)
		{
			FreeLibrary(hAuthDLL);
		}

		hAuthDLL = NULL;
		F_Dll_CheckRunMode = nullptr;

		return iRes;
	}
	catch (...)
	{
		DWORD dw = GetLastError();
		std::string strTran;
		std::string strDebug("unkown exception, error=");
		strDebug += sof_string::itostr((UINT64)dw, strTran);
		EzLog::e(ftag, strDebug);

		return -1;
	}
}

/*
	检查redis是否连接正常
	return:
	0 -- 成功
	其他 -- 失败
	*/
int SystemInit_simutgw_quotation::CheckRedis()
{
	static const string fTag("SystemInit_simutgw_quotation::CheckRedis() ");

	string strTran;

	string strRedisCmd("DBSIZE");

	long long llRedisRes = 0;
	string strRedisRes;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmd(strRedisCmd, &llRedisRes, &strRedisRes,
		nullptr, nullptr, nullptr);
	if (RedisReply_integer != emPcbCallRes)
	{
		EzLog::e(fTag, "redis call error!");

		return -1;
	}

	return 0;
}

/*
初始化日志

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit_simutgw_quotation::Step0_IniLog()
{
	static const string ftag("SystemInit_simutgw_quotation::Step0_IniLog() ");

	EzLog::Init_log_ern("./config/log_quot.ini");

	return 0;
}

/*
读配置文件

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit_simutgw_quotation::Step2_ReadConfig()
{
	static const string ftag("SystemInit_simutgw_quotation::Step2_ReadConfig() ");

	try
	{
		// 打开读文件
		boost::property_tree::ini_parser::read_ini("config_simutgw.ini", simutgw::g_ptConfig);


		// Web管理模式
		if (0 == simutgw::g_ptConfig.get<std::string>("system.web_man_mode").compare("0"))
		{
			int iRunMode = CheckRunMode_Dll();
			if (0 == iRunMode)
			{
				// 	0 -- 非Web管理模式		
				simutgw::g_iWebManMode = simutgw::WebManMode::LocalMode;
			}
			else
			{
				// 1 --Web管理模式
				simutgw::g_iWebManMode = simutgw::WebManMode::WebMode;
			}
		}
		else
		{
			// 0 -- 非Web管理模式
			simutgw::g_iWebManMode = simutgw::WebManMode::WebMode;
		}

		int iValue = 0;
		//
		// 运行模式
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.run_mode"), iValue);
		string strRunModeType("当前运行模式--");
		if (1 == iValue)
		{
			// 1 -- 压力模式;
			simutgw::g_iRunMode = simutgw::SysRunMode::PressureMode;
			EzLog::i(strRunModeType, "压力模式");
		}
		else if (2 == iValue)
		{
			// 2 -- 极简模式
			simutgw::g_iRunMode = simutgw::SysRunMode::MiniMode;
			EzLog::i(strRunModeType, "极简模式");
		}
		else if (3 == iValue)
		{
			// 3 -- 普通模式
			simutgw::g_iRunMode = simutgw::SysRunMode::NormalMode;
			EzLog::i(strRunModeType, "普通模式");
		}
		else
		{
			// 3 -- 普通模式
			simutgw::g_iRunMode = simutgw::SysRunMode::NormalMode;
		}

		//
		// DB MySQL
		simutgw::g_strSQL_HostName = simutgw::g_ptConfig.get<std::string>("db.mysql_host_name");
		simutgw::g_strSQL_UserName = simutgw::g_ptConfig.get<std::string>("db.mysql_user_name");
		simutgw::g_strSQL_Password = simutgw::g_ptConfig.get<std::string>("db.mysql_password");
		simutgw::g_strSQL_CataLog = simutgw::g_ptConfig.get<std::string>("db.mysql_catalog");

		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("db.mysql_port"), iValue);
		if (iValue > 0)
		{
			simutgw::g_uiSQL_Port = iValue;
		}

		//
		// Redis
		simutgw::g_strRedis_HostName = simutgw::g_ptConfig.get<std::string>("redis.redis_host_name");
		simutgw::g_strRedis_Password = simutgw::g_ptConfig.get<std::string>("redis.redis_password");
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("redis.redis_port"), iValue);
		if (iValue > 0)
		{
			simutgw::g_uiRedis_Port = iValue;
		}

		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("redis.redis_requirepass"), iValue);
		if (0 == iValue)
		{
			// 默认模式
			simutgw::g_bRedis_requirepass = false;
		}
		else
		{
			simutgw::g_bRedis_requirepass = true;
		}

		//
		// web管理端配置

		// web管理端ip
		simutgw::g_strSocketServerIp = simutgw::g_ptConfig.get<std::string>("web_manage.web_man_ip");
		// web管理端port
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("web_manage.web_man_port"), iValue);
		if (iValue >= 1)
		{
			// 
			simutgw::g_uiSocketServerPort = iValue;
		}
		else
		{
			simutgw::g_uiSocketServerPort = 10005;
		}

		// 是否是高性能
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.high_pfm"), iValue);
		if (iValue >= 1)
		{
			// 高性能
			simutgw::g_bHighPfm = true;
		}
		else
		{
			simutgw::g_bHighPfm = false;
		}

		//
		// 成交模式
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.match_mode"), iValue);
		string strMatchModeType("当前成交模式--");
		if (0 == iValue)
		{
			// 默认模式
			simutgw::g_iMatchMode = simutgw::SysMatchMode::EnAbleQuta;
			EzLog::i(strMatchModeType, "实盘模拟");
		}
		else if (1 == iValue)
		{
			// 全部成交
			simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchAll;
			EzLog::i(strMatchModeType, "模拟仿真");
		}
		else if (2 == iValue)
		{
			// 分笔成交
			simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchByDivide;
			EzLog::i(strMatchModeType, "模拟仿真");
		}
		else if (3 == iValue)
		{
			// 不成交，挂单，可以撤单
			simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulNotMatch;
			EzLog::i(strMatchModeType, "模拟仿真");
		}
		else if (4 == iValue)
		{
			// 错误单
			simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulErrMatch;
			EzLog::i(strMatchModeType, "模拟仿真");
		}
		else if (5 == iValue)
		{
			// 错误单
			simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchPart;
			EzLog::i(strMatchModeType, "模拟仿真");
		}
		else
		{
			string strError("成交模式未知matchmode=[");
			strError += simutgw::g_ptConfig.get<std::string>("system.match_mode");
			strError += "]";

			EzLog::e("SimutgwConfigInit() ", strError);
		}

		// 是否启用行情
		simutgw::g_bEnable_Quotation_Task = true;

		// 是否启用深圳消息处理
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.sz_link_enable"), iValue);
		if (iValue >= 1)
		{
			simutgw::g_bEnable_Sz_Msg_Task = true;
		}
		else
		{
			simutgw::g_bEnable_Sz_Msg_Task = false;
		}

		// 是否启用上海消息处理
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.sh_link_enable"), iValue);
		if (iValue >= 1)
		{
			simutgw::g_bEnable_Sh_Msg_Task = true;
		}
		else
		{
			simutgw::g_bEnable_Sh_Msg_Task = false;
		}

		// 是否启用验资验股
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.enable_check_assets"), iValue);
		if (iValue >= 1 && simutgw::g_iRunMode == simutgw::SysRunMode::NormalMode)
		{
			// 普通模式下支持验股
			simutgw::g_bEnable_Check_Assets = true;
		}
		else
		{
			// 其他模式不支持
			simutgw::g_bEnable_Check_Assets = false;
		}

		// 实盘模拟行情模式
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.quotation_mode"), iValue);
		if (1 == iValue)
		{
			//  买一卖一
			simutgw::g_Quotation_Type = simutgw::SellBuyPrice;
		}
		else if (2 == iValue)
		{
			//  最近成交价
			simutgw::g_Quotation_Type = simutgw::RecentMatchPrice;
		}
		else
		{
			// 默认为区间段均价
			simutgw::g_Quotation_Type = simutgw::AveragePrice;
		}

		// 分笔成交笔数
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.part_match_num"), iValue);
		if (iValue >= 2)
		{
			simutgw::g_ui32_Part_Match_Num = iValue;
		}
		else
		{
			// 分笔最少为2笔
			simutgw::g_ui32_Part_Match_Num = 2;
		}
	}
	catch (exception& e)
	{
		string sDebug(ftag);
		sDebug += e.what();
		EzLog::e(ftag, sDebug);
		EzLog::ex(ftag, e);

		simutgw::ErrorExit(-1);
	}
	return 0;
}

/*
初始化相关配置

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit_simutgw_quotation::Step5_2_RedisPoolInit(void)
{
	static const string ftag("SystemInit_simutgw_quotation::Step5_2_RedisPoolInit() ");

	//
	// redis
	simutgw::g_redisPool.SetConnection(simutgw::g_strRedis_HostName, simutgw::g_uiRedis_Port, 10,
		simutgw::g_bRedis_requirepass, simutgw::g_strSQL_Password);

	int iRes = Tgw_RedisHelper::LoadHiredisLibrary();
	if (0 != iRes)
	{
		EzLog::e("Method_system_run()", "LoadHiredisLibrary failed");
		perror("LoadHiredisLibrary failed");

		simutgw::ErrorExit(-1);
	}

	iRes = simutgw::g_redisPool.Init();
	if (0 != iRes)
	{
		EzLog::e("Method_system_run()", "g_redisPool.Init failed");
		perror("g_redisPool.Init failed");

		simutgw::ErrorExit(-1);
	}

	Sleep(5);

	return 0;
}

/*
检查redis

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit_simutgw_quotation::Step6_1_CheckRedis(void)
{
	static const string ftag("SystemInit_simutgw_quotation::Step6_1_CheckRedis() ");

	int iRes = CheckRedis();
	if (0 != iRes)
	{
		EzLog::e(ftag, " System Initialization Failed, CheckRedis() Failed");
		Sleep(30000);
		exit(3);
	}

	return 0;
}

/*
子进程行情处理 配置初始化
return:
0 -- 成功
其他 -- 失败
*/
int SystemInit_simutgw_quotation::ChildP_Quotation_ConfigInit(void)
{
	static const string ftag("SystemInit_simutgw_quotation::ChildP_Quotation_ConfigInit() ");

	EzLog::i(ftag, "System Initialization begin");

	Step0_IniLog();

	// step 2, 读配置文件
	Step2_ReadConfig();

	Step5_2_RedisPoolInit();

	switch (simutgw::g_iRunMode)
	{
	case simutgw::SysRunMode::PressureMode:
		// 1 -- 压力模式;
		break;

	case simutgw::SysRunMode::MiniMode:
		// 2 -- 极简模式

	case simutgw::SysRunMode::NormalMode:
		// 3 -- 普通模式

		Step6_1_CheckRedis();
		break;

	default:
		break;
	}

	// Looper处理线程
	simutgw::g_flowManage_quota.StartFlows();

	int iBitSize = 8 * sizeof(int*);

	string sItoa;
	sof_string::itostr(iBitSize, sItoa);

	EzLog::i(ftag, " System Initialization Success, " + sItoa + "bit " + simutgw::g_strSystemVersion);

	return 0;
}

/*
控件自身必要程序回收
*/
void SystemInit_simutgw_quotation::SelfExit(void)
{
	static const string strTag("SystemInit_simutgw_quotation::SelfExit() ");

	simutgw::g_flowManage_quota.StopFlows();
	EzLog::i(strTag, "g_flowManage StopFlows");

	simutgw::g_redisPool.Stop();

	//释放资源

	Tgw_RedisHelper::FreeHiredisLibrary();
}