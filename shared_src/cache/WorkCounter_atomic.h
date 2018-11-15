#ifndef __WORK_COUNTER_ATOMIC_H__
#define __WORK_COUNTER_ATOMIC_H__

#include <stdint.h>
#include <string>
#include "order/define_order_msg.h"

/*
����ͳ�ƣ�ԭ������������ȫ��
*/
class WorkCounter_atomic
{
	//
	//	members
	//
protected:
	struct simutgw::ORDER_MATCH_CNT_atomic m_orderMatchCnt;
	std::string m_name;

	//
	//	functions
	//
public:
	WorkCounter_atomic(void)
		:m_name("")
	{

	}

	explicit WorkCounter_atomic(const std::string& sName)
		:m_name(sName)
	{

	}

	virtual ~WorkCounter_atomic(void)
	{

	}

	const std::string& GetName(void)
	{
		return m_name;
	}

	// �����յ�ί�м���
	void Inc_Received(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_Received_Count, 1ULL);
	}

	// ����ȷ�ϼ���
	void Inc_Confirm(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_Confirm_Count, 1ULL);
	}

	// ����ȫ���ɽ�����
	void Inc_MatchAll(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_MatchAll_Count, 1ULL);
	}

	// ���ӷֱʳɽ�����
	void Inc_MatchPart(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_MatchPart_Count, 1ULL);
	}

	// ���ӳ�������
	void Inc_MatchCancel(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_Cancel_Count, 1ULL);
	}

	// ���Ӵ��󵥼���
	void Inc_Error(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_Error_Count, 1ULL);
	}

	// ȡ�յ�ί�м���
	uint64_t Get_Receive(void) const
	{
		return m_orderMatchCnt.aull_Received_Count.load();
	}

	// ȡȫ���ɽ�����
	uint64_t Get_MatchAll(void) const
	{
		return m_orderMatchCnt.aull_MatchAll_Count.load();
	}

	// ȡ�ֱʳɽ�����
	uint64_t Get_MatchPart(void) const
	{
		return m_orderMatchCnt.aull_MatchPart_Count.load();
	}

	// ȡ��������
	uint64_t Get_MatchCancel(void) const
	{
		return m_orderMatchCnt.aull_Cancel_Count.load();
	}

	// ȡ���󵥼���
	uint64_t Get_Error(void) const
	{
		return m_orderMatchCnt.aull_Error_Count.load();
	}

	// ȡȷ�ϼ���
	uint64_t Get_Confirm(void) const
	{
		return m_orderMatchCnt.aull_Confirm_Count.load();
	}
};

#endif