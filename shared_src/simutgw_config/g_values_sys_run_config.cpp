#include "g_values_sys_run_config.h"

namespace simutgw
{
	// ������Ϣ
	boost::property_tree::ptree g_ptConfig;

	// ���ô��������߳�
	bool g_bEnable_Quotation_Task(false);

	// ��������ί�С��ر���Ϣ������redis��ί���߳�
	bool g_bEnable_Sz_Msg_Task(false);

	// �����Ϻ�ί�С��ر���Ϣ������redis��ί���߳�
	bool g_bEnable_Sh_Msg_Task(false);

	// ��Ȩ������
	std::string dll_AuthCheckName("dll_authcheck");

	volatile bool g_bHighPfm = true;

	/*
	Web����ģʽ
	0 -- ��Web����ģʽ
	1 -- Web����ģʽ
	*/
	volatile WebManMode g_iWebManMode = WebManMode::LocalMode;

	/*
	����ģʽ
	1 -- ѹ��ģʽ
	2 -- ����ģʽ
	3 -- ��ͨģʽ
	*/
	volatile SysRunMode g_iRunMode = SysRunMode::NormalMode;

	// ����ģʽ�Ƿ���json���ûر�
	// true -- ��
	// false -- ��
	bool g_bEnable_Json_Mocker(false);

	// �Ƿ����ʽ�ɷ�
	// true -- ���
	// false -- �����
	bool g_bEnable_Check_Assets(false);

	// ����STEP�ر��Ƿ���Ver1.10
	// true -- ��Ver1.10
	// false -- ����Ver1.10
	bool g_bSZ_Step_ver110(false);

	/*
	�ɽ�ģʽ
	0 -- Ĭ��
	1 -- �������飬ȫ���ɽ�
	2 -- �������飬�ֱʳɽ�
	3 -- �������飬�ҵ������ɽ����ɳ���
	4 -- ����
	*/
	volatile SysMatchMode g_iMatchMode = SysMatchMode::SimulMatchAll;

	// ʵ��ģ������ģʽ
	QuotationType g_Quotation_Type = QuotationType::AveragePrice;

	// �ֱʳɽ�����
	uint32_t g_ui32_Part_Match_Num = 2;

	// ������������
	std::string g_strSzConfig;

	// ���ݿ�����
	std::string g_strSQL_HostName("127.0.0.1");
	std::string g_strSQL_UserName("admin");
	std::string g_strSQL_Password("simutgw");
	unsigned int g_uiSQL_Port = 3306;

	std::string g_strSQL_CataLog("simutgw");

	// Redis����
	std::string g_strRedis_HostName("127.0.0.1");
	std::string g_strRedis_Password("");
	unsigned int g_uiRedis_Port = 6379;
	bool g_bRedis_requirepass = false;

	//�Ƿ����÷�����
	volatile bool g_bEnableServer = false;
	// ������listen��ַ
	std::string g_strServerIp = "127.0.0.1";
	// ������listen�˿�
	unsigned int g_uiServerPort = 50000;

	// web���������
	// web�����id
	std::string g_strWeb_id("");

	// web�����id in local file
	std::string g_strWeb_id_local_file("");

	// web�����id in local db
	std::string g_strWeb_id_local_db("");

	// web����� socket��������ַ
	std::string g_strSocketServerIp = "192.168.60.144";
	// web����� socket�������˿�
	unsigned int g_uiSocketServerPort = 10005;
}