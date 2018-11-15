#ifndef __TASK_ETF_MATCH_H__
#define __TASK_ETF_MATCH_H__

#include <memory>

#include "simutgw_config/config_define.h"
#include "order/define_order_msg.h"

#include "simutgw/biz/MatchUtil.h"
#include "thread_pool_priority/TaskPriorityBase.h"

/*
处理Etf交易task
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
	处理回调
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
	交易成交函数
	Return:
	0 -- 成交
	-1 -- 失败
	*/
	virtual int MatchOrder();

	/*
	对ETF委托进行撮合

	Return :
	simutgw::MatchType
	{
	//全部成交
	MatchAll = 0,
	//部分成交
	MatchPart = 1,
	//未成交
	NotMatch = -1,
	// 超出涨跌幅
	OutOfRange = -2,
	// 停牌
	StopTrans = -3,
	// 交易不合法
	ErrorMatch = -4
	};
	*/
	enum simutgw::MatchType TradeMatch( std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
		bool in_bIsBuy );

	/*
	Etf申购交易 在数据库中获取用户的ETF成份股持仓，并且进行缺口比较并完成交易

	Return :
	0 -- 获取成功
	<0 -- 获取失败
	1 -- 余券不足
	2 -- 超过最大现金替代比例
	*/
	int Match_Creat_UserHoldToEtfComponent(
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

	/*
	Etf申购交易 直接按照的ETF成份股完成交易

	Return :
	0 -- 获取成功
	<0 -- 获取失败
	1 -- 余券不足
	2 -- 超过最大现金替代比例
	*/
	int Match_Simul_Creat_EtfComponent(
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

	/*
	Etf赎回交易 在数据库中获取用户的ETF成份股持仓，并且加上赎回的数量并完成交易

	Return :
	0 -- 获取成功
	-1 -- 获取失败
	*/
	int Match_Redemp_UserHoldToEtfComponent(
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

	/*
	Etf赎回交易 直接按照的ETF成份股完成交易

	Return :
	0 -- 获取成功
	<0 -- 获取失败
	1 -- 余券不足
	2 -- 超过最大现金替代比例
	*/
	int Match_Simul_Redemp_EtfComponent(
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
		std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf );

};

#endif