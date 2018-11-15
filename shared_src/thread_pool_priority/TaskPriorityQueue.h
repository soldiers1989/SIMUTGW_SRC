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
		// 任务总数量
		uint64_t ui64TotalNum;
		// 待交易委托数量
		uint64_t ui64OrderNum;

		TaskNum() :ui64TotalNum(0), ui64OrderNum(0)
		{
		};
	};

}

/*
有优先级的任务队列
*/
class TaskPriorityQueue
{
	//
	// member
	//
public:

	typedef std::map<int, std::deque<T_PTR_TaskPriorityBase>> T_MAP_INT_DEQUE;


	// 队列允许最大总任务数量
	static const uint64_t MAX_QUEUE_SIZE;

protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// id
	uint64_t m_uiTaskQueId;

	// 锁对象
	boost::mutex m_mutexlock;

	// 当前队列任务总数
	uint64_t m_uiTaskNums;

	//
	// Queues
	// 委托挂单，接受由委托无序queue的挂单
	std::list<T_PTR_TaskPriorityBase> m_p1_que;

	// 委托检查
	std::list<T_PTR_TaskPriorityBase> m_p2_que;

	// 委托处理
	// 有序（如价格）queue
	T_MAP_INT_DEQUE m_p3_order_que;
	// 无序queue
	std::list<T_PTR_TaskPriorityBase> m_p3_unorder_que;

	// 撤单处理
	std::list<T_PTR_TaskPriorityBase> m_p4_que;

	//
	//
	// 委托处理 有序（如价格）queue 挂单委托任务追踪
	// value:true挂单 false 不挂单
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
	插入任务

	Param :
	uint64_t& out_CurrentTaskNum : 当前此线程已有任务总数
	const bool in_bIsMatchOrdered : false 需插入委托无序队列
	true 需插入委托有序队列

	Return :
	0 -- 插入成功
	2 -- 数量已满，不允许插入
	-1 -- 插入失败
	*/
	int AddTask(T_PTR_TaskPriorityBase& in_ptrTask, uint64_t& out_CurrentTaskNum, const bool in_bIsMatchOrdered = false);

	/*
	获取任务

	Return :
	0 -- 插入成功
	1 -- 无任务
	-1 -- 失败
	*/
	int GetTask(T_PTR_TaskPriorityBase& out_ptrTask);

	/*
	删除任务(撤单时使用)

	0 -- 有该任务，且删除成功
	1 -- 无该任务
	*/
	int DeleteIfHaveTask(const int& in_iKey, const std::string& in_strClordid,
		T_PTR_TaskPriorityBase& out_ptrTask);

	/*
	获取队列数据大小

	Return :

	*/
	uint64_t QueueSize(void) const
	{
		return m_uiTaskNums;
	}

	/*
	获取当前任务情况

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int GetTaskAssignInfo(std::string& out_strTaskInfo);

protected:
	/*
	在Queue中按ClordId找任务并从queue中删除

	@param T& container : 含任务的容器
	@param const std::string& in_strClordid : ClordId
	@param T_PTR_TaskPriorityBase& out_ptrTask : 找到的任务

	@return :
	0 -- 找到并删除
	-1 -- 未找到
	*/
	template<typename T>
	int DeleteOrderByClordId(T& container, const std::string& in_strClordid, T_PTR_TaskPriorityBase& out_ptrTask)
	{
		for (typename T::iterator it = container.begin(); container.end() != it; ++it)
		{
			if (0 == (*it)->GetCliordid().compare(in_strClordid))
			{
				// 找到并抛出，删除
				out_ptrTask = *it;
				container.erase(it);

				return 0;
			}
		}

		return -1;
	}

	/*
	插入到有序队列

	@param deque& container : 含任务的容器
	@param T_PTR_TaskPriorityBase& out_ptrTask : 要插入的任务

	@return :
	0 -- 插入成功
	-1 -- 失败
	*/
	int Deque_Insert(std::deque<T_PTR_TaskPriorityBase>& container,
		T_PTR_TaskPriorityBase& in_ptrTask);

	/*
	按价格、时间折半插入有序队列
	升序

	@param deque& container : 含任务的容器
	@param T_PTR_TaskPriorityBase& out_ptrTask : 要插入的任务

	@return :
	0 -- 插入成功
	-1 -- 失败
	*/
	int Deque_Price_Time_Binary_Insert_Inc(std::deque<T_PTR_TaskPriorityBase>& container,
		T_PTR_TaskPriorityBase& in_ptrTask);

	/*
	按价格、时间折半插入有序队列
	降序

	@param deque& container : 含任务的容器
	@param T_PTR_TaskPriorityBase& out_ptrTask : 要插入的任务

	@return :
	0 -- 插入成功
	-1 -- 失败
	*/
	int Deque_Price_Time_Binary_Insert_Desc(std::deque<T_PTR_TaskPriorityBase>& container,
		T_PTR_TaskPriorityBase& in_ptrTask);


	/*
	将任务由队列的首端弹出

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int PopFront(T_PTR_TaskPriorityBase& out_ptr_task);

	/*
	将任务压入队列的相同key的位置，有相同key时使用

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int Push(T_PTR_TaskPriorityBase& ptr_task);

	/*
	将任务压入队列的末端，没有找到相同key时使用

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int PushBack(T_PTR_TaskPriorityBase& ptr_task);

	/*
	打印队列
	*/
	static void PrintQueue(std::deque<T_PTR_TaskPriorityBase>& container);
};

#endif