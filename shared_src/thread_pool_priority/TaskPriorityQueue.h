#ifndef __TASK_PRIORITY_QUEUE_H__
#define __TASK_PRIORITY_QUEUE_H__

#include <list>
#include <deque>

#include "boost/thread/mutex.hpp"

#include "TaskPriorityBase.h"

#include "util/EzLog.h"
#include "util/TimeDuration.h"

#include "simutgw_config/config_define.h"

namespace QueueTask
{
	struct TaskNum
	{
		// ����������
		uint64_t ui64TotalNum;
		// ������ί������
		uint64_t ui64OrderNum;

		TaskNum() :ui64TotalNum(0), ui64OrderNum(0)
		{
		};
	};

}

/*
�����ȼ����������
*/
class TaskPriorityQueue
{
	//
	// member
	//
public:

	typedef std::map<int, std::deque<T_PTR_TaskPriorityBase>> T_MAP_INT_DEQUE;


	// ���������������������
	static const uint64_t MAX_QUEUE_SIZE;

protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// id
	uint64_t m_uiTaskQueId;

	// ������
	boost::mutex m_mutexlock;

	// ��ǰ������������
	uint64_t m_uiTaskNums;

	//
	// Queues
	// ί�йҵ���������ί������queue�Ĺҵ�
	std::list<T_PTR_TaskPriorityBase> m_p1_que;

	// ί�м��
	std::list<T_PTR_TaskPriorityBase> m_p2_que;

	// ί�д���
	// ������۸�queue
	T_MAP_INT_DEQUE m_p3_order_que;
	// ����queue
	std::list<T_PTR_TaskPriorityBase> m_p3_unorder_que;

	// ��������
	std::list<T_PTR_TaskPriorityBase> m_p4_que;

	//
	//
	// ί�д��� ������۸�queue �ҵ�ί������׷��
	// value:true�ҵ� false ���ҵ�
	std::map<int, bool> m_map_p3_order_queUnMatch;


	//
	// function
	//
public:
	TaskPriorityQueue() :
		m_scl(keywords::channel = "TaskPriorityQueue"),
		m_uiTaskQueId(0), m_uiTaskNums(0)
	{
	};

	explicit TaskPriorityQueue(uint64_t uiTaskQueId) :
		m_scl(keywords::channel = "TaskPriorityQueue"),
		m_uiTaskQueId(uiTaskQueId), m_uiTaskNums(0)
	{
	};

	virtual ~TaskPriorityQueue()
	{
	};

	void SetQueueId(const uint64_t uiId)
	{
		m_uiTaskQueId = uiId;
	}

	uint64_t GetQueueId(void) const
	{
		return m_uiTaskQueId;
	}

	/*
	��������

	Param :
	uint64_t& out_CurrentTaskNum : ��ǰ���߳�������������
	const bool in_bIsMatchOrdered : false �����ί���������
	true �����ί���������

	Return :
	0 -- ����ɹ�
	2 -- �������������������
	-1 -- ����ʧ��
	*/
	int AddTask(T_PTR_TaskPriorityBase& in_ptrTask, uint64_t& out_CurrentTaskNum, const bool in_bIsMatchOrdered = false);

	/*
	��ȡ����

	Return :
	0 -- ����ɹ�
	1 -- ������
	-1 -- ʧ��
	*/
	int GetTask(T_PTR_TaskPriorityBase& out_ptrTask);

	/*
	ɾ������(����ʱʹ��)

	0 -- �и�������ɾ���ɹ�
	1 -- �޸�����
	*/
	int DeleteIfHaveTask(const int& in_iKey, const std::string& in_strClordid,
		T_PTR_TaskPriorityBase& out_ptrTask);

	/*
	��ȡ�������ݴ�С

	Return :

	*/
	uint64_t QueueSize(void) const
	{
		return m_uiTaskNums;
	}

	/*
	��ȡ��ǰ�������

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int GetTaskAssignInfo(std::string& out_strTaskInfo);

protected:
	/*
	��Queue�а�ClordId�����񲢴�queue��ɾ��

	@param T& container : �����������
	@param const std::string& in_strClordid : ClordId
	@param T_PTR_TaskPriorityBase& out_ptrTask : �ҵ�������

	@return :
	0 -- �ҵ���ɾ��
	-1 -- δ�ҵ�
	*/
	template<typename T>
	int DeleteOrderByClordId(T& container, const std::string& in_strClordid, T_PTR_TaskPriorityBase& out_ptrTask)
	{
		for (typename T::iterator it = container.begin(); container.end() != it; ++it)
		{
			if (0 == (*it)->GetCliordid().compare(in_strClordid))
			{
				// �ҵ����׳���ɾ��
				out_ptrTask = *it;
				container.erase(it);

				return 0;
			}
		}

		return -1;
	}

	/*
	���뵽�������

	@param deque& container : �����������
	@param T_PTR_TaskPriorityBase& out_ptrTask : Ҫ���������

	@return :
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int Deque_Insert(std::deque<T_PTR_TaskPriorityBase>& container,
		T_PTR_TaskPriorityBase& in_ptrTask);

	/*
	���۸�ʱ���۰�����������
	����

	@param deque& container : �����������
	@param T_PTR_TaskPriorityBase& out_ptrTask : Ҫ���������

	@return :
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int Deque_Price_Time_Binary_Insert_Inc(std::deque<T_PTR_TaskPriorityBase>& container,
		T_PTR_TaskPriorityBase& in_ptrTask);

	/*
	���۸�ʱ���۰�����������
	����

	@param deque& container : �����������
	@param T_PTR_TaskPriorityBase& out_ptrTask : Ҫ���������

	@return :
	0 -- ����ɹ�
	-1 -- ʧ��
	*/
	int Deque_Price_Time_Binary_Insert_Desc(std::deque<T_PTR_TaskPriorityBase>& container,
		T_PTR_TaskPriorityBase& in_ptrTask);


	/*
	�������ɶ��е��׶˵���

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int PopFront(T_PTR_TaskPriorityBase& out_ptr_task);

	/*
	������ѹ����е���ͬkey��λ�ã�����ͬkeyʱʹ��

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int Push(T_PTR_TaskPriorityBase& ptr_task);

	/*
	������ѹ����е�ĩ�ˣ�û���ҵ���ͬkeyʱʹ��

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int PushBack(T_PTR_TaskPriorityBase& ptr_task);

	/*
	��ӡ����
	*/
	static void PrintQueue(std::deque<T_PTR_TaskPriorityBase>& container);
};

#endif