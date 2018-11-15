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
交易成交函数
Return:
0 -- 成交
-1 -- 失败
*/
int Task_Etf_Match::MatchOrder()
{
	static const string ftag("Task_Etf_Match::MatchOrder() ");
	
	// 再处理
	switch (m_orderMsg->tradePolicy.iMatchMode)
	{
		// 启用行情成交
	case simutgw::SysMatchMode::EnAbleQuta:
		// 模拟 
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
		// D=申购
		iMatchRes = TradeMatch(m_orderMsg, true);
	}
	else if (0 == m_orderMsg->strSide.compare("E"))
	{
		// E=赎回		
		iMatchRes = TradeMatch(m_orderMsg, false);
	}
	else
	{
		string sDebug("订单clordid[");
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
			string strDebug("订单clordid[");
			strDebug += m_orderMsg->strClordid;
			strDebug += "]成交结果[";
			strDebug += sof_string::itostr(iMatchRes, strItoa);
			strDebug += "],price[";
			strDebug += sof_string::itostr(m_orderMsg->ui64mPrice_matched, strItoa);
			strDebug += "],cjsl[";
			strDebug += sof_string::itostr(m_orderMsg->ui64Orderqty_matched, strItoa);
			BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
		}
	}

	//判断是否成交
	if (simutgw::MatchAll == iMatchRes)
	{
		//成交
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
		// 部分成交
		// Etf不存在部分成交

		// Log
		string sDebug("订单clordid[");
		sDebug += m_orderMsg->strClordid;
		sDebug += "]Etf非法部分成交";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;
		// 写入回报队列
		simutgw::g_outMsg_buffer.PushBack(m_orderMsg);
	}
	else if (simutgw::NotMatch == iMatchRes)
	{
		//未成交

		GenTaskHelper::GenTask_Match(m_orderMsg);
	}
	else if (simutgw::OutOfRange == iMatchRes)
	{
		// 超出涨跌幅
		// 将交易结束
		m_orderMsg->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("订单clordid[");
		sDebug += m_orderMsg->strClordid;
		sDebug += "]超出涨跌幅";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

		// 写入回报队列
		simutgw::g_outMsg_buffer.PushBack(m_orderMsg);

		return -1;
	}
	else if (simutgw::StopTrans == iMatchRes)
	{
		// 股票已停牌
		// 将交易结束
		m_orderMsg->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("订单clordid[");
		sDebug += m_orderMsg->strClordid;
		sDebug += "]股票已停牌";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

		// 写入回报队列
		simutgw::g_outMsg_buffer.PushBack(m_orderMsg);

		return -1;

	}
	else if (simutgw::ErrorMatch == iMatchRes)
	{
		//不合法交易
		m_orderMsg->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("订单clordid[");
		sDebug += m_orderMsg->strClordid;
		sDebug += "]ErrorMatch";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

		// 写入回报队列
		simutgw::g_outMsg_buffer.PushBack(m_orderMsg);

		return -1;
	}
	else
	{
		//
		string strItoa;
		string strDebug("订单clordid[");
		strDebug += m_orderMsg->strClordid;
		strDebug += "]成交结果[";
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
enum simutgw::MatchType Task_Etf_Match::TradeMatch(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
	bool in_bIsBuy)
{
	static const string ftag("Task_Etf_Match::TradeMatch() ");

	try
	{
		in_ptrOrder->enMatchType = simutgw::NotMatch;

		// 记录成交结果
		// 全部成交
		in_ptrOrder->enMatchType = simutgw::MatchAll;
		in_ptrOrder->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		in_ptrOrder->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
		in_ptrOrder->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		//
		// 已成交部分
		// 订单数量
		in_ptrOrder->ui64Orderqty_matched = in_ptrOrder->ui64LeavesQty;
		sof_string::itostr(in_ptrOrder->ui64Orderqty_matched, in_ptrOrder->strLastQty);
		// 价格	
		in_ptrOrder->ui64mPrice_matched = in_ptrOrder->ui64mOrderPrice;
		Tgw_StringUtil::iLiToStr(in_ptrOrder->ui64mPrice_matched, in_ptrOrder->strLastPx, 4);
		// 订单金额
		in_ptrOrder->ui64mCashorderqty_matched = in_ptrOrder->ui64mOrderPrice * in_ptrOrder->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(in_ptrOrder->ui64mCashorderqty_matched, in_ptrOrder->strCashorderqty, 4);
		//
		// 未成交部分
		// 订单数量
		in_ptrOrder->ui64Orderqty_unmatched = 0;
		sof_string::itostr(in_ptrOrder->ui64Orderqty_unmatched, in_ptrOrder->strLeavesQty);
		//累计成交量
		sof_string::itostr(in_ptrOrder->ui64Orderqty_origin, in_ptrOrder->strCumQty);
		// 未成交订单金额
		in_ptrOrder->ui64mCashorderqty_unmatched = 0;

		// 成交，委托价格strOrderPrice变为原样
		Tgw_StringUtil::iLiToStr(in_ptrOrder->ui64mOrderPrice, in_ptrOrder->strOrderPrice, 4);


		// 生成内部订单号
		string strTransId;
		TimeStringUtil::ExRandom15(strTransId);
		//唯一执行id
		in_ptrOrder->strExecID = strTransId;
		//唯一订单id
		in_ptrOrder->strOrderID = strTransId;

		// `trade_time` timestamp NULL DEFAULT NULL COMMENT '交易时间',
		TimeStringUtil::GetCurrTimeInTradeType(in_ptrOrder->strTrade_time);

		UserStockHelper::UpdateAfterTrade(in_ptrOrder);

		// 将交易数据写入数据库
		// 写入回报队列
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
Etf申购交易 在数据库中获取用户的ETF成份股持仓，并且进行缺口比较并完成交易

Return :
0 -- 获取成功
<0 -- 获取失败
1 -- 余券不足
2 -- 超过最大现金替代比例
*/
int Task_Etf_Match::Match_Creat_UserHoldToEtfComponent(
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
	std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf)
{
	static const string ftag("Task_Etf_Match::Match_Creat_UserHoldToEtfComponent()");

	try
	{
		int iRes = 0;

		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Get Connection is NULL";

			return -1;
		}

		std::vector<simutgw::SzETFComponent>::const_iterator cit;

		std::shared_ptr<struct simutgw::TradeStock> ptrUserStock;

		for (cit = in_ptrEtf->vecComponents.begin();
			in_ptrEtf->vecComponents.end() != cit; ++cit)
		{
			// 现金替代标志SubstituteFlag C1
			if (2 == cit->iSubstituteFlag)
			{
				// 2 = 必须用现金替代

				// 记录用户的替代金额
				in_ptrOrder->etfOrderStatus.ui64mCashComponent += cit->ui64mCreationCashSubstitute;
			}
			else
			{
				// 1 = 可以进行现金替代（先用证券，证券不足时差额部分用现金替代）
				// 0 = 禁止现金替代（必须有证券）

				ptrUserStock = std::shared_ptr<struct simutgw::TradeStock>(
					new simutgw::TradeStock());

				iRes = DbUserInfoAsset::GetDb_UserHoldStock(mysqlConn, in_ptrOrder->strAccountId,
					cit->strUnderlyingSecurityID, ptrUserStock);
				if (0 == iRes)
				{
					// 用户持有该只股票
					struct simutgw::TradeEtfComponent etfComp;
					etfComp.strUnderlyingSecurityID = cit->strUnderlyingSecurityID;

					// Etf所需申购股票数量
					uint64_t ui64NeedForCreation = in_ptrOrder->ui64Orderqty_origin * cit->ui64ComponentShare;
					// 计算减持过程中的中间变量
					uint64_t ui64_SubRest = 0;

					// 可用于申购的股票数量
					// 竞价买入量 + 证券持有可用余额，可用于申购etf份额和可竞价卖出
					uint64_t ui64CouldUseForCreation = ptrUserStock->ui64Stock_auction_purchase_beforematch
						+ ptrUserStock->ui64StockAvailable_beforematch;

					if (ui64NeedForCreation <= ui64CouldUseForCreation)
					{
						// 可用数量足够
						etfComp.bIsUserHolding = true;

						// 不需现金替代
						etfComp.bIsNeedCashComponent = false;

						// 发生交易的股票数量
						etfComp.ui64StockExchange = ui64NeedForCreation;

						//
						// 用户交易前持仓
						etfComp.userHold.ui64StockBalance_beforematch = ptrUserStock->ui64StockBalance_beforematch;
						// 用户交易后持仓
						etfComp.userHold.ui64StockBalance_aftermatch =
							ptrUserStock->ui64StockBalance_beforematch - ui64NeedForCreation;

						//
						// 竞价买入量 交易前
						etfComp.userHold.ui64Stock_auction_purchase_beforematch = ptrUserStock->ui64Stock_auction_purchase_beforematch;
						// 交易后
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
						// 大宗买入量
						// 交易前
						etfComp.userHold.ui64Stock_staple_purchase_beforematch = ptrUserStock->ui64Stock_staple_purchase_beforematch;
						// 交易后
						etfComp.userHold.ui64Stock_staple_purchase_aftermatch = ptrUserStock->ui64Stock_staple_purchase_beforematch;

						//
						// etf赎回量，可竞价卖出 或 etf申购量，可竞价卖出
						// 交易前
						etfComp.userHold.ui64Stock_etfredemption_creation_beforematch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch;
						// 交易后
						etfComp.userHold.ui64Stock_etfredemption_creation_aftermatch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch;

						//
						// 证券持有可用余额，可用于申购etf份额和可竞价卖出
						// 用户可用持仓 交易前
						etfComp.userHold.ui64StockAvailable_beforematch = ptrUserStock->ui64StockAvailable_beforematch;
						// 用户可用持仓 交易后
						etfComp.userHold.ui64StockAvailable_aftermatch = ptrUserStock->ui64StockAvailable_beforematch - ui64_SubRest;

						ui64_SubRest = 0;

						in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);
					}
					else // if ( ui64NeedForCreation <= ui64CouldUseForCreation )
					{
						// 是否启用行情成交
						if (simutgw::SysMatchMode::EnAbleQuta == in_ptrOrder->tradePolicy.iMatchMode)
						{
							//归还连接
							simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

							string sDebug("Get Quotation failed zqdm=[");
							sDebug += in_ptrOrder->strStockID;
							sDebug += "]";
							BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

							return 1;
						}

						// 缺少的证券股份数量
						uint64_t ui64CreationShort = ui64NeedForCreation - ui64CouldUseForCreation;

						// 可用数量不足
						if (1 == cit->iSubstituteFlag)
						{
							// 1 = 可以进行现金替代（先用证券，证券不足时差额部分用现金替代）
							// 记录用户的替代金额

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
								// 获取行情失败，交易失败

								//归还连接
								simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

								string sDebug("Get Quotation failed zqdm=[");
								sDebug += in_ptrOrder->strStockID;
								sDebug += "]";
								BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

								return 1;
							}

							simutgw::uint64_t_Money ui64m_CashComponent = ui64CreationShort * (uint64_t)(ui64_Quot_Cjje * cit->dPremiumRatio);
							in_ptrOrder->etfOrderStatus.ui64mCashComponent += ui64m_CashComponent;

							// 可用数量足够
							etfComp.bIsUserHolding = false;

							// 不需现金替代
							etfComp.bIsNeedCashComponent = true;

							etfComp.ui64mCashComponent = ui64m_CashComponent;

							//
							// 用户交易前持仓
							etfComp.userHold.ui64StockBalance_beforematch = ptrUserStock->ui64StockBalance_beforematch;
							// 用户交易后持仓
							etfComp.userHold.ui64StockBalance_aftermatch =
								ptrUserStock->ui64StockBalance_beforematch - ui64CouldUseForCreation;

							//
							// 竞价买入量 交易前
							etfComp.userHold.ui64Stock_auction_purchase_beforematch = ptrUserStock->ui64Stock_auction_purchase_beforematch;
							// 交易后
							etfComp.userHold.ui64Stock_auction_purchase_aftermatch = 0;

							//
							// 大宗买入量
							// 交易前
							etfComp.userHold.ui64Stock_staple_purchase_beforematch = ptrUserStock->ui64Stock_staple_purchase_beforematch;
							// 交易后
							etfComp.userHold.ui64Stock_staple_purchase_aftermatch = ptrUserStock->ui64Stock_staple_purchase_beforematch;

							//
							// etf赎回量，可竞价卖出 或 etf申购量，可竞价卖出
							// 交易前
							etfComp.userHold.ui64Stock_etfredemption_creation_beforematch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch;
							// 交易后
							etfComp.userHold.ui64Stock_etfredemption_creation_aftermatch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch;

							//
							// 证券持有可用余额，可用于申购etf份额和可竞价卖出
							// 用户可用持仓 交易前
							etfComp.userHold.ui64StockAvailable_beforematch = ptrUserStock->ui64StockAvailable_beforematch;
							// 用户可用持仓 交易后
							etfComp.userHold.ui64StockAvailable_aftermatch = 0;

							ui64_SubRest = 0;

							in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);
						}
						else if (0 == cit->iSubstituteFlag)
						{
							// 0 = 禁止现金替代（必须有证券）

							// 用户不持有，交易失败
							//归还连接
							simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

							string sDebug("用户AccountId=[");
							sDebug += in_ptrOrder->strAccountId;
							sDebug += "] 不持有股份zqdm=[";
							sDebug += in_ptrOrder->strStockID;
							sDebug += "]";
							BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

							return 1;
						}
					}
				}
				else if (1 == iRes)
				{
					// 用户不持有该只股票

					if (1 == cit->iSubstituteFlag)
					{
						// 1 = 可以进行现金替代（先用证券，证券不足时差额部分用现金替代）
						// 记录用户的替代金额
						in_ptrOrder->etfOrderStatus.ui64mCashComponent += cit->ui64mCreationCashSubstitute;
					}
					else if (0 == cit->iSubstituteFlag)
					{
						// 0 = 禁止现金替代（必须有证券）

						// 用户不持有，交易失败

						//归还连接
						simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

						string sDebug("用户AccountId=[");
						sDebug += in_ptrOrder->strAccountId;
						sDebug += "] 不持有股份zqdm=[";
						sDebug += in_ptrOrder->strStockID;
						sDebug += "]";
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

						return 1;
					}

				}
				else
				{
					//归还连接
					simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetDb_UserHoldStock error";
					return -2;
				}
			} // if ( 2 == cit->iSubstituteFlag ) else

		}

		//归还连接
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		// 计算是否超过 最大现金替代比例
		// MaxCashRatio	N6(5)
		// 最大现金替代比例，例如：5.551％ 在文件中用0.05551表示

		// 当前ETF的净值
		simutgw::uint64_t_Money ui64m_etfTotalValue =
			in_ptrOrder->ui64Orderqty_origin * in_ptrEtf->ui64mNAV;
		// 按 最大现金替代比例 计算最大允许替代现金金额
		simutgw::uint64_t_Money ui64m_MaxAllowCashComp = (uint64_t)(ui64m_etfTotalValue * (in_ptrEtf->dMaxCashRatio));

		if (in_ptrOrder->etfOrderStatus.ui64mCashComponent > ui64m_MaxAllowCashComp)
		{
			// 当前现金替代的总金额 超过 最大现金替代比例
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
Etf申购交易 直接按照的ETF成份股完成交易

Return :
0 -- 获取成功
<0 -- 获取失败
1 -- 余券不足
2 -- 超过最大现金替代比例
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
			// 现金替代标志SubstituteFlag C1
			if (2 == cit->iSubstituteFlag)
			{
				// 2 = 必须用现金替代

				// 记录用户的替代金额
				in_ptrOrder->etfOrderStatus.ui64mCashComponent += cit->ui64mCreationCashSubstitute;
			}
			else
			{
				// 1 = 可以进行现金替代（先用证券，证券不足时差额部分用现金替代）
				// 0 = 禁止现金替代（必须有证券）

				// 不验股份时按有证券的方式进行成交

				// Etf所需申购股票数量
				uint64_t ui64NeedForCreation = in_ptrOrder->ui64Orderqty_origin * cit->ui64ComponentShare;

				struct simutgw::TradeEtfComponent etfComp;
				etfComp.strUnderlyingSecurityID = cit->strUnderlyingSecurityID;

				// 可用数量足够
				etfComp.bIsUserHolding = true;

				// 不需现金替代
				etfComp.bIsNeedCashComponent = false;

				// 发生交易的股票数量
				etfComp.ui64StockExchange = ui64NeedForCreation;

				in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);
			}

		}

		// 计算是否超过 最大现金替代比例
		// MaxCashRatio	N6(5)
		// 最大现金替代比例，例如：5.551％ 在文件中用0.05551表示

		// 当前ETF的净值
		simutgw::uint64_t_Money ui64m_etfTotalValue =
			in_ptrOrder->ui64Orderqty_origin * in_ptrEtf->ui64mNAV;
		// 按 最大现金替代比例 计算最大允许替代现金金额
		simutgw::uint64_t_Money ui64m_MaxAllowCashComp = (uint64_t)(ui64m_etfTotalValue * (in_ptrEtf->dMaxCashRatio));

		if (in_ptrOrder->etfOrderStatus.ui64mCashComponent > ui64m_MaxAllowCashComp)
		{
			// 当前现金替代的总金额 超过 最大现金替代比例
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
Etf赎回交易 在数据库中获取用户的ETF成份股持仓，并且加上赎回的数量并完成交易

Return :
0 -- 获取成功
-1 -- 获取失败
*/
int Task_Etf_Match::Match_Redemp_UserHoldToEtfComponent(
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrder,
	std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf)
{
	static const string ftag("Task_Etf_Match::Match_Redemp_UserHoldToEtfComponent()");

	try
	{
		int iRes = 0;

		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Get Connection is NULL";

			return -1;
		}

		std::vector<simutgw::SzETFComponent>::const_iterator cit;

		std::shared_ptr<struct simutgw::TradeStock> ptrUserStock;

		for (cit = in_ptrEtf->vecComponents.begin();
			in_ptrEtf->vecComponents.end() != cit; ++cit)
		{
			// 现金替代标志SubstituteFlag C1
			if (2 == cit->iSubstituteFlag)
			{
				// 2 = 必须用现金替代

				// 记录用户的替代金额
				in_ptrOrder->etfOrderStatus.ui64mCashComponent += cit->ui64mRedemptionCashSubstitute;
			}
			else
			{
				// 1 = 可以进行现金替代（先用证券，证券不足时差额部分用现金替代）
				// 0 = 禁止现金替代（必须有证券）

				ptrUserStock = std::shared_ptr<struct simutgw::TradeStock>(
					new simutgw::TradeStock());

				iRes = DbUserInfoAsset::GetDb_UserHoldStock(mysqlConn, in_ptrOrder->strAccountId,
					cit->strUnderlyingSecurityID, ptrUserStock);
				if (0 == iRes)
				{
					// 用户持有该只股票
					struct simutgw::TradeEtfComponent etfComp;

					etfComp.strUnderlyingSecurityID = cit->strUnderlyingSecurityID;

					// Etf赎回后股票数量
					uint64_t ui64NumAfterRedemption = in_ptrOrder->ui64Orderqty_origin * cit->ui64ComponentShare;

					// 持有
					etfComp.bIsUserHolding = true;
					// 不需现金替代
					etfComp.bIsNeedCashComponent = false;

					etfComp.ui64StockExchange = ui64NumAfterRedemption;

					//
					// 用户交易前持仓
					etfComp.userHold.ui64StockBalance_beforematch = ptrUserStock->ui64StockBalance_beforematch;
					// 用户交易后持仓
					etfComp.userHold.ui64StockBalance_aftermatch =
						ptrUserStock->ui64StockBalance_beforematch + ui64NumAfterRedemption;

					//
					// 竞价买入量 交易前
					etfComp.userHold.ui64Stock_auction_purchase_beforematch = ptrUserStock->ui64Stock_auction_purchase_beforematch;
					// 交易后
					etfComp.userHold.ui64Stock_auction_purchase_aftermatch =
						ptrUserStock->ui64Stock_auction_purchase_beforematch;

					//
					// 大宗买入量
					// 交易前
					etfComp.userHold.ui64Stock_staple_purchase_beforematch = ptrUserStock->ui64Stock_staple_purchase_beforematch;
					// 交易后
					etfComp.userHold.ui64Stock_staple_purchase_aftermatch = ptrUserStock->ui64Stock_staple_purchase_beforematch;

					//
					// etf赎回量，可竞价卖出 或 etf申购量，可竞价卖出
					// 交易前
					etfComp.userHold.ui64Stock_etfredemption_creation_beforematch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch;
					// 交易后
					etfComp.userHold.ui64Stock_etfredemption_creation_aftermatch = ptrUserStock->ui64Stock_etfredemption_creation_beforematch + ui64NumAfterRedemption;

					//
					// 证券持有可用余额，可用于申购etf份额和可竞价卖出
					// 用户可用持仓 交易前
					etfComp.userHold.ui64StockAvailable_beforematch = ptrUserStock->ui64StockAvailable_beforematch;
					// 用户可用持仓 交易后
					etfComp.userHold.ui64StockAvailable_aftermatch = ptrUserStock->ui64StockAvailable_beforematch;

					in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);

				}
				else if (1 == iRes)
				{
					// 用户不持有该只股票
					struct simutgw::TradeEtfComponent etfComp;

					etfComp.strUnderlyingSecurityID = cit->strUnderlyingSecurityID;

					// Etf赎回后股票数量
					uint64_t ui64NumAfterRedemption = in_ptrOrder->ui64Orderqty_origin * cit->ui64ComponentShare;

					// 不持有
					etfComp.bIsUserHolding = false;
					// 不需现金替代
					etfComp.bIsNeedCashComponent = false;

					//
					etfComp.ui64StockExchange = ui64NumAfterRedemption;

					//
					// 用户交易前持仓
					etfComp.userHold.ui64StockBalance_beforematch = 0;
					// 用户交易后持仓
					etfComp.userHold.ui64StockBalance_aftermatch = ui64NumAfterRedemption;

					//
					// 竞价买入量 交易前
					etfComp.userHold.ui64Stock_auction_purchase_beforematch = 0;
					// 交易后
					etfComp.userHold.ui64Stock_auction_purchase_aftermatch = 0;

					//
					// 大宗买入量
					// 交易前
					etfComp.userHold.ui64Stock_staple_purchase_beforematch = 0;
					// 交易后
					etfComp.userHold.ui64Stock_staple_purchase_aftermatch = 0;

					//
					// etf赎回量，可竞价卖出 或 etf申购量，可竞价卖出
					// 交易前
					etfComp.userHold.ui64Stock_etfredemption_creation_beforematch = 0;
					// 交易后
					etfComp.userHold.ui64Stock_etfredemption_creation_aftermatch = ui64NumAfterRedemption;

					//
					// 证券持有可用余额，可用于申购etf份额和可竞价卖出
					// 用户可用持仓 交易前
					etfComp.userHold.ui64StockAvailable_beforematch = 0;
					// 用户可用持仓 交易后
					etfComp.userHold.ui64StockAvailable_aftermatch = 0;

					in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);
				}
				else
				{
					//归还连接
					simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetDb_UserHoldStock error";
					return -2;
				}
			} // if ( 2 == cit->iSubstituteFlag ) else

		}

		//归还连接
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
Etf赎回交易 直接按照的ETF成份股完成交易

Return :
0 -- 获取成功
<0 -- 获取失败
1 -- 余券不足
2 -- 超过最大现金替代比例
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
			// 现金替代标志SubstituteFlag C1
			if (2 == cit->iSubstituteFlag)
			{
				// 2 = 必须用现金替代

				// 记录用户的替代金额
				in_ptrOrder->etfOrderStatus.ui64mCashComponent += cit->ui64mCreationCashSubstitute;
			}
			else
			{
				// 1 = 可以进行现金替代（先用证券，证券不足时差额部分用现金替代）
				// 0 = 禁止现金替代（必须有证券）

				// 不验股份时按有证券的方式进行成交

				struct simutgw::TradeEtfComponent etfComp;
				etfComp.strUnderlyingSecurityID = cit->strUnderlyingSecurityID;

				// Etf赎回后股票数量
				uint64_t ui64NumAfterRedemption = in_ptrOrder->ui64Orderqty_origin * cit->ui64ComponentShare;

				// 可用数量足够
				etfComp.bIsUserHolding = true;

				// 不需现金替代
				etfComp.bIsNeedCashComponent = false;

				//
				etfComp.ui64StockExchange = ui64NumAfterRedemption;

				in_ptrOrder->etfOrderStatus.vecComponents.push_back(etfComp);
			}

		}

		// 计算是否超过 最大现金替代比例
		// MaxCashRatio	N6(5)
		// 最大现金替代比例，例如：5.551％ 在文件中用0.05551表示

		// 当前ETF的净值
		simutgw::uint64_t_Money ui64m_etfTotalValue =
			in_ptrOrder->ui64Orderqty_origin * in_ptrEtf->ui64mNAV;
		// 按 最大现金替代比例 计算最大允许替代现金金额
		simutgw::uint64_t_Money ui64m_MaxAllowCashComp = (uint64_t)(ui64m_etfTotalValue * (in_ptrEtf->dMaxCashRatio));

		if (in_ptrOrder->etfOrderStatus.ui64mCashComponent > ui64m_MaxAllowCashComp)
		{
			// 当前现金替代的总金额 超过 最大现金替代比例
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
