#include "g_values_biz.h"

#include <mutex>

namespace simutgw
{
	// 记录深圳接口
	std::map<std::string, SzConnection> g_mapSzConns;

	// 记录深圳接口和Web配置的对应关系，以 对端ID TargetCompID 为Key，strConnId在Value内
	std::map<std::string, struct Connection_webConfig> g_mapSzConn_webConfig;

	// 记录上海接口
	std::map<std::string, ShConnection> g_mapShConns;

	// 记录上海接口和Web配置的对应关系，以 strConnId 为Key的 Map
	std::map<std::string, struct Connection_webConfig> g_mapShConn_webConfig;

	//
	// 成交规则配置
	MatchRule g_matchRule;

	//
	boost::shared_mutex g_mtxMapSZEtf;

	//etf信息
	std::map<std::string, std::shared_ptr<struct simutgw::SzETF>> g_mapSZETF;

	// ETF信息储存类
	ETFContainer g_etfContainer;

	// eft累计申购、赎回缓存
	ETF_TradeVolume g_mapEtfTradeVolume;
};
