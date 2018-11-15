#ifndef __CONFIG_DEFINE_H__
#define __CONFIG_DEFINE_H__

#include <memory>
#include <stdint.h>  
#include <string>

class TaskPriorityBase;

namespace simutgw
{
	//
	// Members
	//
	// 非高性能模式sleep time
	static const int g_iUnHighPfmSleepTime(1 * 1000);

	// 高性能模式sleep time
	static const int g_iHighPfmSleepTime(10);

	/* 处理深圳买单 */
	static const int Flow_MatchType_SZ_BUY = 1;
	/* 处理深圳卖单 */
	static const int Flow_MatchType_SZ_SELL = 2;

	/* 处理上海买单 */
	static const int Flow_MatchType_SH_BUY = 3;
	/* 处理上海卖单 */
	static const int Flow_MatchType_SH_SELL = 4;

	/* 处理深圳消息，包括取深圳委托和发回报 */
	static const int Flow_MatchType_SZ_MSG = 5;
	/* 处理上海消息，包括取深圳委托和发回报 */
	static const int Flow_MatchType_SH_MSG = 6;

	/* 处理深圳redis队列，取redis委托到内存队列 */
	static const int Flow_MatchType_SZ_REDIS = 7;
	/* 处理上海redis队列，取redis委托到内存队列 */
	static const int Flow_MatchType_SH_REDIS = 8;

	// A股行情推送
	static const std::string g_Key_AStockQuotationChange("hqk_change");

	// 静态行情前缀
	static const std::string g_Key_AStockStaticQuotation_Prefix("hqk_static_");

	// A股行情自存前缀
	static const std::string g_Key_AStockQuotTGW_Prefix("tgwhqk_");

	// A股行情容量
	static const std::string g_Key_AStockQuotTGW_TradeVolume("tgwhqk_tv");

	// 已有订单号储存
	static const std::string g_redis_Key_OrderId_Record("tgw_orderids");

	/*
	系统运行模式
	*/
	enum WebManMode
	{
		/* 0 -- 非Web管理模式 */
		LocalMode = 0,

		/* 1 -- Web管理模式 */
		WebMode = 1
	};

	/*
	系统运行模式
	*/
	enum SysRunMode
	{
		/* 1 -- 压力模式 */
		PressureMode = 1,

		/* 2 -- 极简模式 */
		MiniMode = 2,

		/* 3 -- 普通模式 */
		NormalMode = 3
	};

	/*
	系统成交模式
	*/
	enum SysMatchMode
	{
		/* 默认，启用行情 */
		EnAbleQuta = 0,

		/* 1 --不看行情，全部成交 */
		SimulMatchAll = 1,

		/* 2 --不看行情，分笔成交 */
		SimulMatchByDivide = 2,

		/* 3 --不看行情，挂单、不成交，可撤单 */
		SimulNotMatch = 3,

		/* 4 --错误单 */
		SimulErrMatch = 4,

		/* 5 --部分成交，先成交一半，剩余挂单可撤单 */
		SimulMatchPart = 5
	};

	enum QuotationType
	{
		// 根据变动成交总金额和成交总量计算的均价
		AveragePrice = 0,

		// 买一卖一
		SellBuyPrice = 1,

		// 最近成交价
		RecentMatchPrice = 2
	};

	/*
	成交类型
	*/
	enum MatchType
	{
		//全部成交
		MatchAll = 0,
		//部分成交
		MatchPart = 1,
		//撤单
		CancelMatch = 2,
		//未成交
		NotMatch = -1,
		// 超出涨跌幅
		OutOfRange = -2,
		// 停牌
		StopTrans = -3,
		// 交易不合法
		ErrorMatch = -4
	};
};

typedef std::shared_ptr<TaskPriorityBase> T_PTR_TaskPriorityBase;

#endif
