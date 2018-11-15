#include "define_order_msg.h"
#include "util/EzLog.h"

namespace simutgw
{
	void OrderMessage::copy(const OrderMessage& orig)
	{
		// error flag
		bDataError = orig.bDataError;
		// �Ƿ���е�ǰ��Ʊ true--�� false--��
		bIsUserHoldingCurrentStock = orig.bIsUserHoldingCurrentStock;

		// �������� 1/��2/��
		iSide = orig.iSide;

		// time_t ί���յ�ʱ��
		tRcvTime = orig.tRcvTime;

		// ί������
		enDelType = orig.enDelType;

		// �ɽ��������
		enMatchType = orig.enMatchType;

		// ί��ԭʼ����
		ui64Orderqty_origin = orig.ui64Orderqty_origin;
		// ί��ʣ������
		ui64LeavesQty = orig.ui64LeavesQty;
		// ί�м۸�
		ui64mOrderPrice = orig.ui64mOrderPrice;
		// ί�н��
		//simutgw::uint64_t_Money ui64mCashorderqty = orig.ui64mCashorderqty;
		// minqty
		ui64MinQty = orig.ui64MinQty;

		// �û��ĳֹɽ���״̬
		userHold = orig.userHold;

		// Etf�Ľ���״̬
		etfOrderStatus = orig.etfOrderStatus;

		// ��ͨ��Ʊ��������
		sellCps = orig.sellCps;

		// ETF��������
		sellETFCps = orig.sellETFCps;

		// ETF��ع���
		rdpETFCps = orig.rdpETFCps;

		// ETF����ĳɷֹ�
		vecFrozeComponent = orig.vecFrozeComponent;

		//
		// �ѳɽ�����
		// �ɽ���������
		ui64Orderqty_matched = orig.ui64Orderqty_matched;
		// �ɽ��۸�
		ui64mPrice_matched = orig.ui64mPrice_matched;
		// �ɽ��������
		ui64mCashorderqty_matched = orig.ui64mCashorderqty_matched;

		//
		// δ�ɽ�����
		// δ�ɽ�����
		ui64Orderqty_unmatched = orig.ui64Orderqty_unmatched;
		// δ�ɽ����
		ui64mCashorderqty_unmatched = orig.ui64mCashorderqty_unmatched;

		//
		strID = orig.strID;
		//0--���У�1--����
		strTrade_market = orig.strTrade_market;
		//0--A�ɣ�1--B�ɣ�2--���ʣ�3--��ȯ
		iTrade_type = orig.iTrade_type;
		// session id
		strSessionId = orig.strSessionId;
		//������ID
		strDealerId = orig.strDealerId;
		// ϯλ
		strSecurity_seat = orig.strSecurity_seat;
		// ֤ȯ�˺�
		strAccountId = orig.strAccountId;
		// ����Ȧ
		strTrade_group = orig.strTrade_group;
		// �������
		strClordid = orig.strClordid;
		// 
		//strTransId = orig.bIsUserHoldingCurrentStock;
		// ί��ԭʼ����
		strOrderqty_origin = orig.strOrderqty_origin;
		// ί�м۸�
		strOrderPrice = orig.strOrderPrice;
		// ί�н��
		strCashorderqty = orig.strCashorderqty;
		// beginstring
		strBeginString = orig.strBeginString;

		// msgtype
		strMsgType = orig.strMsgType;

		// �Է�ID
		strSenderCompID = orig.strSenderCompID;
		// ����ID
		strTargetCompID = orig.strTargetCompID;

		// applid
		strApplID = orig.strApplID;

		// ownertype
		strOwnerType = orig.strOwnerType;
		// ִ�б��
		strExecID = orig.strExecID;
		// �������������
		strOrderID = orig.strOrderID;

		//exectype
		strExecType = orig.strExecType;
		//ordstatus
		strOrdStatus = orig.strOrdStatus;

		// leavesqty ʣ������
		strLeavesQty = orig.strLeavesQty;

		//cumqty�ۼƳɽ���
		strCumQty = orig.strCumQty;

		//side��������
		strSide = orig.strSide;

		// ֤ȯ����
		strStockID = orig.strStockID;

		// ֤ȯ����Դ
		strSecurityIDSource = orig.strSecurityIDSource;

		// market
		strMarket = orig.strMarket;

		// stoppxֹ��ۣ�Ԥ��
		strStoppx = orig.strStoppx;

		//timeinforce
		strTimeInForce = orig.strTimeInForce;

		//positioneffect
		strPositionEffect = orig.strPositionEffect;

		// ordtype
		strOrdType = orig.strOrdType;

		// ����ʱ��ԭʼ�������
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

		// market_branchidӪҵ������
		strMarket_branchid = orig.strMarket_branchid;

		// ����ʱ��
		strOper_time = orig.strOper_time;

		// `trade_time` timestamp NULL DEFAULT NULL COMMENT '����ʱ��,
		strTrade_time = orig.strTrade_time;

		// �Ƿ񱻴������0--�ǣ�1--��
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
		// �ɽ���
		strLastPx = orig.strLastPx;

		// �ɽ�����
		strLastQty = orig.strLastQty;
		// order--matched end

		// ��ǰ�����Ľ��ײ���
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
