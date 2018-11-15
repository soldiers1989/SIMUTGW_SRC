#include "define_order_msg.h"
#include "util/EzLog.h"

namespace simutgw
{
	void OrderMessage::copy(const OrderMessage& orig)
	{
		// error flag
		bDataError = orig.bDataError;
		// 是否持有当前股票 true--是 false--否
		bIsUserHoldingCurrentStock = orig.bIsUserHoldingCurrentStock;

		// 买卖方向 1/买，2/卖
		iSide = orig.iSide;

		// time_t 委托收到时间
		tRcvTime = orig.tRcvTime;

		// 委托类型
		enDelType = orig.enDelType;

		// 成交结果类型
		enMatchType = orig.enMatchType;

		// 委托原始数量
		ui64Orderqty_origin = orig.ui64Orderqty_origin;
		// 委托剩余数量
		ui64LeavesQty = orig.ui64LeavesQty;
		// 委托价格
		ui64mOrderPrice = orig.ui64mOrderPrice;
		// 委托金额
		//simutgw::uint64_t_Money ui64mCashorderqty = orig.ui64mCashorderqty;
		// minqty
		ui64MinQty = orig.ui64MinQty;

		// 用户的持股交易状态
		userHold = orig.userHold;

		// Etf的交易状态
		etfOrderStatus = orig.etfOrderStatus;

		// 普通股票卖出构成
		sellCps = orig.sellCps;

		// ETF卖出构成
		sellETFCps = orig.sellETFCps;

		// ETF赎回构成
		rdpETFCps = orig.rdpETFCps;

		// ETF冻结的成分股
		vecFrozeComponent = orig.vecFrozeComponent;

		//
		// 已成交部分
		// 成交订单数量
		ui64Orderqty_matched = orig.ui64Orderqty_matched;
		// 成交价格
		ui64mPrice_matched = orig.ui64mPrice_matched;
		// 成交订单金额
		ui64mCashorderqty_matched = orig.ui64mCashorderqty_matched;

		//
		// 未成交部分
		// 未成交数量
		ui64Orderqty_unmatched = orig.ui64Orderqty_unmatched;
		// 未成交金额
		ui64mCashorderqty_unmatched = orig.ui64mCashorderqty_unmatched;

		//
		strID = orig.strID;
		//0--深市，1--沪市
		strTrade_market = orig.strTrade_market;
		//0--A股，1--B股，2--融资，3--融券
		iTrade_type = orig.iTrade_type;
		// session id
		strSessionId = orig.strSessionId;
		//交易商ID
		strDealerId = orig.strDealerId;
		// 席位
		strSecurity_seat = orig.strSecurity_seat;
		// 证券账号
		strAccountId = orig.strAccountId;
		// 交易圈
		strTrade_group = orig.strTrade_group;
		// 订单编号
		strClordid = orig.strClordid;
		// 
		//strTransId = orig.bIsUserHoldingCurrentStock;
		// 委托原始数量
		strOrderqty_origin = orig.strOrderqty_origin;
		// 委托价格
		strOrderPrice = orig.strOrderPrice;
		// 委托金额
		strCashorderqty = orig.strCashorderqty;
		// beginstring
		strBeginString = orig.strBeginString;

		// msgtype
		strMsgType = orig.strMsgType;

		// 对方ID
		strSenderCompID = orig.strSenderCompID;
		// 本方ID
		strTargetCompID = orig.strTargetCompID;

		// applid
		strApplID = orig.strApplID;

		// ownertype
		strOwnerType = orig.strOwnerType;
		// 执行编号
		strExecID = orig.strExecID;
		// 交易所订单编号
		strOrderID = orig.strOrderID;

		//exectype
		strExecType = orig.strExecType;
		//ordstatus
		strOrdStatus = orig.strOrdStatus;

		// leavesqty 剩余数量
		strLeavesQty = orig.strLeavesQty;

		//cumqty累计成交量
		strCumQty = orig.strCumQty;

		//side买卖方向
		strSide = orig.strSide;

		// 证券代码
		strStockID = orig.strStockID;

		// 证券代码源
		strSecurityIDSource = orig.strSecurityIDSource;

		// market
		strMarket = orig.strMarket;

		// stoppx止损价，预留
		strStoppx = orig.strStoppx;

		//timeinforce
		strTimeInForce = orig.strTimeInForce;

		//positioneffect
		strPositionEffect = orig.strPositionEffect;

		// ordtype
		strOrdType = orig.strOrdType;

		// 撤单时，原始订单编号
		strOrigClordid = orig.strOrigClordid;

		// maxpricelevels
		strMaxPriceLevels = orig.strMaxPriceLevels;

		// minqty
		strMinQty = orig.strMinQty;

		// coveredoruncovered
		strCoveredOrUncovered = orig.strCoveredOrUncovered;
		// confirmid
		strConfirmID = orig.strConfirmID;
		// cashmargin
		strCashMargin = orig.strCashMargin;

		// market_branchid营业部代码
		strMarket_branchid = orig.strMarket_branchid;

		// 操作时间
		strOper_time = orig.strOper_time;

		// `trade_time` timestamp NULL DEFAULT NULL COMMENT '交易时间,
		strTrade_time = orig.strTrade_time;

		// 是否被处理过，0--是，1--否
		strIsProc = orig.strIsProc;
		// error code
		strErrorCode = orig.strErrorCode;
		// error comment
		strError = orig.strError;

		//order_record--begin
		// bodylength
		strBodyLength = orig.strBodyLength;

		//checksum
		strCheckSum = orig.strCheckSum;

		//msgseqnum
		strMsgSeqNum = orig.strMsgSeqNum;

		//possdupflag
		strPossDupFlag = orig.strPossDupFlag;

		//sendingtime
		strSendingTime = orig.strSendingTime;

		//text
		strText = orig.strText;

		//transacttime
		strTransactTime = orig.strTransactTime;

		//OrigSendingTime
		strOrigSendingTime = orig.strOrigSendingTime;

		//strNoPartyIds
		strNoPartyIds = orig.strNoPartyIds;

		//strOrderRestrictions
		strOrderRestrictions = orig.strOrderRestrictions;

		//strOrdrejReason
		strOrdrejReason = orig.strOrdrejReason;

		//strRejectText
		strRejectText = orig.strRejectText;

		// strCxlRejReason
		strCxlRejReason = orig.strCxlRejReason;

		//order_record--end

		// order--matched begin
		// 成交价
		strLastPx = orig.strLastPx;

		// 成交数量
		strLastQty = orig.strLastQty;
		// order--matched end

		// 当前订单的交易策略
		tradePolicy = orig.tradePolicy;

		// json format message
		jsOrder.CopyFrom(orig.jsOrder, jsOrder.GetAllocator());

		// SZ fix message
		szfixMsg = orig.szfixMsg;
	}

	void OrderMessage::Print()
	{
		static const std::string strTag("OrderMessage::Print() ");
		
		{
			std::string strMsg, strTrans;

			strMsg += " strTrade_market:";
			strMsg += strTrade_market;
			strMsg += " strSessionId:";
			strMsg += strSessionId;
			strMsg += " strClordid:";
			strMsg += strClordid;
			strMsg += " strStockID:";
			strMsg += strStockID;
			strMsg += " strOrderqty_origin:";
			strMsg += strOrderqty_origin;

			strMsg += " strOrderPrice:";
			strMsg += strOrderPrice;
			strMsg += " strLeavesQty:";
			strMsg += strLeavesQty;
			strMsg += " strCumQty:";
			strMsg += strCumQty;
			strMsg += " strLastPx:";
			strMsg += strLastPx;
			strMsg += " strLastQty:";
			strMsg += strLastQty;

			strMsg += " ui64Orderqty_origin:";
			strMsg += sof_string::itostr(ui64Orderqty_origin, strTrans);
			strMsg += " ui64mOrderPrice:";
			strMsg += sof_string::itostr(ui64mOrderPrice, strTrans);
			strMsg += " ui64LeavesQty:";
			strMsg += sof_string::itostr(ui64LeavesQty, strTrans);
			strMsg += " ui64Orderqty_matched:";
			strMsg += sof_string::itostr(ui64Orderqty_matched, strTrans);
			strMsg += " ui64mPrice_matched:";
			strMsg += sof_string::itostr(ui64mPrice_matched, strTrans);

			EzLog::Out(strTag, trivial::trace, strMsg);
		}
	}
}
