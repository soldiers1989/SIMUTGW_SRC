#ifndef __TASK_QUEUE_BASE_H__
#define __TASK_QUEUE_BASE_H__

#include <map>
#include <stdint.h>

#include "boost/thread/mutex.hpp"

#include "thread_pool_base/TaskBase.h"

/*

*/
class TaskQueueBase
{
	//
	// Members
	//
public:
	// ���������������������
	static const uint64_t MAX_QUEUE_SIZE;

protected:

	// id
	uint64_t m_uiTaskQueId;

	// ������
	boost::mutex m_mutexlock;

	// �������������׷��
	// Key: hash���stringid
	// value: ��key��task�е�����
	std::map<uint64_t, long> m_mapTaskNumTrack;

	// ��ǰ������������
	uint64_t m_uiTaskNums;

	//
	// Functions
	//
public:
	TaskQueueBase(void);
	explicit TaskQueueBase(const uint64_t uiId);
	virtual ~TaskQueueBase(void);

	void SetQueueId(const uint64_t uiId)
	{
		m_uiTaskQueId = uiId;
	}

	const uint64_t GetQueueId(void) const
	{
		return m_uiTaskQueId;
	}

	/*
	��������

	Param :
	uint64_t& out_CurrentTaskNum : ��ǰ���߳�������������

	Return :
	0 -- ����ɹ�
	2 -- �������������������
	-1 -- ����ʧ��
	*/
	int AddTask(std::shared_ptr<TaskBase>& in_ptrTask, uint64_t& out_CurrentTaskNum);

	/*
	��������������Ƿ�����HashKey��ͬ������������У��Ͳ����������û�У��򲻲���

	Param :
	uint64_t& out_CurrentTaskNum : ��ǰ���߳�������������

	Return :
	0 -- ����ͬ�����񣬲��Ҳ���ɹ�
	1 -- ����ͬ������
	2 -- ����ͬ�����񣬲���ʧ��
	*/
	int AddIfHaveTask(std::shared_ptr<TaskBase>& in_ptrTask,
		uint64_t& out_CurrentTaskNum);

	/*
	��ȡ����

	Return :
	0 -- ����ɹ�
	1 -- ������
	-1 -- ʧ��
	*/
	int GetTask(std::shared_ptr<TaskBase>& out_ptrTask);

	/*
	��ȡ��ǰ�������

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int GetTaskAssignInfo(std::string& out_strTaskInfo);

protected:
	/*
	�������ɶ��е��׶˵���

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	virtual int PopFront(std::shared_ptr<TaskBase>& out_ptr_task) = 0;

	/*
	������ѹ����е�ĩ��

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	virtual int PushBack(std::shared_ptr<TaskBase>& ptr_task) = 0;

	/*
	��ȡ�������ݴ�С

	Return :

	*/
	virtual uint64_t QueueSize(void) = 0;

private:


};

#endif