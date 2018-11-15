#ifndef __LOOPER_MANAGER_CHILD_QUOTATION_H__
#define __LOOPER_MANAGER_CHILD_QUOTATION_H__

/*
��Ҫ��פѭ������Ĺ�����

���ӽ��� ���鴦��ר��
*/

#include "thread_pool_loopworker/LoopWorker_ThreadPool.h"

class LooperManager_child_quotation
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	LoopWorker_ThreadPool m_looperPool;

	//
	// Functions
	//
public:
	LooperManager_child_quotation(void);
	virtual ~LooperManager_child_quotation(void);

	/*
	��������ģ��������߳�

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int StartFlows( void );


	/*
	�ر�����ģ��������߳�

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int StopFlows( void );

protected:
	/*
	��������߳� ���鴦��

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int Start_MarketInfo( void );
};

#endif