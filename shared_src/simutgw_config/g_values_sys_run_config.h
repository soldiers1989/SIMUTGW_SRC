#ifndef __VALUES_SYS_RUN_CONFIG_H__
#define __VALUES_SYS_RUN_CONFIG_H__

#include <string>
#include <map>
#include <stdint.h>

#include "boost/property_tree/ptree.hpp"    
#include "boost/property_tree/ini_parser.hpp"    

#include "config_define.h"

namespace simutgw
{
	// ������Ϣ
	extern boost::property_tree::ptree g_ptConfig;

	// ���ô��������߳�
	extern bool g_bEnable_Quotation_Task;

	// ��������ί�С��ر���Ϣ�����߳�
	extern bool g_bEnable_Sz_Msg_Task;

	// �����Ϻ�ί�С��ر���Ϣ�����߳�
	extern bool g_bEnable_Sh_Msg_Task;

	// ��Ȩ������
	extern std::string dll_AuthCheckName;

	/*
	���ʱ������ģʽ
	true -- ������
	false -- �Ǹ�����

	*/
	extern volatile bool g_bHighPfm;

	/*
	Web����ģʽ
	0 -- ��Web����ģʽ
	1 -- Web����ģʽ
	*/
	extern volatile enum WebManMode g_iWebManMode;

	/*
	����ģʽ
	1 -- ѹ��ģʽ
	2 -- ����ģʽ
	3 -- ��ͨģʽ
	*/
	extern volatile enum SysRunMode g_iRunMode;

	// ����ģʽ�Ƿ���json���ûر�
	// true -- ��
	// false -- ��
	extern bool g_bEnable_Json_Mocker;

	// �Ƿ����ʽ�ɷ�
	// true -- ���
	// false -- �����
	extern bool g_bEnable_Check_Assets;

	// ����STEP�ر��Ƿ���Ver1.10
	// true -- ��Ver1.10
	// false -- ����Ver1.10
	extern bool g_bSZ_Step_ver110;

	/*
	�ɽ�ģʽ
	0 -- Ĭ��
	1 -- �������飬ȫ���ɽ�
	2 -- �������飬�ֱʳɽ�
	3 -- �������飬�ҵ������ɽ����ɳ���
	4 -- ����
	*/
	extern volatile enum SysMatchMode g_iMatchMode;

	// ʵ��ģ������ģʽ
	extern enum QuotationType g_Quotation_Type;

	// �ֱʳɽ�����
	extern uint32_t g_ui32_Part_Match_Num;

	// ������������
	extern std::string g_strSzConfig;

	// ���ݿ�����
	extern std::string g_strSQL_HostName;
	extern std::string g_strSQL_UserName;
	extern std::string g_strSQL_Password;
	extern std::string g_strSQL_CataLog;
	extern unsigned int g_uiSQL_Port;

	//
	// Redis����
	extern std::string g_strRedis_HostName;
	extern std::string g_strRedis_Password;
	extern unsigned int g_uiRedis_Port;
	extern bool g_bRedis_requirepass;

	//�Ƿ����÷�����
	extern volatile bool g_bEnableServer;
	// ������listen��ַ
	extern std::string g_strServerIp;
	// ������listen�˿�
	extern unsigned int g_uiServerPort;

	// web���������
	// web�����id
	extern std::string g_strWeb_id;

	// web�����id in local file
	extern std::string g_strWeb_id_local_file;

	// web�����id in local db
	extern std::string g_strWeb_id_local_db;

	// web����� socket��������ַ
	extern std::string g_strSocketServerIp;
	// web����� socket�������˿�
	extern unsigned int g_uiSocketServerPort;
}

#endif