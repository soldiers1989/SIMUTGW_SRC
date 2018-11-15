#ifndef __WORK_COUNTER_ATOMIC_H__
#define __WORK_COUNTER_ATOMIC_H__

#include <stdint.h>
#include <string>
#include "order/define_order_msg.h"

/*
订单统计，原子锁，适用于全局
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

	// 增加收到委托计数
	void Inc_Received(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_Received_Count, 1ULL);
	}

	// 增加确认计数
	void Inc_Confirm(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_Confirm_Count, 1ULL);
	}

	// 增加全部成交计数
	void Inc_MatchAll(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_MatchAll_Count, 1ULL);
	}

	// 增加分笔成交计数
	void Inc_MatchPart(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_MatchPart_Count, 1ULL);
	}

	// 增加撤单计数
	void Inc_MatchCancel(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_Cancel_Count, 1ULL);
	}

	// 增加错误单计数
	void Inc_Error(void)
	{
		std::atomic_fetch_add(&m_orderMatchCnt.aull_Error_Count, 1ULL);
	}

	// 取收到委托计数
	uint64_t Get_Receive(void) const
	{
		return m_orderMatchCnt.aull_Received_Count.load();
	}

	// 取全部成交计数
	uint64_t Get_MatchAll(void) const
	{
		return m_orderMatchCnt.aull_MatchAll_Count.load();
	}

	// 取分笔成交计数
	uint64_t Get_MatchPart(void) const
	{
		return m_orderMatchCnt.aull_MatchPart_Count.load();
	}

	// 取撤单计数
	uint64_t Get_MatchCancel(void) const
	{
		return m_orderMatchCnt.aull_Cancel_Count.load();
	}

	// 取错误单计数
	uint64_t Get_Error(void) const
	{
		return m_orderMatchCnt.aull_Error_Count.load();
	}

	// 取确认计数
	uint64_t Get_Confirm(void) const
	{
		return m_orderMatchCnt.aull_Confirm_Count.load();
	}
};

#endif