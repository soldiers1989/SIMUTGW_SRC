#ifndef __TASK_QUEUE_QUEUE_H__
#define __TASK_QUEUE_QUEUE_H__

#include <queue>

#include "TaskQueueBase.h"

class TaskQueue_queue :
	public TaskQueueBase
{
	//
	// Members
	//
protected:
	std::queue<std::shared_ptr<TaskBase>> m_taskQueue;

public:
	TaskQueue_queue(void);
	explicit TaskQueue_queue(const uint64_t uiId);
	virtual ~TaskQueue_queue(void);

protected:
	/*
	�������ɶ��е��׶˵���

	Return :
	0 -- �ɹ�
	1 -- ������
	*/
	virtual int PopFront(std::shared_ptr<TaskBase>& out_ptr_task);

	/*
	������ѹ����е�ĩ��

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	virtual int PushBack(std::shared_ptr<TaskBase>& in_ptr_task);

	/*
	��ȡ�������ݴ�С

	Return :

	*/
	virtual uint64_t QueueSize(void);

private:
	
};

#endif