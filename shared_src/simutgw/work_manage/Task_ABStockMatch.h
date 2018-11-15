#ifndef __TASK_A_B_STOCK_MATCH_H__
#define __TASK_A_B_STOCK_MATCH_H__

#include <memory>

#include "simutgw_config/config_define.h"
#include "order/define_order_msg.h"

#include "simutgw/biz/MatchUtil.h"
#include "thread_pool_priority/TaskPriorityBase.h"

/*
使用内存进行模拟撮合交易的处理
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
			// 成交模式——0:实盘模拟
			if (simutgw::DECLARATION_TYPE::deltype_ordinary_limit == in_msg->enDelType)
			{
				// 限价要排序
				//m_bIsMatchOrdered = true;
			}

			m_bIsMatchOrdered = false;
			break;

		default:
			// 模拟交易不要排序
			m_bIsMatchOrdered = false;
		}
	}

	int GetOrderMsg(std::shared_ptr<struct simutgw::OrderMessage>& in_msg)
	{
		in_msg = m_orderMsg;

		return 0;
	}

	/*
	处理回调
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
	交易成交函数
	Return:
	0 -- 成交
	-1 -- 失败
	*/
	int MatchOrder();

	/*
		处理启用行情的交易

		Return:
		0 -- 成功
		-1 -- 失败
		*/
	int ABStockEnableQuoMatch(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	处理实盘模拟交易

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
	virtual enum simutgw::MatchType Quot_MatchOrder(
		std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	实盘 按照市场、成交方式成交
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
	virtual enum simutgw::MatchType Quot_MatchByMarket(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const simutgw::uint64_t_Money in_ui64mMaxGain, const simutgw::uint64_t_Money in_ui64mMinFall,
		const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
		uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain,
		const string& in_strTpbz);

	/*
		处理委托类型 -- 普通限价

		处理方式：
		按照限价成交，未成交部分挂单
		*/
	int Match_Ordinary_Limit(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
		uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain);

	/*
		处理委托类型 -- 本方最优、对手方最优

		处理方式：
		按照市场均价成交，未成交部分的转成限价挂单
		*/
	int Match_Optimal(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
		uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain);

	/*
	处理委托类型
	--市价立即成交剩余撤销、市价最优五档全额成交剩余撤销

	处理方式：
	按照市场均价成交，未成交部分的撤单
	*/
	int Match_Optimal_And_Remainder_Cancel(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
		uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain);

	/*
	处理委托类型
	--市价全额成交或撤销

	处理方式：
	按照市价全部成交，或者全部撤单
	*/
	int Match_All_Market_Or_Cancel(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
		uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain);

	/*
	处理普通AB股委托
	全部成交
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int ABStockMatch_All(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	处理普通AB股委托
	分笔成交
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int ABStockMatch_Divide(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	处理普通AB股委托
	不成交
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int ABStockMatch_UnMatch(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	处理普通AB股委托
	废单
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int ABStockMatch_Error(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

	/*
	处理普通AB股委托
	部分成交
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int ABStockMatch_Part(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);
};

#endif