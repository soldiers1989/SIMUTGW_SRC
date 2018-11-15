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
// dll ��Ȩ��֤
/*
int Dll_ValidAuth( const int type, char* out_buffer, const unsigned int uiOutbufferSize )
��ȡ��Ȩ�ļ�����֤��Ȩ

@param int type: ���ͣ��ݱ���
@param char* out_buffer: ������Ϣbuffer
@param int iOutbufferSize: ������Ϣbuffer size

@return:
0 -- δ����
1 -- �ӽ�����ʱ�䣬����ͨ�� @param char* out_buffer ���ؾ�����Ϣ
-1 -- ����
*/
typedef int(*pfDll_ValidAuth)(const int, char*, const unsigned int);

/*
��ȡģʽ�ļ�����֤����ģʽ

@param int type: ���ͣ��ݱ���

@return:
0 -- ��ģʽ��Ȩ�ļ�
-1 -- ��ģʽ��Ȩ�ļ�
*/
typedef int(*pfDll_CheckRunMode)(const int type);


/*
ʹ�ö�̬������Ȩ�Ƿ���

return:
0 -- δ����
-1 -- ����
*/
int SystemInit::ValidateLicense_Dll()
{
	static const string ftag("SystemInit::ValidateLicense_Dll() ");

#ifdef _MSC_VER
	try
	{
		//���ض�̬���ӿ�dll_AuthCheck.dll�ļ�
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

			//ж��dll�ļ���
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

		//ж��dll�ļ���
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
		//���ض�̬���ӿ�dll_AuthCheck.dll�ļ�
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

			//ж��dll�ļ���
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

		//ж��dll�ļ���
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
ʹ�ö�̬��������ģʽ

return:
0 -- ��ģʽ�ļ�
-1 -- ��ģʽ�ļ�
*/
int SystemInit::CheckRunMode_Dll()
{
	static const string ftag("SystemInit::CheckRunMode_Dll() ");

#ifdef _MSC_VER
	try
	{
		//���ض�̬���ӿ�dll_AuthCheck.dll�ļ�
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
		strDebug += sof_string::itostr((uint64_t)dw, strTran);
		EzLog::e(ftag, strDebug);

		return -1;
	}
#else
	try
	{
		//���ض�̬���ӿ�hiredis_wrapper.dll�ļ�
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

			//ж��dll�ļ���
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

		//ж��dll�ļ���
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
	���mysql�Ƿ���������
	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
int SystemInit::CheckMysql()
{
	static const string fTag("SystemInit::CheckMysql()");

	int iReturn = 0;

	//��mysql���ӳ�ȡ����
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (NULL == mysqlConn)
	{
		//ȡ����mysql����ΪNULL

		//�黹����
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

		// �ͷ�
		mysqlConn->FreeResult(&pResultSet);
		pResultSet = NULL;
	}
	else if (iRes < 0)
	{
		string strItoa;
		string strDebug("����[");
		strDebug += strQueryString;
		strDebug += "]�õ�";
		strDebug += sof_string::itostr(iRes, strItoa);
		EzLog::e(fTag, strDebug);

		iReturn = -1;
	}

	simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

	return iReturn;
}

/*
	���redis�Ƿ���������
	return:
	0 -- �ɹ�
	���� -- ʧ��
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
����id���ļ�

return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit::SaveIdToFile(void)
{
	static const string ftag("SystemInit::SaveIdToFile() ");

	try
	{
		boost::property_tree::ptree fileId;

		// д�ֶ�
		fileId.put<std::string>("web.id", simutgw::g_strWeb_id);

		// д���ļ� 
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
����id
return:
0 -- �ɹ�
���� -- ʧ��
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

		// ��ѯid�����

		//��mysql���ӳ�ȡ����
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//ȡ����mysql����ΪNULL

			//�黹����
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
			string strDebug("����[");
			strDebug += strQueryString;
			strDebug += "]�õ�";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(ftag, strDebug);

			iReturn = -2;
		}

		//�黹����
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
��ʼ����־

return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit::Step0_IniLog()
{
	static const string ftag("SystemInit::Step0_IniLog() ");

	EzLog::Init_log_ern("./config/log_simutgw.ini");

	return 0;
}

/*
�������ļ�

return:
0 -- �ɹ�
���� -- ʧ��
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
�������ļ�

return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit::Step2_ReadConfig()
{
	static const string ftag("SystemInit::Step2_ReadConfig() ");

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
				EzLog::i("", "��ǰ����ģʽ--���ع���ģʽ");
			}
			else
			{
				// 1 --Web����ģʽ
				simutgw::g_iWebManMode = simutgw::WebManMode::WebMode;
				EzLog::i("", "��ǰ����ģʽ--Web����ģʽ");
			}
		}
		else
		{
			// 0 -- ��Web����ģʽ
			simutgw::g_iWebManMode = simutgw::WebManMode::WebMode;
			EzLog::i("", "��ǰ����ģʽ--Web����ģʽ");
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
			EzLog::i(strRunModeType, "��ͨģʽ");
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
			EzLog::i("", "��ǰ����ģʽ--������");
		}
		else
		{
			simutgw::g_bHighPfm = false;
			EzLog::i("", "��ǰ����ģʽ--Ĭ��");
		}

		// ����STEP�ر��Ƿ���Ver1.10
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
			EzLog::i("", "��ǰ���ģʽ--���");
		}
		else
		{
			// ����ģʽ��֧��
			simutgw::g_bEnable_Check_Assets = false;
			EzLog::i("", "��ǰ���ģʽ--����֤");
		}

		// ʵ��ģ������ģʽ
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("system.quotation_mode"), iValue);
		if (1 == iValue)
		{
			//  ��һ��һ
			simutgw::g_Quotation_Type = simutgw::SellBuyPrice;
			EzLog::i("", "ʵ��ģ������ģʽ--��һ��һ");
		}
		else if (2 == iValue)
		{
			//  ����ɽ���
			simutgw::g_Quotation_Type = simutgw::RecentMatchPrice;
			EzLog::i("", "ʵ��ģ������ģʽ--����ɽ���");
		}
		else
		{
			// Ĭ��Ϊ����ξ���
			simutgw::g_Quotation_Type = simutgw::AveragePrice;
			EzLog::i("", "ʵ��ģ������ģʽ--����ξ���");
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

		// ��web����ģʽ���Ϻ����ݿ�����
		if (simutgw::WebManMode::LocalMode == simutgw::g_iWebManMode)
		{
			string strName = ("Sh_Conn1");
			string strConn = simutgw::g_ptConfig.get<std::string>("Sh_Conn1.connection");
			// �Ƿ������Ϻ��ӿ�1
			Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("Sh_Conn1.enable"), iValue);
			if (iValue >= 1)
			{
				simutgw::g_mapShConns[strName].SetName(strName);
				simutgw::g_mapShConns[strName].SetConnection(strConn);

				// ����sh���Ӽ�����
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
			// �Ƿ������Ϻ��ӿ�2
			Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("Sh_Conn2.enable"), iValue);
			if (iValue >= 1)
			{
				simutgw::g_mapShConns[strName].SetName(strName);
				simutgw::g_mapShConns[strName].SetConnection(strConn);

				// ����sh���Ӽ�����
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
			// �Ƿ������Ϻ��ӿ�3
			Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("Sh_Conn3.enable"), iValue);
			if (iValue >= 1)
			{
				simutgw::g_mapShConns[strName].SetName(strName);
				simutgw::g_mapShConns[strName].SetConnection(strConn);

				// ����sh���Ӽ�����
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
			// �Ƿ������Ϻ��ӿ�4
			Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("Sh_Conn4.enable"), iValue);
			if (iValue >= 1)
			{
				simutgw::g_mapShConns[strName].SetName(strName);
				simutgw::g_mapShConns[strName].SetConnection(strConn);

				// ����sh���Ӽ�����
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
			// �Ƿ������Ϻ��ӿ�5
			Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("Sh_Conn5.enable"), iValue);
			if (iValue >= 1)
			{
				simutgw::g_mapShConns[strName].SetName(strName);
				simutgw::g_mapShConns[strName].SetConnection(strConn);

				// ����sh���Ӽ�����
				simutgw::g_counter.AddSh_LinkCounter(strName);

				EzLog::i(ftag, "add sh db link name=" + strName);
			}
			else
			{
			}
		}

		//�Ƿ����÷�����
		Tgw_StringUtil::String2Int_atoi(simutgw::g_ptConfig.get<std::string>("server.enable_server"), iValue);
		if (iValue >= 1)
		{
			simutgw::g_bEnableServer = true;
		}
		else
		{
		}

		// ������listen��ַ
		simutgw::g_strServerIp = simutgw::g_ptConfig.get<std::string>("server.simutgw_ip");
		// ������listen�˿�
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
����mysql���ӳ�

return:
0 -- �ɹ�
���� -- ʧ��
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
���mysql

return:
0 -- �ɹ�
���� -- ʧ��
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
��mysql���ݿ���������

return:
0 -- �ɹ�
���� -- ʧ��
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
��file��id

return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit::Step4_1_ReadIdFromFile(void)
{
	static const string ftag("SystemInit::Step4_1_ReadIdFromFile() ");

	try
	{
		boost::property_tree::ptree fileId;
		// �򿪶��ļ�
		boost::property_tree::ini_parser::read_ini("config_webid.ini", fileId);

		// web�����id in local file
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
��mysql���id

return:
0 -- �ɹ�
���� -- ʧ��
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

		// ��ѯid�����

		//��mysql���ӳ�ȡ����
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//ȡ����mysql����ΪNULL

			//�黹����
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
					// ����ļ���id����db��Ϊ׼
					simutgw::g_strWeb_id = simutgw::g_strWeb_id_local_db;
				}
			}
			else
			{
				string strDebug("userid not exists in local_db");
				EzLog::i(ftag, strDebug);

				iReturn = -1;
			}

			// �ͷ�
			mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

		}
		else
		{
			string strItoa;
			string strDebug("����[");
			strDebug += strQueryString;
			strDebug += "]�õ�";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(ftag, strDebug);

			iReturn = -2;
		}

		//�黹����
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
��WebȡId

@param bool bSaveIdToDb :
true -- ��db��ID
flase -- ����db��ID

return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit::Step4_3_GetIdFromWeb(bool bSaveIdToDb)
{
	static const string ftag("SystemInit::Step4_3_GetIdFromWeb() ");

	// �����ͻ���
	simutgw::g_SocketClient = std::shared_ptr<Clienter>(new Clienter);

	int iRes = simutgw::g_SocketClient->Connect(simutgw::g_strSocketServerIp,
		simutgw::g_uiSocketServerPort);
	if (-1 == iRes)
	{
		string strError("����Socket������ʧ�ܣ���ַ=[");
		strError += simutgw::g_strSocketServerIp;
		strError += "]���˿�=";
		string strTrans;
		sof_string::itostr(simutgw::g_uiSocketServerPort, strTrans);
		strError += strTrans;

		EzLog::e(ftag, strError);

		return -1;
	}

	if (!simutgw::g_strWeb_id.empty())
	{
		// ��id������ע��

		// ��file id
		if (simutgw::g_strWeb_id_local_file.empty())
		{
			// ��file����id
			SaveIdToFile();

			simutgw::g_strWeb_id_local_file = simutgw::g_strWeb_id;
		}

		if (bSaveIdToDb && simutgw::g_strWeb_id_local_db.empty())
		{
			// ��mysql����id
			SaveIdFromWeb();

			simutgw::g_strWeb_id_local_db = simutgw::g_strWeb_id;
		}

		return 0;
	}

	// ע��
	iRes = simutgw::g_SocketClient->SendMsg_RegisterToServer();
	if (-1 == iRes)
	{
		string strError("��Socket������ע��ʧ�ܣ���ַ=[");
		strError += simutgw::g_strSocketServerIp;
		strError += "]���˿�=";
		string strTrans;
		sof_string::itostr(simutgw::g_uiSocketServerPort, strTrans);
		strError += strTrans;

		EzLog::e(ftag, strError);

		return -1;
	}

	if (bSaveIdToDb)
	{
		// ע��ɹ�����mysql����id
		SaveIdFromWeb();
	}

	// ע��ɹ�����file����id
	SaveIdToFile();

	return 0;
}

/*
��Web serverȡredis�����ӵ�����

return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit::Step4_4_GetConfigFromWebServer(void)
{
	static const string ftag("SystemInit::Step4_4_GetConfigFromWebServer() ");

	// ��ȡ��������
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
��ʼ���������

return:
0 -- �ɹ�
���� -- ʧ��
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
��ʼ���������

return:
0 -- �ɹ�
���� -- ʧ��
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
���redis

return:
0 -- �ɹ�
���� -- ʧ��
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
�����ⲿ����

return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit::Step7_OuterClean(void)
{
	static const string ftag("SystemInit::Step7_OuterClean() ");


	return 0;
}

/*
���ͨ�����Ժ����յ�����ͬ�����

return:
0 -- �ɹ�
���� -- ʧ��
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
�߳�����

return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit::Step9_ThreadsStartup(void)
{
	static const string ftag("SystemInit::Step9_ThreadsStartup() ");

	//
	// �첽д���ݿ�thread
	simutgw::g_asyncDbwriter.Start();

	//
	// �ڲ�������ˮ��
	// У��
	simutgw::g_mtskPool_valid.InitPool();
	simutgw::g_mtskPool_valid.StartPool();

	// ���׼�����
	simutgw::g_mtskPool_match_cancel.InitPool();
	simutgw::g_mtskPool_match_cancel.StartPool();

	// Looper�����߳�
	simutgw::g_flowManage.StartFlows();

	if (simutgw::g_bEnableServer)
	{
		// ����server
		simutgw::g_SocketIOCPServer = std::shared_ptr<SimutgwTcpServer>(
			new SimutgwTcpServer(simutgw::g_strServerIp, simutgw::g_uiServerPort));

		int iRes = simutgw::g_SocketIOCPServer->StartServer();
		if (0 != iRes)
		{
			return -1;
		}
	}

	// sh db �Ѿ����̴߳�����

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
				// ����STEP�ر���Ver1.10
				// ��ƽ̨��Ϣ��Ϣ Platform Info
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
���ó�ʼ��
return:
0 -- �ɹ�
���� -- ʧ��
*/
int SystemInit::Simutgw_ConfigInit(void)
{
	static const string ftag("SystemInit::Simutgw_ConfigInit() ");

	EzLog::i(ftag, "System Initialization begin");

	int iRes = Step0_IniLog();
	if (0 != iRes)
	{
		string strError("Step0_IniLogʧ��");

		EzLog::e(ftag, strError);

		return -1;
	}

	// step 1, �����Ȩ
	iRes = Step1_CheckLicense();
	if (0 != iRes)
	{
		string strError("Step1_CheckLicenseʧ��");
		EzLog::e(ftag, strError);
		return -1;
	}

	// step 2, �������ļ�
	iRes = Step2_ReadConfig();
	if (0 != iRes)
	{
		string strError("Step2_ReadConfigʧ��");
		EzLog::e(ftag, strError);
		return -1;
	}

	if (simutgw::SysRunMode::NormalMode == simutgw::g_iRunMode)
	{
		// 3 -- ��ͨģʽ
		// ����mysql���ӳ�
		iRes = Step3_1_MysqlPoolInit();
		if (0 != iRes)
		{
			string strError("Step3_1_MysqlPoolInitʧ��");
			EzLog::e(ftag, strError);
			return -1;
		}

		iRes = Step3_2_CheckMysql();
		if (0 != iRes)
		{
			string strError("Step3_2_CheckMysqlʧ��");
			EzLog::e(ftag, strError);
			return -1;
		}

		iRes = Step3_3_LoadConfigFromDb();
		if (0 != iRes)
		{
			string strError("Step3_3_LoadConfigFromDbʧ��");
			EzLog::e(ftag, strError);
			return -1;
		}
	}

	switch (simutgw::g_iWebManMode)
	{
	case simutgw::WebManMode::LocalMode:
		// 0 -- ��Web����ģʽ
		break;

	case simutgw::WebManMode::WebMode:

	default:

		// ���ļ��в�ѯid
		iRes = Step4_1_ReadIdFromFile();
		if (0 != iRes)
		{
			string strError("Step4_1_ReadIdFromFileʧ��");
			EzLog::e(ftag, strError);
			return -1;
		}

		switch (simutgw::g_iRunMode)
		{
		case simutgw::SysRunMode::PressureMode:
			// 1 -- ѹ��ģʽ;
		case simutgw::SysRunMode::MiniMode:
			// 2 -- ����ģʽ

			iRes = Step4_3_GetIdFromWeb(true);
			if (0 != iRes)
			{
				string strError("Step4_3_GetIdFromWebʧ��");
				EzLog::e(ftag, strError);
				return -1;
			}

			break;

		case simutgw::SysRunMode::NormalMode:
			// 3 -- ��ͨģʽ

			// �ӱ��в�ѯid
			iRes = Step4_2_ReadIdFromMysql();
			if (0 != iRes)
			{
				string strError("Step4_2_ReadIdFromMysqlʧ��");
				EzLog::e(ftag, strError);				
			}

			iRes = Step4_3_GetIdFromWeb(true);
			if (0 != iRes)
			{
				string strError("Step4_3_GetIdFromWebʧ��");
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
			string strError("Step4_4_GetConfigFromWebServerʧ��");
			EzLog::e(ftag, strError);
			return -1;
		}

		break;
	}

	iRes = Step5_1_InnerValuesInit();
	if (0 != iRes)
	{
		string strError("Step5_1_InnerValuesInitʧ��");
		EzLog::e(ftag, strError);
		return -1;
	}

	iRes = Step5_2_RedisPoolInit();
	if (0 != iRes)
	{
		string strError("Step5_2_RedisPoolInitʧ��");
		EzLog::e(ftag, strError);
		return -1;
	}

	switch (simutgw::g_iRunMode)
	{
	case simutgw::SysRunMode::PressureMode:
		// 1 -- ѹ��ģʽ;
		break;

	case simutgw::SysRunMode::MiniMode:
		// 2 -- ����ģʽ

	case simutgw::SysRunMode::NormalMode:
		// 3 -- ��ͨģʽ

		iRes = Step6_1_CheckRedis();
		if (0 != iRes)
		{
			string strError("Step6_1_CheckRedisʧ��");
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
		string strError("Step7_OuterCleanʧ��");
		EzLog::e(ftag, strError);
		return -1;
	}

	iRes = Step8_CheckLinkRuleSync();
	if (0 != iRes)
	{
		string strError("Step8_CheckShDbʧ��");
		EzLog::e(ftag, strError);
		return -1;
	}

	iRes = Step9_ThreadsStartup();
	if (0 != iRes)
	{
		string strError("Step9_ThreadsStartupʧ��");
		EzLog::e(ftag, strError);
		return -1;
	}

	int iBitSize = 8 * sizeof(int*);

	string sItoa;
	sof_string::itostr(iBitSize, sItoa);

	EzLog::i(ftag, " System Initialization Success, " + sItoa + "bit " + simutgw::g_strSystemVersion);

	return 0;
}

