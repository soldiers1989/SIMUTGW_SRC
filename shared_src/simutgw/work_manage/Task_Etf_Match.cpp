#include "Task_Etf_Match.h"

#include <memory>

#include "simutgw/order/OrderMemoryStoreFactory.h"
#include "order/StockOrderHelper.h"
#include "simutgw/db_oper/RecordNewOrderHelper.h"

#include "simutgw/db_oper/DbUserInfoAsset.h"

#include "simutgw/msg_biz/ProcCancelOrder.h"

#include "quotation/MarketInfoHelper.h"

#include "util/SystemCounter.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/sys_function.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "etf/ETFHelper.h"
#include "cache/UserStockHelper.h"

#include "GenTaskHelper.h"

/*
���׳ɽ�����
Return:
0 -- �ɽ�
-1 -- ʧ��
*/
int Task_Etf_Match::MatchOrder()
{
	static const string ftag("Task_Etf_Match::MatchOrder() ");
	
	// �ٴ���
	switch (m_orderMsg->tradePolicy.iMatchMode)
	{
		// ��������ɽ�
	case simutgw::SysMatchMode::EnAbleQuta:
		// ģ�� 
	case simutgw::SysMatchMode::SimulMatchAll:
	case simutgw::SysMatchMode::SimulMatchByDivide:
	case simutgw::SysMatchMode::SimulNotMatch:
	case simutgw::SysMatchMode::SimulErrMatch:
	case simutgw::SysMatchMode::SimulMatchPart:

		break;

	default:
		// 
		string strValue;
		string strError("Match mode[");
		strError += sof_string::itostr(m_orderMsg->tradePolicy.iMatchMode, strValue);
		strError += "] doesn't support";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strError;
		break;
	}

	enum simutgw::MatchType iMatchRes = simutgw::MatchType::NotMatch;

	if (0 == m_orderMsg->strSide.compare("D"))
	{
		// D=�깺
		iMatchRes = TradeMatch(m_orderMsg, true);
	}
	else if (0 == m_orderMsg->strSide.compare("E"))
	{
		// E=���		
		iMatchRes = TradeMatch(m_orderMsg, false);
	}
	else
	{
		string sDebug("����clordid[");
		sDebug += m_orderMsg->strClordid;
		sDebug += "] error side=[";
		sDebug += m_orderMsg->strSide;
		sDebug += "]";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;
		return -1;
	}

	{
		if (iMatchRes != simutgw::NotMatch)
		{
			string strItoa;
			string strDebug("����clordid[");
			strDebug += m_orderMsg->strClordid;
			strDebug += "]�ɽ����[";
			strDebug += sof_string::itostr(iMatchRes, strItoa);
			strDebug += "],price[";
			strDebug += sof_string::itostr(m_orderMsg->ui64mPrice_matched, strItoa);
			strDebug += "],cjsl[";
			strDebug += sof_string::itostr(m_orderMsg->ui64Orderqty_matched, strItoa);
			BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
		}
	}

	//�ж��Ƿ�ɽ�
	if (simutgw::MatchAll == iMatchRes)
	{
		//�ɽ�
		if (0 == m_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchAll();
		}
		else if (0 == m_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchAll();
		}
	}
	else if (simutgw::MatchPart == iMatchRes)
	{
		// ���ֳɽ�
		// Etf�����ڲ��ֳɽ�

		// Log
		string sDebug("����clordid[");
		sDebug += m_orderMsg->strClordid;
		sDebug += "]Etf�Ƿ����ֳɽ�";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;
		// д��ر�����
		simutgw::g_outMsg_buffer.PushBack(m_orderMsg);
	}
	else if (simutgw::NotMatch == iMatchRes)
	{
		//δ�ɽ�

		GenTaskHelper::GenTask_Match(m_orderMsg);
	}
	else if (simutgw::OutOfRange == iMatchRes)
	{
		// �����ǵ���
		// �����׽���
		m_orderMsg->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("����clordid[");
		sDebug += m_orderMsg->strClordid;
		sDebug += "]�����ǵ���";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

		// д��ر�����
		simutgw::g_outMsg_buffer.PushBack(m_orderMsg);

		return -1;
	}
	else if (simutgw::StopTrans == iMatchRes)
	{
		// ��Ʊ��ͣ��
		// �����׽���
		m_orderMsg->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("����clordid[");
		sDebug += m_orderMsg->strClordid;
		sDebug += "]��Ʊ��ͣ��";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

		// д��ر�����
		simutgw::g_outMsg_buffer.PushBack(m_orderMsg);

		return -1;

	}
	else if (simutgw::ErrorMatch == iMatchRes)
	{
		//���Ϸ�����
		m_orderMsg->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("����clordid[");
		sDebug += m_orderMsg->strClordid;
		sDebug += "]ErrorMatch";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

		// д��ر�����
		simutgw::g_outMsg_buffer.PushBack(m_orderMsg);

		return -1;
	}
	else
	{
		//
		string strItoa;
		string strDebug("����clordid[");
		strDebug += m_orderMsg->strClordid;
		strDebug += "]�ɽ����[";
		strDebug += sof_string::itostr(iMatchRes, strItoa);
		strDebug += "],price[";
		strDebug += sof_string::itostr(m_orderMsg->ui64mPrice_matched, strItoa);
		strDebug += "],cjsl[";
		strDebug += sof_string::itostr(m_orderMsg->ui64Orderqty_matched, strItoa);
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
	}

	return 0;
}

/*
��ETFί�н��д��

Return :
simutgw::MatchType
{
//ȫ���ɽ�
MatchAll = 0,
//���ֳɽ�
MatchPart = 1,
//δ�ɽ�
NotMatch = -1,
// �����ǵ���
OutOfRange = -2,
// ͣ��
StopTrans = -3,
// ���ײ��Ϸ�
ErrorMatch = -4
};
*/
enum simutgw::MatchType Task_Etf_Match::TradeMatch(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
	bool in_bIsBuy)
{
	static const string ftag("Task_Etf_Match::TradeMatch() ");

	try
	{
		in_ptrOrder->enMatchType = simutgw::NotMatch;

		// ��¼�ɽ����
		// ȫ���ɽ�
		in_ptrOrder->enMatchType = simutgw::MatchAll;
		in_ptrOrder->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		in_ptrOrder->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
		in_ptrOrder->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		//
		// �ѳɽ�����
		// ��������
		in_ptrOrder->ui64Orderqty_matched = in_ptrOrder->ui64LeavesQty;
		sof_string::itostr(in_ptrOrder->ui64Orderqty_matched, in_ptrOrder->strLastQty);
		// �۸�	
		in_ptrOrder->ui64mPrice_matched = in_ptrOrder->ui64mOrderPrice;
		Tgw_StringUtil::iLiToStr(in_ptrOrder->ui64mPrice_matched, in_ptrOrder->strLastPx, 4);
		// �������
		in_ptrOrder->ui64mCashorderqty_matched = in_ptrOrder->ui64mOrderPrice * in_ptrOrder->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(in_ptrOrder->ui64mCashorderqty_matched, in_ptrOrder->strCashorderqty, 4);
		//
		// δ�ɽ�����
		// ��������
		in_ptrOrder->ui64Orderqty_unmatched = 0;
		sof_string::itostr(in_ptrOrder->ui64Orderqty_unmatched, in_ptrOrder->strLeavesQty);
		//�ۼƳɽ���
		sof_string::itostr(in_ptrOrder->ui64Orderqty_origin, in_ptrOrder->strCumQty);
		// δ�ɽ��������
		in_ptrOrder->ui64mCashorderqty_unmatched = 0;

		// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
		Tgw_StringUtil::iLiToStr(in_ptrOrder->ui64mOrderPrice, in_ptrOrder->strOrderPrice, 4);


		// �����ڲ�������
		string strTransId;
		TimeStringUtil::ExRandom15(strTransId);
		//Ψһִ��id
		in_ptrOrder->strExecID = strTransId;
		//Ψһ����id
		in_ptrOrder->strOrderID = strTransId;

		// `trade_time` timestamp NULL DEFAULT NULL COMMENT '����ʱ��',
		TimeStringUtil::GetCurrTimeInTradeType(in_ptrOrder->strTrade_time);

		UserStockHelper::UpdateAfterTrade(in_ptrOrder);

		// ����������д�����ݿ�
		// д��ر�����
		simutgw::g_outMsg_buffer.PushBack(in_ptrOrder);
		
		return simutgw::MatchType::MatchAll;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
		return simutgw::MatchType::NotMatch;
	}


}


/*
Etf�깺���� �����ݿ��л�ȡ�û���ETF�ɷݹɳֲ֣����ҽ���ȱ�ڱȽϲ���ɽ���

Return :
0 -- ��ȡ�ɹ�
<0 -- ��ȡʧ��
1 -- ��ȯ����
2 -- ��������ֽ��������
*/
int Task_Etf_Match::Match_Creat_UserHoldToEtfComponent(
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
	std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf)
{
	static const string ftag("Task_Etf_Match::Match_Creat_UserHoldToEtfComponent()");

	try
	{
		int iRes = 0;

		//��mysql���ӳ�ȡ����
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//ȡ����mysql����ΪNULL

			//�黹����
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Get Connection is NULL";

			return -1;
		}

		std::vector<simutgw::SzETFComponent>::const_iterator cit;

		std::shared_ptr<struct simutgw::TradeStock> ptrUserStock;

		for (cit = in_ptrEtf->vecComponents.begin();
			in_ptrEtf->vecComponents.end() != cit; ++cit)
		{
			// �ֽ������־SubstituteFlag C1
			if (2 == cit->iSubstituteFlag)
			{
				// 2 = �������ֽ����

				// ��¼�û���������
				in_ptrOrder->etfOrderStatus.ui64mCashComponent += cit->ui64mCreationCashSubstitute;
			}
			else
			{
				// 1 = ���Խ����ֽ����������֤ȯ��֤ȯ����ʱ�������ֽ������
				// 0 = ��ֹ�ֽ������������֤ȯ��

				ptrUserStock = std::shared_ptr<struct simutgw::TradeStock>(
					new simutgw::TradeStock());

				iRes = DbUserInfoAsset::GetDb_UserHoldStock(mysqlConn, in_ptrOrder->strAccountId,
					cit->strUnderlyingSecurityID, ptrUserStock);
				if (0 == iRes)
				{
					// �û����и�ֻ��Ʊ
					struct simutgw::TradeEtfComponent etfComp;
					etfComp.strUnderlyingSecurityID = cit->strUnderlyingSecurityID;

					// Etf�����깺��Ʊ����
					uint64_t ui64NeedForCreation = in_ptrOrder->ui64Orderqty_origin * cit->ui64ComponentShare;
					// ������ֹ����е��м����
					uint64_t ui64_SubRest = 0;

					// �������깺�Ĺ�Ʊ����
					// ���������� + ֤ȯ���п������������깺etf�ݶ�Ϳɾ�������
					uint64_t ui64CouldUseForCreation = ptrUserStock->ui64Stock_auction_purchase_beforematch
						+ ptrUserStock->ui64StockAvailable_beforematch;

					if (ui64NeedForCreation <= ui64CouldUseForCreation)
					{
						// ���������㹻
						etfComp.bIsUserHolding = true;

						// �����ֽ����
						etfComp.bIsNeedCashComponent = false;

						// �������׵Ĺ�Ʊ����
						etfComp.ui64StockExchange = ui64NeedForCreation;

						//
						// �û�����ǰ�ֲ�
						etfComp.userHold.ui64StockBalance_beforematch = ptrUserStock->ui64StockBalance_beforematch;
						// �û����׺�ֲ�
						etfComp.userHold.ui64StockBalance_aftermatch =
							ptrUserStock->ui64StockBalance_beforematch - ui64NeedForCreation;

						//
						// ���������� ����ǰ
						etfComp.userHold.ui64Stock_auction_purchase_beforematch = ptrUserStock->ui64Stock_auction_purchase_beforematch;
						// ���׺�
						if (ui64NeedForCreation <= ptrUserStock->ui64Stock_auction_purchase_beforematch)
						{
							// 
							etfComp.userHold.ui64Stock_auction_purchase_aftermatch =
								ptrUserStock->ui64Stock_auction_purchase_beforematch - ui64NeedForCreation;
							ui64_SubRest = 0;
						}
						else
						{
							etfComp.userHold.ui64Stock_auction_purchase_aftermatch = 0;
							ui64_SubRest = ui64NeedForCreation
								- ptrUserStock->ui64Stock_auction_purchase_beforematch;
						}

						//
						// ����������
						// ����ǰ
						etfComp.userHold.ui64Stock_staple_purchase_beforematch = ptrUserStock->ui64Stock_staple_purchase_beforematch;
						// ���׺�
						etfComp.userHold.ui64Stock_staple_purchase_aftermatch = ptrUserStock->ui64Stock_staple_purchase_beforematch;

						//
						// etf��������ɾ������� �� etf�깺�����ɾ�������
						// ����ǰ
						etfComp.userHold.ui64Stock_etfredemption_creation_beforematch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch;
						// ���׺�
						etfComp.userHold.ui64Stock_etfredemption_creation_aftermatch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch;

						//
						// ֤ȯ���п������������깺etf�ݶ�Ϳɾ�������
						// �û����óֲ� ����ǰ
						etfComp.userHold.ui64StockAvailable_beforematch = ptrUserStock->ui64StockAvailable_beforematch;
						// �û����óֲ� ���׺�
						etfComp.userHold.ui64StockAvailable_aftermatch = ptrUserStock->ui64StockAvailable_beforematch - ui64_SubRest;

						ui64_SubRest = 0;

						in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);
					}
					else // if ( ui64NeedForCreation <= ui64CouldUseForCreation )
					{
						// �Ƿ���������ɽ�
						if (simutgw::SysMatchMode::EnAbleQuta == in_ptrOrder->tradePolicy.iMatchMode)
						{
							//�黹����
							simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

							string sDebug("Get Quotation failed zqdm=[");
							sDebug += in_ptrOrder->strStockID;
							sDebug += "]";
							BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

							return 1;
						}

						// ȱ�ٵ�֤ȯ�ɷ�����
						uint64_t ui64CreationShort = ui64NeedForCreation - ui64CouldUseForCreation;

						// ������������
						if (1 == cit->iSubstituteFlag)
						{
							// 1 = ���Խ����ֽ����������֤ȯ��֤ȯ����ʱ�������ֽ������
							// ��¼�û���������

							simutgw::uint64_t_Money ui64m_Quot_MaxGain = 0;
							simutgw::uint64_t_Money ui64m_Quot_MinFall = 0;
							uint64_t ui64_Quot_Cjsl = 0;
							simutgw::uint64_t_Money ui64_Quot_Cjje = 0;
							string out_strHqsj;
							string out_strTpbz;

							int iGetQuot = MarketInfoHelper::GetCurrQuotGapByCircle_RecentPrice(
								in_ptrOrder->strStockID, in_ptrOrder->strTrade_group,
								ui64m_Quot_MaxGain, ui64m_Quot_MinFall, ui64_Quot_Cjsl, ui64_Quot_Cjje,
								out_strHqsj, out_strTpbz);
							if (0 != iGetQuot)
							{
								// ��ȡ����ʧ�ܣ�����ʧ��

								//�黹����
								simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

								string sDebug("Get Quotation failed zqdm=[");
								sDebug += in_ptrOrder->strStockID;
								sDebug += "]";
								BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

								return 1;
							}

							simutgw::uint64_t_Money ui64m_CashComponent = ui64CreationShort * (uint64_t)(ui64_Quot_Cjje * cit->dPremiumRatio);
							in_ptrOrder->etfOrderStatus.ui64mCashComponent += ui64m_CashComponent;

							// ���������㹻
							etfComp.bIsUserHolding = false;

							// �����ֽ����
							etfComp.bIsNeedCashComponent = true;

							etfComp.ui64mCashComponent = ui64m_CashComponent;

							//
							// �û�����ǰ�ֲ�
							etfComp.userHold.ui64StockBalance_beforematch = ptrUserStock->ui64StockBalance_beforematch;
							// �û����׺�ֲ�
							etfComp.userHold.ui64StockBalance_aftermatch =
								ptrUserStock->ui64StockBalance_beforematch - ui64CouldUseForCreation;

							//
							// ���������� ����ǰ
							etfComp.userHold.ui64Stock_auction_purchase_beforematch = ptrUserStock->ui64Stock_auction_purchase_beforematch;
							// ���׺�
							etfComp.userHold.ui64Stock_auction_purchase_aftermatch = 0;

							//
							// ����������
							// ����ǰ
							etfComp.userHold.ui64Stock_staple_purchase_beforematch = ptrUserStock->ui64Stock_staple_purchase_beforematch;
							// ���׺�
							etfComp.userHold.ui64Stock_staple_purchase_aftermatch = ptrUserStock->ui64Stock_staple_purchase_beforematch;

							//
							// etf��������ɾ������� �� etf�깺�����ɾ�������
							// ����ǰ
							etfComp.userHold.ui64Stock_etfredemption_creation_beforematch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch;
							// ���׺�
							etfComp.userHold.ui64Stock_etfredemption_creation_aftermatch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch;

							//
							// ֤ȯ���п������������깺etf�ݶ�Ϳɾ�������
							// �û����óֲ� ����ǰ
							etfComp.userHold.ui64StockAvailable_beforematch = ptrUserStock->ui64StockAvailable_beforematch;
							// �û����óֲ� ���׺�
							etfComp.userHold.ui64StockAvailable_aftermatch = 0;

							ui64_SubRest = 0;

							in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);
						}
						else if (0 == cit->iSubstituteFlag)
						{
							// 0 = ��ֹ�ֽ������������֤ȯ��

							// �û������У�����ʧ��
							//�黹����
							simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

							string sDebug("�û�AccountId=[");
							sDebug += in_ptrOrder->strAccountId;
							sDebug += "] �����йɷ�zqdm=[";
							sDebug += in_ptrOrder->strStockID;
							sDebug += "]";
							BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

							return 1;
						}
					}
				}
				else if (1 == iRes)
				{
					// �û������и�ֻ��Ʊ

					if (1 == cit->iSubstituteFlag)
					{
						// 1 = ���Խ����ֽ����������֤ȯ��֤ȯ����ʱ�������ֽ������
						// ��¼�û���������
						in_ptrOrder->etfOrderStatus.ui64mCashComponent += cit->ui64mCreationCashSubstitute;
					}
					else if (0 == cit->iSubstituteFlag)
					{
						// 0 = ��ֹ�ֽ������������֤ȯ��

						// �û������У�����ʧ��

						//�黹����
						simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

						string sDebug("�û�AccountId=[");
						sDebug += in_ptrOrder->strAccountId;
						sDebug += "] �����йɷ�zqdm=[";
						sDebug += in_ptrOrder->strStockID;
						sDebug += "]";
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

						return 1;
					}

				}
				else
				{
					//�黹����
					simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetDb_UserHoldStock error";
					return -2;
				}
			} // if ( 2 == cit->iSubstituteFlag ) else

		}

		//�黹����
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		// �����Ƿ񳬹� ����ֽ��������
		// MaxCashRatio	N6(5)
		// ����ֽ�������������磺5.551�� ���ļ�����0.05551��ʾ

		// ��ǰETF�ľ�ֵ
		simutgw::uint64_t_Money ui64m_etfTotalValue =
			in_ptrOrder->ui64Orderqty_origin * in_ptrEtf->ui64mNAV;
		// �� ����ֽ�������� ���������������ֽ���
		simutgw::uint64_t_Money ui64m_MaxAllowCashComp = (uint64_t)(ui64m_etfTotalValue * (in_ptrEtf->dMaxCashRatio));

		if (in_ptrOrder->etfOrderStatus.ui64mCashComponent > ui64m_MaxAllowCashComp)
		{
			// ��ǰ�ֽ�������ܽ�� ���� ����ֽ��������
			string sItoa;
			string sDebug("etf out MaxCashRatio, zqdm=[");
			sDebug += in_ptrOrder->strStockID;
			sDebug += "], AccountId=[";
			sDebug += in_ptrOrder->strAccountId;
			sDebug += "], MaxAllowCashComp=[";
			sDebug += sof_string::itostr(ui64m_MaxAllowCashComp, sItoa);
			sDebug += "], CurrentCashComponent=[";
			sDebug += sof_string::itostr(in_ptrOrder->etfOrderStatus.ui64mCashComponent, sItoa);
			sDebug += "]";

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

			return 2;
		}

	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

/*
Etf�깺���� ֱ�Ӱ��յ�ETF�ɷݹ���ɽ���

Return :
0 -- ��ȡ�ɹ�
<0 -- ��ȡʧ��
1 -- ��ȯ����
2 -- ��������ֽ��������
*/
int Task_Etf_Match::Match_Simul_Creat_EtfComponent(
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
	std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf)
{
	static const string ftag("Task_Etf_Match::Match_Simul_Creat_EtfComponent() ");

	try
	{
		std::vector<simutgw::SzETFComponent>::const_iterator cit;

		for (cit = in_ptrEtf->vecComponents.begin();
			in_ptrEtf->vecComponents.end() != cit; ++cit)
		{
			// �ֽ������־SubstituteFlag C1
			if (2 == cit->iSubstituteFlag)
			{
				// 2 = �������ֽ����

				// ��¼�û���������
				in_ptrOrder->etfOrderStatus.ui64mCashComponent += cit->ui64mCreationCashSubstitute;
			}
			else
			{
				// 1 = ���Խ����ֽ����������֤ȯ��֤ȯ����ʱ�������ֽ������
				// 0 = ��ֹ�ֽ������������֤ȯ��

				// ����ɷ�ʱ����֤ȯ�ķ�ʽ���гɽ�

				// Etf�����깺��Ʊ����
				uint64_t ui64NeedForCreation = in_ptrOrder->ui64Orderqty_origin * cit->ui64ComponentShare;

				struct simutgw::TradeEtfComponent etfComp;
				etfComp.strUnderlyingSecurityID = cit->strUnderlyingSecurityID;

				// ���������㹻
				etfComp.bIsUserHolding = true;

				// �����ֽ����
				etfComp.bIsNeedCashComponent = false;

				// �������׵Ĺ�Ʊ����
				etfComp.ui64StockExchange = ui64NeedForCreation;

				in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);
			}

		}

		// �����Ƿ񳬹� ����ֽ��������
		// MaxCashRatio	N6(5)
		// ����ֽ�������������磺5.551�� ���ļ�����0.05551��ʾ

		// ��ǰETF�ľ�ֵ
		simutgw::uint64_t_Money ui64m_etfTotalValue =
			in_ptrOrder->ui64Orderqty_origin * in_ptrEtf->ui64mNAV;
		// �� ����ֽ�������� ���������������ֽ���
		simutgw::uint64_t_Money ui64m_MaxAllowCashComp = (uint64_t)(ui64m_etfTotalValue * (in_ptrEtf->dMaxCashRatio));

		if (in_ptrOrder->etfOrderStatus.ui64mCashComponent > ui64m_MaxAllowCashComp)
		{
			// ��ǰ�ֽ�������ܽ�� ���� ����ֽ��������
			string sItoa;
			string sDebug("etf out MaxCashRatio, zqdm=[");
			sDebug += in_ptrOrder->strStockID;
			sDebug += "], AccountId=[";
			sDebug += in_ptrOrder->strAccountId;
			sDebug += "], MaxAllowCashComp=[";
			sDebug += sof_string::itostr(ui64m_MaxAllowCashComp, sItoa);
			sDebug += "], CurrentCashComponent=[";
			sDebug += sof_string::itostr(in_ptrOrder->etfOrderStatus.ui64mCashComponent, sItoa);
			sDebug += "]";

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

			return 2;
		}

	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

/*
Etf��ؽ��� �����ݿ��л�ȡ�û���ETF�ɷݹɳֲ֣����Ҽ�����ص���������ɽ���

Return :
0 -- ��ȡ�ɹ�
-1 -- ��ȡʧ��
*/
int Task_Etf_Match::Match_Redemp_UserHoldToEtfComponent(
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
	std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf)
{
	static const string ftag("Task_Etf_Match::Match_Redemp_UserHoldToEtfComponent()");

	try
	{
		int iRes = 0;

		//��mysql���ӳ�ȡ����
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//ȡ����mysql����ΪNULL

			//�黹����
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Get Connection is NULL";

			return -1;
		}

		std::vector<simutgw::SzETFComponent>::const_iterator cit;

		std::shared_ptr<struct simutgw::TradeStock> ptrUserStock;

		for (cit = in_ptrEtf->vecComponents.begin();
			in_ptrEtf->vecComponents.end() != cit; ++cit)
		{
			// �ֽ������־SubstituteFlag C1
			if (2 == cit->iSubstituteFlag)
			{
				// 2 = �������ֽ����

				// ��¼�û���������
				in_ptrOrder->etfOrderStatus.ui64mCashComponent += cit->ui64mRedemptionCashSubstitute;
			}
			else
			{
				// 1 = ���Խ����ֽ����������֤ȯ��֤ȯ����ʱ�������ֽ������
				// 0 = ��ֹ�ֽ������������֤ȯ��

				ptrUserStock = std::shared_ptr<struct simutgw::TradeStock>(
					new simutgw::TradeStock());

				iRes = DbUserInfoAsset::GetDb_UserHoldStock(mysqlConn, in_ptrOrder->strAccountId,
					cit->strUnderlyingSecurityID, ptrUserStock);
				if (0 == iRes)
				{
					// �û����и�ֻ��Ʊ
					struct simutgw::TradeEtfComponent etfComp;

					etfComp.strUnderlyingSecurityID = cit->strUnderlyingSecurityID;

					// Etf��غ��Ʊ����
					uint64_t ui64NumAfterRedemption = in_ptrOrder->ui64Orderqty_origin * cit->ui64ComponentShare;

					// ����
					etfComp.bIsUserHolding = true;
					// �����ֽ����
					etfComp.bIsNeedCashComponent = false;

					etfComp.ui64StockExchange = ui64NumAfterRedemption;

					//
					// �û�����ǰ�ֲ�
					etfComp.userHold.ui64StockBalance_beforematch = ptrUserStock->ui64StockBalance_beforematch;
					// �û����׺�ֲ�
					etfComp.userHold.ui64StockBalance_aftermatch =
						ptrUserStock->ui64StockBalance_beforematch + ui64NumAfterRedemption;

					//
					// ���������� ����ǰ
					etfComp.userHold.ui64Stock_auction_purchase_beforematch = ptrUserStock->ui64Stock_auction_purchase_beforematch;
					// ���׺�
					etfComp.userHold.ui64Stock_auction_purchase_aftermatch =
						ptrUserStock->ui64Stock_auction_purchase_beforematch;

					//
					// ����������
					// ����ǰ
					etfComp.userHold.ui64Stock_staple_purchase_beforematch = ptrUserStock->ui64Stock_staple_purchase_beforematch;
					// ���׺�
					etfComp.userHold.ui64Stock_staple_purchase_aftermatch = ptrUserStock->ui64Stock_staple_purchase_beforematch;

					//
					// etf��������ɾ������� �� etf�깺�����ɾ�������
					// ����ǰ
					etfComp.userHold.ui64Stock_etfredemption_creation_beforematch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch;
					// ���׺�
					etfComp.userHold.ui64Stock_etfredemption_creation_aftermatch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch + ui64NumAfterRedemption;

					//
					// ֤ȯ���п������������깺etf�ݶ�Ϳɾ�������
					// �û����óֲ� ����ǰ
					etfComp.userHold.ui64StockAvailable_beforematch = ptrUserStock->ui64StockAvailable_beforematch;
					// �û����óֲ� ���׺�
					etfComp.userHold.ui64StockAvailable_aftermatch = ptrUserStock->ui64StockAvailable_beforematch;

					in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);

				}
				else if (1 == iRes)
				{
					// �û������и�ֻ��Ʊ
					struct simutgw::TradeEtfComponent etfComp;

					etfComp.strUnderlyingSecurityID = cit->strUnderlyingSecurityID;

					// Etf��غ��Ʊ����
					uint64_t ui64NumAfterRedemption = in_ptrOrder->ui64Orderqty_origin * cit->ui64ComponentShare;

					// ������
					etfComp.bIsUserHolding = false;
					// �����ֽ����
					etfComp.bIsNeedCashComponent = false;

					//
					etfComp.ui64StockExchange = ui64NumAfterRedemption;

					//
					// �û�����ǰ�ֲ�
					etfComp.userHold.ui64StockBalance_beforematch = 0;
					// �û����׺�ֲ�
					etfComp.userHold.ui64StockBalance_aftermatch = ui64NumAfterRedemption;

					//
					// ���������� ����ǰ
					etfComp.userHold.ui64Stock_auction_purchase_beforematch = 0;
					// ���׺�
					etfComp.userHold.ui64Stock_auction_purchase_aftermatch = 0;

					//
					// ����������
					// ����ǰ
					etfComp.userHold.ui64Stock_staple_purchase_beforematch = 0;
					// ���׺�
					etfComp.userHold.ui64Stock_staple_purchase_aftermatch = 0;

					//
					// etf��������ɾ������� �� etf�깺�����ɾ�������
					// ����ǰ
					etfComp.userHold.ui64Stock_etfredemption_creation_beforematch = 0;
					// ���׺�
					etfComp.userHold.ui64Stock_etfredemption_creation_aftermatch = ui64NumAfterRedemption;

					//
					// ֤ȯ���п������������깺etf�ݶ�Ϳɾ�������
					// �û����óֲ� ����ǰ
					etfComp.userHold.ui64StockAvailable_beforematch = 0;
					// �û����óֲ� ���׺�
					etfComp.userHold.ui64StockAvailable_aftermatch = 0;

					in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);
				}
				else
				{
					//�黹����
					simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetDb_UserHoldStock error";
					return -2;
				}
			} // if ( 2 == cit->iSubstituteFlag ) else

		}

		//�黹����
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		return 0;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

/*
Etf��ؽ��� ֱ�Ӱ��յ�ETF�ɷݹ���ɽ���

Return :
0 -- ��ȡ�ɹ�
<0 -- ��ȡʧ��
1 -- ��ȯ����
2 -- ��������ֽ��������
*/
int Task_Etf_Match::Match_Simul_Redemp_EtfComponent(
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
	std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf)
{
	static const string ftag("Task_Etf_Match::Match_Simul_Redemp_EtfComponent() ");

	try
	{
		std::vector<simutgw::SzETFComponent>::const_iterator cit;

		for (cit = in_ptrEtf->vecComponents.begin();
			in_ptrEtf->vecComponents.end() != cit; ++cit)
		{
			// �ֽ������־SubstituteFlag C1
			if (2 == cit->iSubstituteFlag)
			{
				// 2 = �������ֽ����

				// ��¼�û���������
				in_ptrOrder->etfOrderStatus.ui64mCashComponent += cit->ui64mCreationCashSubstitute;
			}
			else
			{
				// 1 = ���Խ����ֽ����������֤ȯ��֤ȯ����ʱ�������ֽ������
				// 0 = ��ֹ�ֽ������������֤ȯ��

				// ����ɷ�ʱ����֤ȯ�ķ�ʽ���гɽ�

				struct simutgw::TradeEtfComponent etfComp;
				etfComp.strUnderlyingSecurityID = cit->strUnderlyingSecurityID;

				// Etf��غ��Ʊ����
				uint64_t ui64NumAfterRedemption = in_ptrOrder->ui64Orderqty_origin * cit->ui64ComponentShare;

				// ���������㹻
				etfComp.bIsUserHolding = true;

				// �����ֽ����
				etfComp.bIsNeedCashComponent = false;

				//
				etfComp.ui64StockExchange = ui64NumAfterRedemption;

				in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);
			}

		}

		// �����Ƿ񳬹� ����ֽ��������
		// MaxCashRatio	N6(5)
		// ����ֽ�������������磺5.551�� ���ļ�����0.05551��ʾ

		// ��ǰETF�ľ�ֵ
		simutgw::uint64_t_Money ui64m_etfTotalValue =
			in_ptrOrder->ui64Orderqty_origin * in_ptrEtf->ui64mNAV;
		// �� ����ֽ�������� ���������������ֽ���
		simutgw::uint64_t_Money ui64m_MaxAllowCashComp = (uint64_t)(ui64m_etfTotalValue * (in_ptrEtf->dMaxCashRatio));

		if (in_ptrOrder->etfOrderStatus.ui64mCashComponent > ui64m_MaxAllowCashComp)
		{
			// ��ǰ�ֽ�������ܽ�� ���� ����ֽ��������
			string sItoa;
			string sDebug("etf out MaxCashRatio, zqdm=[");
			sDebug += in_ptrOrder->strStockID;
			sDebug += "], AccountId=[";
			sDebug += in_ptrOrder->strAccountId;
			sDebug += "], MaxAllowCashComp=[";
			sDebug += sof_string::itostr(ui64m_MaxAllowCashComp, sItoa);
			sDebug += "], CurrentCashComponent=[";
			sDebug += sof_string::itostr(in_ptrOrder->etfOrderStatus.ui64mCashComponent, sItoa);
			sDebug += "]";

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

			return 2;
		}

	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}
