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
��������

Param :
uint64_t& out_CurrentTaskNum : ��ǰ���߳�������������

Return :
0 -- ����ɹ�
2 -- �������������������
-1 -- ����ʧ��
*/
int TaskQueueBase::AddTask(std::shared_ptr<TaskBase>& in_ptrTask, uint64_t& out_CurrentTaskNum)
{
	static const string ftag("TaskQueueBase::AddTask()");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	// ��鵱ǰ�������Ƿ��ѳ���
	if( MAX_QUEUE_SIZE <= m_uiTaskNums )
	{
		// ����ͬ�����񣬵����������������������
		// ��������̫��Խ��

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
		// �������
		return -1;
	}

	uint64_t uiTaskId = in_ptrTask->GetTaskId();

	map<uint64_t, long>::iterator it = m_mapTaskNumTrack.find(uiTaskId);

	if( m_mapTaskNumTrack.end() != it )
	{
		// ��������HashKey��ͬ������
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
��������������Ƿ�����HashKey��ͬ������������У��Ͳ����������û�У��򲻲���

Param :
uint64_t& out_CurrentTaskNum : ��ǰ���߳�������������

Return :
0 -- ����ͬ�����񣬲��Ҳ���ɹ�
1 -- ����ͬ������
2 -- �������������������
-1 -- ����ͬ�����񣬵��ǲ������
*/
int TaskQueueBase::AddIfHaveTask(std::shared_ptr<TaskBase>& in_ptrTask,
	uint64_t& out_CurrentTaskNum)
{
	static const string ftag("TaskQueueBase::AddIfHaveTask()");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	// ��鵱ǰ�������Ƿ��ѳ���
	if( MAX_QUEUE_SIZE <= m_uiTaskNums )
	{
		// ��������̫��Խ��

		out_CurrentTaskNum = m_uiTaskNums;

		return 2;
	}

	map<uint64_t, long>::iterator it = m_mapTaskNumTrack.find(in_ptrTask->GetTaskId());

	if( m_mapTaskNumTrack.end() != it )
	{
		// ��������HashKey��ͬ������

		int iRes = PushBack(in_ptrTask);
		if( 0 != iRes )
		{
			// ����ͬ�����񣬵��ǲ������
			string strTran;
			string strDebug("pushback into queue failed, id=");
			strDebug += sof_string::itostr(m_uiTaskQueId, strTran);
			EzLog::e(ftag, strDebug);

			return -1;
		}

		// 0 -- ����ͬ�����񣬲��Ҳ���ɹ�
		++(it->second);

		++m_uiTaskNums;

		out_CurrentTaskNum = m_uiTaskNums;

		return 0;
	}
	else
	{
		// ����ͬ������

		out_CurrentTaskNum = m_uiTaskNums;

		return 1;
	}
}

/*
��ȡ����

Return :
0 -- ��ȡ�ɹ�
1 -- ������
-1 -- ʧ��
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
		// �д��������
		string strTran;
		string strDebug("poped, but statics empty, id=");
		strDebug += sof_string::itostr(m_uiTaskQueId, strTran);
		EzLog::e(ftag, strDebug);
		return 1;
	}

	map<uint64_t, long>::iterator it = m_mapTaskNumTrack.find(out_ptrTask->GetTaskId());

	if( m_mapTaskNumTrack.end() != it )
	{
		// ��������HashKey��ͬ������
		--(it->second);

		if( 0 == it->second )
		{
			// ���������Ϊ�㣬������
			m_mapTaskNumTrack.erase(it);
		}
	}
	else
	{
		// ����ͬ������

		// �д��������
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
��ȡ��ǰ�������

Return :
0 -- �ɹ�
-1 -- ʧ��
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