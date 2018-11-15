#include "quickfix/fix50/ExecutionReport.h"

#include "StgwFixReport.h"
#include "simutgw/stgw_fix_acceptor/StgwFixUtil.h"

#include "tool_string/TimeStringUtil.h"

#include "simutgw/stgw_config/g_values_biz.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw_config/g_values_sys_run_config.h"

#include "simutgw/db_oper/RecordReportHelper.h"
#include "simutgw/db_oper/RecordTradeInfo.h"

#include "match_rule/RuleWordProc_SzEzSTEP.h"

src::severity_channel_logger<trivial::severity_level, std::string>
StgwFixReport::m_scl(keywords::channel = "StgwFixReport");

StgwFixReport::StgwFixReport()
{
}


StgwFixReport::~StgwFixReport()
{
}

/*
�������ڻر�ҵ��
Return:
0 -- �ɹ�
-1 -- ʧ��
1 -- �޻ر�
*/
int StgwFixReport::Send_SZReport()
{
	static const string ftag("StgwFixReport::Get_SZReport() ");

	int iRes = 0;

	map<string, SzConnection>::iterator it = simutgw::g_mapSzConns.begin();
	for (; it != simutgw::g_mapSzConns.end(); ++it)
	{
		string szConnName = it->first;

		if (it->second.GetLogSta() && it->second.GetRptSta())
		{
			// ȡ���ڻر�			
			std::shared_ptr<struct simutgw::OrderMessage> ptrReport;
			FIX::Message fixReport;

			iRes = simutgw::g_outMsg_buffer.PopFront_sz(szConnName, ptrReport);
			if (iRes < 0)
			{
				EzLog::e(ftag, "ReadReport() faild");

				return -1;
			}
			else if (iRes > 0)
			{
				// �޻ر�
				continue;
			}
			else
			{
				if (nullptr == ptrReport)
				{
					string sDebug("Report nullptr, ConnName=");
					sDebug += szConnName;

					EzLog::e(ftag, sDebug);

					continue;
				}
				else
				{
					// ��¼������ˮ
					RecordReportHelper::RecordReportToDb(ptrReport);
					if (simutgw::MatchAll == ptrReport->enMatchType)
					{
						// ���������ɽ��ر�
						iRes = ProcSingleReport(ptrReport, fixReport);
						if (0 == iRes)
						{
							//simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchAll();
						}
					}
					else if (simutgw::MatchPart == ptrReport->enMatchType)
					{
						// ���������ɽ��ر�
						iRes = ProcSingleReport(ptrReport, fixReport);
						if (0 == iRes)
						{
							//simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchPart();
						}
					}
					else if (simutgw::CancelMatch == ptrReport->enMatchType)
					{
						// �������ر�
						iRes = ProcSZCancelOrder(ptrReport, fixReport);
						if (0 == iRes)
						{
							simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchCancel();
						}
					}
					else if (simutgw::ErrorMatch == ptrReport->enMatchType)
					{
						// ����ϵ��ر�
						iRes = ProcSZErrorOrder(ptrReport, fixReport);
						if (0 == iRes)
						{
							simutgw::g_counter.GetSz_InnerCounter()->Inc_Error();
						}
					}
					else if (simutgw::NotMatch == ptrReport->enMatchType)
					{
						// ȷ��
						ptrReport->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;
						iRes = ProcSZConfirmOrder(ptrReport, fixReport);

						if (0 == iRes)
						{
							//simutgw::g_counter.GetSz_InnerCounter()->Inc_Confirm();
						}
					}
					else
					{
						std::string strTrans, strError("clordid[");
						strError += ptrReport->strClordid;
						strError += "]MatchType[";
						strError += sof_string::itostr(ptrReport->enMatchType, strTrans);
						EzLog::e(ftag, strError);

						continue;
					}
				}
			}

			if (0 == iRes)
			{
				// �Ѿ�����˴����ͻر���
				simutgw::g_fixaccptor.SendMsg(fixReport);

				// ��¼��·����
				simutgw::g_counter.IncSz_Link_SendBack(ptrReport->strSenderCompID);

				return 0;
			}
			else if (1 == iRes)
			{
				// ���跢�͵���Ϣ
				return 0;
			}
		}
		else if (it->second.GetLogSta() && !it->second.GetRptIdex())
		{
			// �ѵ�¼δ�ر�ͬ������ʱ�����ͻر�
			// ��������ȣ�����1000ʱ����
			it->second.IncQueLkupTimes();
			uint64_t ui64Times = it->second.GetQueLkupTimes();
			if (ui64Times >= 10)
			{
				size_t stLen = simutgw::g_outMsg_buffer.GetSize_sz(szConnName);
				if (stLen >= 10)
				{
					string strDebug("outMsgBuffer������ȳ���10������=");
					strDebug += szConnName;
					EzLog::w(ftag, strDebug);

					it->second.ResetQueLkupTimes();
				}
			}
		}
		else
		{
			// δ��¼
		}
	}


	return 1;
}

/*
����һ���ر�

Reutrn:
0 -- �ɹ�
1 -- �޻ظ���Ϣ
-1 -- ʧ��
*/
int StgwFixReport::ProcSingleReport(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	FIX::Message& fixReport)
{
	static const string ftag("StgwFixReport::ProcSingleReport() ");

	int iRes = 0;

	// ִ�б���
	if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		//	����
		if (0 != in_ptrReport->tradePolicy.ui64RuleId)
		{
			// �Ѷ����˳ɽ�����
			if (nullptr == in_ptrReport->tradePolicy.ptrRule_Sz)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error order cliordid=" << in_ptrReport->strClordid
					<< "nullptr Rule Sz, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

				return -1;
			}

			// report with rule
			iRes = ProcMsgType_8_Report_JsonRule(in_ptrReport, fixReport);
		}
		else
		{
			/* �ȸ��±� */
			iRes = RecordTradeInfo::WriteTransInfoInDb(in_ptrReport);

			// �����ڲ�������
			string strTransId;
			TimeStringUtil::ExRandom15(strTransId);
			//Ψһִ��id
			in_ptrReport->strExecID = strTransId;
			//Ψһ����id
			in_ptrReport->strOrderID = strTransId;

			iRes = ProcMsgType_8_Report(in_ptrReport, fixReport);
		}

		if (1 == iRes)
		{
			return 1;
		}
		else if (0 != iRes)
		{
			return -1;
		}

		return 0;
	}
	else if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// �Ϻ�
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "wrong place SH Report clordid=" << in_ptrReport->strClordid
			<< ", client=" << in_ptrReport->strSenderCompID;

		return -1;
	}

	BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "clordid=" << in_ptrReport->strClordid
		<< ", client=" << in_ptrReport->strSenderCompID << " error Trade_market=" << in_ptrReport->strTrade_market;

	return -1;
}

/*
�������ڳ���

Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int StgwFixReport::ProcSZCancelOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	FIX::Message& fixReport)
{
	static const string ftag("StgwFixReport::ProcSZCancelOrder() ");

	string strReport;

	int iRes = 0;
	if (in_ptrReport->enMatchType == simutgw::CancelMatch)
	{

		if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_EXECREPORT))
		{
			// �����ɹ�
			RecordTradeInfo::WriteTransInfoInDb_CancelSuccess(in_ptrReport);

			iRes = ProcMsgType_8_Report(in_ptrReport, fixReport);
		}
		else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_CANCELREJECT))
		{
			// ����ʧ��
			RecordTradeInfo::WriteTransInfoInDb_CancelFail(in_ptrReport);
			iRes = ProcMsgType_9_Report(in_ptrReport, fixReport);
		}
	}

	string strSendLog("Sended SZ Cancel Report origClordid=");
	strSendLog += in_ptrReport->strOrigClordid;
	strSendLog += (", client=");
	strSendLog += in_ptrReport->strSenderCompID;

	EzLog::i(ftag, strSendLog);

	return iRes;
}

/*
�������ڴ��󶩵�

Return:
0 -- �ɹ�
-1 -- ʧ��
1 -- �޻ر�
*/
int StgwFixReport::ProcSZErrorOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	FIX::Message& fixReport)
{
	static const string ftag("StgwFixReport::ProcSZErrorOrder() ");

	int iRes = 0;
	if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_NEW_ORDER))
	{
		// ������
		in_ptrReport->strMsgType = "8";
		in_ptrReport->strExecType = "8";
		in_ptrReport->strOrdStatus = "8";
		in_ptrReport->bDataError = true;
		in_ptrReport->strOrdrejReason = in_ptrReport->strErrorCode;

		iRes = ProcMsgType_8_Report(in_ptrReport, fixReport);
	}
	else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
	{
		// ����
		// ������
		in_ptrReport->strMsgType = "9";
		in_ptrReport->strOrdStatus = "8";
		in_ptrReport->strOrdrejReason = in_ptrReport->strErrorCode;
		iRes = ProcMsgType_9_Report(in_ptrReport, fixReport);
	}

	RecordTradeInfo::WriteTransInfoInDb_Error(in_ptrReport);

	string strSendLog("Sended SZ Report clordid=");
	strSendLog += in_ptrReport->strClordid;
	strSendLog += (", client=");
	strSendLog += in_ptrReport->strSenderCompID;

	EzLog::i(ftag, strSendLog);

	return iRes;
}

/*
��������ȷ��

Return:
0 -- �ɹ�
-1 -- ʧ��
1 -- �޻ر�
*/
int StgwFixReport::ProcSZConfirmOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	FIX::Message& fixReport)
{
	static const string ftag("StgwFixReport::ProcSZConfirmOrder() ");

	int iRes = 0;

	// ִ�б���
	if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		//	����
		if (0 != in_ptrReport->tradePolicy.ui64RuleId)
		{
			// �Ѷ����˳ɽ�����
			if (nullptr == in_ptrReport->tradePolicy.ptrRule_Sz)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error order cliordid=" << in_ptrReport->strClordid
					<< "nullptr Rule Sz, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

				return -1;
			}

			// report with rule
			iRes = ProcMsgType_8_Confirm_JsonRule(in_ptrReport, fixReport);
		}
		else
		{
			// �����ڲ�������
			string strTransId;
			TimeStringUtil::ExRandom15(strTransId);
			//Ψһִ��id
			in_ptrReport->strExecID = strTransId;
			//Ψһ����id
			in_ptrReport->strOrderID = strTransId;

			iRes = ProcMsgType_8_Report(in_ptrReport, fixReport);
		}

		if (0 != iRes)
		{
			return -1;
		}

		return 0;
	}
	else if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// �Ϻ�
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "wrong place SH Report clordid=" << in_ptrReport->strClordid
			<< ", client=" << in_ptrReport->strSenderCompID;

		return -1;
	}

	BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "clordid=" << in_ptrReport->strClordid
		<< ", client=" << in_ptrReport->strSenderCompID << " error Trade_market=" << in_ptrReport->strTrade_market;

	return -1;
}

/*
����һ��ί�гɹ���ִ�б���ر�,msgtype = 8
*/
int StgwFixReport::ProcMsgType_8_Report(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	FIX::Message& fixReport)
{
	static const string ftag("StgwFixReport::ProcMsgType_8_Report() ");

	try
	{
		//beginstring, sendercompid, targetcompid
		if (in_ptrReport->strBeginString.empty())
		{
			//	���ֶβ���Ϊ��
			string strError("�ֶ�[beginstring]����Ϊ��");

			EzLog::e(ftag, strError);
			return -1;
		}
		StgwFixUtil::SetField(fixReport, FIX::FIELD::BeginString, in_ptrReport->strBeginString);

		if (in_ptrReport->strSenderCompID.empty())
		{
			//	���ֶβ���Ϊ��
			string strError("�ֶ�[sendercompid]����Ϊ��");

			EzLog::e(ftag, strError);
			return -1;
		}

		if (in_ptrReport->strTargetCompID.empty())
		{
			//	���ֶβ���Ϊ��
			string strError("�ֶ�[targetcompid]����Ϊ��");

			EzLog::e(ftag, strError);
			return -1;
		}
		StgwFixUtil::SetField(fixReport, FIX::FIELD::TargetCompID, in_ptrReport->strSenderCompID);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::SenderCompID, in_ptrReport->strTargetCompID);

		// msgtype, reportindex, applid, ownertype, orderrestrictions
		if (in_ptrReport->strMsgType.empty())
		{
			//	���ֶβ���Ϊ��
			string strError("�ֶ�[msgtype]����Ϊ��");

			EzLog::e(ftag, strError);
			return -1;
		}
		StgwFixUtil::SetField(fixReport, FIX::FIELD::MsgType, in_ptrReport->strMsgType);

		string strValue = in_ptrReport->strSenderCompID;
		//sof_string::itostr(simutgw::g_iReportIndex, strValue);
		uint64_t ui64Num = 0;
		string strItoa;
		int iPartitionNo = 0;

		if (simutgw::g_mapSzConns.end() != simutgw::g_mapSzConns.find(strValue))
		{
			ui64Num = simutgw::g_mapSzConns[strValue].GetRptIdex();

			if (simutgw::g_bSZ_Step_ver110)
			{
				// ����STEP�ر���Ver1.10
				// ȡPartitionNo
				std::shared_ptr<std::map<int, uint64_t>> prtMapPati = simutgw::g_mapSzConns[strValue].GetPartitionsMap();
				if (nullptr != prtMapPati)
				{
					std::map<int, uint64_t>::iterator it = prtMapPati->begin();
					if (prtMapPati->end() != it)
					{
						iPartitionNo = it->first;
					}
				}
			}
		}
		sof_string::itostr(ui64Num, strItoa);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::ReportIndex, strItoa);

		if (simutgw::g_bSZ_Step_ver110)
		{
			sof_string::itostr(iPartitionNo, strItoa);
			StgwFixUtil::SetField(fixReport, FIX::FIELD::PartitionNo, strItoa);
		}

		if (in_ptrReport->strApplID.empty())
		{
			//	���ֶβ���Ϊ��
			string strError("�ֶ�[applid]����Ϊ��");

			EzLog::e(ftag, strError);
			return -1;
		}
		StgwFixUtil::SetField(fixReport, FIX::FIELD::ApplID, in_ptrReport->strApplID);

		if (in_ptrReport->strOwnerType.empty())
		{
			//	���ֶβ���Ϊ��
			string strError("�ֶ�[ownertype]����Ϊ��");

			EzLog::e(ftag, strError);
			return -1;
		}
		StgwFixUtil::SetField(fixReport, FIX::FIELD::OwnerType, in_ptrReport->strOwnerType);

		if (!in_ptrReport->strOrderRestrictions.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::OrderRestrictions, in_ptrReport->strOrderRestrictions);
		}

		// execid, orderid, exectype, ordstatus, lastPx
		if (!in_ptrReport->strExecID.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::ExecID, in_ptrReport->strExecID);
		}

		if (!in_ptrReport->strOrderID.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::OrderID, in_ptrReport->strOrderID);
		}

		StgwFixUtil::SetField(fixReport, FIX::FIELD::ExecType, in_ptrReport->strExecType);

		StgwFixUtil::SetField(fixReport, FIX::FIELD::OrdStatus, in_ptrReport->strOrdStatus);

		if (!in_ptrReport->strLastPx.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::LastPx, in_ptrReport->strLastPx);
		}

		//	lastqty, leavesqty, cumqty, ordrejreason
		if (!in_ptrReport->strLastQty.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::LastQty, in_ptrReport->strLastQty);
		}

		sof_string::itostr(in_ptrReport->ui64LeavesQty, strValue);
		strValue += ".00";
		StgwFixUtil::SetField(fixReport, FIX::FIELD::LeavesQty, strValue);

		sof_string::itostr(in_ptrReport->ui64Orderqty_origin - in_ptrReport->ui64LeavesQty, strValue);
		strValue += ".00";
		StgwFixUtil::SetField(fixReport, FIX::FIELD::CumQty, strValue);

		/*
		����������ر���Ϊ�˼���U���̨����Ҫ�����ֶμ��ϣ����ֵΪ��
		*/
		if (in_ptrReport->strOrdrejReason.empty())
		{
			in_ptrReport->strOrdrejReason = "0";
		}
		StgwFixUtil::SetField(fixReport, FIX::FIELD::OrdRejReason, in_ptrReport->strOrdrejReason);

		//	rejecttext, side, transactTime, clordid, origclordid
		if (!in_ptrReport->strRejectText.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::RejectText, in_ptrReport->strRejectText);
		}

		StgwFixUtil::SetField(fixReport, FIX::FIELD::Side, in_ptrReport->strSide);

		StgwFixUtil::SetField(fixReport, FIX::FIELD::TransactTime, in_ptrReport->strTransactTime);

		StgwFixUtil::SetField(fixReport, FIX::FIELD::ClOrdID, in_ptrReport->strClordid);

		if (!in_ptrReport->strOrigClordid.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::OrigClOrdID, in_ptrReport->strOrigClordid);
		}

		//	securityid, securityidsource, cashorderqty, orderqty, price
		StgwFixUtil::SetField(fixReport, FIX::FIELD::SecurityID, in_ptrReport->strStockID);

		StgwFixUtil::SetField(fixReport, FIX::FIELD::SecurityIDSource, in_ptrReport->strSecurityIDSource);

		if (!in_ptrReport->strCashorderqty.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::CashOrderQty, in_ptrReport->strCashorderqty);
		}

		strValue = in_ptrReport->strOrderqty_origin;
		StgwFixUtil::SetField(fixReport, FIX::FIELD::OrderQty, strValue);

		//	��ת��Ϊstring,��4λС��
		StgwFixUtil::SetField(fixReport, FIX::FIELD::Price, in_ptrReport->strOrderPrice);

		//	stoppx, timeinforce, ordtype, maxpricelevels, minqty
		StgwFixUtil::SetField(fixReport, FIX::FIELD::StopPx, in_ptrReport->strStoppx);

		if (!in_ptrReport->strTimeInForce.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::TimeInForce, in_ptrReport->strTimeInForce);
		}

		StgwFixUtil::SetField(fixReport, FIX::FIELD::OrdType, in_ptrReport->strOrdType);

		StgwFixUtil::SetField(fixReport, FIX::FIELD::MaxPriceLevels, in_ptrReport->strMaxPriceLevels);

		StgwFixUtil::SetField(fixReport, FIX::FIELD::MinQty, in_ptrReport->strMinQty);

		//	cashmargin, positioneffect, coveredoruncovered, confirmid, text
		StgwFixUtil::SetField(fixReport, FIX::FIELD::CashMargin, in_ptrReport->strCashMargin);

		if (!in_ptrReport->strPositionEffect.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::PositionEffect, in_ptrReport->strPositionEffect);
		}

		if (!in_ptrReport->strCoveredOrUncovered.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::CoveredOrUncovered, in_ptrReport->strCoveredOrUncovered);
		}

		if (!in_ptrReport->strConfirmID.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::ConfirmID, in_ptrReport->strConfirmID);
		}

		//	�ɽ��ر��������ɹ�
		if (in_ptrReport->strSecurity_seat.empty())
		{
			//	δ�ҵ�ϯλ��valueΪ��
			EzLog::e(ftag, "field security_seat is NULL");
			return -1;
		}
		string &strSeat = in_ptrReport->strSecurity_seat;

		if (in_ptrReport->strAccountId.empty())
		{
			//	δ�ҵ��˻���valueΪ��
			EzLog::e(ftag, "field security_account is NULL");
			return -1;
		}
		string &strAccount = in_ptrReport->strAccountId;

		if (in_ptrReport->strMarket_branchid.empty())
		{
			//	δ�ҵ�Ӫҵ�������valueΪ��
			EzLog::e(ftag, "field market_branchid is NULL");
			return -1;
		}
		string &strMarket_branchid = in_ptrReport->strMarket_branchid;

		AddNoPartyIds(fixReport, strSeat, strAccount, strMarket_branchid);

		//etf component
		AddNoSecurity(fixReport, in_ptrReport);

		if (!in_ptrReport->strText.empty())
		{
			StgwFixUtil::SetField(fixReport, FIX::FIELD::Text, in_ptrReport->strText);
		}
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
	catch (...)
	{
		EzLog::e(ftag, "exception.");
		return -1;
	}

	return 0;
}

/*
����һ��ί�гɹ���ִ�б���ر�,msgtype = 8
��JSON���ù���

Reutrn:
0 -- �ɹ�
-1 -- ʧ��
*/
int StgwFixReport::ProcMsgType_8_Confirm_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	FIX::Message& out_fixReport)
{
	static const string ftag("ProcMsgType_8_Confirm_JsonRule() ");

	try
	{
		if (!in_ptrReport->tradePolicy.ptrRule_Sz->docRuleConfirm.IsObject())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] error Rule match, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

			return -1;
		}

		rapidjson::Value& elem = in_ptrReport->tradePolicy.ptrRule_Sz->docRuleConfirm;
		if (!elem.IsObject())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] error Rule confirm, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

			return -1;
		}

		RuleWordProc_SzEzSTEP szRuleProc;
		int iResolveRes = szRuleProc.ResolveRule(in_ptrReport->tradePolicy.ptrRule_Sz->docRuleConfirm, in_ptrReport, out_fixReport);
		if (0 != iResolveRes)
		{
			return -1;
		}

		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "Sz Order ordrec=" << in_ptrReport->strClordid
			<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " transed fix string " << out_fixReport.toString();

	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
	catch (...)
	{
		EzLog::e(ftag, "exception.");
		return -1;
	}

	return 0;
}

/*
����һ��ί�гɹ���ִ�б���ر�,msgtype = 8
��JSON���ù���

Reutrn:
0 -- �ɹ�
1 -- �޻ظ���Ϣ
-1 -- ʧ��
*/
int StgwFixReport::ProcMsgType_8_Report_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	FIX::Message& out_fixReport)
{
	static const string ftag("ProcMsgType_8_Report_JsonRule() ");

	try
	{
		if (!in_ptrReport->tradePolicy.ptrRule_Sz->docRuleMatch.IsArray())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] error Rule match not array, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

			return -1;
		}

		if (0 == in_ptrReport->tradePolicy.ptrRule_Sz->docRuleMatch.Size())
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] no Rule match ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

			return 1;
		}

		rapidjson::Value& elem = in_ptrReport->tradePolicy.ptrRule_Sz->docRuleMatch[0];
		if (!elem.IsObject())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] error Rule confirm, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

			return -1;
		}

		RuleWordProc_SzEzSTEP szRuleProc;
		int iResolveRes = szRuleProc.ResolveRule(elem, in_ptrReport, out_fixReport);
		if (0 != iResolveRes)
		{
			return -1;
		}

		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "Sz Order ordrec=" << in_ptrReport->strClordid
			<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " transed fix string " << out_fixReport.toString();

	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
	catch (...)
	{
		EzLog::e(ftag, "exception.");
		return -1;
	}

	return 0;
}

// ���ӻر�NoPartyIds�ظ���
int StgwFixReport::AddNoPartyIds(FIX::Message& report, const std::string& strSeat,
	const std::string& strAccount, const std::string& strMarket_branchid)
{
	FIX50::ExecutionReport::NoPartyIDs group_sub1;
	//FIX::Group group_sub1(FIX::FIELD::NoPartyIDs, 0);
	group_sub1.setField(FIX::FIELD::PartyID, strSeat);
	group_sub1.setField(FIX::FIELD::PartyIDSource, "C");
	group_sub1.setField(FIX::FIELD::PartyRole, "27");
	report.addGroup(group_sub1);

	FIX50::ExecutionReport::NoPartyIDs group_sub2;
	group_sub2.setField(FIX::FIELD::PartyID, strSeat);
	group_sub2.setField(FIX::FIELD::PartyIDSource, "C");
	group_sub2.setField(FIX::FIELD::PartyRole, "1");
	report.addGroup(group_sub2);

	FIX50::ExecutionReport::NoPartyIDs group_sub3;
	group_sub3.setField(FIX::FIELD::PartyID, strAccount);
	group_sub3.setField(FIX::FIELD::PartyIDSource, "5");
	group_sub3.setField(FIX::FIELD::PartyRole, "5");
	report.addGroup(group_sub3);

	FIX50::ExecutionReport::NoPartyIDs group_sub4;
	group_sub4.setField(FIX::FIELD::PartyID, strMarket_branchid);
	group_sub4.setField(FIX::FIELD::PartyIDSource, "D");
	group_sub4.setField(FIX::FIELD::PartyRole, "4001");
	report.addGroup(group_sub4);

	FIX50::ExecutionReport::NoPartyIDs group_sub5;
	group_sub5.setField(FIX::FIELD::PartyID, "01");
	group_sub5.setField(FIX::FIELD::PartyIDSource, "F");
	group_sub5.setField(FIX::FIELD::PartyRole, "4");
	report.addGroup(group_sub5);

	return 0;
}

// ���ӻر�NoSecurity�ظ���
int StgwFixReport::AddNoSecurity(FIX::Message& report,
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport)
{
	//etf component
	uint64_t ui64Num = 0;
	string strValue;
	std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent> ptrTemp(new struct simutgw::EtfCrt_FrozeComponent);
	for (size_t st = 0; st < in_ptrReport->vecFrozeComponent.size(); ++st)
	{
		ptrTemp = in_ptrReport->vecFrozeComponent[st];

		FIX::Group group_sub1(FIX::FIELD::NoSecurity, 0);
		group_sub1.setField(FIX::FIELD::UnderlyingSecurityID, ptrTemp->strSecurityID);
		group_sub1.setField(FIX::FIELD::UnderlyingSecurityIDSource, "102");

		if (0 == in_ptrReport->strSide.compare("D"))
		{
			ui64Num = ptrTemp->ui64act_pch_count + ptrTemp->ui64avl_count;
		}
		else if (0 == in_ptrReport->strSide.compare("E"))
		{
			ui64Num = ptrTemp->ui64rdp_count;
		}

		sof_string::itostr(ui64Num, strValue);
		group_sub1.setField(FIX::FIELD::DeliverQty, strValue);

		Tgw_StringUtil::iLiToStr(ptrTemp->ui64Cash, strValue, 2);
		group_sub1.setField(FIX::FIELD::SubstCash, strValue);

		report.addGroup(group_sub1);
	}

	return 0;
}

/*
����һ������ʧ�ܳɹ���ִ�б���ر�, msgtype = 9
*/
int StgwFixReport::ProcMsgType_9_Report(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	FIX::Message& fixReport)
{
	static const string ftag("StgwFixReport::ProcMsgType_9_Report() ");
	try
	{
		//beginstring, sendercompid, targetcompid
		StgwFixUtil::SetField(fixReport, FIX::FIELD::BeginString, in_ptrReport->strBeginString);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::SenderCompID, in_ptrReport->strTargetCompID);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::TargetCompID, in_ptrReport->strSenderCompID);

		// msgtype, reportindex, applid, ownertype, orderid
		StgwFixUtil::SetField(fixReport, FIX::FIELD::MsgType, in_ptrReport->strMsgType);

		//string strValue;
		//sof_string::itostr(simutgw::g_iReportIndex, strValue);
		string strValue = in_ptrReport->strSenderCompID;
		uint64_t ui64Num = 0;
		string strItoa;
		int iPartitionNo = 0;

		if (simutgw::g_mapSzConns.end() != simutgw::g_mapSzConns.find(strValue))
		{
			ui64Num = simutgw::g_mapSzConns[strValue].GetRptIdex();

			if (simutgw::g_bSZ_Step_ver110)
			{
				// ����STEP�ر���Ver1.10
				// ȡPartitionNo
				std::shared_ptr<std::map<int, uint64_t>> prtMapPati = simutgw::g_mapSzConns[strValue].GetPartitionsMap();
				if (nullptr != prtMapPati)
				{
					std::map<int, uint64_t>::iterator it = prtMapPati->begin();
					if (prtMapPati->end() != it)
					{
						iPartitionNo = it->first;
					}
				}
			}
		}
		sof_string::itostr(ui64Num, strItoa);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::ReportIndex, strItoa);

		if (simutgw::g_bSZ_Step_ver110)
		{
			sof_string::itostr(iPartitionNo, strItoa);
			StgwFixUtil::SetField(fixReport, FIX::FIELD::PartitionNo, strItoa);
		}

		StgwFixUtil::SetField(fixReport, FIX::FIELD::ApplID, in_ptrReport->strApplID);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::OwnerType, in_ptrReport->strOwnerType);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::OrderID, in_ptrReport->strOrderID);

		//	clordid, transactTime, origclordid, ordstatus, cxlrejreason
		StgwFixUtil::SetField(fixReport, FIX::FIELD::ClOrdID, in_ptrReport->strClordid);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::TransactTime, in_ptrReport->strTransactTime);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::OrigClOrdID, in_ptrReport->strOrigClordid);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::OrdStatus, in_ptrReport->strOrdStatus);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::CxlRejReason, in_ptrReport->strCxlRejReason);

		//	rejecttext, securityid, securityidsource, text
		StgwFixUtil::SetField(fixReport, FIX::FIELD::RejectText, in_ptrReport->strRejectText);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::SecurityID, in_ptrReport->strStockID);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::SecurityIDSource, in_ptrReport->strSecurityIDSource);
		StgwFixUtil::SetField(fixReport, FIX::FIELD::Text, in_ptrReport->strText);

		// ����ʧ��
		if (in_ptrReport->strSecurity_seat.empty())
		{
			// δ�ҵ�ϯλ��valueΪ��
			EzLog::e(ftag, "field security_seat is NULL");
			return -1;
		}
		string strSeat = in_ptrReport->strSecurity_seat;

		FIX50::ExecutionReport::NoPartyIDs group_sub1;
		group_sub1.setField(FIX::FIELD::PartyID, strSeat);
		group_sub1.setField(FIX::FIELD::PartyIDSource, "C");
		group_sub1.setField(FIX::FIELD::PartyRole, "1");
		fixReport.addGroup(group_sub1);
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
	catch (...)
	{
		EzLog::e(ftag, "exception.");
		return -1;
	}
	return 0;
}

/*
���������ظ���ҵ��ܾ���Ϣ
*/
int StgwFixReport::Send_SZ_RepeatRejectMsg(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{
	static const string ftag("StgwFixReport::CheckIfRepeatOrder_MySql() ");

	FIX::Message fixReject;

	//BeginString
	StgwFixUtil::SetField(fixReject, FIX::FIELD::BeginString, orderMsg->strBeginString);

	//MsgType
	StgwFixUtil::SetField(fixReject, FIX::FIELD::MsgType, "j");

	//SenderCompID
	StgwFixUtil::SetField(fixReject, FIX::FIELD::SenderCompID, orderMsg->strTargetCompID);

	//TargetCompID
	StgwFixUtil::SetField(fixReject, FIX::FIELD::TargetCompID, orderMsg->strSenderCompID);

	//RefSeqNum
	StgwFixUtil::SetField(fixReject, FIX::FIELD::RefSeqNum, orderMsg->strMsgSeqNum);

	//RefMsgType
	StgwFixUtil::SetField(fixReject, FIX::FIELD::RefMsgType, orderMsg->strMsgType);

	//ApplID
	StgwFixUtil::SetField(fixReject, FIX::FIELD::ApplID, orderMsg->strApplID);

	//TransactTime
	StgwFixUtil::SetField(fixReject, FIX::FIELD::TransactTime, orderMsg->strTransactTime);

	//SecurityID
	StgwFixUtil::SetField(fixReject, FIX::FIELD::SecurityID, orderMsg->strStockID);

	//SecurityIDSource
	StgwFixUtil::SetField(fixReject, FIX::FIELD::SecurityIDSource, orderMsg->strSecurityIDSource);

	FIX50::ExecutionReport::NoPartyIDs group_sub1;
	group_sub1.setField(FIX::FIELD::PartyID, orderMsg->strSecurity_seat);
	group_sub1.setField(FIX::FIELD::PartyIDSource, "C");
	group_sub1.setField(FIX::FIELD::PartyRole, "1");
	fixReject.addGroup(group_sub1);

	//BusinessRejectRefID
	StgwFixUtil::SetField(fixReject, FIX::FIELD::BusinessRejectRefID, orderMsg->strClordid);

	//BusinessRejectReason
	StgwFixUtil::SetField(fixReject, FIX::FIELD::BusinessRejectReason, "20099");

	//Text
	StgwFixUtil::SetField(fixReject, FIX::FIELD::Text, "order message repetition");

	simutgw::g_fixaccptor.SendMsg(fixReject);

	return 0;
}