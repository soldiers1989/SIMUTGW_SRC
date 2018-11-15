#ifndef __PRE_TRADE_H__
#define __PRE_TRADE_H__

#include "order/define_order_msg.h"

#include "etf/ETFHelper.h"

/*
	����ǰ��ش�����

	���ɷ��Ƿ��������
	���ETF�깺�Ƿ��ޣ��ɷֹ��Ƿ��㹻
	���йɷݶ��ᣬ����ʧ�ܵ����ϵ���������ɹ�����뽻�׶���
*/
class PreTrade
{
public:	
	virtual ~PreTrade();

	/*
		����׼��
		��Ҫ�Ƕ���ɷ�

		Return:
		0 -- �ɹ�
		-1 -- ʧ��
	*/
	static int TradePrep( std::shared_ptr<struct simutgw::OrderMessage>& io_order );

private:
	/*
		ȡ��ǰ�Ķ�������
	*/
	static int GetTradeType( std::shared_ptr<struct simutgw::OrderMessage>& in_order, enum simutgw::TADE_TYPE& io_type );

	/*
		���������ж�ί�еĺϷ���
		Return:
		0 -- �Ϸ�
		-1 -- ���Ϸ�
	*/
	static int CheckOrderByQuotation( std::shared_ptr<struct simutgw::OrderMessage>& io_order );

	/*
	�鿴�Ƿ�ͣ��
	Return:
	0 -- δͣ��
	1 -- ��ͣ��
	*/
	static int CheckTPBZ( std::shared_ptr<struct simutgw::OrderMessage>& io_order, const string& in_strTpbz );

	/*
	�鿴�Ƿ񳬳��ǵ���
	Return:
	0 -- δ��
	1 -- �ѳ�
	*/
	static int Check_MaxGain_And_MinFall( std::shared_ptr<struct simutgw::OrderMessage>& io_order,
		const simutgw::uint64_t_Money in_ui64mMaxGain, const simutgw::uint64_t_Money in_ui64mMinFall);

	/*
		ETF���

		Return:
		0 -- �ɹ�
		-1 -- ʧ��
	*/
	static int ValidateETF( std::shared_ptr<struct simutgw::OrderMessage>& in_order );

	/*
	ETF�깺���ӳɷֹɽ�����Ϣ
	*/
	static int AddCreationComponent( std::shared_ptr<struct simutgw::OrderMessage>& io_order,
		const std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
		ETF������ӳɷֹɽ�����Ϣ
	*/
	static int AddRedeptionComponent( std::shared_ptr<struct simutgw::OrderMessage>& io_order,
		const std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

private:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	PreTrade();
};

#endif