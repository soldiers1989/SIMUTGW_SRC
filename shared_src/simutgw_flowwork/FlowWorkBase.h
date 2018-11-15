#ifndef __FLOW_WORK_BASE_H__
#define __FLOW_WORK_BASE_H__

#include "thread_pool_base/TaskBase.h"

class FlowWorkBase
	: public TaskBase
{
protected:

	/* 
	�����flow����
	1 -- ������
	2 -- ������
	3 -- �Ϻ���
	4 -- �Ϻ���

	5 -- ȡ����ί�У����ر�
	6 -- ȡ�Ϻ�ί�У����ر�

	7 -- ������redisί��
	8 -- ���Ϻ�redisί��

	*/
	int m_iMatchType;

public:
	FlowWorkBase(void);
	virtual ~FlowWorkBase(void);

	virtual int TaskProc(void) = 0;

	/*
	���ô����flow����
	Param:
	iType 1 -- ������
	2 -- ������
	3 -- �Ϻ���
	4 -- �Ϻ���

	5 -- ȡ����ί�У����ر�
	6 -- ȡ�Ϻ�ί�У����ر�

	7 -- ������redisί��
	8 -- ���Ϻ�redisί��

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int SetFlowMatchType(const int iType);

	/*
	��ȡ�����flow����
	
	Return :
	*/
	int GetFlowMatchType(void) const
	{
		return m_iMatchType;
	}
	
};

#endif