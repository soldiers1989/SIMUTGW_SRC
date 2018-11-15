#include "PriorityThreadPool.h"

#include "config/g_values_inner_IdGen.h"

/*
�����̳߳ض���

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int PriorityThreadPool::InitPool(void)
{
	static const string ftag("Priority_ThreadPool::InitPool() ");

	m_pool_state = ThreadPool_Conf::STOPPED;

	unsigned int i = 0;
	for (i = 0; i < m_uiThreadMaxNums; ++i)
	{
		std::shared_ptr<PriorityThread> ptrThread(new PriorityThread(simutgw::g_uidThreadGen.GetId()));

		if ( NULL == ptrThread )
		{
			EzLog::e(ftag, "new ThreadBase() failed, NULL");

			return -1;
		}

		m_hdThreads.push_back(ptrThread);

		++m_uiCurrThreadNums;
	}

	return 0;
}
