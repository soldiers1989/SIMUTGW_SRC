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
	// 数据统计类
	extern SystemCounter g_counter;
	
	//
	extern FixAcceptorManager g_fixaccptor;

	// 处理线程管理
	extern LooperManager g_flowManage;

	// 异步写数据库线程
	extern AsyncDbWriter g_asyncDbwriter;

	// 消息进入缓存
	extern MemoryStoreCell<std::shared_ptr<struct simutgw::OrderMessage>> g_inMsg_buffer;

	// 消息返回缓存
	extern OutMemoryManage g_outMsg_buffer;

	// 多任务流水线管理线程
	extern PriorityThreadPool g_mtskPool_valid;

	// 多任务流水线管理线程
	extern PriorityThreadPool g_mtskPool_match_cancel;

	// 交易模式、策略控制储存
	extern TradePolicyManage g_tradePolicy;
	
	// 流量统计
	extern std::shared_ptr<TaskBase> g_prtTraffic;

	//
	extern boost::mutex g_mapUStockMutex;

	// 股份信息缓存
	extern std::map<std::string, std::map<string, std::shared_ptr<UserStockVolume> > > g_mapUserStock;

	// 配置读出来的json文件内容缓存
	extern MockerResponseManager g_mockerResponses;
}

#endif