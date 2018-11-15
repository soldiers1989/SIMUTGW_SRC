#include "TaskQueue_queue.h"

TaskQueue_queue::TaskQueue_queue(void)
: TaskQueueBase(0)
{
}


TaskQueue_queue::TaskQueue_queue(const uint64_t uiId)
: TaskQueueBase(uiId)
{
}

TaskQueue_queue::~TaskQueue_queue(void)
{
}

/*
�������ɶ��е��׶˵���

Return :
0 -- �ɹ�
1 -- ������
*/
int TaskQueue_queue::PopFront(std::shared_ptr<TaskBase>& out_ptr_task)
{
	if( m_taskQueue.empty() )
	{
		return 1;
	}

	// �ɶ��е��׶˵���
	out_ptr_task = m_taskQueue.front();

	m_taskQueue.pop();

	return 0;
}

/*
������ѹ����е�ĩ��

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int TaskQueue_queue::PushBack(std::shared_ptr<TaskBase>& in_ptr_task)
{
	// ѹ����е�ĩ��
	m_taskQueue.push(in_ptr_task);

	return 0;
}

/*
��ȡ�������ݴ�С

Return :

*/
uint64_t TaskQueue_queue::QueueSize(void)
{
	return m_taskQueue.size();
}