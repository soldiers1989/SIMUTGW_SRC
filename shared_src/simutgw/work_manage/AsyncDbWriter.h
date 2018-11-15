#ifndef __ASYNC_DB_WRITER_H__
#define __ASYNC_DB_WRITER_H__

#include <memory>

/*
�첽��DataBase����д�����
*/

#include "thread_pool_pipeline/PipeLine_ThreadPool.h"

class AsyncDbWriter
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	PipeLine_ThreadPool m_pipelinePool;

	//
	// Functions
	//
public:
	AsyncDbWriter( void );
	virtual ~AsyncDbWriter( void );

	/*
	�����߳�

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int Start( void );

	/*
	�ر��߳�

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int Stop( void );

	/*
	��������

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int AssignTask( std::shared_ptr<TaskBase>& ptr_task );

	/*
	��ȡ��ǰ�������

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int GetTaskAssignInfo(std::string& out_strTaskInfo)
	{
		// static const std::string ftag("ThreadPool::AssignTask() ");

		m_pipelinePool.GetTaskAssignInfo(out_strTaskInfo);

		return 0;
	}

protected:

};

#endif