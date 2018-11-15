#ifndef __DEFINE_ORDER_MSG_H__
#define __DEFINE_ORDER_MSG_H__

#include <stdint.h>
#include <atomic>

#include "quickfix/Message.h"

#include "simutgw_config/config_define.h"
#include "config/conf_msg.h"

#include "etf/conf_etf_info.h"

#include "cache/TradePolicyManage.h"

namespace simutgw
{
	enum ORDER_TYPE
	{
		ordtype_buy = 1,
		ordtype_sell = 2,
		ordtype_error = 3,
		ordtype_cancel = 4
	};

	// 订单的交易类型
	enum TADE_TYPE
	{
		// 错误
		error = -1,
		// 撤单
		cancel = 0,
		// A股
		a_trade = 1,
		// B股
		b_trade,
		// 融资交易，融资买入和卖券还款
		margin_cash,
		// 融券交易，融券卖出和买券还券
		margin_stock,
		// ETF买入
		etf_buy,
		// ETF卖出
		etf_sell,
		// ETF申购
		etf_crt,
		// ETF赎回
		etf_rdp
	};

	// 申报类型
	enum DECLARATION_TYPE
	{
		deltype_ordinary_limit = 1,// 限价
		deltype_the_side_optimal = 2, // 本方最优
		deltype_the_oppsite_side_optimaland_remainder_hold = 3, // 对手方最优剩余转限价
		deltype_match_at_market_and_remainder_cancel = 4, // 市价立即成交剩余撤销
		//deltype_all_match_within_limit_or_cancel = 5, // 限价全额成交或撤销
		deltype_all_match_at_market_or_cancel = 6, // 市价全额成交或撤销
		deltype_match_at_market_in_five_and_remainder_cancel = 7, // 市价最优五档全额成交剩余撤销
		deltype_match_at_market_in_five_and_remainder_hold = 8 //市价最优五档全额成交剩余挂单
	};

	/*
		成交量计数
		不带内置锁
		*/
	struct ORDER_MATCH_CNT_nolock
	{
		// 收到委托或撤单
		uint64_t ui64_Received_Count;

		// 全部成交
		uint64_t ui64_MatchAll_Count;

		// 部分成交
		uint64_t ui64_MatchPart_Count;

		// 撤单
		uint64_t ui64_Cancel_Count;

		// 错误单
		uint64_t ui64_Error_Count;

		// 发出的确认数
		uint64_t ui64_Confirm_Count;

		ORDER_MATCH_CNT_nolock()
		{
			ui64_Received_Count = 0;
			ui64_MatchAll_Count = 0;
			ui64_MatchPart_Count = 0;
			ui64_Cancel_Count = 0;
			ui64_Error_Count = 0;
			ui64_Confirm_Count = 0;
		}
	};

	/*
	成交量计数
	原子变量
	*/
	struct ORDER_MATCH_CNT_atomic
	{
		// 收到委托或撤单
		atomic<unsigned long long> aull_Received_Count;

		// 全部成交
		atomic<unsigned long long> aull_MatchAll_Count;

		// 部分成交
		atomic<unsigned long long> aull_MatchPart_Count;

		// 撤单
		atomic<unsigned long long> aull_Cancel_Count;

		// 错误单
		atomic<unsigned long long> aull_Error_Count;

		// 发出的确认数
		atomic<unsigned long long> aull_Confirm_Count;

		ORDER_MATCH_CNT_atomic()
			:aull_Received_Count(0),
			aull_MatchAll_Count(0),
			aull_MatchPart_Count(0),
			aull_Cancel_Count(0),
			aull_Error_Count(0),
			aull_Confirm_Count(0)
		{

		}
	};

	// 交易时用的证券订单变化
	struct TradeStock
	{
		//
		// 证券持有数量，股份余额
		// 用户交易前持仓		
		uint64_t ui64StockBalance_beforematch;
		// 用户交易后持仓
		uint64_t ui64StockBalance_aftermatch;
		//
		// 竞价买入量
		// 交易前
		uint64_t ui64Stock_auction_purchase_beforematch;
		// 交易后
		uint64_t ui64Stock_auction_purchase_aftermatch;
		//
		// 大宗买入量
		// 交易前
		uint64_t ui64Stock_staple_purchase_beforematch;
		// 交易后
		uint64_t ui64Stock_staple_purchase_aftermatch;
		//
		// etf赎回量，可竞价卖出 或 etf申购量，可竞价卖出
		// 交易前
		uint64_t ui64Stock_etfredemption_creation_beforematch;
		// 交易后
		uint64_t ui64Stock_etfredemption_creation_aftermatch;
		//
		// 证券持有可用余额，可用于申购etf份额和可竞价卖出
		// 用户可用持仓 交易前
		uint64_t ui64StockAvailable_beforematch;
		// 用户可用持仓 交易后
		uint64_t ui64StockAvailable_aftermatch;

		// 上次余额
		uint64_t ui64Stock_last_balance;

		TradeStock()
		{
			ui64StockBalance_beforematch = 0;
			ui64StockBalance_aftermatch = 0;

			ui64Stock_auction_purchase_beforematch = 0;
			ui64Stock_auction_purchase_aftermatch = 0;

			ui64Stock_staple_purchase_beforematch = 0;
			ui64Stock_staple_purchase_aftermatch = 0;

			ui64Stock_etfredemption_creation_beforematch = 0;
			ui64Stock_etfredemption_creation_aftermatch = 0;

			ui64StockAvailable_beforematch = 0;
			ui64StockAvailable_aftermatch = 0;

			ui64Stock_last_balance = 0;
		}
	};

	// 交易时用的Etf成份股订单变化
	struct TradeEtfComponent : SzETFComponent
	{
		// 是否持有当前股票 true--是 false--否
		bool bIsUserHolding;

		// 用户的持股状态
		struct TradeStock userHold;

		// 用户是否需要用现金替代 true--是 false--否
		bool bIsNeedCashComponent;

		// 需要现金替代的金额
		uint64_t_Money ui64mCashComponent;

		// 发生交易的股票数量
		uint64_t ui64StockExchange;

		TradeEtfComponent() :SzETFComponent()
		{
			bIsUserHolding = false;

			bIsNeedCashComponent = false;
			ui64mCashComponent = 0;
			ui64StockExchange = 0;
		}
	};

	struct TradeEtf
	{
		// 需要现金替代的金额
		uint64_t_Money ui64mCashComponent;

		// 所有成分股信息
		std::vector<TradeEtfComponent> vecComponents;

		TradeEtf()
		{
			ui64mCashComponent = 0;
		}
	};

	// 普通股票卖出构成
	struct SellCps
	{
		// etf赎回部分
		uint64_t ui64Etf_rdp;
		// 可用余额部分
		uint64_t ui64Avl;

		SellCps()
		{
			ui64Etf_rdp = 0;
			ui64Avl = 0;
		}

		SellCps(const SellCps& orig)
		{
			ui64Etf_rdp = orig.ui64Etf_rdp;
			ui64Avl = orig.ui64Avl;
		}

		SellCps& operator =( const SellCps& orig )
		{
			ui64Etf_rdp = orig.ui64Etf_rdp;
			ui64Avl = orig.ui64Avl;

			return *this;
		}
	};

	// ETF卖出构成
	struct SellETFCps
	{
		// etf申购部分
		uint64_t ui64Etf_crt;
		// 可用余额部分
		uint64_t ui64Avl;

		SellETFCps()
		{
			ui64Etf_crt = 0;
			ui64Avl = 0;
		}
		SellETFCps(const SellETFCps& orig)
		{
			ui64Etf_crt = orig.ui64Etf_crt;
			ui64Avl = orig.ui64Avl;
		}

		SellETFCps& operator =( const SellETFCps& orig )
		{
			ui64Etf_crt = orig.ui64Etf_crt;
			ui64Avl = orig.ui64Avl;

			return *this;
		}
	};

	// ETF赎回构成
	struct RdpETFCps
	{
		// 竞价买入部分
		uint64_t ui64Act_pch;
		// 可用余额部分
		uint64_t ui64Avl;

		RdpETFCps()
		{
			ui64Act_pch = 0;
			ui64Avl = 0;
		}
		RdpETFCps(const RdpETFCps& orig)
		{
			ui64Act_pch = orig.ui64Act_pch;
			ui64Avl = orig.ui64Avl;
		}

		RdpETFCps& operator =( const RdpETFCps& orig )
		{
			ui64Act_pch = orig.ui64Act_pch;
			ui64Avl = orig.ui64Avl;

			return *this;
		}
	};

	// ETF申购冻结成分股的组成/赎回成分股的增加量
	struct EtfCrt_FrozeComponent
	{
		// 证券代码
		std::string strSecurityID;
		// 竞价买入的数量
		uint64_t ui64act_pch_count;
		// 可用余额的数量
		uint64_t ui64avl_count;
		// 替代总金额
		uint64_t_Money ui64Cash;
		// 赎回数量
		uint64_t ui64rdp_count;

		EtfCrt_FrozeComponent()
		{
			ui64act_pch_count = 0;
			ui64avl_count = 0;
			ui64Cash = 0;
			ui64rdp_count = 0;
		}

		EtfCrt_FrozeComponent(const EtfCrt_FrozeComponent& orig)
		{
			ui64act_pch_count = orig.ui64act_pch_count;
			ui64avl_count = orig.ui64avl_count;
			ui64Cash = orig.ui64Cash;
			ui64rdp_count = orig.ui64rdp_count;
		}

		EtfCrt_FrozeComponent& operator =( const EtfCrt_FrozeComponent& orig )
		{
			ui64act_pch_count = orig.ui64act_pch_count;
			ui64avl_count = orig.ui64avl_count;
			ui64Cash = orig.ui64Cash;
			ui64rdp_count = orig.ui64rdp_count;

			return *this;
		}
	};

	/*
	委托消息定义
	*/
	struct OrderMessage
	{
		// error flag
		bool bDataError;
		// 是否持有当前股票 true--是 false--否
		bool bIsUserHoldingCurrentStock;

		// 买卖方向 1/买，2/卖
		int iSide;

		// time_t 委托收到时间
		time_t tRcvTime;

		// 委托类型
		simutgw::DECLARATION_TYPE enDelType;

		// 成交结果类型
		simutgw::MatchType enMatchType;

		// 委托原始数量
		uint64_t ui64Orderqty_origin;
		// 委托剩余数量
		uint64_t ui64LeavesQty;
		// 委托价格
		simutgw::uint64_t_Money ui64mOrderPrice;
		// 委托金额
		//simutgw::uint64_t_Money ui64mCashorderqty;
		// minqty
		uint64_t ui64MinQty;

		// 用户的持股交易状态
		struct TradeStock userHold;

		// Etf的交易状态
		struct TradeEtf etfOrderStatus;

		// 普通股票卖出构成
		struct SellCps sellCps;

		// ETF卖出构成
		struct SellETFCps sellETFCps;

		// ETF赎回构成
		struct RdpETFCps rdpETFCps;

		// ETF冻结的成分股
		vector<std::shared_ptr<struct EtfCrt_FrozeComponent>> vecFrozeComponent;

		//
		// 已成交部分
		// 成交订单数量
		uint64_t ui64Orderqty_matched;
		// 成交价格
		simutgw::uint64_t_Money ui64mPrice_matched;
		// 成交订单金额
		simutgw::uint64_t_Money ui64mCashorderqty_matched;

		//
		// 未成交部分
		// 未成交数量
		uint64_t ui64Orderqty_unmatched;
		// 未成交金额
		simutgw::uint64_t_Money ui64mCashorderqty_unmatched;

		//
		std::string strID;
		//0--深市，1--沪市
		std::string strTrade_market;
		//0--A股，1--B股，2--融资，3--融券
		int iTrade_type;
		// session id
		std::string strSessionId;
		//交易商ID
		std::string strDealerId;
		// 席位
		std::string strSecurity_seat;
		// 证券账号
		std::string strAccountId;
		// 交易圈
		std::string strTrade_group;
		// 订单编号
		std::string strClordid;
		// 
		//std::string strTransId;
		// 委托原始数量
		std::string strOrderqty_origin;
		// 委托价格
		std::string strOrderPrice;
		// 委托金额
		std::string strCashorderqty;
		// beginstring
		std::string strBeginString;

		// msgtype
		std::string strMsgType;

		// 对方ID
		std::string strSenderCompID;
		// 本方ID
		std::string strTargetCompID;

		// applid
		std::string strApplID;

		// ownertype
		std::string strOwnerType;
		// 执行编号
		std::string strExecID;
		// 交易所订单编号
		std::string strOrderID;

		//exectype
		std::string strExecType;
		//ordstatus
		std::string strOrdStatus;

		// leavesqty 剩余数量
		std::string strLeavesQty;

		//cumqty累计成交量
		std::string strCumQty;

		//side买卖方向
		std::string strSide;

		// 证券代码
		std::string strStockID;

		// 证券代码源
		std::string strSecurityIDSource;

		// 组合券代码
		std::string strStockID2;

		// market
		std::string strMarket;

		// stoppx止损价，预留
		std::string strStoppx;

		//timeinforce
		std::string strTimeInForce;

		//positioneffect
		std::string strPositionEffect;

		// ordtype
		std::string strOrdType;

		// 撤单时，原始订单编号
		std::string strOrigClordid;

		// maxpricelevels
		std::string strMaxPriceLevels;

		// minqty
		std::string strMinQty;

		// coveredoruncovered
		std::string strCoveredOrUncovered;
		// confirmid
		std::string strConfirmID;
		// cashmargin
		std::string strCashMargin;

		// market_branchid营业部代码
		std::string strMarket_branchid;

		// 操作时间
		std::string strOper_time;

		// `trade_time` timestamp NULL DEFAULT NULL COMMENT '交易时间,
		std::string strTrade_time;

		// 是否被处理过，0--是，1--否
		std::string strIsProc;
		// error code
		std::string strErrorCode;
		// error comment
		std::string strError;

		//order_record--begin
		// bodylength
		std::string strBodyLength;

		//checksum
		std::string strCheckSum;

		//msgseqnum
		std::string strMsgSeqNum;

		//possdupflag
		std::string strPossDupFlag;

		//sendingtime
		std::string strSendingTime;

		//text
		std::string strText;

		//transacttime
		std::string strTransactTime;

		//OrigSendingTime
		std::string strOrigSendingTime;

		//strNoPartyIds
		std::string strNoPartyIds;

		//strOrderRestrictions
		std::string strOrderRestrictions;

		//strOrdrejReason
		std::string strOrdrejReason;

		//strRejectText
		std::string strRejectText;

		// strCxlRejReason
		std::string strCxlRejReason;

		//order_record--end

		// order--matched begin
		// 成交价
		std::string strLastPx;

		// 成交数量
		std::string strLastQty;
		// order--matched end

		// 当前订单的交易策略
		struct TradePolicyCell tradePolicy;

		// json format message
		rapidjson::Document jsOrder;

		// SZ fix message
		FIX::Message szfixMsg;

		//
		// constructor
		OrderMessage()
		{
			bDataError = false;
			bIsUserHoldingCurrentStock = false;

			iTrade_type = simutgw::TADE_TYPE::error;
			iSide = 0;
			tRcvTime = 0;
			enDelType = simutgw::DECLARATION_TYPE::deltype_all_match_at_market_or_cancel;
			enMatchType = simutgw::MatchType::NotMatch;

			ui64Orderqty_origin = 0;
			ui64LeavesQty = 0;
			ui64mOrderPrice = 0;
			ui64MinQty = 0;
			ui64Orderqty_matched = 0;
			ui64mPrice_matched = 0;
			ui64mCashorderqty_matched = 0;

			ui64Orderqty_unmatched = 0;
			ui64mCashorderqty_unmatched = 0;

			jsOrder.SetObject();
		}

		void copy(const OrderMessage& orig);

		OrderMessage(const OrderMessage& orig)
		{
			copy(orig);
		}

		OrderMessage& operator=( const OrderMessage& orig )
		{
			copy(orig);
			return *this;
		}

		void Print();
	};

};

#endif