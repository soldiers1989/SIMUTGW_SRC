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
��ȡģʽ�ļ�����֤����ģʽ

@param int type: ���ͣ��ݱ���

@return:
0 -- ��ģʽ��Ȩ�ļ�
-1 -- ��ģʽ��Ȩ�ļ�
*/
typedef int(*pfDll_CheckRunMode)(const int type);

/*
ʹ�ö�̬��������ģʽ

return:
0 -- ��ģʽ�ļ�
-1 -- ��ģʽ�ļ�
*/
int SystemInit_simutgw_quotation::CheckRunMode_Dll()
{
	static const string ftag("SystemInit_simutgw_quotation::CheckRunMode_Dll() ");

	try
	{
		//���ض�̬���ӿ�hiredis_wrapper.dll�ļ�
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

			//ж��dll�ļ���
			FreeLibrary(hAuthDLL);
			return -1;
		}

		int iRes = F_Dll_CheckRunMode(0);
		if (0 == iRes)
		{
			EzLog::i(ftag, "Mode file good");
		}

		//ж��dll�ļ���
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
	���redis�Ƿ���������
	return:
	0 -- �ɹ�
	���� -- ʧ��
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
��ʼ����־

return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit_simutgw_quotation::Step0_IniLog()
{
	static const string ftag("SystemInit_simutgw_quotation::Step0_IniLog() ");

	EzLog::Init_log_ern("./config/log_quot.ini");

	return 0;
}

/*
�������ļ�

return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit_simutgw_quotation::Step2_ReadConfig()
{
	static const string ftag("SystemInit_simutgw_quotation::Step2_ReadConfig() ");

	try
	{
		// �򿪶��ļ�
		boost::property_tree::ini_parser::read_ini("config_simutgw.ini", simutgw::g_ptConfig);


		// Web����ģʽ
		if (0 == simutgw::g_ptConfig.get<std::string>("system.web_man_mode").compare("0"))
		{
			int iRunMode = CheckRunMode_Dll();
			if (0 == iRunMode)
			{
				// 	0 -- ��Web����ģʽ		
				simutgw::g_iWebManMode = simutgw::WebManMode::LocalMode;
			}
			else
			{
				// 1 --Web����ģʽ
				simutgw::g_iWebManMode = simutgw::WebManMode::WebMode;
			}
		}
		else
		{
			// 0 -- ��Web����ģʽ
			simutgw::g_iWebManMode = simutgw::WebManMode::WebMode;
		}

		int iValue = 0;
		//
		// ����ģʽ
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.run_mode"), iValue);
		string strRunModeType("��ǰ����ģʽ--");
		if (1 == iValue)
		{
			// 1 -- ѹ��ģʽ;
			simutgw::g_iRunMode = simutgw::SysRunMode::PressureMode;
			EzLog::i(strRunModeType, "ѹ��ģʽ");
		}
		else if (2 == iValue)
		{
			// 2 -- ����ģʽ
			simutgw::g_iRunMode = simutgw::SysRunMode::MiniMode;
			EzLog::i(strRunModeType, "����ģʽ");
		}
		else if (3 == iValue)
		{
			// 3 -- ��ͨģʽ
			simutgw::g_iRunMode = simutgw::SysRunMode::NormalMode;
			EzLog::i(strRunModeType, "��ͨģʽ");
		}
		else
		{
			// 3 -- ��ͨģʽ
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
			// Ĭ��ģʽ
			simutgw::g_bRedis_requirepass = false;
		}
		else
		{
			simutgw::g_bRedis_requirepass = true;
		}

		//
		// web���������

		// web�����ip
		simutgw::g_strSocketServerIp = simutgw::g_ptConfig.get<std::string>("web_manage.web_man_ip");
		// web�����port
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

		// �Ƿ��Ǹ�����
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.high_pfm"), iValue);
		if (iValue >= 1)
		{
			// ������
			simutgw::g_bHighPfm = true;
		}
		else
		{
			simutgw::g_bHighPfm = false;
		}

		//
		// �ɽ�ģʽ
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.match_mode"), iValue);
		string strMatchModeType("��ǰ�ɽ�ģʽ--");
		if (0 == iValue)
		{
			// Ĭ��ģʽ
			simutgw::g_iMatchMode = simutgw::SysMatchMode::EnAbleQuta;
			EzLog::i(strMatchModeType, "ʵ��ģ��");
		}
		else if (1 == iValue)
		{
			// ȫ���ɽ�
			simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchAll;
			EzLog::i(strMatchModeType, "ģ�����");
		}
		else if (2 == iValue)
		{
			// �ֱʳɽ�
			simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchByDivide;
			EzLog::i(strMatchModeType, "ģ�����");
		}
		else if (3 == iValue)
		{
			// ���ɽ����ҵ������Գ���
			simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulNotMatch;
			EzLog::i(strMatchModeType, "ģ�����");
		}
		else if (4 == iValue)
		{
			// ����
			simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulErrMatch;
			EzLog::i(strMatchModeType, "ģ�����");
		}
		else if (5 == iValue)
		{
			// ����
			simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchPart;
			EzLog::i(strMatchModeType, "ģ�����");
		}
		else
		{
			string strError("�ɽ�ģʽδ֪matchmode=[");
			strError += simutgw::g_ptConfig.get<std::string>("system.match_mode");
			strError += "]";

			EzLog::e("SimutgwConfigInit() ", strError);
		}

		// �Ƿ���������
		simutgw::g_bEnable_Quotation_Task = true;

		// �Ƿ�����������Ϣ����
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.sz_link_enable"), iValue);
		if (iValue >= 1)
		{
			simutgw::g_bEnable_Sz_Msg_Task = true;
		}
		else
		{
			simutgw::g_bEnable_Sz_Msg_Task = false;
		}

		// �Ƿ������Ϻ���Ϣ����
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.sh_link_enable"), iValue);
		if (iValue >= 1)
		{
			simutgw::g_bEnable_Sh_Msg_Task = true;
		}
		else
		{
			simutgw::g_bEnable_Sh_Msg_Task = false;
		}

		// �Ƿ������������
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.enable_check_assets"), iValue);
		if (iValue >= 1 && simutgw::g_iRunMode == simutgw::SysRunMode::NormalMode)
		{
			// ��ͨģʽ��֧�����
			simutgw::g_bEnable_Check_Assets = true;
		}
		else
		{
			// ����ģʽ��֧��
			simutgw::g_bEnable_Check_Assets = false;
		}

		// ʵ��ģ������ģʽ
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.quotation_mode"), iValue);
		if (1 == iValue)
		{
			//  ��һ��һ
			simutgw::g_Quotation_Type = simutgw::SellBuyPrice;
		}
		else if (2 == iValue)
		{
			//  ����ɽ���
			simutgw::g_Quotation_Type = simutgw::RecentMatchPrice;
		}
		else
		{
			// Ĭ��Ϊ����ξ���
			simutgw::g_Quotation_Type = simutgw::AveragePrice;
		}

		// �ֱʳɽ�����
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.part_match_num"), iValue);
		if (iValue >= 2)
		{
			simutgw::g_ui32_Part_Match_Num = iValue;
		}
		else
		{
			// �ֱ�����Ϊ2��
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
��ʼ���������

return:
0 -- �ɹ�
���� -- ʧ��
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
���redis

return:
0 -- �ɹ�
���� -- ʧ��
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
�ӽ������鴦�� ���ó�ʼ��
return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit_simutgw_quotation::ChildP_Quotation_ConfigInit(void)
{
	static const string ftag("SystemInit_simutgw_quotation::ChildP_Quotation_ConfigInit() ");

	EzLog::i(ftag, "System Initialization begin");

	Step0_IniLog();

	// step 2, �������ļ�
	Step2_ReadConfig();

	Step5_2_RedisPoolInit();

	switch (simutgw::g_iRunMode)
	{
	case simutgw::SysRunMode::PressureMode:
		// 1 -- ѹ��ģʽ;
		break;

	case simutgw::SysRunMode::MiniMode:
		// 2 -- ����ģʽ

	case simutgw::SysRunMode::NormalMode:
		// 3 -- ��ͨģʽ

		Step6_1_CheckRedis();
		break;

	default:
		break;
	}

	// Looper�����߳�
	simutgw::g_flowManage_quota.StartFlows();

	int iBitSize = 8 * sizeof(int*);

	string sItoa;
	sof_string::itostr(iBitSize, sItoa);

	EzLog::i(ftag, " System Initialization Success, " + sItoa + "bit " + simutgw::g_strSystemVersion);

	return 0;
}

/*
�ؼ������Ҫ�������
*/
void SystemInit_simutgw_quotation::SelfExit(void)
{
	static const string strTag("SystemInit_simutgw_quotation::SelfExit() ");

	simutgw::g_flowManage_quota.StopFlows();
	EzLog::i(strTag, "g_flowManage StopFlows");

	simutgw::g_redisPool.Stop();

	//�ͷ���Դ

	Tgw_RedisHelper::FreeHiredisLibrary();
}