#include "TaskQueueBase.h"

#include <limits>

#include "util/EzLog.h"
#include "tool_string/sof_string.h"

const uint64_t TaskQueueBase::MAX_QUEUE_SIZE = (uint64_t) ( std::numeric_limits<uint64_t>::max )( ) - 10;

TaskQueueBase::TaskQueueBase(const uint64_t uiId)
: m_uiTaskQueId(uiId), m_uiTaskNums(0)
{
}

TaskQueueBase::~TaskQueueBase(void)
{
}

/*
插入任务

Param :
uint64_t& out_CurrentTaskNum : 当前此线程已有任务总数

Return :
0 -- 插入成功
2 -- 数量已满，不允许插入
-1 -- 插入失败
*/
int TaskQueueBase::AddTask(std::shared_ptr<TaskBase>& in_ptrTask, uint64_t& out_CurrentTaskNum)
{
	static const string ftag("TaskQueueBase::AddTask()");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	// 检查当前任务数是否已超标
	if( MAX_QUEUE_SIZE <= m_uiTaskNums )
	{
		// 有相同的任务，但是数量已满，不允许插入
		// 避免数量太大，越界

		out_CurrentTaskNum = m_uiTaskNums;
		return 2;
	}

	int iRes = PushBack(in_ptrTask);
	if( 0 != iRes )
	{
		string strTran;
		string strDebug("pushback into queue failed, id=");
		strDebug += sof_string::itostr(m_uiTaskQueId, strTran);
		EzLog::e(ftag, strDebug);
		// 插入错误
		return -1;
	}

	uint64_t uiTaskId = in_ptrTask->GetTaskId();

	map<uint64_t, long>::iterator it = m_mapTaskNumTrack.find(uiTaskId);

	if( m_mapTaskNumTrack.end() != it )
	{
		// 队列中有HashKey相同的任务
		++(it->second);
	}
	else
	{
		m_mapTaskNumTrack.insert(pair<uint64_t, long>(uiTaskId, 1));
	}

	++m_uiTaskNums;

	out_CurrentTaskNum = m_uiTaskNums;

	return 0;
}

/*
查找任务队列中是否已有HashKey相同的任务，如果已有，就插入任务；如果没有，则不插入

Param :
uint64_t& out_CurrentTaskNum : 当前此线程已有任务总数

Return :
0 -- 有相同的任务，并且插入成功
1 -- 无相同的任务
2 -- 数量已满，不允许插入
-1 -- 有相同的任务，但是插入错误
*/
int TaskQueueBase::AddIfHaveTask(std::shared_ptr<TaskBase>& in_ptrTask,
	uint64_t& out_CurrentTaskNum)
{
	static const string ftag("TaskQueueBase::AddIfHaveTask()");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	// 检查当前任务数是否已超标
	if( MAX_QUEUE_SIZE <= m_uiTaskNums )
	{
		// 避免数量太大，越界

		out_CurrentTaskNum = m_uiTaskNums;

		return 2;
	}

	map<uint64_t, long>::iterator it = m_mapTaskNumTrack.find(in_ptrTask->GetTaskId());

	if( m_mapTaskNumTrack.end() != it )
	{
		// 队列中有HashKey相同的任务

		int iRes = PushBack(in_ptrTask);
		if( 0 != iRes )
		{
			// 有相同的任务，但是插入错误
			string strTran;
			string strDebug("pushback into queue failed, id=");
			strDebug += sof_string::itostr(m_uiTaskQueId, strTran);
			EzLog::e(ftag, strDebug);

			return -1;
		}

		// 0 -- 有相同的任务，并且插入成功
		++(it->second);

		++m_uiTaskNums;

		out_CurrentTaskNum = m_uiTaskNums;

		return 0;
	}
	else
	{
		// 无相同的任务

		out_CurrentTaskNum = m_uiTaskNums;

		return 1;
	}
}

/*
获取任务

Return :
0 -- 获取成功
1 -- 无任务
-1 -- 失败
*/
int TaskQueueBase::GetTask(std::shared_ptr<TaskBase>& out_ptrTask)
{
	static const string ftag("TaskQueueBase::GetTask()");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	int iRes = PopFront(out_ptrTask);
	if( 0 != iRes )
	{
		return 1;
	}

	if( NULL == out_ptrTask )
	{
		return -1;
	}

	if( m_mapTaskNumTrack.empty() || 0 == m_uiTaskNums )
	{
		// 有错误或问题
		string strTran;
		string strDebug("poped, but statics empty, id=");
		strDebug += sof_string::itostr(m_uiTaskQueId, strTran);
		EzLog::e(ftag, strDebug);
		return 1;
	}

	map<uint64_t, long>::iterator it = m_mapTaskNumTrack.find(out_ptrTask->GetTaskId());

	if( m_mapTaskNumTrack.end() != it )
	{
		// 队列中有HashKey相同的任务
		--(it->second);

		if( 0 == it->second )
		{
			// 如果任务数为零，则清理
			m_mapTaskNumTrack.erase(it);
		}
	}
	else
	{
		// 无相同的任务

		// 有错误或问题
		string strTran;
		string strDebug("poped, but empty, id=");
		strDebug += sof_string::itostr(m_uiTaskQueId, strTran);
		strDebug += "taskid=";
		strDebug += sof_string::itostr(out_ptrTask->GetTaskId(), strTran);
		EzLog::e(ftag, strDebug);
	}

	--m_uiTaskNums;
	return 0;
}

/*
获取当前任务情况

Return :
0 -- 成功
-1 -- 失败
*/
int TaskQueueBase::GetTaskAssignInfo(string& out_strTaskInfo)
{
	static const string ftag("TaskQueueBase::GetTaskAssignInfo()");

	map<uint64_t, long>::const_iterator it = m_mapTaskNumTrack.begin();

	string strTran;

	out_strTaskInfo = "Queue[";
	out_strTaskInfo += sof_string::itostr(m_uiTaskQueId, strTran);
	out_strTaskInfo += "] size[";
	out_strTaskInfo += sof_string::itostr(QueueSize(), strTran);
	out_strTaskInfo += "] { ";

	for(it = m_mapTaskNumTrack.begin(); it != m_mapTaskNumTrack.end(); ++it)
	{
		out_strTaskInfo += sof_string::itostr(it->first, strTran);
		out_strTaskInfo += ",";
		out_strTaskInfo += sof_string::itostr(it->second, strTran);
		out_strTaskInfo += "; ";
	}

	out_strTaskInfo += "}";

	return 0;
}