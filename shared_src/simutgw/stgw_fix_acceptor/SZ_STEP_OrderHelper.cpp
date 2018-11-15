#include "SZ_STEP_OrderHelper.h"
#include "simutgw/stgw_fix_acceptor/StgwFixUtil.h"
#include "order/StockOrderHelper.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"

#include "simutgw_config/g_values_sys_run_config.h"

#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/stgw_config/g_values_biz.h"

SZ_STEP_OrderHelper::SZ_STEP_OrderHelper()
{
}


SZ_STEP_OrderHelper::~SZ_STEP_OrderHelper()
{
}

// �¶���ת�ṹ��
int SZ_STEP_OrderHelper::NewOrder2Struct(const FIX::Message& message,
	std::shared_ptr<struct simutgw::OrderMessage>& ptrorder)
{
	static const std::string ftag("SZ_STEP_OrderHelper::NewOrder2Struct() ");
	try
	{
		//������¶����������� ���� �ǳ���		
		
		// ί���յ�ʱ��
		ptrorder->tRcvTime = time(NULL);

		//`trade_market`,`trade_type`
		ptrorder->strTrade_market = simutgw::TRADE_MARKET_SZ;

		//`sessionid`,`security_account`,`security_seat`,`beginstring`,`bodylength`,
		//FIX::SessionID &sessionId = message.getSessionID();
		FIX::SessionID sessionId = message.getSessionID();
		ptrorder->strSessionId = sessionId.toString();

		GetNopartyIds(message, ptrorder);

		ptrorder->strBeginString = sessionId.getBeginString();

		if (message.isSetField(FIX::FIELD::BodyLength))
		{
			ptrorder->strBodyLength = message.getHeader().getField(FIX::FIELD::BodyLength);
		}

		//`checksum`,`clordid`,`securityidsource`,`msgseqnum`,`msgtype`,
		if (message.getTrailer().isSetField(FIX::FIELD::CheckSum))
		{
			ptrorder->strCheckSum = message.getTrailer().getField(FIX::FIELD::CheckSum);
		}

		if (message.isSetField(FIX::FIELD::ClOrdID))
		{
			ptrorder->strClordid = message.getField(FIX::FIELD::ClOrdID);
		}

		if (message.isSetField(FIX::FIELD::SecurityIDSource))
		{
			ptrorder->strSecurityIDSource = message.getField(FIX::FIELD::SecurityIDSource);
		}

		if (message.getHeader().isSetField(FIX::FIELD::MsgSeqNum))
		{
			ptrorder->strMsgSeqNum = message.getHeader().getField(FIX::FIELD::MsgSeqNum);
		}

		if (message.getHeader().isSetField(FIX::FIELD::MsgType))
		{
			ptrorder->strMsgType = message.getHeader().getField(FIX::FIELD::MsgType);
		}

		//`orderqty_origin`,`leavesqty`,`ordtype`,`possdupflag`,`order_price`,
		if (message.isSetField(FIX::FIELD::OrderQty))
		{
			ptrorder->strOrderqty_origin = message.getField(FIX::FIELD::OrderQty);
		}

		int iRes = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(ptrorder->strOrderqty_origin, ptrorder->ui64Orderqty_origin);
		if (0 != iRes)
		{
			return -1;
		}

		//����ʣ����������ԭʼ����
		ptrorder->ui64LeavesQty = ptrorder->ui64Orderqty_origin;
		ptrorder->strLeavesQty = ptrorder->strOrderqty_origin;

		if (message.isSetField(FIX::FIELD::OrdType))
		{
			ptrorder->strOrdType = message.getField(FIX::FIELD::OrdType);
		}

		if (message.isSetField(FIX::FIELD::PossDupFlag))
		{
			ptrorder->strPossDupFlag = message.getField(FIX::FIELD::PossDupFlag);
		}

		//ί�м۸��Է�Ϊ��λ
		if (message.isSetField(FIX::FIELD::Price))
		{
			ptrorder->strOrderPrice = message.getField(FIX::FIELD::Price);
		}
		iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(ptrorder->strOrderPrice, ptrorder->ui64mOrderPrice);
		if (0 != iRes)
		{
			return -1;
		}
		sof_string::itostr(ptrorder->ui64mOrderPrice, ptrorder->strOrderPrice);

		//`securityid`,`sendercompid`,`sendingtime`,`side`,`targetcompid`,
		if (message.isSetField(FIX::FIELD::SecurityID))
		{
			ptrorder->strStockID = message.getField(FIX::FIELD::SecurityID);
		}

		ptrorder->strSenderCompID = message.getHeader().getField(FIX::FIELD::SenderCompID);

		if (message.isSetField(FIX::FIELD::SendingTime))
		{
			ptrorder->strSendingTime = message.getField(FIX::FIELD::SendingTime);
		}

		if (message.isSetField(FIX::FIELD::Side))
		{
			ptrorder->strSide = message.getField(FIX::FIELD::Side);
		}

		ptrorder->strTargetCompID = message.getHeader().getField(FIX::FIELD::TargetCompID);

		//`text`,`timeinforce`,`transacttime`,`positioneffect`,`stoppx`,
		if (message.isSetField(FIX::FIELD::Text))
		{
			ptrorder->strText = message.getField(FIX::FIELD::Text);
		}

		if (message.isSetField(FIX::FIELD::TimeInForce))
		{
			ptrorder->strTimeInForce = message.getField(FIX::FIELD::TimeInForce);
		}

		if (message.isSetField(FIX::FIELD::TransactTime))
		{
			ptrorder->strTransactTime = message.getField(FIX::FIELD::TransactTime);
		}

		if (message.isSetField(FIX::FIELD::PositionEffect))
		{
			ptrorder->strPositionEffect = message.getField(FIX::FIELD::PositionEffect);
		}

		if (message.isSetField(FIX::FIELD::StopPx))
		{
			ptrorder->strStoppx = message.getField(FIX::FIELD::StopPx);
		}

		//`minqty`,`origsendingtime`,`cashorderqty`,`coveredoruncovered`,`nopartyids`,
		if (message.isSetField(FIX::FIELD::MinQty))
		{
			ptrorder->strMinQty = message.getField(FIX::FIELD::MinQty);
		}

		if (message.isSetField(FIX::FIELD::OrigSendingTime))
		{
			ptrorder->strOrigSendingTime = message.getField(FIX::FIELD::OrigSendingTime);
		}

		if (message.isSetField(FIX::FIELD::CashOrderQty))
		{
			ptrorder->strCashorderqty = message.getField(FIX::FIELD::CashOrderQty);
			uint64_t ui64Num = 0;
			Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(ptrorder->strCashorderqty, ui64Num);
			sof_string::itostr(ui64Num, ptrorder->strCashorderqty);
		}

		if (message.isSetField(FIX::FIELD::CoveredOrUncovered))
		{
			ptrorder->strCoveredOrUncovered = message.getField(FIX::FIELD::CoveredOrUncovered);
		}

		//`ownertype`,`orderrestrictions`,`cashmargin`,`confirmid`,`maxpricelevels`,
		if (message.isSetField(FIX::FIELD::OwnerType))
		{
			ptrorder->strOwnerType = message.getField(FIX::FIELD::OwnerType);
		}

		if (message.isSetField(FIX::FIELD::OrderRestrictions))
		{
			ptrorder->strOrderRestrictions = message.getField(FIX::FIELD::OrderRestrictions);
		}

		if (message.isSetField(FIX::FIELD::CashMargin))
		{
			ptrorder->strCashMargin = message.getField(FIX::FIELD::CashMargin);
		}

		if (message.isSetField(FIX::FIELD::ConfirmID))
		{
			ptrorder->strConfirmID = message.getField(FIX::FIELD::ConfirmID);
		}

		if (message.isSetField(FIX::FIELD::MaxPriceLevels))
		{
			ptrorder->strMaxPriceLevels = message.getField(FIX::FIELD::MaxPriceLevels);
		}

		//`applid`,`market_branchid`,`origclordid`
		if (message.isSetField(FIX::FIELD::ApplID))
		{
			ptrorder->strApplID = message.getField(FIX::FIELD::ApplID);
		}

		if (message.isSetField(FIX::FIELD::OrigClOrdID))
		{
			ptrorder->strOrigClordid = message.getField(FIX::FIELD::OrigClOrdID);
		}

		// `oper_time` YYYY-MM-DD HH:MM:SS
		TimeStringUtil::GetCurrTimeInTradeType(ptrorder->strOper_time);

		//
		// ��ȡ����ر���
		const string strSenderCompID = ptrorder->strSenderCompID;

		// ���ײ���
		string strSzConn_Name("");
		string strSzConn_SettleGroupName("");

		// web����ģʽ�²���Ҫ��ѯ
		if (simutgw::WebManMode::WebMode == simutgw::g_iWebManMode)
		{
			std::map<std::string, struct Connection_webConfig>::const_iterator itSz = simutgw::g_mapSzConn_webConfig.find(strSenderCompID);
			if (simutgw::g_mapSzConn_webConfig.end() == itSz)
			{
				//
				string sDebug("δ�� simutgw::g_mapSzConn_webConfig �ҵ� SenderCompID=");
				sDebug += strSenderCompID;
				EzLog::e(ftag, sDebug);
			}
			else
			{
				strSzConn_Name = itSz->second.strWebLinkid;
				strSzConn_SettleGroupName = itSz->second.strSettleGroupName;
				ptrorder->tradePolicy.mapLinkRules.insert(itSz->second.mapLinkRules.begin(), itSz->second.mapLinkRules.end());
			}

			// ��¼ԭʼ��Ϣ
			ptrorder->szfixMsg = message;

			// �жϹ�Ʊ�ĳɽ����ù���
			StockOrderHelper::GetOrderMatchRule(ptrorder, ptrorder->tradePolicy.mapLinkRules);
		}

		// ��ȡ��ǰ���ӵĽ��ײ���
		simutgw::g_tradePolicy.Get_Sz(strSzConn_Name, ptrorder->strSecurity_seat,
			ptrorder->tradePolicy.iRunMode, ptrorder->tradePolicy.bCheck_Assets,
			ptrorder->tradePolicy.iMatchMode, ptrorder->tradePolicy.iQuotationMatchType, ptrorder->tradePolicy.iPart_Match_Num);

		ptrorder->tradePolicy.strSettleGroupName = strSzConn_SettleGroupName;
		
		// �ж��Ƿ����й���
		if (0 == ptrorder->tradePolicy.ui64RuleId)
		{
			// �޳ɽ����ù���ʱ�����뽻����
			// �жϹ�Ʊ��������
			iRes = StockOrderHelper::CheckOrder_TradeType(ptrorder);
			if (0 != iRes)
			{
				return -1;
			}
		}

	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

// ȡnopartyids�ظ�������ݣ������˺š�ϯλ��Ӫҵ������
int SZ_STEP_OrderHelper::GetNopartyIds(const FIX::FieldMap& message,
	std::shared_ptr<struct simutgw::OrderMessage>& ptrorder)
{
	// static const std::string ftag("SZ_STEP_OrderHelper::GetNopartyIds() ");

	return StgwFixUtil::GetNopartyIds(message,
		ptrorder->strSecurity_seat,
		ptrorder->strAccountId,
		ptrorder->strMarket_branchid);
}