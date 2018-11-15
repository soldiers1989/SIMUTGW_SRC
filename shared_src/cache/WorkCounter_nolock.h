#ifndef __WORK_COUNTER_NOLOCK_H__
#define __WORK_COUNTER_NOLOCK_H__

#include <string>
#include "order/define_order_msg.h"

/*
订单统计，无锁，适用于单个物理连接
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

	// 增加收到委托计数
	void Inc_Received(void)
	{
		++(m_orderMatchCnt.ui64_Received_Count);
	}

	// 增加确认计数
	void Inc_Confirm(void)
	{
		++(m_orderMatchCnt.ui64_Confirm_Count);
	}

	// 增加全部成交计数
	void Inc_MatchAll(void)
	{
		++(m_orderMatchCnt.ui64_MatchAll_Count);
	}

	// 增加分笔成交计数
	void Inc_MatchPart(void)
	{
		++(m_orderMatchCnt.ui64_MatchPart_Count);
	}

	// 增加撤单计数
	void Inc_MatchCancel(void)
	{
		++(m_orderMatchCnt.ui64_Cancel_Count);
	}

	// 增加错误单计数
	void Inc_Error(void)
	{
		++(m_orderMatchCnt.ui64_Error_Count);
	}

	// 取收到委托计数
	uint64_t Get_Receive(void) const
	{
		return m_orderMatchCnt.ui64_Received_Count;
	}

	// 取全部成交计数
	uint64_t Get_MatchAll(void) const
	{
		return m_orderMatchCnt.ui64_MatchAll_Count;
	}

	// 取分笔成交计数
	uint64_t Get_MatchPart(void) const
	{
		return m_orderMatchCnt.ui64_MatchPart_Count;
	}

	// 取撤单计数
	uint64_t Get_MatchCancel(void) const
	{
		return m_orderMatchCnt.ui64_Cancel_Count;
	}

	// 取错误单计数
	uint64_t Get_Error(void) const
	{
		return m_orderMatchCnt.ui64_Error_Count;
	}

	// 取确认计数
	uint64_t Get_Confirm(void) const
	{
		return m_orderMatchCnt.ui64_Confirm_Count;
	}
};

#endif