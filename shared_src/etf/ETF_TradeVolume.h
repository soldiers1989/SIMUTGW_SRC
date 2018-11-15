#ifndef __ETF_TRADE_VOLUME_H__
#define __ETF_TRADE_VOLUME_H__

#include <string>
#include <vector>
#include <memory>

#include "boost/thread/mutex.hpp"

#include "conf_etf_info.h"
#include "ETF_TradeVolume_Basket.h"

/*
	etf����ʱ�ۼ��깺������ܶ������ͳ�ƻ�����
	*/
class ETF_TradeVolume
{
	//
	// Members
	//
protected:
	std::map<std::string, ETF_TradeVolume_Basket> m_etfsTv;

	//
	// Functions
	//
public:
	ETF_TradeVolume( void )
	{ }

	virtual ~ETF_TradeVolume()
	{ }

	/*
	�����깺���׶�

	Return:
	0 -- δ����������ƶ�ȣ�����ɹ�
	-1 -- ����ʧ��
	*/
	int FrozeCreation( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

	/*
	��������깺���׶�

	Return:
	0 -- �������ɹ�
	-1 -- ����ʧ��
	*/
	int DefrozeCreation( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId );

	/*
	ȷ���깺���׶�

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	int ConfirmCreation( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId );

	/*
	������ؽ��׶�

	Return:
	0 -- δ����������ƶ�ȣ�����ɹ�
	-1 -- ����ʧ��
	*/
	int FrozeRedemption( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

	/*
	���������ؽ��׶�

	Return:
	0 -- �������ɹ�
	-1 -- �������ʧ��
	*/
	int DefrozeRedemption( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId );

	/*
	ȷ����ؽ��׶�

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	int ConfirmRedemption( const uint64_t in_num,
		const std::string& in_sSecurityId, const std::string& in_sCustId );

};

#endif