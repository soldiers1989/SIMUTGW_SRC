#ifndef __ETF_TRADE_VOLUME_BASKET_H__
#define __ETF_TRADE_VOLUME_BASKET_H__

#include <string>
#include <vector>

#include "boost/thread/mutex.hpp"

#include "cache/FrozeVolume.h"

/*
	etf����ʱ��ֻETF�ۼ��깺������ܶ������ͳ�ƻ�����
	*/
class ETF_TradeVolume_Basket
{
	//
	// Members
	//
protected:

	// access mutex
	boost::mutex m_mutex;

	//�ۼ��깺�ܶ����� CreationLimit N18(2) �����ۼƿ��깺�Ļ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	uint64_t m_ui64CreationLimit;
	FrozeVolume m_CreationLimit;

	//�ۼ�����ܶ����� RedemptionLimit N18(2) �����ۼƿ���صĻ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ� Ŀǰֻ��Ϊ����
	uint64_t m_ui64RedemptionLimit;
	FrozeVolume m_RedemptionLimit;

	//�����˻��ۼ��깺�ܶ����� CreationLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ��깺�Ļ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ���������˻��ۼ�����ܶ�����
	uint64_t m_ui64CreationLimitPerUser;
	std::map<std::string, FrozeVolume> m_CreationLimitPerUser;

	//�����˻��ۼ�����ܶ����� RedemptionLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ���صĻ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	uint64_t m_ui64RedemptionLimitPerUser;
	std::map<std::string, FrozeVolume> m_RedemptionLimitPerUser;

	//���깺�ܶ����� NetCreationLimit N18(2) ���쾻�깺�Ļ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	uint64_t m_ui64NetCreationLimit;
	FrozeVolume m_NetCreationLimit;

	//������ܶ����� NetRedemptionLimit N18(2) ���쾻��صĻ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	uint64_t m_ui64NetRedemptionLimit;
	FrozeVolume m_NetRedemptionLimit;

	//�����˻����깺�ܶ����� NetCreationLimitPerUser N18(2) ����֤ȯ�˻����쾻�깺�Ļ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	uint64_t m_ui64NetCreationLimitPerUser;
	std::map<std::string, FrozeVolume> m_NetCreationLimitPerUser;

	//�����˻�������ܶ����� NetRedemptionLimitPerUser N18(2) ����֤ȯ�˻����쾻��صĻ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	uint64_t m_ui64NetRedemptionLimitPerUser;
	std::map<std::string, FrozeVolume> m_NetRedemptionLimitPerUser;

	//
	// Functions
	//
public:
	ETF_TradeVolume_Basket(void) :
		m_ui64CreationLimit(0), m_CreationLimit(0),
		m_ui64RedemptionLimit(0), m_RedemptionLimit(0),
		m_ui64CreationLimitPerUser(0),
		m_ui64RedemptionLimitPerUser(0),
		m_ui64NetCreationLimit(0), m_NetCreationLimit(0),
		m_ui64NetRedemptionLimit(0), m_NetRedemptionLimit(0),
		m_ui64NetCreationLimitPerUser(0),
		m_ui64NetRedemptionLimitPerUser(0)
	{
	}

	ETF_TradeVolume_Basket(const uint64_t in_CreationLimit, const uint64_t in_RedemptionLimit,
		const uint64_t in_CreationLimitPerUser, const uint64_t in_RedemptionLimitPerUser,
		const uint64_t in_NetCreationLimit, const uint64_t in_NetRedemptionLimit,
		const uint64_t in_NetCreationLimitPerUser, const uint64_t  in_NetRedemptionLimitPerUser)
		:m_ui64CreationLimit(in_CreationLimit), m_CreationLimit(in_CreationLimit),
		m_ui64RedemptionLimit(in_RedemptionLimit), m_RedemptionLimit(in_RedemptionLimit),
		m_ui64CreationLimitPerUser(in_CreationLimitPerUser),
		m_ui64RedemptionLimitPerUser(in_RedemptionLimitPerUser),
		m_ui64NetCreationLimit(in_NetCreationLimit), m_NetCreationLimit(in_NetCreationLimit),
		m_ui64NetRedemptionLimit(in_NetRedemptionLimit), m_NetRedemptionLimit(in_NetRedemptionLimit),
		m_ui64NetCreationLimitPerUser(in_NetCreationLimitPerUser),
		m_ui64NetRedemptionLimitPerUser(in_NetRedemptionLimitPerUser)
	{
	}

	ETF_TradeVolume_Basket(const ETF_TradeVolume_Basket & src)
		:m_ui64CreationLimit(src.m_ui64CreationLimit), m_CreationLimit(src.m_CreationLimit),
		m_ui64RedemptionLimit(src.m_ui64RedemptionLimit), m_RedemptionLimit(src.m_RedemptionLimit),
		m_ui64CreationLimitPerUser(src.m_ui64CreationLimitPerUser),
		m_ui64RedemptionLimitPerUser(src.m_ui64RedemptionLimitPerUser),
		m_ui64NetCreationLimit(src.m_ui64NetCreationLimit), m_NetCreationLimit(src.m_NetCreationLimit),
		m_ui64NetRedemptionLimit(src.m_ui64NetRedemptionLimit),
		m_NetRedemptionLimit(src.m_NetRedemptionLimit),
		m_ui64NetCreationLimitPerUser(src.m_ui64NetCreationLimitPerUser),
		m_ui64NetRedemptionLimitPerUser(src.m_ui64NetRedemptionLimitPerUser)
	{
	}
	
	~ETF_TradeVolume_Basket()
	{
	}

	/*
	�����깺���׶�

	Return:
	0 -- δ����������ƶ�ȣ�����ɹ�
	-1 -- ����ʧ��
	*/
	int FrozeCreation(const uint64_t in_num, const std::string& in_sCustId);

	/*
	��������깺���׶�

	Return:
	0 -- �������ɹ�
	-1 -- �������ʧ��
	*/
	int DefrozeCreation(const uint64_t in_num, const std::string& in_sCustId);

	/*
	ȷ���깺���׶�

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	int ConfirmCreation(const uint64_t in_num, const std::string& in_sCustId);

	/*
	������ؽ��׶�

	Return:
	0 -- δ����������ƶ�ȣ�����ɹ�
	-1 -- ����ʧ��
	*/
	int FrozeRedemption(const uint64_t in_num, const std::string& in_sCustId);

	/*
	���������ؽ��׶�

	Return:
	0 -- �������ɹ�
	-1 -- ����ʧ��
	*/
	int DefrozeRedemption(const uint64_t in_num, const std::string& in_sCustId);

	/*
	ȷ����ؽ��׶�

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	int ConfirmRedemption(const uint64_t in_num, const std::string& in_sCustId);

private:
	// ��ֹʹ��
	ETF_TradeVolume_Basket& operator =(const ETF_TradeVolume_Basket& src);
};

#endif
