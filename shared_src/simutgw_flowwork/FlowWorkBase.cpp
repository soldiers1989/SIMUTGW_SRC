#include "FlowWorkBase.h"

#include <string>

FlowWorkBase::FlowWorkBase(void)
: TaskBase(0), m_iMatchType(0)
{
}

FlowWorkBase::~FlowWorkBase(void)
{
}

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
int FlowWorkBase::SetFlowMatchType(const int iType)
{
	// static const std::string fTag("FlowWorkBase::SetFlowMatchType");

	m_iMatchType = iType;

	return 0;
}
