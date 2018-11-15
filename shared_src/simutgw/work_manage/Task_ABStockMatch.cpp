#include "Task_ABStockMatch.h"

#include <memory>

#include "GenTaskHelper.h"

#include "json/json.h"

#include "simutgw/biz/MatchUtil.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "util/SystemCounter.h"
#include "util/TimeDuration.h"

#include "simutgw/order/OrderMemoryStoreFactory.h"
#include "order/StockOrderHelper.h"

#include "quotation/MarketInfoHelper.h"
#include "simutgw/msg_biz/ProcCancelOrder.h"

#include "simutgw/db_oper/RecordNewOrderHelper.h"
#include "simutgw/db_oper/DbUserInfoAsset.h"

#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/stgw_config/sys_function.h"

#include "cache/UserStockHelper.h"

/*
���׳ɽ�����
Return:
0 -- �ɽ�
-1 -- ʧ��
*/
int Task_ABStockMatch::MatchOrder()
{
	static const string ftag("Task_ABStockMatch::MatchOrder() ");

	TimeDuration dra;
	dra.Begin();

	int iRes = 0;
	// �ɽ�
	switch (m_orderMsg->tradePolicy.iMatchMode)
	{
	case simutgw::SysMatchMode::EnAbleQuta:
		iRes = ABStockEnableQuoMatch(m_orderMsg);
		break;

	case simutgw::SysMatchMode::SimulMatchAll:
		iRes = ABStockMatch_All(m_orderMsg);
		break;

	case simutgw::SysMatchMode::SimulMatchByDivide:
		iRes = ABStockMatch_Divide(m_orderMsg);
		break;

	case simutgw::SysMatchMode::SimulNotMatch:
		iRes = ABStockMatch_UnMatch(m_orderMsg);
		break;

	case simutgw::SysMatchMode::SimulErrMatch:
		iRes = ABStockMatch_Error(m_orderMsg);
		break;

	case simutgw::SysMatchMode::SimulMatchPart:
		iRes = ABStockMatch_Part(m_orderMsg);
		break;

	default:
		// 
		string strValue;
		string strError("Match mode[");
		strError += sof_string::itostr(m_orderMsg->tradePolicy.iMatchMode, strValue);
		strError += "] doesn't support";
		EzLog::e(ftag, strError);
		break;
	}

	return iRes;
}

/*
������������Ľ���

Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int Task_ABStockMatch::ABStockEnableQuoMatch(std::shared_ptr<struct simutgw::OrderMessage>& ptrObj)
{
	static const string ftag("Task_ABStockMatch::ABStockEnableQuoMatch() ");

	enum simutgw::MatchType iMatchRes = Quot_MatchOrder(ptrObj);

	if (iMatchRes != simutgw::NotMatch)
	{
		string strItoa;
		string strDebug("����clordid[");
		strDebug += ptrObj->strClordid;
		strDebug += "]�ɽ����[";
		strDebug += sof_string::itostr(iMatchRes, strItoa);
		strDebug += "],price[";
		strDebug += sof_string::itostr(ptrObj->ui64mPrice_matched, strItoa);
		strDebug += "],cjsl[";
		strDebug += sof_string::itostr(ptrObj->ui64Orderqty_matched, strItoa);
		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
	}

	//�ж��Ƿ�ɽ�
	if (simutgw::MatchAll == iMatchRes)
	{
		//�ɽ�
		if (0 == ptrObj->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchAll();
		}
		else if (0 == ptrObj->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchAll();
		}
	}
	else if (simutgw::MatchPart == iMatchRes)
	{
		//���ֳɽ�
		if (0 == ptrObj->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchPart();
		}
		else if (0 == ptrObj->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchPart();
		}

		// �޼ۻ���ʣ��ҵ� ����ɽ�����
		if (simutgw::deltype_ordinary_limit == ptrObj->enDelType ||
			simutgw::deltype_match_at_market_in_five_and_remainder_hold == ptrObj->enDelType ||
			simutgw::deltype_the_oppsite_side_optimaland_remainder_hold == ptrObj->enDelType)
		{
			GenTaskHelper::GenTask_Match(ptrObj);
		}
		else
		{
			// nothing
			BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << "clordid=" << ptrObj->strClordid << ", unkown DelType=" << ptrObj->enDelType;
		}
	}
	else if (simutgw::NotMatch == iMatchRes)
	{
		//δ�ɽ�
		// �����׹ҵ����ٲ��ȥ

		// ����ɽ�����
		GenTaskHelper::GenTask_Match(ptrObj);
	}
	else if (simutgw::OutOfRange == iMatchRes)
	{
		// �����ǵ���
		// �����׽���
		ptrObj->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("ί��clordid[");
		sDebug += ptrObj->strClordid;
		sDebug += "]�����ǵ���";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;
		// д��ر�����
		simutgw::g_outMsg_buffer.PushBack(ptrObj);
		return -1;
	}
	else if (simutgw::StopTrans == iMatchRes)
	{
		// ��Ʊ��ͣ��
		// �����׽���
		ptrObj->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("ί��clordid[");
		sDebug += ptrObj->strClordid;
		sDebug += "]��Ʊ��ͣ��";

		EzLog::e(ftag, sDebug);

		// д��ر�����
		simutgw::g_outMsg_buffer.PushBack(ptrObj);
		return -1;
	}
	else if (simutgw::ErrorMatch == iMatchRes)
	{
		// ���Ϸ�����
		ptrObj->enMatchType = simutgw::ErrorMatch;

		// Log
		string sDebug("ί��clordid[");
		sDebug += ptrObj->strClordid;
		sDebug += "]ErrorMatch";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

		// д��ر�����
		simutgw::g_outMsg_buffer.PushBack(ptrObj);
		return -1;
	}
	else
	{
		//
		BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << "clordid=" << ptrObj->strClordid << ", unkown MatchType=" << iMatchRes;
	}

	return 0;
}

/*
����ʵ��ģ�⽻��

Param :
�µ��ķ���
const int iOrderDirection :
1 -- ��
2 -- ����

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
enum simutgw::MatchType Task_ABStockMatch::Quot_MatchOrder(
	std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{
	static const string ftag("Task_ABStockMatch::Quot_MatchOrder()");

	string strCircleId;
	MatchUtil::Get_Order_CircleID(orderMsg, strCircleId);

	//��redis���ȡ��Ӧ������
	// read redis 
	string strRedisRes;

	Json::Value jsonRedisCmd;
	string strRedisCmd;

	enum simutgw::MatchType enRes = simutgw::NotMatch;

	try
	{
		simutgw::uint64_t_Money ui64mMaxGain = 0;
		simutgw::uint64_t_Money ui64mMinFall = 0;
		uint64_t ui64Cjsl = 0;
		simutgw::uint64_t_Money ui64mCjje = 0;
		string strHqsj;
		string strTpbz;

		//�õ�����
		int iRes = MarketInfoHelper::GetCurrQuotGapByCircle(orderMsg->strStockID,
			orderMsg->strSide, strCircleId, orderMsg->tradePolicy.iQuotationMatchType,
			ui64mMaxGain, ui64mMinFall, ui64Cjsl, ui64mCjje, strHqsj, strTpbz);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "����clordid[" << orderMsg->strClordid
				<< "], stockid=" << orderMsg->strStockID << ", GetQuotation error";

			orderMsg->enMatchType = simutgw::NotMatch;
			return simutgw::NotMatch;
		}

		// �г� ʣ��ɽ�����
		uint64_t ui64Cjsl_Remain = 0;
		// �г� ʣ��ɽ����
		simutgw::uint64_t_Money ui64mCjje_Remain = 0;
		enRes = Quot_MatchByMarket(orderMsg, ui64mMaxGain, ui64mMinFall,
			ui64Cjsl, ui64mCjje, ui64Cjsl_Remain, ui64mCjje_Remain,
			strTpbz);
		if (enRes != simutgw::MatchAll && enRes != simutgw::MatchPart)
		{
			return enRes;
		}

		// ����������д�����ݿ�
		// д��ر�����
		simutgw::g_outMsg_buffer.PushBack(orderMsg);
		if (0 != iRes)
		{
			enRes = simutgw::NotMatch;
			orderMsg->enMatchType = simutgw::NotMatch;
			return simutgw::NotMatch;
		}

		// ��д���׺����������
		iRes = MarketInfoHelper::SetCurrQuotGapByCircle(orderMsg, strCircleId, ui64Cjsl_Remain,
			ui64mCjje_Remain, strHqsj);
		if (0 != iRes)
		{
			enRes = simutgw::NotMatch;
			orderMsg->enMatchType = simutgw::NotMatch;
			return simutgw::NotMatch;
		}

		return enRes;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
		return simutgw::NotMatch;
	}
}

/*
ʵ�� �����г����ɽ���ʽ�ɽ�
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
enum simutgw::MatchType Task_ABStockMatch::Quot_MatchByMarket(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const simutgw::uint64_t_Money in_ui64mMaxGain, const simutgw::uint64_t_Money in_ui64mMinFall,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain,
	const string& in_strTpbz)
{
	static const string ftag("Task_ABStockMatch::Quot_MatchByMarket()");

	io_orderMsg->enMatchType = simutgw::NotMatch;

	//  �鿴�Ƿ�ͣ��
	if (0 > MatchUtil::CheckTPBZ(io_orderMsg, in_strTpbz))
	{
		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "����clordid[" << io_orderMsg->strClordid
			<< "], stockid=" << io_orderMsg->strStockID << ", TPBZ error, not match";

		return io_orderMsg->enMatchType;
	}

	// ���м���
	if (0 == in_ui64Cjsl || 0 == in_ui64mCjje)
	{
		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "����clordid[" << io_orderMsg->strClordid
			<< "], stockid=" << io_orderMsg->strStockID
			<< ", quot Cjsl=" << in_ui64Cjsl << ", Cjje=" << in_ui64mCjje << ", not match";

		// �г����������Ѽ��ף����ܳɽ�
		io_orderMsg->enMatchType = simutgw::NotMatch;

		return io_orderMsg->enMatchType;
	}

	switch (io_orderMsg->enDelType)
	{
	case simutgw::deltype_ordinary_limit:
		// �Ƿ񳬳��ǵ���
		if (0 > MatchUtil::Check_MaxGain_And_MinFall(io_orderMsg, in_ui64mMaxGain, in_ui64mMinFall))
		{
			BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "����clordid[" << io_orderMsg->strClordid
				<< "], stockid=" << io_orderMsg->strStockID
				<< ", out price range quot MaxGain=" << in_ui64mMaxGain << ", MinFall=" << in_ui64mMinFall << ", not match";

			io_orderMsg->enMatchType = simutgw::OutOfRange;
			return io_orderMsg->enMatchType;
		}

		Match_Ordinary_Limit(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_the_side_optimal:
		Match_Optimal(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_the_oppsite_side_optimaland_remainder_hold:
		Match_Optimal(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_match_at_market_and_remainder_cancel:
		Match_Optimal_And_Remainder_Cancel(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_all_match_at_market_or_cancel:
		Match_All_Market_Or_Cancel(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_match_at_market_in_five_and_remainder_cancel:
		Match_Optimal_And_Remainder_Cancel(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	case simutgw::deltype_match_at_market_in_five_and_remainder_hold:
		Match_Optimal(io_orderMsg, in_ui64Cjsl, in_ui64mCjje,
			out_ui64Cjsl_Remain, out_ui64mCjje_Remain);
		break;

	default:

		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "����clordid[" << io_orderMsg->strClordid
			<< "], stockid=" << io_orderMsg->strStockID
			<< ", unkown delType" << io_orderMsg->enDelType;

		io_orderMsg->enMatchType = simutgw::OutOfRange;
		return io_orderMsg->enMatchType;
		break;
	}

	return io_orderMsg->enMatchType;
}

/*
	����ί������ -- ��ͨ�޼�

	����ʽ��
	�����޼۳ɽ���δ�ɽ����ֹҵ�
	*/
int Task_ABStockMatch::Match_Ordinary_Limit(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain)
{
	static const string strTag("Task_ABStockMatch::Match_Ordinary_Limit() ");

	// �����г��ľ���
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;

	// �жϳɽ�����
	if (0 > MatchUtil::Check_Match_Method(io_orderMsg, in_ui64Cjsl, in_ui64mCjje))
	{
		return 0;
	}

	// ���д�ϳɽ�����Ϊ���ּ�ȫ���ɽ�
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// �г�����������ֻ�ܲ��ֳɽ�
		io_orderMsg->enMatchType = simutgw::MatchPart;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = 0;
		out_ui64mCjje_Remain = 0;

		// ���ֳɽ�������
		uint64_t ui64MatchedOrderqty = in_ui64Cjsl;

		//
		// �ѳɽ�����
		// ί������
		io_orderMsg->ui64Orderqty_matched = ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// �۸�	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// ί�н��
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * ui64MatchedOrderqty;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);

		//
		// δ�ɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_unmatched = io_orderMsg->ui64LeavesQty - ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		io_orderMsg->ui64LeavesQty = io_orderMsg->ui64Orderqty_unmatched;

		//�ۼƳɽ���
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin - io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strCumQty);
		// �������
		io_orderMsg->ui64mCashorderqty_unmatched = io_orderMsg->ui64Orderqty_unmatched * io_orderMsg->ui64mOrderPrice;

		// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}
	else
	{
		// �г������㹻������ȫ���ɽ�
		io_orderMsg->enMatchType = simutgw::MatchAll;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = in_ui64Cjsl - io_orderMsg->ui64LeavesQty;
		out_ui64mCjje_Remain = in_ui64mCjje - io_orderMsg->ui64LeavesQty * ui64mAveragePrice;

		//
		// �ѳɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_matched = io_orderMsg->ui64LeavesQty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// �۸�	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// �������
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * io_orderMsg->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);
		//
		// δ�ɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_unmatched = 0;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		//�ۼƳɽ���
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin, io_orderMsg->strCumQty);
		// �������
		io_orderMsg->ui64mCashorderqty_unmatched = 0;

		// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}

	UserStockHelper::UpdateAfterTrade(io_orderMsg);

	// �����ڲ�������
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//Ψһִ��id
	io_orderMsg->strExecID = strTransId;
	//Ψһ����id
	io_orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '����ʱ��',
	TimeStringUtil::GetCurrTimeInTradeType(io_orderMsg->strTrade_time);

	return 0;
}

/*
	����ί������ -- �������š����ַ�����

	����ʽ��
	�����г����۳ɽ���δ�ɽ����ֵ�ת���޼۹ҵ�
	*/
int Task_ABStockMatch::Match_Optimal(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain)
{
	static const string strTag("Task_ABStockMatch::Match_Optimal() ");

	// �����г��ľ���
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;

	// �жϳɽ�����
	if (0 > MatchUtil::Check_Match_Method(io_orderMsg, in_ui64Cjsl, in_ui64mCjje, false))
	{
		return 0;
	}

	// ���д�ϳɽ�����Ϊ���ּ�ȫ���ɽ�
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// �г�����������ֻ�ܲ��ֳɽ�
		io_orderMsg->enMatchType = simutgw::MatchPart;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = 0;
		out_ui64mCjje_Remain = 0;

		// ���ֳɽ�������
		uint64_t ui64MatchedOrderqty = in_ui64Cjsl;

		//
		// �ѳɽ�����
		// ί������
		io_orderMsg->ui64Orderqty_matched = ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// �۸�	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// ί�н��
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * ui64MatchedOrderqty;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);

		//
		// δ�ɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_unmatched = io_orderMsg->ui64LeavesQty - ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		io_orderMsg->ui64LeavesQty = io_orderMsg->ui64Orderqty_unmatched;

		//�ۼƳɽ���
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin - io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strCumQty);
		// �������
		io_orderMsg->ui64mCashorderqty_unmatched = io_orderMsg->ui64Orderqty_unmatched * io_orderMsg->ui64mOrderPrice;

		// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}
	else
	{
		// �г������㹻������ȫ���ɽ�
		io_orderMsg->enMatchType = simutgw::MatchAll;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = in_ui64Cjsl - io_orderMsg->ui64LeavesQty;
		out_ui64mCjje_Remain = in_ui64mCjje - io_orderMsg->ui64LeavesQty * ui64mAveragePrice;

		//
		// �ѳɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_matched = io_orderMsg->ui64LeavesQty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// �۸�	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// �������
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * io_orderMsg->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);
		//
		// δ�ɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_unmatched = 0;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		//�ۼƳɽ���
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin, io_orderMsg->strCumQty);
		// �������
		io_orderMsg->ui64mCashorderqty_unmatched = 0;

		// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}

	UserStockHelper::UpdateAfterTrade(io_orderMsg);

	// �����ڲ�������
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//Ψһִ��id
	io_orderMsg->strExecID = strTransId;
	//Ψһ����id
	io_orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '����ʱ��',
	TimeStringUtil::GetCurrTimeInTradeType(io_orderMsg->strTrade_time);

	return 0;
}

/*
	����ί������
	--�м������ɽ�ʣ�೷�����м������嵵ȫ��ɽ�ʣ�೷��

	����ʽ��
	�����г����۳ɽ���δ�ɽ����ֵĳ���
	*/
int Task_ABStockMatch::Match_Optimal_And_Remainder_Cancel(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain)
{
	static const string strTag("Task_ABStockMatch::Match_Optimal_And_Remainder_Cancel() ");

	// �����г��ľ���
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;

	// �жϳɽ�����
	if (0 > MatchUtil::Check_Match_Method(io_orderMsg, in_ui64Cjsl, in_ui64mCjje, false))
	{
		return 0;
	}

	// ���д�ϳɽ�����Ϊ���ּ�ȫ���ɽ�
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// �г�����������ֻ�ܲ��ֳɽ�
		io_orderMsg->enMatchType = simutgw::MatchPart;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = 0;
		out_ui64mCjje_Remain = 0;

		// ���ֳɽ�������
		uint64_t ui64MatchedOrderqty = in_ui64Cjsl;

		//
		// �ѳɽ�����
		// ί������
		io_orderMsg->ui64Orderqty_matched = ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// �۸�	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// ί�н��
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * ui64MatchedOrderqty;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);

		//
		// δ�ɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_unmatched = io_orderMsg->ui64LeavesQty - ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		io_orderMsg->ui64LeavesQty = io_orderMsg->ui64Orderqty_unmatched;

		//�ۼƳɽ���
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin - io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strCumQty);
		// �������
		io_orderMsg->ui64mCashorderqty_unmatched = io_orderMsg->ui64Orderqty_unmatched * io_orderMsg->ui64mOrderPrice;

		// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);

		// ʣ�೷��
		ProcCancelOrder::ProcSingleCancelOrder_Without_Request(io_orderMsg);
	}
	else
	{
		// �г������㹻������ȫ���ɽ�
		io_orderMsg->enMatchType = simutgw::MatchAll;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = in_ui64Cjsl - io_orderMsg->ui64LeavesQty;
		out_ui64mCjje_Remain = in_ui64mCjje - io_orderMsg->ui64LeavesQty * ui64mAveragePrice;

		//
		// �ѳɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_matched = io_orderMsg->ui64LeavesQty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// �۸�	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// �������
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * io_orderMsg->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);
		//
		// δ�ɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_unmatched = 0;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		//�ۼƳɽ���
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin, io_orderMsg->strCumQty);
		// �������
		io_orderMsg->ui64mCashorderqty_unmatched = 0;

		// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}

	UserStockHelper::UpdateAfterTrade(io_orderMsg);

	// �����ڲ�������
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//Ψһִ��id
	io_orderMsg->strExecID = strTransId;
	//Ψһ����id
	io_orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '����ʱ��',
	TimeStringUtil::GetCurrTimeInTradeType(io_orderMsg->strTrade_time);

	return 0;
}

/*
	����ί������
	--�м�ȫ��ɽ�����

	����ʽ��
	�����м�ȫ���ɽ�������ȫ������
	*/
int Task_ABStockMatch::Match_All_Market_Or_Cancel(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	uint64_t& out_ui64Cjsl_Remain, simutgw::uint64_t_Money& out_ui64mCjje_Remain)
{
	static const string strTag("Task_ABStockMatch::Match_All_Limit_Or_Cancel() ");

	// �����г��ľ���
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;

	// �жϳɽ�����
	if (0 > MatchUtil::Check_Match_Method(io_orderMsg, in_ui64Cjsl, in_ui64mCjje, false))
	{
		return 0;
	}

	// ���д�ϳɽ�����Ϊ���ּ�ȫ���ɽ�
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// �г�����������ֻ�ܲ��ֳɽ�
		io_orderMsg->enMatchType = simutgw::MatchPart;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = 0;
		out_ui64mCjje_Remain = 0;

		// ���ֳɽ�������
		uint64_t ui64MatchedOrderqty = in_ui64Cjsl;

		//
		// �ѳɽ�����
		// ί������
		io_orderMsg->ui64Orderqty_matched = ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// �۸�	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// ί�н��
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * ui64MatchedOrderqty;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);

		//
		// δ�ɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_unmatched = io_orderMsg->ui64LeavesQty - ui64MatchedOrderqty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		io_orderMsg->ui64LeavesQty = io_orderMsg->ui64Orderqty_unmatched;

		//�ۼƳɽ���
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin - io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strCumQty);
		// �������
		io_orderMsg->ui64mCashorderqty_unmatched = io_orderMsg->ui64Orderqty_unmatched * io_orderMsg->ui64mOrderPrice;

		// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);

		// ʣ�೷��
		ProcCancelOrder::ProcSingleCancelOrder_Without_Request(io_orderMsg);
	}
	else
	{
		// �г������㹻������ȫ���ɽ�
		io_orderMsg->enMatchType = simutgw::MatchAll;
		io_orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		io_orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
		io_orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		out_ui64Cjsl_Remain = in_ui64Cjsl - io_orderMsg->ui64LeavesQty;
		out_ui64mCjje_Remain = in_ui64mCjje - io_orderMsg->ui64LeavesQty * ui64mAveragePrice;

		//
		// �ѳɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_matched = io_orderMsg->ui64LeavesQty;
		sof_string::itostr(io_orderMsg->ui64Orderqty_matched, io_orderMsg->strLastQty);
		// �۸�	
		io_orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mPrice_matched, io_orderMsg->strLastPx, 4);
		// �������
		io_orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * io_orderMsg->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mCashorderqty_matched, io_orderMsg->strCashorderqty, 4);
		//
		// δ�ɽ�����
		// ��������
		io_orderMsg->ui64Orderqty_unmatched = 0;
		sof_string::itostr(io_orderMsg->ui64Orderqty_unmatched, io_orderMsg->strLeavesQty);
		//�ۼƳɽ���
		sof_string::itostr(io_orderMsg->ui64Orderqty_origin, io_orderMsg->strCumQty);
		// �������
		io_orderMsg->ui64mCashorderqty_unmatched = 0;

		// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
		Tgw_StringUtil::iLiToStr(io_orderMsg->ui64mOrderPrice, io_orderMsg->strOrderPrice, 4);
	}

	UserStockHelper::UpdateAfterTrade(io_orderMsg);

	// �����ڲ�������
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//Ψһִ��id
	io_orderMsg->strExecID = strTransId;
	//Ψһ����id
	io_orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '����ʱ��',
	TimeStringUtil::GetCurrTimeInTradeType(io_orderMsg->strTrade_time);

	return 0;
}

/*
������ͨAB��ί��
ȫ���ɽ�
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int Task_ABStockMatch::ABStockMatch_All(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{
	static const string ftag("Task_ABStockMatch::ABStockMatch_All() ");

	// Ԥ�� �ɽ�ʱ�ĳɽ�����
	uint64_t ui64Cjsl_predict_match = 0;
	// Ԥ�� �ɽ�ʱ�ĳɽ����
	simutgw::uint64_t_Money ui64mCjje_predict_match = 0;

	// �����г��ľ���
	simutgw::uint64_t_Money ui64mAveragePrice = 0;

	// ��Ϊ����Ҫ�����飬���Գɽ����ۼ�Ϊ�µ��۸�
	ui64mAveragePrice = orderMsg->ui64mOrderPrice;

	// �г������㹻������ȫ���ɽ�

	// Ԥ�� �ɽ�ʱ�ĳɽ�����
	ui64Cjsl_predict_match = orderMsg->ui64LeavesQty;
	// Ԥ�� �ɽ�ʱ�ĳɽ����
	ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;

	//
	// �г������㹻������ȫ���ɽ�
	orderMsg->enMatchType = simutgw::MatchAll;
	orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
	orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_FILL;
	orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

	//
	// �ѳɽ�����
	// ��������
	orderMsg->ui64Orderqty_matched = orderMsg->ui64LeavesQty;
	sof_string::itostr(orderMsg->ui64Orderqty_matched, orderMsg->strLastQty);
	// �۸�	
	orderMsg->ui64mPrice_matched = ui64mAveragePrice;
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mPrice_matched, orderMsg->strLastPx, 4);
	// �������
	orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * orderMsg->ui64Orderqty_matched;
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mCashorderqty_matched, orderMsg->strCashorderqty, 4);
	//
	// δ�ɽ�����
	// ��������
	orderMsg->ui64Orderqty_unmatched = 0;
	sof_string::itostr(orderMsg->ui64Orderqty_unmatched, orderMsg->strLeavesQty);
	//�ۼƳɽ���
	sof_string::itostr(orderMsg->ui64Orderqty_origin, orderMsg->strCumQty);
	// �������
	orderMsg->ui64mCashorderqty_unmatched = 0;

	// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mOrderPrice, orderMsg->strOrderPrice, 4);

	// �����ڲ�������
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//Ψһִ��id
	orderMsg->strExecID = strTransId;
	//Ψһ����id
	orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '����ʱ��',
	TimeStringUtil::GetCurrTimeInTradeType(orderMsg->strTrade_time);

	{
		string strItoa;
		string strDebug("����clordid[");
		strDebug += orderMsg->strClordid;
		strDebug += "]�ɽ����[";
		strDebug += sof_string::itostr(simutgw::MatchAll, strItoa);
		strDebug += "],price[";
		strDebug += sof_string::itostr(orderMsg->ui64mPrice_matched, strItoa);
		strDebug += "],cjsl[";
		strDebug += sof_string::itostr(orderMsg->ui64Orderqty_matched, strItoa);
		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
	}

	//�ɽ�
	if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchAll();
	}
	else if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchAll();
	}

	UserStockHelper::UpdateAfterTrade(orderMsg);

	// д��ر�����
	simutgw::g_outMsg_buffer.PushBack(orderMsg);

	return 0;
}

/*
������ͨAB��ί��
�ֱʳɽ�
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int Task_ABStockMatch::ABStockMatch_Divide(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{
	static const string ftag("Task_ABStockMatch::ABStockMatch_Divide() ");

	// Ԥ�� �ɽ�ʱ�ĳɽ�����
	uint64_t ui64Cjsl_predict_match = 0;
	// Ԥ�� �ɽ�ʱ�ĳɽ����
	simutgw::uint64_t_Money ui64mCjje_predict_match = 0;

	// �����г��ľ���
	simutgw::uint64_t_Money ui64mAveragePrice = 0;

	// ��Ϊ����Ҫ�����飬���Գɽ����ۼ�Ϊ�µ��۸�
	ui64mAveragePrice = orderMsg->ui64mOrderPrice;

	// ƽ��һ�ʵ�����
	uint64_t ui64PartQty = orderMsg->ui64Orderqty_origin / orderMsg->tradePolicy.iPart_Match_Num;

	if (0 == ui64PartQty)
	{
		// ����һ�ʳɽ� ���һ��
		orderMsg->enMatchType = simutgw::MatchAll;
		// Ԥ�� �ɽ�ʱ�ĳɽ�����
		ui64Cjsl_predict_match = orderMsg->ui64LeavesQty;
	}
	else
	{
		if (orderMsg->ui64LeavesQty >= (2 * ui64PartQty))
		{
			//  �������һ��
			orderMsg->enMatchType = simutgw::MatchPart;
			// Ԥ�� �ɽ�ʱ�ĳɽ�����
			ui64Cjsl_predict_match = ui64PartQty;
		}
		else if (orderMsg->ui64LeavesQty >= ui64PartQty)
		{
			// ���һ��
			orderMsg->enMatchType = simutgw::MatchAll;
			// Ԥ�� �ɽ�ʱ�ĳɽ�����
			ui64Cjsl_predict_match = orderMsg->ui64LeavesQty;
		}
		else
		{
			// �����ֱ� ���һ��
			orderMsg->enMatchType = simutgw::MatchAll;
			// Ԥ�� �ɽ�ʱ�ĳɽ�����
			ui64Cjsl_predict_match = orderMsg->ui64LeavesQty;
		}
	}

	// Ԥ�� �ɽ�ʱ�ĳɽ����
	ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;

	//
	orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
	orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
	orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

	//
	// �ѳɽ�����
	// ��������
	orderMsg->ui64Orderqty_matched = ui64Cjsl_predict_match;
	sof_string::itostr(orderMsg->ui64Orderqty_matched, orderMsg->strLastQty);
	// �۸�	
	orderMsg->ui64mPrice_matched = ui64mAveragePrice;
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mPrice_matched, orderMsg->strLastPx, 4);
	// �������
	orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * orderMsg->ui64Orderqty_matched;
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mCashorderqty_matched, orderMsg->strCashorderqty, 4);
	//
	// δ�ɽ�����
	// ��������
	orderMsg->ui64Orderqty_unmatched = orderMsg->ui64LeavesQty - orderMsg->ui64Orderqty_matched;
	orderMsg->ui64LeavesQty = orderMsg->ui64Orderqty_unmatched;
	sof_string::itostr(orderMsg->ui64Orderqty_unmatched, orderMsg->strLeavesQty);
	//�ۼƳɽ���
	sof_string::itostr(orderMsg->ui64Orderqty_matched, orderMsg->strCumQty);
	// �������
	orderMsg->ui64mCashorderqty_unmatched = 0;

	// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
	Tgw_StringUtil::iLiToStr(orderMsg->ui64mOrderPrice, orderMsg->strOrderPrice, 4);

	// �����ڲ�������
	string strTransId;
	TimeStringUtil::ExRandom15(strTransId);
	//Ψһִ��id
	orderMsg->strExecID = strTransId;
	//Ψһ����id
	orderMsg->strOrderID = strTransId;

	// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '����ʱ��',
	TimeStringUtil::GetCurrTimeInTradeType(orderMsg->strTrade_time);

	{
		string strItoa;
		string strDebug("����orderid[");
		strDebug += orderMsg->strClordid;
		strDebug += "]�ɽ����[";
		strDebug += sof_string::itostr(simutgw::MatchAll, strItoa);
		strDebug += "],price[";
		strDebug += sof_string::itostr(orderMsg->ui64mPrice_matched, strItoa);
		strDebug += "],cjsl[";
		strDebug += sof_string::itostr(orderMsg->ui64Orderqty_matched, strItoa);
		BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
	}

	//�ɽ�
	if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchPart();
	}
	else if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchPart();
	}

	UserStockHelper::UpdateAfterTrade(orderMsg);

	// д��ر�����
	simutgw::g_outMsg_buffer.PushBack(orderMsg);

	if (orderMsg->ui64LeavesQty != 0)
	{
		// ��д��order��
		GenTaskHelper::GenTask_Match(orderMsg);
	}

	return 0;
}

/*
������ͨAB��ί��
���ɽ�
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int Task_ABStockMatch::ABStockMatch_UnMatch(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{
	static const string ftag("Task_ABStockMatch::ABStockMatch_UnMatch() ");

	GenTaskHelper::GenTask_Match(orderMsg);

	return 0;
}

/*
������ͨAB��ί��
�ϵ�
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int Task_ABStockMatch::ABStockMatch_Error(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{

	//���Ϸ�����
	if (orderMsg->ui64LeavesQty == orderMsg->ui64Orderqty_origin)
	{
		// ���ڷϵ�ֻ���������ϵ�������Ѿ��ɽ���һ���֣�������Ϊ�ϵ�
		orderMsg->enMatchType = simutgw::ErrorMatch;
		orderMsg->bDataError = true;
		orderMsg->strError = "ģ�����Ӧ��";
		if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			orderMsg->strErrorCode = simutgw::SZ_ERRCODE::c20001;
		}
		else if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			orderMsg->strErrorCode = simutgw::SH_ERRCODE::c215;
		}

		// д��ر�����
		simutgw::g_outMsg_buffer.PushBack(orderMsg);
	}
	else
	{
		ABStockMatch_UnMatch(orderMsg);
	}

	return 0;
}

/*
������ͨAB��ί��
���ֳɽ�
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int Task_ABStockMatch::ABStockMatch_Part(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg)
{
	static const string ftag("Task_ABStockMatch::ABStockMatch_Part() ");

	if (orderMsg->ui64LeavesQty != orderMsg->ui64Orderqty_origin)
	{
		// ʣ��һ��ҵ�
		GenTaskHelper::GenTask_Match(orderMsg);
	}
	else
	{
		// Ԥ�� �ɽ�ʱ�ĳɽ�����
		uint64_t ui64Cjsl_predict_match = 0;
		// Ԥ�� �ɽ�ʱ�ĳɽ����
		simutgw::uint64_t_Money ui64mCjje_predict_match = 0;

		// �����г��ľ���
		simutgw::uint64_t_Money ui64mAveragePrice = 0;

		// ��Ϊ����Ҫ�����飬���Գɽ����ۼ�Ϊ�µ��۸�
		ui64mAveragePrice = orderMsg->ui64mOrderPrice;

		// ��������һ��
		uint64_t ui64PartQty = orderMsg->ui64Orderqty_origin / 2;

		// ���������������Ϊ1
		if (orderMsg->ui64Orderqty_origin == 1)
		{
			ui64PartQty = orderMsg->ui64Orderqty_origin;
			orderMsg->enMatchType = simutgw::MatchAll;
		}
		else
		{
			orderMsg->enMatchType = simutgw::MatchPart;
		}
		ui64Cjsl_predict_match = ui64PartQty;

		// Ԥ�� �ɽ�ʱ�ĳɽ����
		ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;

		//
		orderMsg->strExecType = simutgw::STEPMSG_EXECTYPE_TRADE;
		orderMsg->strOrdStatus = simutgw::STEPMSG_ORDSTATUS_PART_FILL;
		orderMsg->strMsgType = simutgw::STEPMSG_MSGTYPE_EXECREPORT;

		//
		// �ѳɽ�����
		// ��������
		orderMsg->ui64Orderqty_matched = ui64Cjsl_predict_match;
		sof_string::itostr(orderMsg->ui64Orderqty_matched, orderMsg->strLastQty);
		// �۸�	
		orderMsg->ui64mPrice_matched = ui64mAveragePrice;
		Tgw_StringUtil::iLiToStr(orderMsg->ui64mPrice_matched, orderMsg->strLastPx, 4);
		// �������
		orderMsg->ui64mCashorderqty_matched = ui64mAveragePrice * orderMsg->ui64Orderqty_matched;
		Tgw_StringUtil::iLiToStr(orderMsg->ui64mCashorderqty_matched, orderMsg->strCashorderqty, 4);
		//
		// δ�ɽ�����
		// ��������
		orderMsg->ui64Orderqty_unmatched = orderMsg->ui64LeavesQty - orderMsg->ui64Orderqty_matched;
		orderMsg->ui64LeavesQty = orderMsg->ui64Orderqty_unmatched;
		sof_string::itostr(orderMsg->ui64Orderqty_unmatched, orderMsg->strLeavesQty);
		//�ۼƳɽ���
		sof_string::itostr(orderMsg->ui64Orderqty_matched, orderMsg->strCumQty);
		// �������
		orderMsg->ui64mCashorderqty_unmatched = 0;

		// �ɽ���ί�м۸�strOrderPrice��Ϊԭ��
		Tgw_StringUtil::iLiToStr(orderMsg->ui64mOrderPrice, orderMsg->strOrderPrice, 4);

		// �����ڲ�������
		string strTransId;
		TimeStringUtil::ExRandom15(strTransId);
		//Ψһִ��id
		orderMsg->strExecID = strTransId;
		//Ψһ����id
		orderMsg->strOrderID = strTransId;

		// `trade_time` timestamp nullptr DEFAULT nullptr COMMENT '����ʱ��',
		TimeStringUtil::GetCurrTimeInTradeType(orderMsg->strTrade_time);

		{
			string strItoa;
			string strDebug("����orderid[");
			strDebug += orderMsg->strClordid;
			strDebug += "]�ɽ����[";
			strDebug += sof_string::itostr(simutgw::MatchAll, strItoa);
			strDebug += "],price[";
			strDebug += sof_string::itostr(orderMsg->ui64mPrice_matched, strItoa);
			strDebug += "],cjsl[";
			strDebug += sof_string::itostr(orderMsg->ui64Orderqty_matched, strItoa);
			BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
		}

		//�ɽ�
		if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			simutgw::g_counter.GetSz_InnerCounter()->Inc_MatchPart();
		}
		else if (0 == orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchPart();
		}

		UserStockHelper::UpdateAfterTrade(orderMsg);

		// д��ر�����
		simutgw::g_outMsg_buffer.PushBack(orderMsg);

		if (orderMsg->ui64LeavesQty != 0)
		{
			// ��д��order��
			GenTaskHelper::GenTask_Match(orderMsg);
		}
	}

	return 0;
}