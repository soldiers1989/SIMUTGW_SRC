#ifndef __CONF_BASE_H__
#define __CONF_BASE_H__

#include "simutgw_config/g_values_inner_redis.h"
#include "simutgw_config/g_values_inner_mysql.h"

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
#include "simutgw/stgw_fix_acceptor/MockerResponseManager.h"

#include "cache/TradePolicyManage.h"
#include "util/SystemCounter.h"

namespace simutgw
{
	// ����ͳ����
	extern SystemCounter g_counter;
	
	//
	extern FixAcceptorManager g_fixaccptor;

	// �����̹߳���
	extern LooperManager g_flowManage;

	// �첽д���ݿ��߳�
	extern AsyncDbWriter g_asyncDbwriter;

	// ��Ϣ���뻺��
	extern MemoryStoreCell<std::shared_ptr<struct simutgw::OrderMessage>> g_inMsg_buffer;

	// ��Ϣ���ػ���
	extern OutMemoryManage g_outMsg_buffer;

	// ��������ˮ�߹����߳�
	extern PriorityThreadPool g_mtskPool_valid;

	// ��������ˮ�߹����߳�
	extern PriorityThreadPool g_mtskPool_match_cancel;

	// ����ģʽ�����Կ��ƴ���
	extern TradePolicyManage g_tradePolicy;
	
	// ����ͳ��
	extern std::shared_ptr<TaskBase> g_prtTraffic;

	//
	extern boost::mutex g_mapUStockMutex;

	// �ɷ���Ϣ����
	extern std::map<std::string, std::map<string, std::shared_ptr<UserStockVolume> > > g_mapUserStock;

	// ���ö�������json�ļ����ݻ���
	extern MockerResponseManager g_mockerResponses;
}

#endif