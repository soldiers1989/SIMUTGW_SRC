#ifndef __TASK_A_B_STOCK_MATCH_H__
#define __TASK_A_B_STOCK_MATCH_H__

#include <memory>

#include "simutgw_config/config_define.h"
#include "order/define_order_msg.h"

#include "simutgw/biz/MatchUtil.h"
#include "thread_pool_priority/TaskPriorityBase.h"

/*
ʹ���ڴ����ģ���Ͻ��׵Ĵ���
*/
class Task_ABStockMatch : public TaskPriorityBase
{
	//
	// Members
	//
private:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	std::shared_ptr<struct simutgw::OrderMessage> m_orderMsg;

	//
	// Functions
	//
public:
	explicit Task_ABStockMatch(const uint64_t uiId) :
		TaskPriorityBase(uiId, QueueTask::Level_Match_Order),
		m_scl(keywords::channel = "Task_ABStockMatch")
	{
	}

	void SetOrderMsg(std::shared_ptr<struct simutgw::OrderMessage>& in_msg)
	{
		m_orderMsg = std::shared_ptr<struct simutgw::OrderMessage>(new struct simutgw::OrderMessage(*in_msg));

		m_iKey = TaskPriorityBase::GenerateKey(in_msg->strStockID, in_msg->strSide, true);

		m_strSide = in_msg->strSide;
		m_ui64Price = in_msg->ui64mOrderPrice;
		m_strClordid = in_msg->strClordid;
		m_tOrderTime = in_msg->tRcvTime;

		switch (in_msg->tradePolicy.iMatchMode)
		{
		case simutgw::SysMatchMode::EnAbleQuta:
			// �ɽ�ģʽ����0:ʵ��ģ��
			if (simutgw::DECLARATION_TYPE::deltype_ordinary_limit == in_msg->enDelType)
			{
				// �޼�Ҫ����
				//m_bIsMatchOrdered = true;
			}

			m_bIsMatchOrdered = false;
			break;

		default:
			// ģ�⽻�ײ�Ҫ����
			m_bIsMatchOrdered = false;
		}
	}

	int GetOrderMsg(std::shared_ptr<struct simutgw::OrderMessage>& in_msg)
	{
		in_msg = m_orderMsg;

		return 0;
	}

	/*
	����ص�
	*/
	virtual int TaskProc(void)
	{
		int iRes = MatchOrder();
		return iRes;
	}

	virtual ~Task_ABStockMatch(void)
	{
	}

protected:
	/*
	���׳ɽ�����
	Return:
	0 -- �ɽ�
	-1 -- ʧ��
	*/
	int MatchOrder();

	/*
		������������Ľ���

		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	int ABStockEnableQuoMatch(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	����ʵ��ģ�⽻��

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
	virtual enum simutgw::MatchType Quot_MatchOrder(
		std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	ʵ�� �����г����ɽ���ʽ�ɽ�
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
	virtual enum simutgw::MatchType Quot_MatchByMarket(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const simutgw::uint64_t_Money in_ui64mMaxGain, const simutgw::uint64_t_Money in_ui64mMinFall,
		const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
		uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain,
		const string& in_strTpbz);

	/*
		����ί������ -- ��ͨ�޼�

		����ʽ��
		�����޼۳ɽ���δ�ɽ����ֹҵ�
		*/
	int Match_Ordinary_Limit(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
		uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain);

	/*
		����ί������ -- �������š����ַ�����

		����ʽ��
		�����г����۳ɽ���δ�ɽ����ֵ�ת���޼۹ҵ�
		*/
	int Match_Optimal(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
		uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain);

	/*
	����ί������
	--�м������ɽ�ʣ�೷�����м������嵵ȫ��ɽ�ʣ�೷��

	����ʽ��
	�����г����۳ɽ���δ�ɽ����ֵĳ���
	*/
	int Match_Optimal_And_Remainder_Cancel(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
		uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain);

	/*
	����ί������
	--�м�ȫ��ɽ�����

	����ʽ��
	�����м�ȫ���ɽ�������ȫ������
	*/
	int Match_All_Market_Or_Cancel(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
		uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain);

	/*
	������ͨAB��ί��
	ȫ���ɽ�
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int ABStockMatch_All(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	������ͨAB��ί��
	�ֱʳɽ�
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int ABStockMatch_Divide(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	������ͨAB��ί��
	���ɽ�
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ABStockMatch_UnMatch(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	������ͨAB��ί��
	�ϵ�
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int ABStockMatch_Error(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	������ͨAB��ί��
	���ֳɽ�
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int ABStockMatch_Part(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);
};

#endif