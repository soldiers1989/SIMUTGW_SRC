#include "AsyncDbWriter.h"

#include <memory>

#include "simutgw_flowwork/FlowWorkBase.h"

#include "quotation/TgwMarketInfoProc.h"

AsyncDbWriter::AsyncDbWriter(void)
	: m_scl(keywords::channel = "AsyncDbWriter"),
	m_pipelinePool(1)
{
}

AsyncDbWriter::~AsyncDbWriter(void)
{
}

/*
��������ģ��������߳�

Return :
0 -- �����ɹ�
-1 -- ����ʧ��
*/
int AsyncDbWriter::Start(void)
{
	static const string ftag("AsyncDbWriter::Start() ");

	//
	//
	int iRes = m_pipelinePool.InitPool();
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "InitPool failed";

		return -1;
	}

	iRes = m_pipelinePool.StartPool();
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "StartPool failed";

		return -1;
	}

	return iRes;
}


/*
�ر�����ģ��������߳�

Return :
0 -- �����ɹ�
-1 -- ����ʧ��
*/
int AsyncDbWriter::Stop(void)
{
	m_pipelinePool.StopPool();
	return 0;
}

/*
��������

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int AsyncDbWriter::AssignTask(std::shared_ptr<TaskBase>& ptr_task)
{
	static const string ftag("AsyncDbWriter::AssignTask() ");

	int iRes = m_pipelinePool.AssignTask(ptr_task);

	if (0 != iRes)
	{
		string strDebug("insert task failed");
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
	}

	return iRes;
}