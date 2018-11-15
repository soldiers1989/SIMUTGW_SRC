#ifndef __CONF_BIZ_H__
#define __CONF_BIZ_H__

#include <string>
#include <map>
#include <memory>

#include "boost/thread.hpp"

#include "simutgw/mkt_interface/SzConnection_define.h"
#include "simutgw/mkt_interface/ShConnection_define.h"
#include "etf/conf_etf_info.h"
#include "etf/ETF_TradeVolume.h"
#include "etf/ETFContainer.h"

#include "simutgw/mkt_interface/Connection_webConfig.h"

#include "match_rule/MatchRule.h"

namespace simutgw
{
	// 记录深圳接口
	extern std::map<std::string, SzConnection> g_mapSzConns;

	// 记录深圳接口和Web配置的对应关系，以 对端ID TargetCompID 为Key，strConnId在Value内
	extern std::map<std::string, struct Connection_webConfig> g_mapSzConn_webConfig;

	// 记录上海接口
	extern std::map<std::string, ShConnection> g_mapShConns;

	// 记录上海接口和Web配置的对应关系，以 strConnId 为Key的 Map
	extern std::map<std::string, struct Connection_webConfig> g_mapShConn_webConfig;

	//
	// 成交规则配置
	extern MatchRule g_matchRule;

	typedef boost::shared_lock<boost::shared_mutex> etfReadLock;
	typedef boost::unique_lock<boost::shared_mutex> etfWriteLock;

	//
	extern boost::shared_mutex g_mtxMapSZEtf;

	//etf信息
	extern std::map<std::string, std::shared_ptr<struct simutgw::SzETF>> g_mapSZETF;
	
	// ETF信息储存类
	extern ETFContainer g_etfContainer;

	// eft累计申购、赎回缓存
	extern ETF_TradeVolume g_mapEtfTradeVolume;
};

#endif