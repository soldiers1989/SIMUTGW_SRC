#include "g_values_biz.h"

#include <mutex>

namespace simutgw
{
	// ��¼���ڽӿ�
	std::map<std::string, SzConnection> g_mapSzConns;

	// ��¼���ڽӿں�Web���õĶ�Ӧ��ϵ���� �Զ�ID TargetCompID ΪKey��strConnId��Value��
	std::map<std::string, struct Connection_webConfig> g_mapSzConn_webConfig;

	// ��¼�Ϻ��ӿ�
	std::map<std::string, ShConnection> g_mapShConns;

	// ��¼�Ϻ��ӿں�Web���õĶ�Ӧ��ϵ���� strConnId ΪKey�� Map
	std::map<std::string, struct Connection_webConfig> g_mapShConn_webConfig;

	//
	// �ɽ���������
	MatchRule g_matchRule;

	//
	boost::shared_mutex g_mtxMapSZEtf;

	//etf��Ϣ
	std::map<std::string, std::shared_ptr<struct simutgw::SzETF>> g_mapSZETF;

	// ETF��Ϣ������
	ETFContainer g_etfContainer;

	// eft�ۼ��깺����ػ���
	ETF_TradeVolume g_mapEtfTradeVolume;
};
