#include "SystemInit.h"

#include "boost/date_time.hpp"
#include "boost/property_tree/ptree.hpp"    
#include "boost/property_tree/ini_parser.hpp"    

#include "simutgw_config/define_version.h"
#include "simutgw_config/config_define.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/sys_function.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/stgw_config/g_values_biz.h"
#include "simutgw/stgw_config/g_values_net.h"
#include "config/conf_fix.h"
#include "config/conf_mysql_table.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "tool_odbc/OTLConn40240.h"
#include "tool_redis/Tgw_RedisHelper.h"
#include "tool_file/FileOperHelper.h"

#include "etf/ETFHelper.h"

#include "util/FileHandler.h"
#include "util/EzLog.h"

#include "simutgw/db_oper/RecordNewOrderHelper.h"
#include "simutgw/work_manage/Proc_WorkLoadTrafficStat.h"


/* function pointers */
// dll 授权验证
/*
int Dll_ValidAuth( const int type, char* out_buffer, const unsigned int uiOutbufferSize )
读取授权文件，验证授权

@param int type: 类型，暂保留
@param char* out_buffer: 返回消息buffer
@param int iOutbufferSize: 返回消息buffer size

@return:
0 -- 未到期
1 -- 接近到期时间，将会通过 @param char* out_buffer 返回警告信息
-1 -- 到期
*/
typedef int(*pfDll_ValidAuth)(const int, char*, const unsigned int);

/*
读取模式文件，验证运行模式

@param int type: 类型，暂保留

@return:
0 -- 有模式授权文件
-1 -- 无模式授权文件
*/
typedef int(*pfDll_CheckRunMode)(const int type);


/*
使用动态库检查授权是否到期

return:
0 -- 未到期
-1 -- 到期
*/
int SystemInit::ValidateLicense_Dll()
{
	static const string ftag("SystemInit::ValidateLicense_Dll() ");

#ifdef _MSC_VER
	try
	{
		//加载动态链接库dll_AuthCheck.dll文件
		HINSTANCE hAuthDLL = LoadLibrary("dll_AuthCheck.dll");
		if (NULL == hAuthDLL)
		{
			DWORD dw = GetLastError();
			std::string strTran;
			std::string strDebug("couldn't load dll_AuthCheck.dll, error=");
			strDebug += sof_string::itostr((uint64_t)dw, strTran);

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
		pfDll_ValidAuth F_Dll_ValidAuth = (pfDll_ValidAuth)GetProcAddress(hAuthDLL, "Dll_ValidAuth");
		if (NULL == F_Dll_ValidAuth)
		{
			DWORD dw = GetLastError();
			std::string strTran;
			std::string strDebug("couldn't load Dll_ValidAuth() in dll_AuthCheck.dll, error=");
			strDebug += sof_string::itostr((uint64_t)dw, strTran);
			EzLog::e(ftag, strDebug);

			//卸载dll文件；
			FreeLibrary(hAuthDLL);
			return -1;
		}

		const unsigned int uiWarnMsgSize = 200;
		char szWarnMsg[uiWarnMsgSize];

		int iRes = F_Dll_ValidAuth(0, szWarnMsg, uiWarnMsgSize);
		if (1 == iRes)
		{
			EzLog::w(ftag, szWarnMsg);
		}

		//卸载dll文件；
		FreeLibrary(hAuthDLL);

		hAuthDLL = NULL;
		F_Dll_ValidAuth = nullptr;

		return iRes;
	}
	catch (...)
	{
		DWORD dw = GetLastError();
		std::string strTran;
		std::string strDebug("unkown exception, error=");
		strDebug += sof_string::itostr((uint64_t)dw, strTran);
		EzLog::e(ftag, strDebug);

		return -1;
	}
#else
	try
	{
		//加载动态链接库dll_AuthCheck.dll文件
		void* hAuthDLL = dlopen("libdll_AuthCheck.so", RTLD_LAZY);
		if (NULL == hAuthDLL)
		{			
			std::string strDebug("couldn't load libdll_AuthCheck.so, error=");
			strDebug += dlerror();

			EzLog::e(ftag, strDebug);

			return -1;
		}

		// Load functions addresses.
		//
		pfDll_ValidAuth F_Dll_ValidAuth = (pfDll_ValidAuth)dlsym(hAuthDLL, "Dll_ValidAuth");
		if (NULL == F_Dll_ValidAuth)
		{
			std::string strDebug("couldn't load Dll_ValidAuth() in libdll_AuthCheck.so, error=");
			strDebug += dlerror();
			EzLog::e(ftag, strDebug);

			//卸载dll文件；
			if (NULL != hAuthDLL)
			{
				dlclose(hAuthDLL);
			}

			return -1;
		}

		const unsigned int uiWarnMsgSize = 200;
		char szWarnMsg[uiWarnMsgSize];

		int iRes = F_Dll_ValidAuth(0, szWarnMsg, uiWarnMsgSize);
		if (1 == iRes)
		{
			EzLog::w(ftag, szWarnMsg);
		}

		//卸载dll文件；
		if (NULL != hAuthDLL)
		{
			dlclose(hAuthDLL);
		}

		hAuthDLL = NULL;
		F_Dll_ValidAuth = nullptr;

		return iRes;
	}
	catch (...)
	{
		int iErr = errno;
		std::string strTran;
		std::string strDebug("unkown exception, error=");
		strDebug += sof_string::itostr(iErr, strTran);
		strDebug += " serror=";
		strDebug += strerror(iErr);

		EzLog::e(ftag, strDebug);

		return -1;
	}
#endif

}

/*
使用动态库检查运行模式

return:
0 -- 有模式文件
-1 -- 无模式文件
*/
int SystemInit::CheckRunMode_Dll()
{
	static const string ftag("SystemInit::CheckRunMode_Dll() ");

#ifdef _MSC_VER
	try
	{
		//加载动态链接库dll_AuthCheck.dll文件
		HINSTANCE hAuthDLL = LoadLibrary("dll_AuthCheck.dll");
		if (NULL == hAuthDLL)
		{
			DWORD dw = GetLastError();
			std::string strTran;
			std::string strDebug("couldn't load dll_AuthCheck.dll, error=");
			strDebug += sof_string::itostr((uint64_t)dw, strTran);

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
			strDebug += sof_string::itostr((uint64_t)dw, strTran);
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
		strDebug += sof_string::itostr((uint64_t)dw, strTran);
		EzLog::e(ftag, strDebug);

		return -1;
	}
#else
	try
	{
		//加载动态链接库hiredis_wrapper.dll文件
		void* hAuthDLL = dlopen("libdll_AuthCheck.so", RTLD_LAZY);
		if (NULL == hAuthDLL)
		{
			std::string strDebug("couldn't load libdll_AuthCheck.dll, error=");
			strDebug += dlerror();

			EzLog::e(ftag, strDebug);

			return -1;
		}

		// Load functions addresses.
		//
		pfDll_CheckRunMode F_Dll_CheckRunMode = (pfDll_CheckRunMode)dlsym(hAuthDLL, "Dll_CheckRunMode");
		if (NULL == F_Dll_CheckRunMode)
		{
			std::string strDebug("couldn't load Dll_CheckRunMode() in libdll_AuthCheck.so, error=");
			strDebug += dlerror();
			EzLog::e(ftag, strDebug);

			//卸载dll文件；
			if (NULL != hAuthDLL)
			{
				dlclose(hAuthDLL);
			}

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
			dlclose(hAuthDLL);
		}

		hAuthDLL = NULL;
		F_Dll_CheckRunMode = nullptr;

		return iRes;
	}
	catch (...)
	{
		int err = errno;
		std::string strTran;
		std::string strDebug("unkown exception, error=");
		strDebug += sof_string::itostr(err, strTran);
		EzLog::e(ftag, strDebug);

		return -1;
	}
#endif
}

/*
	检查mysql是否连接正常
	return:
	0 -- 成功
	其他 -- 失败
	*/
int SystemInit::CheckMysql()
{
	static const string fTag("SystemInit::CheckMysql()");

	int iReturn = 0;

	//从mysql连接池取连接
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (NULL == mysqlConn)
	{
		//取出的mysql连接为NULL

		//归还连接
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		EzLog::e(fTag, "Get Connection is NULL");

		return -1;
	}

	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	string strQueryString = "SELECT `security_seat` FROM `account` WHERE `security_account`='autotest'";

	int iRes = mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
	if (1 == iRes)
	{
		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		if (0 != mysqlConn->FetchNextRow(&pResultSet, mapRowData))
		{
			string strBalance(mapRowData["security_seat"].strValue);

			uint64_t ui64Seat;
			Tgw_StringUtil::String2UInt64_strtoui64(strBalance, ui64Seat);

		}
		else
		{
			iReturn = 0;
		}

		// 释放
		mysqlConn->FreeResult(&pResultSet);
		pResultSet = NULL;
	}
	else if (iRes < 0)
	{
		string strItoa;
		string strDebug("运行[");
		strDebug += strQueryString;
		strDebug += "]得到";
		strDebug += sof_string::itostr(iRes, strItoa);
		EzLog::e(fTag, strDebug);

		iReturn = -1;
	}

	simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

	return iReturn;
}

/*
	检查redis是否连接正常
	return:
	0 -- 成功
	其他 -- 失败
	*/
int SystemInit::CheckRedis()
{
	static const string fTag("SystemInit::CheckRedis() ");

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
保存id到文件

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::SaveIdToFile(void)
{
	static const string ftag("SystemInit::SaveIdToFile() ");

	try
	{
		boost::property_tree::ptree fileId;

		// 写字段
		fileId.put<std::string>("web.id", simutgw::g_strWeb_id);

		// 写到文件 
		boost::property_tree::ini_parser::write_ini("config_webid.ini", fileId);
	}
	catch (exception& e)
	{
		string sDebug(ftag);
		sDebug += e.what();
		EzLog::e(ftag, sDebug);

		simutgw::ErrorExit(-1);
		return -1;
	}
	return 0;
}

/*
保存id
return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::SaveIdFromWeb()
{
	static const string ftag("SystemInit::SaveIdFromWeb() ");

	// insert into user(id, userid) values (1, 'test') on duplicate key update id=1, userid='test';
	string strQueryString = "insert into user(id, userid) values (1,'";
	strQueryString += simutgw::g_strWeb_id;
	strQueryString += "') on duplicate key update id=1, userid='";
	strQueryString += simutgw::g_strWeb_id;
	strQueryString += "'";

	int iReturn = 0;

	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		// 查询id及余额

		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(ftag, "Get Connection is NULL");

			return -1;
		}

		int iRes = mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (2 == iRes)
		{
			// update or insert
			mysqlConn->Commit();
		}
		else
		{
			mysqlConn->RollBack();

			string strItoa;
			string strDebug("运行[");
			strDebug += strQueryString;
			strDebug += "]得到";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(ftag, strDebug);

			iReturn = -2;
		}

		//归还连接
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
		return iReturn;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return iReturn;
}

/*
初始化日志

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step0_IniLog()
{
	static const string ftag("SystemInit::Step0_IniLog() ");

	EzLog::Init_log_ern("./config/log_simutgw.ini");

	return 0;
}

/*
读配置文件

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step1_CheckLicense()
{
	static const string ftag("SystemInit::Step1_CheckLicense() ");

	int iRes = ValidateLicense_Dll();
	if (0 <= iRes)
	{
		EzLog::i(ftag, "simutgw auth check OK.");
	}
	else
	{
		EzLog::e(ftag, "simutgw auth check Failed.");

		return -1;
	}

	return 0;
}

/*
读配置文件

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step2_ReadConfig()
{
	static const string ftag("SystemInit::Step2_ReadConfig() ");

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
				EzLog::i("", "当前管理模式--本地管理模式");
			}
			else
			{
				// 1 --Web管理模式
				simutgw::g_iWebManMode = simutgw::WebManMode::WebMode;
				EzLog::i("", "当前管理模式--Web管理模式");
			}
		}
		else
		{
			// 0 -- 非Web管理模式
			simutgw::g_iWebManMode = simutgw::WebManMode::WebMode;
			EzLog::i("", "当前管理模式--Web管理模式");
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
			EzLog::i(strRunModeType, "普通模式");
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
			EzLog::i("", "当前性能模式--高性能");
		}
		else
		{
			simutgw::g_bHighPfm = false;
			EzLog::i("", "当前性能模式--默认");
		}

		// 深圳STEP回报是否是Ver1.10
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.sz_step_ver110"), iValue);
		if (iValue >= 1)
		{
			simutgw::g_bSZ_Step_ver110 = true;
		}
		else
		{
			simutgw::g_bSZ_Step_ver110 = false;
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
			EzLog::i("", "当前验股模式--验股");
		}
		else
		{
			// 其他模式不支持
			simutgw::g_bEnable_Check_Assets = false;
			EzLog::i("", "当前验股模式--不验证");
		}

		// 实盘模拟行情模式
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.quotation_mode"), iValue);
		if (1 == iValue)
		{
			//  买一卖一
			simutgw::g_Quotation_Type = simutgw::SellBuyPrice;
			EzLog::i("", "实盘模拟行情模式--买一卖一");
		}
		else if (2 == iValue)
		{
			//  最近成交价
			simutgw::g_Quotation_Type = simutgw::RecentMatchPrice;
			EzLog::i("", "实盘模拟行情模式--最近成交价");
		}
		else
		{
			// 默认为区间段均价
			simutgw::g_Quotation_Type = simutgw::AveragePrice;
			EzLog::i("", "实盘模拟行情模式--区间段均价");
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

		// 非web管理模式下上海数据库连接
		if (simutgw::WebManMode::LocalMode == simutgw::g_iWebManMode)
		{
			string strName = ("Sh_Conn1");
			string strConn = simutgw::g_ptConfig.get<std::string>("Sh_Conn1.connection");
			// 是否启用上海接口1
			Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("Sh_Conn1.enable"), iValue);
			if (iValue >= 1)
			{
				simutgw::g_mapShConns[strName].SetName(strName);
				simutgw::g_mapShConns[strName].SetConnection(strConn);

				// 增加sh链接计数器
				simutgw::g_counter.AddSh_LinkCounter(strName);

				EzLog::i(ftag, "add sh db link name=" + strName);
			}
			else
			{
			}

			strName.clear();
			strConn.clear();
			strName = ("Sh_Conn1");
			strConn = simutgw::g_ptConfig.get<std::string>("Sh_Conn2.connection");
			// 是否启用上海接口2
			Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("Sh_Conn2.enable"), iValue);
			if (iValue >= 1)
			{
				simutgw::g_mapShConns[strName].SetName(strName);
				simutgw::g_mapShConns[strName].SetConnection(strConn);

				// 增加sh链接计数器
				simutgw::g_counter.AddSh_LinkCounter(strName);

				EzLog::i(ftag, "add sh db link name=" + strName);
			}
			else
			{
			}

			strName.clear();
			strConn.clear();
			strName = ("Sh_Conn3");
			strConn = simutgw::g_ptConfig.get<std::string>("Sh_Conn3.connection");
			// 是否启用上海接口3
			Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("Sh_Conn3.enable"), iValue);
			if (iValue >= 1)
			{
				simutgw::g_mapShConns[strName].SetName(strName);
				simutgw::g_mapShConns[strName].SetConnection(strConn);

				// 增加sh链接计数器
				simutgw::g_counter.AddSh_LinkCounter(strName);

				EzLog::i(ftag, "add sh db link name=" + strName);
			}
			else
			{
			}

			strName.clear();
			strConn.clear();
			strName = ("Sh_Conn4");
			strConn = simutgw::g_ptConfig.get<std::string>("Sh_Conn4.connection");
			// 是否启用上海接口4
			Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("Sh_Conn4.enable"), iValue);
			if (iValue >= 1)
			{
				simutgw::g_mapShConns[strName].SetName(strName);
				simutgw::g_mapShConns[strName].SetConnection(strConn);

				// 增加sh链接计数器
				simutgw::g_counter.AddSh_LinkCounter(strName);

				EzLog::i(ftag, "add sh db link name=" + strName);
			}
			else
			{
			}

			strName.clear();
			strConn.clear();
			strName = ("Sh_Conn5");
			strConn = simutgw::g_ptConfig.get<std::string>("Sh_Conn5.connection");
			// 是否启用上海接口5
			Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("Sh_Conn5.enable"), iValue);
			if (iValue >= 1)
			{
				simutgw::g_mapShConns[strName].SetName(strName);
				simutgw::g_mapShConns[strName].SetConnection(strConn);

				// 增加sh链接计数器
				simutgw::g_counter.AddSh_LinkCounter(strName);

				EzLog::i(ftag, "add sh db link name=" + strName);
			}
			else
			{
			}
		}

		//是否启用服务器
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("server.enable_server"), iValue);
		if (iValue >= 1)
		{
			simutgw::g_bEnableServer = true;
		}
		else
		{
		}

		// 服务器listen地址
		simutgw::g_strServerIp = simutgw::g_ptConfig.get<std::string>("server.simutgw_ip");
		// 服务器listen端口
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("server.simutgw_port"), iValue);
		if (iValue > 0)
		{
			simutgw::g_uiServerPort = iValue;
		}
	}
	catch (exception& e)
	{
		string sDebug(ftag);
		sDebug += e.what();
		EzLog::e(ftag, sDebug);
		EzLog::ex(ftag, e);

		return -1;
	}
	return 0;
}

/*
启动mysql连接池

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step3_1_MysqlPoolInit(void)
{
	//
	// mysql
	simutgw::g_mysqlPool.SetConnection(simutgw::g_strSQL_HostName, simutgw::g_strSQL_UserName,
		simutgw::g_strSQL_Password, simutgw::g_uiSQL_Port, simutgw::g_strSQL_CataLog, 10);

	int iRes = simutgw::g_mysqlPool.Init();
	if (0 != iRes)
	{
		EzLog::e("Method_system_run()", "g_mysqlPool.Init failed");

		return -1;
	}

	return 0;
}

/*
检查mysql

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step3_2_CheckMysql(void)
{
	static const string ftag("SystemInit::Step3_2_CheckMysql() ");

	int	iRes = CheckMysql();
	if (0 != iRes)
	{
		EzLog::e(ftag, " System Initialization Failed, CheckMysql() Failed");

		// perror
		return -1;
	}

	return 0;
}

/*
由mysql数据库载入配置

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step3_3_LoadConfigFromDb(void)
{
	static const string ftag("SystemInit::Step3_3_LoadConfigFromDb() ");

	int	iRes = simutgw::g_matchRule.ReloadFromDb();
	if (0 != iRes)
	{
		EzLog::e(ftag, " System Initialization Failed, g_matchRule ReloadFromDb() Failed");

		// perror
		return -1;
	}

	iRes = simutgw::g_etfContainer.ReloadFromDb();
	if (0 != iRes)
	{
		EzLog::e(ftag, " System Initialization Failed, g_etfContainer ReloadFromDb() Failed");

		// perror
		return -1;
	}
	
	return 0;
}

/*
从file读id

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step4_1_ReadIdFromFile(void)
{
	static const string ftag("SystemInit::Step4_1_ReadIdFromFile() ");

	try
	{
		boost::property_tree::ptree fileId;
		// 打开读文件
		boost::property_tree::ini_parser::read_ini("config_webid.ini", fileId);

		// web分配的id in local file
		simutgw::g_strWeb_id_local_file = fileId.get<std::string>("web.id");

		if (simutgw::g_strWeb_id_local_file.empty())
		{
			simutgw::g_strWeb_id = "";
		}
		else
		{
			simutgw::g_strWeb_id = simutgw::g_strWeb_id_local_file;
		}
	}
	catch (exception& e)
	{
		string sDebug(ftag);
		sDebug += e.what();
		EzLog::e(ftag, sDebug);
	}
	return 0;
}

/*
从mysql表读id

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step4_2_ReadIdFromMysql(void)
{
	static const string ftag("SystemInit::Step4_2_ReadIdFromMysql() ");

	string strQueryString = "SELECT `userid` FROM `user` WHERE `id`=1";

	int iReturn = 0;

	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		// 查询id及余额

		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(ftag, "Get Connection is NULL");

			return -1;
		}

		int iRes = mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
			if (0 != mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				// web_id in local db
				simutgw::g_strWeb_id_local_db = mapRowData["userid"].strValue;

				if (!simutgw::g_strWeb_id_local_db.empty())
				{
					// 相比文件中id，以db中为准
					simutgw::g_strWeb_id = simutgw::g_strWeb_id_local_db;
				}
			}
			else
			{
				string strDebug("userid not exists in local_db");
				EzLog::i(ftag, strDebug);

				iReturn = -1;
			}

			// 释放
			mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

		}
		else
		{
			string strItoa;
			string strDebug("运行[");
			strDebug += strQueryString;
			strDebug += "]得到";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(ftag, strDebug);

			iReturn = -2;
		}

		//归还连接
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
		return iReturn;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return iReturn;
}

/*
由Web取Id

@param bool bSaveIdToDb :
true -- 向db存ID
flase -- 不向db存ID

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step4_3_GetIdFromWeb(bool bSaveIdToDb)
{
	static const string ftag("SystemInit::Step4_3_GetIdFromWeb() ");

	// 启动客户端
	simutgw::g_SocketClient = std::shared_ptr<Clienter>(new Clienter);

	int iRes = simutgw::g_SocketClient->Connect(simutgw::g_strSocketServerIp,
		simutgw::g_uiSocketServerPort);
	if (-1 == iRes)
	{
		string strError("连接Socket服务器失败，地址=[");
		strError += simutgw::g_strSocketServerIp;
		strError += "]，端口=";
		string strTrans;
		sof_string::itostr(simutgw::g_uiSocketServerPort, strTrans);
		strError += strTrans;

		EzLog::e(ftag, strError);

		return -1;
	}

	if (!simutgw::g_strWeb_id.empty())
	{
		// 有id，无须注册

		// 无file id
		if (simutgw::g_strWeb_id_local_file.empty())
		{
			// 向file保存id
			SaveIdToFile();

			simutgw::g_strWeb_id_local_file = simutgw::g_strWeb_id;
		}

		if (bSaveIdToDb && simutgw::g_strWeb_id_local_db.empty())
		{
			// 向mysql保存id
			SaveIdFromWeb();

			simutgw::g_strWeb_id_local_db = simutgw::g_strWeb_id;
		}

		return 0;
	}

	// 注册
	iRes = simutgw::g_SocketClient->SendMsg_RegisterToServer();
	if (-1 == iRes)
	{
		string strError("向Socket服务器注册失败，地址=[");
		strError += simutgw::g_strSocketServerIp;
		strError += "]，端口=";
		string strTrans;
		sof_string::itostr(simutgw::g_uiSocketServerPort, strTrans);
		strError += strTrans;

		EzLog::e(ftag, strError);

		return -1;
	}

	if (bSaveIdToDb)
	{
		// 注册成功，向mysql保存id
		SaveIdFromWeb();
	}

	// 注册成功，向file保存id
	SaveIdToFile();

	return 0;
}

/*
由Web server取redis、连接等配置

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step4_4_GetConfigFromWebServer(void)
{
	static const string ftag("SystemInit::Step4_4_GetConfigFromWebServer() ");

	// 获取启动参数
	int iRes = simutgw::g_SocketClient->SendMsg_GetParamFromServer();

	// shdbs

	// sz step

	if (0 != iRes)
	{
		return -1;
	}

	return 0;
}

/*
初始化相关配置

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step5_1_InnerValuesInit(void)
{
	static const string ftag("SystemInit::Step5_1_InnerValuesInit() ");

	SzStepOrderFieldName::InitMap();

	//
	ETFHelper::LoadETF("./data/etf_config");

	TimeStringUtil::FeedRandSeed();

	OTLConn_Guard::OTLConn_Init();

	return 0;
}

/*
初始化相关配置

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step5_2_RedisPoolInit(void)
{
	static const string ftag("SystemInit::Step5_2_RedisPoolInit() ");

	//
	// redis
	simutgw::g_redisPool.SetConnection(simutgw::g_strRedis_HostName, simutgw::g_uiRedis_Port, 10,
		simutgw::g_bRedis_requirepass, simutgw::g_strSQL_Password);

	int iRes = Tgw_RedisHelper::LoadHiredisLibrary();
	if (0 != iRes)
	{
		EzLog::e("Method_system_run()", "LoadHiredisLibrary failed");
		perror("LoadHiredisLibrary failed");

		return -1;
	}

	iRes = simutgw::g_redisPool.Init();
	if (0 != iRes)
	{
		EzLog::e("Method_system_run()", "g_redisPool.Init failed");
		perror("g_redisPool.Init failed");

		return -1;
	}

#ifdef _MSC_VER
	Sleep(5);
#else		
	usleep(5 * 1000L);
#endif

	return 0;
}

/*
检查redis

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step6_1_CheckRedis(void)
{
	static const string ftag("SystemInit::Step6_1_CheckRedis() ");

	int iRes = CheckRedis();
	if (0 != iRes)
	{
		EzLog::e(ftag, " System Initialization Failed, CheckRedis() Failed");
#ifdef _MSC_VER
		Sleep(30000);
#else		
		usleep(30000 * 1000L);
#endif
		exit(3);
	}

	return 0;
}

/*
清理外部环境

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step7_OuterClean(void)
{
	static const string ftag("SystemInit::Step7_OuterClean() ");


	return 0;
}

/*
检查通道策略和已收到策略同步情况

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step8_CheckLinkRuleSync(void)
{
	static const string ftag("SystemInit::Step8_CheckLinkRuleSync() ");

	std::vector<uint64_t> vctNeedFetchRule_Sh;
	std::vector<uint64_t> vctNeedFetchRule_Sz;
	simutgw::g_matchRule.CompareConnRuleRelation_ToLocalRule(vctNeedFetchRule_Sh, vctNeedFetchRule_Sz);
		
	int	iRes = simutgw::g_SocketClient->SendMsg_GetMatchRuleContentFromServer(vctNeedFetchRule_Sh, vctNeedFetchRule_Sz);
	if (0 != iRes)
	{
		EzLog::e(ftag, " System Initialization Failed, SendMsg_GetMatchRuleContentFromServer() Failed");

		// perror
		return -1;
	}

	return 0;
}

/*
线程启动

return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Step9_ThreadsStartup(void)
{
	static const string ftag("SystemInit::Step9_ThreadsStartup() ");

	//
	// 异步写数据库thread
	simutgw::g_asyncDbwriter.Start();

	//
	// 内部处理流水线
	// 校验
	simutgw::g_mtskPool_valid.InitPool();
	simutgw::g_mtskPool_valid.StartPool();

	// 交易及撤单
	simutgw::g_mtskPool_match_cancel.InitPool();
	simutgw::g_mtskPool_match_cancel.StartPool();

	// Looper处理线程
	simutgw::g_flowManage.StartFlows();

	if (simutgw::g_bEnableServer)
	{
		// 启用server
		simutgw::g_SocketIOCPServer = std::shared_ptr<SimutgwTcpServer>(
			new SimutgwTcpServer(simutgw::g_strServerIp, simutgw::g_uiServerPort));

		int iRes = simutgw::g_SocketIOCPServer->StartServer();
		if (0 != iRes)
		{
			return -1;
		}
	}

	// sh db 已经用线程处理了

	// fix engine
	if (simutgw::g_bEnable_Sz_Msg_Task)
	{
		if (simutgw::WebManMode::WebMode == simutgw::g_iWebManMode)
		{
			/*
			std::string configuration =
			"[DEFAULT]\n"
			"ConnectionType=initiator\n"
			"BeginString=FIX.4.0\n"
			"Value=4\n"
			"Empty=\n"
			"[SESSION]\n"
			"BeginString=FIX.4.2\n"
			"SenderCompID=ISLD\n"
			"TargetCompID=TW\n"
			"Value=1\n"
			"# this is a comment\n"
			"[SESSION]\n"
			"BeginString=FIX.4.1\n"
			"SenderCompID=ISLD\n"
			"TargetCompID=WT\n"
			"Value=2\n"
			"[SESSION]\n"
			"SenderCompID=ARCA\n"
			"TargetCompID=TW\n"
			"Value=3\n"
			"[SESSION]\n"
			"SenderCompID=ARCA\n"
			"TargetCompID=WT\n"
			"[SESSION]\n"
			"SenderCompID=NYSE\n"
			"TargetCompID=TW\n"
			"SessionQualifier=QUAL1\n"
			"Value=5\n"
			"[SESSION]\n"
			"SenderCompID=NYSE\n"
			"TargetCompID=TW\n"
			"SessionQualifier=QUAL2\n"
			"Value=6\n"
			"[SESSION]\n"
			"BeginString=FIXT.1.1\n"
			"SenderCompID=NYSE\n"
			"TargetCompID=TW\n";
			*/
			if (!simutgw::g_strSzConfig.empty())
			{
				simutgw::g_fixaccptor.StartByStrconfig(simutgw::g_strSzConfig);
			}
		}
		else
		{
			std::string strProgramPath("");

			// 
			FileOperHelper::GetProgramDir(strProgramPath);

			if (simutgw::g_bSZ_Step_ver110)
			{
				// 深圳STEP回报是Ver1.10
				// 回平台信息消息 Platform Info
				strProgramPath += "/config/config_fix.ini";
			}
			else
			{
				strProgramPath += "/config/config_fix_beforev110.ini";
			}
			simutgw::g_fixaccptor.Start(strProgramPath);
		}
	}

	// starts first
#ifdef _MSC_VER
	Sleep(5);
#else		
	usleep(5 * 1000L);
#endif

	return 0;
}

/*
配置初始化
return:
0 -- 成功
其他 -- 失败
*/
int SystemInit::Simutgw_ConfigInit(void)
{
	static const string ftag("SystemInit::Simutgw_ConfigInit() ");

	EzLog::i(ftag, "System Initialization begin");

	int iRes = Step0_IniLog();
	if (0 != iRes)
	{
		string strError("Step0_IniLog失败");

		EzLog::e(ftag, strError);

		return -1;
	}

	// step 1, 检查授权
	iRes = Step1_CheckLicense();
	if (0 != iRes)
	{
		string strError("Step1_CheckLicense失败");
		EzLog::e(ftag, strError);
		return -1;
	}

	// step 2, 读配置文件
	iRes = Step2_ReadConfig();
	if (0 != iRes)
	{
		string strError("Step2_ReadConfig失败");
		EzLog::e(ftag, strError);
		return -1;
	}

	if (simutgw::SysRunMode::NormalMode == simutgw::g_iRunMode)
	{
		// 3 -- 普通模式
		// 启动mysql连接池
		iRes = Step3_1_MysqlPoolInit();
		if (0 != iRes)
		{
			string strError("Step3_1_MysqlPoolInit失败");
			EzLog::e(ftag, strError);
			return -1;
		}

		iRes = Step3_2_CheckMysql();
		if (0 != iRes)
		{
			string strError("Step3_2_CheckMysql失败");
			EzLog::e(ftag, strError);
			return -1;
		}

		iRes = Step3_3_LoadConfigFromDb();
		if (0 != iRes)
		{
			string strError("Step3_3_LoadConfigFromDb失败");
			EzLog::e(ftag, strError);
			return -1;
		}
	}

	switch (simutgw::g_iWebManMode)
	{
	case simutgw::WebManMode::LocalMode:
		// 0 -- 非Web管理模式
		break;

	case simutgw::WebManMode::WebMode:

	default:

		// 从文件中查询id
		iRes = Step4_1_ReadIdFromFile();
		if (0 != iRes)
		{
			string strError("Step4_1_ReadIdFromFile失败");
			EzLog::e(ftag, strError);
			return -1;
		}

		switch (simutgw::g_iRunMode)
		{
		case simutgw::SysRunMode::PressureMode:
			// 1 -- 压力模式;
		case simutgw::SysRunMode::MiniMode:
			// 2 -- 极简模式

			iRes = Step4_3_GetIdFromWeb(true);
			if (0 != iRes)
			{
				string strError("Step4_3_GetIdFromWeb失败");
				EzLog::e(ftag, strError);
				return -1;
			}

			break;

		case simutgw::SysRunMode::NormalMode:
			// 3 -- 普通模式

			// 从表中查询id
			iRes = Step4_2_ReadIdFromMysql();
			if (0 != iRes)
			{
				string strError("Step4_2_ReadIdFromMysql失败");
				EzLog::e(ftag, strError);				
			}

			iRes = Step4_3_GetIdFromWeb(true);
			if (0 != iRes)
			{
				string strError("Step4_3_GetIdFromWeb失败");
				EzLog::e(ftag, strError);
				return -1;
			}
			break;

		default:
			break;
		}

		iRes = Step4_4_GetConfigFromWebServer();
		if (0 != iRes)
		{
			string strError("Step4_4_GetConfigFromWebServer失败");
			EzLog::e(ftag, strError);
			return -1;
		}

		break;
	}

	iRes = Step5_1_InnerValuesInit();
	if (0 != iRes)
	{
		string strError("Step5_1_InnerValuesInit失败");
		EzLog::e(ftag, strError);
		return -1;
	}

	iRes = Step5_2_RedisPoolInit();
	if (0 != iRes)
	{
		string strError("Step5_2_RedisPoolInit失败");
		EzLog::e(ftag, strError);
		return -1;
	}

	switch (simutgw::g_iRunMode)
	{
	case simutgw::SysRunMode::PressureMode:
		// 1 -- 压力模式;
		break;

	case simutgw::SysRunMode::MiniMode:
		// 2 -- 极简模式

	case simutgw::SysRunMode::NormalMode:
		// 3 -- 普通模式

		iRes = Step6_1_CheckRedis();
		if (0 != iRes)
		{
			string strError("Step6_1_CheckRedis失败");
			EzLog::e(ftag, strError);
			return -1;
		}
		break;

	default:
		break;
	}

	iRes = Step7_OuterClean();
	if (0 != iRes)
	{
		string strError("Step7_OuterClean失败");
		EzLog::e(ftag, strError);
		return -1;
	}

	iRes = Step8_CheckLinkRuleSync();
	if (0 != iRes)
	{
		string strError("Step8_CheckShDb失败");
		EzLog::e(ftag, strError);
		return -1;
	}

	iRes = Step9_ThreadsStartup();
	if (0 != iRes)
	{
		string strError("Step9_ThreadsStartup失败");
		EzLog::e(ftag, strError);
		return -1;
	}

	int iBitSize = 8 * sizeof(int*);

	string sItoa;
	sof_string::itostr(iBitSize, sItoa);

	EzLog::i(ftag, " System Initialization Success, " + sItoa + "bit " + simutgw::g_strSystemVersion);

	return 0;
}

