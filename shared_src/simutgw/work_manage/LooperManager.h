#ifndef __LOOPER__MANAGER_H__
#define __LOOPER__MANAGER_H__

/*
��Ҫ��פѭ������Ĺ�����
*/

#include "thread_pool_loopworker/LoopWorker_ThreadPool.h"

class LooperManager
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
	LooperManager(void);
	virtual ~LooperManager(void);

	/*
	��������ģ��������߳�

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int StartFlows(void);


	/*
	�ر�����ģ��������߳�

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int StopFlows(void);

protected:

	/*
	��������߳� ����ί�С��ر���Ϣ����

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int Start_SzConn(void);

	/*
	��������߳� �Ϻ�ί�С��ر���Ϣ����

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int Start_ShConn(void);

	/*
	�����߳�
	�����µ���Ϣ����

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int Start_Distribute_InMsg(void);

	/*
	�����߳�
	������ʱͳ��

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int Start_Traffic_count(void);
};

#endif