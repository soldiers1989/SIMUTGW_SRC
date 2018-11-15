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
	// ��¼���ڽӿ�
	extern std::map<std::string, SzConnection> g_mapSzConns;

	// ��¼���ڽӿں�Web���õĶ�Ӧ��ϵ���� �Զ�ID TargetCompID ΪKey��strConnId��Value��
	extern std::map<std::string, struct Connection_webConfig> g_mapSzConn_webConfig;

	// ��¼�Ϻ��ӿ�
	extern std::map<std::string, ShConnection> g_mapShConns;

	// ��¼�Ϻ��ӿں�Web���õĶ�Ӧ��ϵ���� strConnId ΪKey�� Map
	extern std::map<std::string, struct Connection_webConfig> g_mapShConn_webConfig;

	//
	// �ɽ���������
	extern MatchRule g_matchRule;

	typedef boost::shared_lock<boost::shared_mutex> etfReadLock;
	typedef boost::unique_lock<boost::shared_mutex> etfWriteLock;

	//
	extern boost::shared_mutex g_mtxMapSZEtf;

	//etf��Ϣ
	extern std::map<std::string, std::shared_ptr<struct simutgw::SzETF>> g_mapSZETF;
	
	// ETF��Ϣ������
	extern ETFContainer g_etfContainer;

	// eft�ۼ��깺����ػ���
	extern ETF_TradeVolume g_mapEtfTradeVolume;
};

#endif