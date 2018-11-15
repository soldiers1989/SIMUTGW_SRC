#ifndef __PIPELINE_THREAD_H__
#define __PIPELINE_THREAD_H__

#include "thread_pool_base/ThreadBase.h"
#include "TaskQueue_queue.h"

class PipeLine_Thread :
	public ThreadBase
{
	//
	// Members
	//
protected:
	//
	// �߳���Դ
	//
	TaskQueue_queue m_prtQueTask;

	//
	// Functions
	//
public:
	PipeLine_Thread(void);
	virtual ~PipeLine_Thread(void);

	/*
	�����߳�

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	virtual int StartThread(void);

	/*
	��������������Ƿ�����HashKey��ͬ������������У��Ͳ����������û�У��򲻲���

	Param :
	unsigned int& out_CurrentTaskNum : ��ǰ���߳�������������

	Return :
	0 -- ����ͬ�����񣬲��Ҳ���ɹ�
	1 -- ����ͬ������
	2 -- ����ͬ�����񣬲���ʧ��
	*/
	int AddIfHaveTask(std::shared_ptr<TaskBase>& ptr_task,
		uint64_t& out_CurrentTaskNum);

	/*
	��������

	Return :
	0 -- ����ɹ�
	-1 -- ����ʧ��
	*/
	int AddTask(std::shared_ptr<TaskBase>& ptr_task);


	/*
	��ȡ����

	Return :
	0 -- ����ɹ�
	-1 -- ����ʧ��
	*/
	int GetTask(std::shared_ptr<TaskBase>& ptr_task);

	/*
	��ȡ��ǰ�������

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int GetTaskAssignInfo(std::string& out_strTaskInfo);

protected:	
#ifdef _MSC_VER
	static DWORD WINAPI Run(LPVOID  pParam);
#else
	static void* Run(void* pParam);
#endif
};

#endif