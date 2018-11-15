#ifndef __SYSTEM_INIT_H__
#define __SYSTEM_INIT_H__

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include <vector>
#include <memory>

#include "order/define_order_msg.h"

/*
ϵͳ��ʼ������
��Ҫ�Ǽ��mysql�Ƿ��������ϣ�
���redis�Ƿ���������,
ɾ��redis��ί�кͻر�
��mysql��δ�����ί�е��뵽redis��
*/
class SystemInit
{
	//
	// Members
	//

	//
	// Functions
	//
private:
	// ��ֹʹ��
	SystemInit(void)
	{
	}

	// ��ֹʹ��
	virtual ~SystemInit(void)
	{
	}

	/*
	ʹ�ö�̬������Ȩ�Ƿ���

	return:
	0 -- δ����
	-1 -- ����
	*/
	static int ValidateLicense_Dll();

	/*
	ʹ�ö�̬��������ģʽ

	return:
	0 -- ��ģʽ�ļ�
	-1 -- ��ģʽ�ļ�
	*/
	static int CheckRunMode_Dll();

	/*
	��ʼ����־

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step0_IniLog();

	/*
	�������ļ�

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step1_CheckLicense();

	/*
	�������ļ�

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step2_ReadConfig(void);

	/*
	����mysql���ӳ�

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step3_1_MysqlPoolInit(void);

	/*
	���mysql

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step3_2_CheckMysql(void);

	/*
	��mysql���ݿ���������

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step3_3_LoadConfigFromDb(void);

	/*
	��file��id

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step4_1_ReadIdFromFile(void);


	/*
	��mysql���id

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step4_2_ReadIdFromMysql(void);

	/*
	��WebȡId

	@param bool bSaveIdToDb :
	true -- ��db��ID
	flase -- ����db��ID

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step4_3_GetIdFromWeb(bool bSaveIdToDb);

	/*
	��Web serverȡredis�����ӵ�����

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step4_4_GetConfigFromWebServer(void);

	/*
	��ʼ���������

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step5_1_InnerValuesInit(void);

	/*
	��ʼ���������

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step5_2_RedisPoolInit(void);

	/*
	���redis

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step6_1_CheckRedis(void);

	/*
	�����ⲿ����

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step7_OuterClean(void);

	/*
	�߳�����

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step9_ThreadsStartup(void);

	/*
		���mysql�Ƿ���������
		return:
		0 -- �ɹ�
		���� -- ʧ��
		*/
	static int CheckMysql();

	/*
		���redis�Ƿ���������
		return:
		0 -- �ɹ�
		���� -- ʧ��
		*/
	static int CheckRedis();

	/*
	����id���ļ�

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int SaveIdToFile(void);

	/*
	����id
	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int SaveIdFromWeb();

public:
	/*
	���ó�ʼ��
	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Simutgw_ConfigInit(void);

	/*
	���ͨ�����Ժ����յ�����ͬ�����

	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Step8_CheckLinkRuleSync(void);
};

#endif