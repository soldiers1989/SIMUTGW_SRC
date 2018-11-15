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

	// �����Ľ�������
	enum TADE_TYPE
	{
		// ����
		error = -1,
		// ����
		cancel = 0,
		// A��
		a_trade = 1,
		// B��
		b_trade,
		// ���ʽ��ף������������ȯ����
		margin_cash,
		// ��ȯ���ף���ȯ��������ȯ��ȯ
		margin_stock,
		// ETF����
		etf_buy,
		// ETF����
		etf_sell,
		// ETF�깺
		etf_crt,
		// ETF���
		etf_rdp
	};

	// �걨����
	enum DECLARATION_TYPE
	{
		deltype_ordinary_limit = 1,// �޼�
		deltype_the_side_optimal = 2, // ��������
		deltype_the_oppsite_side_optimaland_remainder_hold = 3, // ���ַ�����ʣ��ת�޼�
		deltype_match_at_market_and_remainder_cancel = 4, // �м������ɽ�ʣ�೷��
		//deltype_all_match_within_limit_or_cancel = 5, // �޼�ȫ��ɽ�����
		deltype_all_match_at_market_or_cancel = 6, // �м�ȫ��ɽ�����
		deltype_match_at_market_in_five_and_remainder_cancel = 7, // �м������嵵ȫ��ɽ�ʣ�೷��
		deltype_match_at_market_in_five_and_remainder_hold = 8 //�м������嵵ȫ��ɽ�ʣ��ҵ�
	};

	/*
		�ɽ�������
		����������
		*/
	struct ORDER_MATCH_CNT_nolock
	{
		// �յ�ί�л򳷵�
		uint64_t ui64_Received_Count;

		// ȫ���ɽ�
		uint64_t ui64_MatchAll_Count;

		// ���ֳɽ�
		uint64_t ui64_MatchPart_Count;

		// ����
		uint64_t ui64_Cancel_Count;

		// ����
		uint64_t ui64_Error_Count;

		// ������ȷ����
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
	�ɽ�������
	ԭ�ӱ���
	*/
	struct ORDER_MATCH_CNT_atomic
	{
		// �յ�ί�л򳷵�
		atomic<unsigned long long> aull_Received_Count;

		// ȫ���ɽ�
		atomic<unsigned long long> aull_MatchAll_Count;

		// ���ֳɽ�
		atomic<unsigned long long> aull_MatchPart_Count;

		// ����
		atomic<unsigned long long> aull_Cancel_Count;

		// ����
		atomic<unsigned long long> aull_Error_Count;

		// ������ȷ����
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

	// ����ʱ�õ�֤ȯ�����仯
	struct TradeStock
	{
		//
		// ֤ȯ�����������ɷ����
		// �û�����ǰ�ֲ�		
		uint64_t ui64StockBalance_beforematch;
		// �û����׺�ֲ�
		uint64_t ui64StockBalance_aftermatch;
		//
		// ����������
		// ����ǰ
		uint64_t ui64Stock_auction_purchase_beforematch;
		// ���׺�
		uint64_t ui64Stock_auction_purchase_aftermatch;
		//
		// ����������
		// ����ǰ
		uint64_t ui64Stock_staple_purchase_beforematch;
		// ���׺�
		uint64_t ui64Stock_staple_purchase_aftermatch;
		//
		// etf��������ɾ������� �� etf�깺�����ɾ�������
		// ����ǰ
		uint64_t ui64Stock_etfredemption_creation_beforematch;
		// ���׺�
		uint64_t ui64Stock_etfredemption_creation_aftermatch;
		//
		// ֤ȯ���п������������깺etf�ݶ�Ϳɾ�������
		// �û����óֲ� ����ǰ
		uint64_t ui64StockAvailable_beforematch;
		// �û����óֲ� ���׺�
		uint64_t ui64StockAvailable_aftermatch;

		// �ϴ����
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

	// ����ʱ�õ�Etf�ɷݹɶ����仯
	struct TradeEtfComponent : SzETFComponent
	{
		// �Ƿ���е�ǰ��Ʊ true--�� false--��
		bool bIsUserHolding;

		// �û��ĳֹ�״̬
		struct TradeStock userHold;

		// �û��Ƿ���Ҫ���ֽ���� true--�� false--��
		bool bIsNeedCashComponent;

		// ��Ҫ�ֽ�����Ľ��
		uint64_t_Money ui64mCashComponent;

		// �������׵Ĺ�Ʊ����
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
		// ��Ҫ�ֽ�����Ľ��
		uint64_t_Money ui64mCashComponent;

		// ���гɷֹ���Ϣ
		std::vector<TradeEtfComponent> vecComponents;

		TradeEtf()
		{
			ui64mCashComponent = 0;
		}
	};

	// ��ͨ��Ʊ��������
	struct SellCps
	{
		// etf��ز���
		uint64_t ui64Etf_rdp;
		// ��������
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

	// ETF��������
	struct SellETFCps
	{
		// etf�깺����
		uint64_t ui64Etf_crt;
		// ��������
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

	// ETF��ع���
	struct RdpETFCps
	{
		// �������벿��
		uint64_t ui64Act_pch;
		// ��������
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

	// ETF�깺����ɷֹɵ����/��سɷֹɵ�������
	struct EtfCrt_FrozeComponent
	{
		// ֤ȯ����
		std::string strSecurityID;
		// �������������
		uint64_t ui64act_pch_count;
		// ������������
		uint64_t ui64avl_count;
		// ����ܽ��
		uint64_t_Money ui64Cash;
		// �������
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
	ί����Ϣ����
	*/
	struct OrderMessage
	{
		// error flag
		bool bDataError;
		// �Ƿ���е�ǰ��Ʊ true--�� false--��
		bool bIsUserHoldingCurrentStock;

		// �������� 1/��2/��
		int iSide;

		// time_t ί���յ�ʱ��
		time_t tRcvTime;

		// ί������
		simutgw::DECLARATION_TYPE enDelType;

		// �ɽ��������
		simutgw::MatchType enMatchType;

		// ί��ԭʼ����
		uint64_t ui64Orderqty_origin;
		// ί��ʣ������
		uint64_t ui64LeavesQty;
		// ί�м۸�
		simutgw::uint64_t_Money ui64mOrderPrice;
		// ί�н��
		//simutgw::uint64_t_Money ui64mCashorderqty;
		// minqty
		uint64_t ui64MinQty;

		// �û��ĳֹɽ���״̬
		struct TradeStock userHold;

		// Etf�Ľ���״̬
		struct TradeEtf etfOrderStatus;

		// ��ͨ��Ʊ��������
		struct SellCps sellCps;

		// ETF��������
		struct SellETFCps sellETFCps;

		// ETF��ع���
		struct RdpETFCps rdpETFCps;

		// ETF����ĳɷֹ�
		vector<std::shared_ptr<struct EtfCrt_FrozeComponent>> vecFrozeComponent;

		//
		// �ѳɽ�����
		// �ɽ���������
		uint64_t ui64Orderqty_matched;
		// �ɽ��۸�
		simutgw::uint64_t_Money ui64mPrice_matched;
		// �ɽ��������
		simutgw::uint64_t_Money ui64mCashorderqty_matched;

		//
		// δ�ɽ�����
		// δ�ɽ�����
		uint64_t ui64Orderqty_unmatched;
		// δ�ɽ����
		simutgw::uint64_t_Money ui64mCashorderqty_unmatched;

		//
		std::string strID;
		//0--���У�1--����
		std::string strTrade_market;
		//0--A�ɣ�1--B�ɣ�2--���ʣ�3--��ȯ
		int iTrade_type;
		// session id
		std::string strSessionId;
		//������ID
		std::string strDealerId;
		// ϯλ
		std::string strSecurity_seat;
		// ֤ȯ�˺�
		std::string strAccountId;
		// ����Ȧ
		std::string strTrade_group;
		// �������
		std::string strClordid;
		// 
		//std::string strTransId;
		// ί��ԭʼ����
		std::string strOrderqty_origin;
		// ί�м۸�
		std::string strOrderPrice;
		// ί�н��
		std::string strCashorderqty;
		// beginstring
		std::string strBeginString;

		// msgtype
		std::string strMsgType;

		// �Է�ID
		std::string strSenderCompID;
		// ����ID
		std::string strTargetCompID;

		// applid
		std::string strApplID;

		// ownertype
		std::string strOwnerType;
		// ִ�б��
		std::string strExecID;
		// �������������
		std::string strOrderID;

		//exectype
		std::string strExecType;
		//ordstatus
		std::string strOrdStatus;

		// leavesqty ʣ������
		std::string strLeavesQty;

		//cumqty�ۼƳɽ���
		std::string strCumQty;

		//side��������
		std::string strSide;

		// ֤ȯ����
		std::string strStockID;

		// ֤ȯ����Դ
		std::string strSecurityIDSource;

		// ���ȯ����
		std::string strStockID2;

		// market
		std::string strMarket;

		// stoppxֹ��ۣ�Ԥ��
		std::string strStoppx;

		//timeinforce
		std::string strTimeInForce;

		//positioneffect
		std::string strPositionEffect;

		// ordtype
		std::string strOrdType;

		// ����ʱ��ԭʼ�������
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

		// market_branchidӪҵ������
		std::string strMarket_branchid;

		// ����ʱ��
		std::string strOper_time;

		// `trade_time` timestamp NULL DEFAULT NULL COMMENT '����ʱ��,
		std::string strTrade_time;

		// �Ƿ񱻴������0--�ǣ�1--��
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
		// �ɽ���
		std::string strLastPx;

		// �ɽ�����
		std::string strLastQty;
		// order--matched end

		// ��ǰ�����Ľ��ײ���
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