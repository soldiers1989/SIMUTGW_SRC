#ifndef __WORK_COUNTER_NOLOCK_H__
#define __WORK_COUNTER_NOLOCK_H__

#include <string>
#include "order/define_order_msg.h"

/*
����ͳ�ƣ������������ڵ�����������
*/
class WorkCounter_nolock
{
	//
	//	members
	//
protected:
	struct simutgw::ORDER_MATCH_CNT_nolock m_orderMatchCnt;
	std::string m_name;

	//
	//	functions
	//
public:
	WorkCounter_nolock(void)
		:m_name("")
	{

	}

	explicit WorkCounter_nolock(const std::string& sName)
		:m_name(sName)
	{

	}

	virtual ~WorkCounter_nolock(void)
	{

	}

	const std::string& GetName(void)
	{
		return m_name;
	}

	// �����յ�ί�м���
	void Inc_Received(void)
	{
		++(m_orderMatchCnt.ui64_Received_Count);
	}

	// ����ȷ�ϼ���
	void Inc_Confirm(void)
	{
		++(m_orderMatchCnt.ui64_Confirm_Count);
	}

	// ����ȫ���ɽ�����
	void Inc_MatchAll(void)
	{
		++(m_orderMatchCnt.ui64_MatchAll_Count);
	}

	// ���ӷֱʳɽ�����
	void Inc_MatchPart(void)
	{
		++(m_orderMatchCnt.ui64_MatchPart_Count);
	}

	// ���ӳ�������
	void Inc_MatchCancel(void)
	{
		++(m_orderMatchCnt.ui64_Cancel_Count);
	}

	// ���Ӵ��󵥼���
	void Inc_Error(void)
	{
		++(m_orderMatchCnt.ui64_Error_Count);
	}

	// ȡ�յ�ί�м���
	uint64_t Get_Receive(void) const
	{
		return m_orderMatchCnt.ui64_Received_Count;
	}

	// ȡȫ���ɽ�����
	uint64_t Get_MatchAll(void) const
	{
		return m_orderMatchCnt.ui64_MatchAll_Count;
	}

	// ȡ�ֱʳɽ�����
	uint64_t Get_MatchPart(void) const
	{
		return m_orderMatchCnt.ui64_MatchPart_Count;
	}

	// ȡ��������
	uint64_t Get_MatchCancel(void) const
	{
		return m_orderMatchCnt.ui64_Cancel_Count;
	}

	// ȡ���󵥼���
	uint64_t Get_Error(void) const
	{
		return m_orderMatchCnt.ui64_Error_Count;
	}

	// ȡȷ�ϼ���
	uint64_t Get_Confirm(void) const
	{
		return m_orderMatchCnt.ui64_Confirm_Count;
	}
};

#endif