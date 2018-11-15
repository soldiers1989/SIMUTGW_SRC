#ifndef __MARKET_INFO_HELPER_H__
#define __MARKET_INFO_HELPER_H__

#include <string>
#include <memory>

#include "config/conf_msg.h"
#include "order/define_order_msg.h"

class MarketInfoHelper
{
	//
	// Members
	//

	//
	// Functions
	//
public:
	/*
	��ȡ��ǰ�����ʵʱ����
	Return:
	0 -- ��ȡ�ɹ�
	��0 -- ��ȡʧ��
	*/
	static int GetCurrentQuotationByStockId(const std::string& in_cstrZqdm,
		uint64_t& out_ui64Cjsl, uint64_t& out_ui64Cjje, std::string& out_strHqsj);

	/*
	��ȡ��ǰ����Ľ���Ȧʵʱ��������
	�����ǰ����Ȧ����û�У���ȡ�������ģ����������Ҳû�У��򷵻�ʧ��
	Return:
	0 -- ��ȡ�ɹ�
	��0 -- ��ȡʧ��
	*/
	static int GetCurrQuotGapByCircle(const std::string& in_cstrZqdm,
		const std::string& in_strSide, const std::string& in_cstrTradeCircle,
		simutgw::QuotationType& in_quotType,
		simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
		uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje, std::string& out_strHqsj,
		std::string& out_strTpbz);

	/*
	���浱ǰ����Ľ���Ȧʵʱ��������
	Return:
	0 -- ����ɹ�
	��0 -- ����ʧ��
	*/
	static int SetCurrQuotGapByCircle(const std::string& in_cstrZqdm, const std::string& in_cstrTradeCircle,
		const uint64_t& in_cui64Cjsl, const uint64_t& in_cui64Cjje, const std::string& in_cstrHqsj);

	/*
	���浱ǰ����Ľ���Ȧʵʱ��������
	Return:
	0 -- ����ɹ�
	��0 -- ����ʧ��
	*/
	static int SetCurrQuotGapByCircle(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg,
		const std::string& in_cstrTradeCircle, const uint64_t& in_cui64Cjsl,
		const uint64_t& in_cui64Cjje, const std::string& in_cstrHqsj);

	/*
	��ȡ��ǰ����Ľ���Ȧʵʱ��������
	�����ǰ����Ȧ����û�У���ȡ�������ģ����������Ҳû�У��򷵻�ʧ��

	����ɽ���

	Return:
	0 -- ��ȡ�ɹ�
	��0 -- ��ȡʧ��
	*/
	static int GetCurrQuotGapByCircle_RecentPrice(const std::string& in_cstrZqdm, const std::string& in_cstrTradeCircle,
		simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
		uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje, std::string& out_strHqsj,
		std::string& out_strTpbz);

	/*
	��ѯ��ǰ�ľ�̬������Ϣ
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int GetCurrStaticQuot(const std::string& in_cstrZqdm, uint64_t& out_ui64Zrsp, std::string& out_strTPBZ);

protected:

	/*
	��ȡ��ǰ����Ľ���Ȧʵʱ��������
	�����ǰ����Ȧ����û�У���ȡ�������ģ����������Ҳû�У��򷵻�ʧ��
	ȡ�ɽ������ͳɽ����
	Return:
	0 -- ��ȡ�ɹ�
	��0 -- ��ȡʧ��
	*/
	static int GetCurrQuotGapByCircle(const std::string& in_cstrZqdm, const std::string& in_cstrTradeCircle,
		simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
		uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje, std::string& out_strHqsj,
		std::string& out_strTpbz);

	/*
	��ȡ��ǰ����Ľ���Ȧʵʱ��������
	�����ǰ����Ȧ����û�У���ȡ�������ģ����������Ҳû�У��򷵻�ʧ��

	����ξ���

	Return:
	0 -- ��ȡ�ɹ�
	��0 -- ��ȡʧ��
	*/
	static int GetCurrQuotGapByCircle_AveragePrice(const std::string& in_cstrZqdm, const std::string& in_cstrTradeCircle,
		simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
		uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje,	std::string& out_strHqsj,
		std::string& out_strTpbz);

	/*
	��ȡ��ǰ����Ľ���Ȧʵʱ��������
	�����ǰ����Ȧ����û�У���ȡ�������ģ����������Ҳû�У��򷵻�ʧ��

	��һ��һ

	Return:
	0 -- ��ȡ�ɹ�
	��0 -- ��ȡʧ��
	*/
	static int GetCurrQuotGapByCircle_SellBuyPrice(const std::string& in_cstrZqdm, 
		const std::string& in_strSide, const std::string& in_cstrTradeCircle,
		simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
		uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje,	std::string& out_strHqsj,
		std::string& out_strTpbz);

private:
	// ��ֹ��ʹ��
	MarketInfoHelper(void);
	virtual ~MarketInfoHelper(void);
};

#endif