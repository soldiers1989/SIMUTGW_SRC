#ifndef __SYSTEM_INIT_CHILD_QUOTATION_H__
#define __SYSTEM_INIT_CHILD_QUOTATION_H__

#ifdef _MSC_VER
#include <Windows.h>
#else

#endif

#include <vector>

#include "order/define_order_msg.h"

/*
ϵͳ��ʼ������
��Ҫ�Ǽ��mysql�Ƿ��������ϣ�
���redis�Ƿ���������,
ɾ��redis��ί�кͻر�
��mysql��δ�����ί�е��뵽redis��
*/
class SystemInit_simutgw_quotation
{
	//
	// Members
	//

	//
	// Functions
	//
private:
	// ��ֹʹ��
	SystemInit_simutgw_quotation(void)
	{
	}

	// ��ֹʹ��
	virtual ~SystemInit_simutgw_quotation(void)
	{
	}

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
	static int Step2_ReadConfig(void);

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
		���redis�Ƿ���������
		return:
		0 -- �ɹ�
		���� -- ʧ��
		*/
	static int CheckRedis();

public:
	/*
	�ӽ������鴦�� ���ó�ʼ��
	return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int ChildP_Quotation_ConfigInit(void);

	/*
	�ؼ������Ҫ�������
	*/
	static void SelfExit(void);

};

#endif