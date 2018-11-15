#ifndef __TASK_ETF_MATCH_H__
#define __TASK_ETF_MATCH_H__

#include <memory>

#include "simutgw_config/config_define.h"
#include "order/define_order_msg.h"

#include "simutgw/biz/MatchUtil.h"
#include "thread_pool_priority/TaskPriorityBase.h"

/*
����Etf����task
*/
class Task_Etf_Match : public TaskPriorityBase
{
	//
	// member
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;
	
	std::shared_ptr<struct simutgw::OrderMessage> m_orderMsg;

	//
	// function
	//
public:
	explicit Task_Etf_Match(const uint64_t uiId) :
		TaskPriorityBase(uiId, QueueTask::Level_Match_Order),
		m_scl(keywords::channel = "Task_Etf_Match")
	{
	}

	void SetOrderMsg(std::shared_ptr<struct simutgw::OrderMessage>& in_msg)
	{
		m_orderMsg = std::shared_ptr<struct simutgw::OrderMessage>(new struct simutgw::OrderMessage(*in_msg));

		m_iKey = TaskPriorityBase::GenerateKey( in_msg->strStockID, in_msg->strSide, false );

		m_strSide = in_msg->strSide;
		m_ui64Price = in_msg->ui64mOrderPrice;
		m_strClordid = in_msg->strClordid;
		m_tOrderTime = in_msg->tRcvTime;
	}

	/*
	����ص�
	*/
	virtual int TaskProc(void)
	{
		int iRes = MatchOrder();
		return iRes;
	}

	virtual ~Task_Etf_Match(void)
	{
	}

protected:
	/*
	���׳ɽ�����
	Return:
	0 -- �ɽ�
	-1 -- ʧ��
	*/
	virtual int MatchOrder();

	/*
	��ETFί�н��д��

	Return :
	simutgw::MatchType
	{
	//ȫ���ɽ�
	MatchAll = 0,
	//���ֳɽ�
	MatchPart = 1,
	//δ�ɽ�
	NotMatch = -1,
	// �����ǵ���
	OutOfRange = -2,
	// ͣ��
	StopTrans = -3,
	// ���ײ��Ϸ�
	ErrorMatch = -4
	};
	*/
	enum simutgw::MatchType TradeMatch( std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
		bool in_bIsBuy );

	/*
	Etf�깺���� �����ݿ��л�ȡ�û���ETF�ɷݹɳֲ֣����ҽ���ȱ�ڱȽϲ���ɽ���

	Return :
	0 -- ��ȡ�ɹ�
	<0 -- ��ȡʧ��
	1 -- ��ȯ����
	2 -- ��������ֽ��������
	*/
	int Match_Creat_UserHoldToEtfComponent(
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

	/*
	Etf�깺���� ֱ�Ӱ��յ�ETF�ɷݹ���ɽ���

	Return :
	0 -- ��ȡ�ɹ�
	<0 -- ��ȡʧ��
	1 -- ��ȯ����
	2 -- ��������ֽ��������
	*/
	int Match_Simul_Creat_EtfComponent(
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

	/*
	Etf��ؽ��� �����ݿ��л�ȡ�û���ETF�ɷݹɳֲ֣����Ҽ�����ص���������ɽ���

	Return :
	0 -- ��ȡ�ɹ�
	-1 -- ��ȡʧ��
	*/
	int Match_Redemp_UserHoldToEtfComponent(
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

	/*
	Etf��ؽ��� ֱ�Ӱ��յ�ETF�ɷݹ���ɽ���

	Return :
	0 -- ��ȡ�ɹ�
	<0 -- ��ȡʧ��
	1 -- ��ȯ����
	2 -- ��������ֽ��������
	*/
	int Match_Simul_Redemp_EtfComponent(
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

};

#endif