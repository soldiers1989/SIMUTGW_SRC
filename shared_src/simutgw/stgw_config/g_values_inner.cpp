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
	// 数据统计类
	SystemCounter g_counter;
	
	//
	FixAcceptorManager g_fixaccptor;

	// 处理线程管理
	LooperManager g_flowManage;

	// 异步写数据库线程
	AsyncDbWriter g_asyncDbwriter;

	// 消息进入缓存
	MemoryStoreCell<std::shared_ptr<struct simutgw::OrderMessage>> g_inMsg_buffer;

	// 消息返回缓存
	OutMemoryManage g_outMsg_buffer;

	// 多任务流水线管理线程
	PriorityThreadPool g_mtskPool_valid(1);

	// 多任务流水线管理线程
	PriorityThreadPool g_mtskPool_match_cancel(1);

	// 交易模式、策略控制储存
	TradePolicyManage g_tradePolicy;
	
	// 流量统计
	std::shared_ptr<TaskBase> g_prtTraffic;

	//
	boost::mutex g_mapUStockMutex;

	// 股份信息缓存
	std::map<std::string, std::map<string, std::shared_ptr<UserStockVolume> > > g_mapUserStock;

	// 配置读出来的json文件内容缓存
	MockerResponseManager g_mockerResponses;

}