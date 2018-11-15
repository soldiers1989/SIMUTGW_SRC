#include "g_values_inner.h"

#include "tool_mysql/MysqlConnectionPool.h"
#include "tool_redis/RedisConnectionPool.h"
#include "simutgw/order/OrderMemoryStoreFactory.h"
#include "simutgw/order/OutMemoryManage.h"
#include "cache/UserStockVolume.h"

#include "simutgw/work_manage/LooperManager.h"
#include "simutgw/work_manage/AsyncDbWriter.h"

#include "tool_redis/Redis3_0Cnn_dll.h"

#include "thread_pool_priority/PriorityThreadPool.h"

#include "simutgw/stgw_config/g_values_inner.h"

#include "simutgw/stgw_fix_acceptor/FixAcceptor.h"

namespace simutgw
{
	// ����ͳ����
	SystemCounter g_counter;
	
	//
	FixAcceptorManager g_fixaccptor;

	// �����̹߳���
	LooperManager g_flowManage;

	// �첽д���ݿ��߳�
	AsyncDbWriter g_asyncDbwriter;

	// ��Ϣ���뻺��
	MemoryStoreCell<std::shared_ptr<struct simutgw::OrderMessage>> g_inMsg_buffer;

	// ��Ϣ���ػ���
	OutMemoryManage g_outMsg_buffer;

	// ��������ˮ�߹����߳�
	PriorityThreadPool g_mtskPool_valid(1);

	// ��������ˮ�߹����߳�
	PriorityThreadPool g_mtskPool_match_cancel(1);

	// ����ģʽ�����Կ��ƴ���
	TradePolicyManage g_tradePolicy;
	
	// ����ͳ��
	std::shared_ptr<TaskBase> g_prtTraffic;

	//
	boost::mutex g_mapUStockMutex;

	// �ɷ���Ϣ����
	std::map<std::string, std::map<string, std::shared_ptr<UserStockVolume> > > g_mapUserStock;

	// ���ö�������json�ļ����ݻ���
	MockerResponseManager g_mockerResponses;

}