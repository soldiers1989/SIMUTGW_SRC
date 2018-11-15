#include "StockOrderHelper.h"

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"

#include "tool_json/RapidJsonHelper_tgw.h"

#include "simutgw_config/g_values_sys_run_config.h"

#include "etf/conf_etf_info.h"
#include "quotation/MarketInfoHelper.h"

#include "order/StockHelper.h"

#include "simutgw/stgw_config/g_values_biz.h"

StockOrderHelper::StockOrderHelper(void)
{
}


StockOrderHelper::~StockOrderHelper(void)
{
}

/*
��֤�µ����ݵĺϷ���

Return :
0 -- �Ϸ�
��0 -- ���Ϸ�
*/
int StockOrderHelper::OrderMsgValidate(std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg,
	int& out_iStockId)
{
	const string ftag("StockOrderHelper::OrderMsgValidate() ");
	if (0 == io_orderMsg->strStockID.length())
	{
		EzLog::e(ftag, "��ȡ��ƱIDΪ��");

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "��ȡ��ƱIDΪ��";
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	int iResTrans = Tgw_StringUtil::String2Int_atoi(io_orderMsg->strStockID, out_iStockId);
	if (0 != iResTrans)
	{
		string strDebug("ת��StockId[");
		strDebug += io_orderMsg->strStockID;
		strDebug += "]ΪIntʧ��";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// �������� -- ԭʼ
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strOrderqty_origin, io_orderMsg->ui64Orderqty_origin);
	if (0 != iResTrans)
	{
		string strDebug("ת��Orderqty_origin[");
		strDebug += io_orderMsg->strOrderqty_origin;
		strDebug += "]ΪIntʧ��";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// �������� -- leavesqtyʣ����
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strLeavesQty, io_orderMsg->ui64LeavesQty);
	if (0 != iResTrans)
	{
		string strDebug("ת��LeavesQty[");
		strDebug += io_orderMsg->strLeavesQty;
		strDebug += "]ΪIntʧ��";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// �۸�
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strOrderPrice, io_orderMsg->ui64mOrderPrice);
	if (0 != iResTrans)
	{
		string strDebug("ת��Price[");
		strDebug += io_orderMsg->strOrderPrice;
		strDebug += "]ΪIntʧ��";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// trade side
	iResTrans = Tgw_StringUtil::String2Int_atoi(io_orderMsg->strSide, io_orderMsg->iSide);
	if (0 != iResTrans)
	{
		string strDebug("ת��side[");
		strDebug += io_orderMsg->strSide;
		strDebug += "]ΪIntʧ��";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// ��������1 ��/ 2 ��
	if (1 != io_orderMsg->iSide && 2 != io_orderMsg->iSide &&
		0 != io_orderMsg->strSide.compare("D") && 0 != io_orderMsg->strSide.compare("E"))
	{
		string strDebug("side[");
		strDebug += io_orderMsg->strSide;
		strDebug += "]unknown";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	// minqty
	iResTrans = Tgw_StringUtil::String2UInt64_strtoui64(io_orderMsg->strMinQty, io_orderMsg->ui64MinQty);
	if (0 != iResTrans)
	{
		string strDebug("ת��MinQty[");
		strDebug += io_orderMsg->strMinQty;
		strDebug += "]ΪIntʧ��";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		// 0--����A��
		/*
		//	ί������			TimeInForce OrdType MaxPriceLevels MinQty
		// ��ͨ�޼�				0			2			0		0
		// �޼�ȫ��ɽ�����	3			2			0	=OrderQty
		// ��������				0			U			0		0
		//���ַ�����ʣ��ת�޼�	0			1			1		0
		//�м������ɽ�ʣ�೷��	3			1			0		0
		//�м�ȫ��ɽ�����	3			1			0	=OrderQty
		//�м������嵵ȫ��ɽ�ʣ�೷��3		1			5		0

		*/
		if (0 == io_orderMsg->strTimeInForce.compare("0") && 0 == io_orderMsg->strOrdType.compare("2")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("0") && 0 == io_orderMsg->ui64MinQty)
		{
			// ��ͨ�޼�				0			2			0		0
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("3") && 0 == io_orderMsg->strOrdType.compare("2")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("0") &&
			io_orderMsg->ui64Orderqty_origin == io_orderMsg->ui64MinQty)
		{
			// �޼�ȫ��ɽ�����	3			2			0	=OrderQty
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("0") && 0 == io_orderMsg->strOrdType.compare("U")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("0") && 0 == io_orderMsg->ui64MinQty)
		{
			// ��������				0			U			0		0
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("0") && 0 == io_orderMsg->strOrdType.compare("1")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("1") && 0 == io_orderMsg->ui64MinQty)
		{
			//���ַ�����ʣ��ת�޼�	0			1			1		0
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("3") && 0 == io_orderMsg->strOrdType.compare("1")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("0") && 0 == io_orderMsg->ui64MinQty)
		{
			//�м������ɽ�ʣ�೷��	3			1			0		0
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("3") && 0 == io_orderMsg->strOrdType.compare("1")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("0") &&
			io_orderMsg->ui64Orderqty_origin == io_orderMsg->ui64MinQty)
		{
			//�м�ȫ��ɽ�����	3			1			0	=OrderQty
		}
		else if (0 == io_orderMsg->strTimeInForce.compare("3") && 0 == io_orderMsg->strOrdType.compare("1")
			&& 0 == io_orderMsg->strMaxPriceLevels.compare("5") && 0 == io_orderMsg->ui64MinQty)
		{
			//�м������嵵ȫ��ɽ�ʣ�೷��3		1			5		0
		}
		else if (0 == io_orderMsg->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
		{
			// ����
		}
		else
		{
			string strDebug("δ֪ί�����market[");
			strDebug += io_orderMsg->strTrade_market;
			strDebug += "].ordType[";
			strDebug += io_orderMsg->strOrdType;
			strDebug += "]";
			EzLog::e(ftag, strDebug);

			io_orderMsg->strErrorCode = simutgw::SZ_ERRCODE::c20005;
			io_orderMsg->bDataError = true;
			io_orderMsg->strError = strDebug;
			io_orderMsg->enMatchType = simutgw::ErrorMatch;

			return -1;
		}
	}
	else if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// 1--�Ϻ�A��
		if (0 == io_orderMsg->strOrdType.compare("LPT") || 0 == io_orderMsg->strOrdType.compare("ORD") // ��ͨί��
			|| 0 == io_orderMsg->strOrdType.compare("LRZ") || 0 == io_orderMsg->strOrdType.compare("LRQ") // ������ȯ
			|| 0 == io_orderMsg->strOrdType.compare("MPT") // �����嵵��ʱ�ɽ�ʣ�೷��
			|| 0 == io_orderMsg->strOrdType.compare("NPT") // �����嵵��ʱ�ɽ�ʣ��ת�޼�
			)
		{
			// 
		}
		else
		{
			string strDebug("δ֪ί�����market[");
			strDebug += io_orderMsg->strTrade_market;
			strDebug += "].ordType[";
			strDebug += io_orderMsg->strOrdType;
			strDebug += "]";
			EzLog::e(ftag, strDebug);

			io_orderMsg->strErrorCode = simutgw::SH_ERRCODE::c209;
			io_orderMsg->bDataError = true;
			io_orderMsg->strError = strDebug;
			io_orderMsg->enMatchType = simutgw::ErrorMatch;

			return -1;
		}

	}
	else
	{
		string strDebug("δ֪�г�market[");
		strDebug += io_orderMsg->strTrade_market;
		strDebug += "]";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	if (simutgw::TADE_TYPE::error == io_orderMsg->iTrade_type)
	{
		// 0--A�ɹ�Ʊ
		// 1--B�ɹ�Ʊ
		// 2--����
		// 3--��ȯ
		string strDebug("��֧�ֵĽ���stockType[");
		string strTran;
		strDebug += sof_string::itostr(io_orderMsg->iTrade_type, strTran);
		strDebug += "]";
		EzLog::e(ftag, strDebug);

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = strDebug;
		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	return 0;
}

/*
ת��Ϊ��Ļ��ӡ��������

*/
string& StockOrderHelper::OrderToScreenOut(const std::shared_ptr<struct simutgw::OrderMessage>& order,
	string& out_strScreenOut)
{
	static const string ftag("StockOrderHelper::OrderToScreenOut()");

	string strTran;

	// `trade_market` tinyint(4) NOT NULL COMMENT '0--����A�ɣ�1--�Ϻ�A��',
	out_strScreenOut = "{\"trade_market\":";
	out_strScreenOut += order->strTrade_market;

	//`trade_type` tinyint(4) NOT NULL COMMENT '0--A�ɹ�Ʊ',
	out_strScreenOut += ",\"trade_type\":";
	out_strScreenOut += sof_string::itostr(order->iTrade_type, strTran);

	// `sessionid` varchar(50) CHARACTER SET gbk DEFAULT NULL COMMENT '��ʶΨһsession�Ự',
	out_strScreenOut += ",\"sessionid\":";
	out_strScreenOut += order->strSessionId;

	// `security_account` varchar(12) CHARACTER SET gbk DEFAULT NULL COMMENT '֤ȯ�˺�',
	out_strScreenOut += ",\"security_account\":";
	out_strScreenOut += order->strAccountId;

	// `security_seat` varchar(6) CHARACTER SET gbk DEFAULT NULL COMMENT 'ϯλ',
	out_strScreenOut += ",\"security_seat\":";
	out_strScreenOut += order->strSecurity_seat;

	// `trade_group` varchar(10) CHARACTER SET gbk DEFAULT NULL COMMENT '����Ȧ',
	out_strScreenOut += ",\"trade_group\":";
	out_strScreenOut += order->strTrade_group;

	// `clordid` varchar(10) CHARACTER SET gbk DEFAULT NULL COMMENT '�ͻ��������',
	out_strScreenOut += ",\"clordid\":";
	out_strScreenOut += order->strClordid;

	// `msgtype` varchar(6) CHARACTER SET gbk DEFAULT NULL COMMENT '��Ϣ����,D=�¶���,F=����',
	out_strScreenOut += ",\"msgtype\":";
	out_strScreenOut += order->strMsgType;

	// `orderqty_origin` bigint(20) unsigned DEFAULT NULL COMMENT '��������(�¶���)/ԭʼ��������(����)',
	out_strScreenOut += ",\"orderqty_origin\":";
	out_strScreenOut += order->strOrderqty_origin;

	// `ordtype` varchar(3) CHARACTER SET gbk DEFAULT NULL COMMENT '��������1=�м�,2=�޼�,U=��������',
	out_strScreenOut += ",\"ordtype\":";
	out_strScreenOut += order->strOrdType;

	// `order_price` bigint(20) unsigned DEFAULT NULL COMMENT '�۸�,OrdTypeΪ 2 ,���м�ί��ʱ��д',
	out_strScreenOut += ",\"order_price\":";
	out_strScreenOut += order->strOrderPrice;

	// `securityid` varchar(8) CHARACTER SET gbk DEFAULT NULL COMMENT '֤ȯ����',
	out_strScreenOut += ",\"securityid\":";
	out_strScreenOut += order->strStockID;

	// `side` varchar(1) CHARACTER SET gbk DEFAULT NULL COMMENT '��������, 1=��2=��',
	out_strScreenOut += ",\"side\":";
	out_strScreenOut += order->strSide;

	// `text` varchar(256) CHARACTER SET gbk DEFAULT NULL COMMENT '��ע',
	out_strScreenOut += ",\"text\":";
	out_strScreenOut += order->strError;

	// `timeinforce` varchar(1) CHARACTER SET gbk DEFAULT NULL COMMENT '��Чʱ������0=������Ч,3=��ʱ�ɽ���ȡ��,9=�۹�ͨ�����޼���,ȱʡֵΪ 0',
	out_strScreenOut += ",\"timeinforce\":";
	out_strScreenOut += order->strTimeInForce;

	// `applid` varchar(3) CHARACTER SET gbk DEFAULT NULL COMMENT 'Ӧ�ñ�ʶ',
	out_strScreenOut += ",\"applid\":";
	out_strScreenOut += order->strApplID;

	// `oper_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '����ʱ��',
	out_strScreenOut += ",\"oper_time\":";
	out_strScreenOut += order->strOper_time;

	// `trade_time` timestamp NULL DEFAULT NULL COMMENT '����ʱ��',
	out_strScreenOut += ",\"trade_time\":";
	out_strScreenOut += order->strTrade_time;

	out_strScreenOut += ",\"cumqty\":";
	out_strScreenOut += order->strCumQty;

	out_strScreenOut += ",\"leavesqty\":";
	out_strScreenOut += order->strLeavesQty;

	out_strScreenOut += ",\"cashorderqty\":";
	out_strScreenOut += order->strCashorderqty;

	// `is_error` tinyint(4) NOT NULL DEFAULT '0' COMMENT '���������ʶ,0=��ȷ,1=����',	
	out_strScreenOut += ",\"is_error\":";
	out_strScreenOut += sof_string::itostr(order->bDataError, strTran);

	// `is_proc` tinyint(4) NOT NULL DEFAULT '0' COMMENT '���������ʶ,0=δ����,1=�Ѵ���',
	out_strScreenOut += ",\"is_proc\":";
	out_strScreenOut += order->strIsProc;

	// `error_code` varchar(10) COLLATE gbk_bin DEFAULT NULL,
	out_strScreenOut += ",\"error_code\":";
	out_strScreenOut += order->strErrorCode;

	out_strScreenOut += "}";

	return out_strScreenOut;
}

/*
���ݹ�Ʊ�����ȡ��Ʊ��������
"0" -- A��
"1" -- B��
"2" -- ���ʽ���
"3" -- ��ȯ����

����A�ɵĴ�������000��ͷ��
����B�ɵĴ�������200��ͷ��

����A�ɵĴ�������600��601��603��ͷ��
����B�ɵĴ�������900��ͷ��

Return:
0 -- �ɹ�
��0 -- ʧ��
*/
int StockOrderHelper::CheckOrder_TradeType(std::shared_ptr<struct simutgw::OrderMessage> &io_OrderMsg)
{
	static const string strTag("StockOrderHelper::CheckOrder_TradeType() ");

	if (io_OrderMsg->strStockID.length() != 6)
	{
		// ��Ʊ����Ϊ6λ�������Ĳ��Ϸ�
		string strError("StockId[");
		strError += io_OrderMsg->strStockID;
		strError += "] illeagal";

		EzLog::e(strTag, strError);
		return -1;
	}
	/*
	*00��ͷ�Ĺ�Ʊ����֤A�ɣ����ڴ��̹ɣ�����6006��ͷ�Ĺ�Ʊ���������еĹ�Ʊ��6016��ͷ�Ĺ�ƱΪ���������;
	*900��ͷ�Ĺ�Ʊ����֤B��;
	*000��ͷ�Ĺ�Ʊ����֤A�ɣ�001��002��ͷ�Ĺ�ƱҲ��������֤A�ɣ�����002��ͷ�Ĺ�Ʊ����֤A����С��ҵ��Ʊ;
	*200��ͷ�Ĺ�Ʊ����֤B��;
	*300��ͷ�Ĺ�Ʊ�Ǵ�ҵ���Ʊ;(ֻ������)
	*400��ͷ�Ĺ�Ʊ�������г���Ʊ��
	*/
	string strHead = io_OrderMsg->strStockID.substr(0, 3);
	string strFundHead = io_OrderMsg->strStockID.substr(0, 2); //�������

	if (0 == strHead.compare("000") || 0 == strHead.compare("001")
		|| 0 == strHead.compare("002") || 0 == strHead.compare("300")
		|| 0 == strFundHead.compare("15") || 0 == strFundHead.compare("16")
		|| 0 == strFundHead.compare("18"))
	{
		// ��A
		// �����������ȯ
		if (
			(0 == io_OrderMsg->strCashMargin.compare("2")
			&& (0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
			)
			|| (0 == io_OrderMsg->strCashMargin.compare("3")
			&& (0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
			)
			)
		{
			// ���ʽ��ף������������ȯ����
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::margin_cash;
		}
		else if (
			(0 == io_OrderMsg->strCashMargin.compare("2")
			&& (0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
			)
			|| (
			0 == io_OrderMsg->strCashMargin.compare("3")
			&& (0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == io_OrderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
			)
			)
		{
			// ��ȯ���ף���ȯ��������ȯ��ȯ
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::margin_stock;
		}
		else
		{
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::a_trade;
		}
	}
	else if (0 == strHead.compare("600") || 0 == strHead.compare("601")
		|| 0 == strHead.compare("603") || 0 == strFundHead.compare("50")
		|| 0 == strFundHead.compare("51"))
	{
		// ��A
		if (3 == io_OrderMsg->strOrdType.length() &&
			0 == io_OrderMsg->strOrdType.substr(1, 2).compare("RZ"))
		{
			// ����
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::margin_cash;
		}
		else if (3 == io_OrderMsg->strOrdType.length() &&
			0 == io_OrderMsg->strOrdType.substr(1, 2).compare("RQ"))
		{
			// ��ȯ
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::margin_stock;
		}
		else
		{
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::a_trade;
		}
	}
	else if (0 == strHead.compare("200")
		|| 0 == strHead.compare("900"))
	{
		// ��B �� ��B
		io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::b_trade;
	}
	else
	{
		// δ֧��
		string strError("StockId[");
		strError += io_OrderMsg->strStockID;
		strError += "] illegal";

		EzLog::e(strTag, strError);
		return -1;
	}

	if (0 == io_OrderMsg->strApplID.compare("120"))
	{
		if (0 == io_OrderMsg->strSide.compare("D"))
		{
			// ETF�깺
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::etf_crt;
		}
		else if (0 == io_OrderMsg->strSide.compare("E"))
		{
			// ETF���
			io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::etf_rdp;
		}
	}

	if (0 == io_OrderMsg->strMsgType.compare("F"))
	{
		// ������Ϣ
		io_OrderMsg->iTrade_type = simutgw::TADE_TYPE::cancel;
	}

	return 0;
}

/*
�ж϶��������ĸ����׹���

@param const std::map<uint64_t, std::string>& in_mapLinkRules : �ɽ����ú�ͨ���Ĺ�ϵ

Return:
0 -- �ɹ�
��0 -- ʧ��
*/
int StockOrderHelper::GetOrderMatchRule(std::shared_ptr<struct simutgw::OrderMessage> &io_OrderMsg,
	const std::map<uint64_t, uint64_t>& in_mapLinkRules)
{
	static const string strTag("StockOrderHelper::CheckOrder_TradeType_OrRule() ");

	uint64_t ui64RuleId = 0;

	int iRes = 0;
	if (simutgw::TRADE_MARKET_SH == io_OrderMsg->strTrade_market)
	{
		// �Ϻ��г�
		std::shared_ptr<struct MATCH_RULE_SH> ptrRule;

		iRes = simutgw::g_matchRule.GetShRule(io_OrderMsg->jsOrder,
			in_mapLinkRules, ui64RuleId, ptrRule);
		if (0 == iRes)
		{
			io_OrderMsg->tradePolicy.ui64RuleId = ui64RuleId;
			io_OrderMsg->tradePolicy.ptrRule_Sh = ptrRule;

			return 0;
		}
	}
	else if (simutgw::TRADE_MARKET_SZ == io_OrderMsg->strTrade_market)
	{
		// �����г�		
		std::shared_ptr<struct MATCH_RULE_SZ> ptrRule;

		iRes = simutgw::g_matchRule.GetSzRule(io_OrderMsg->szfixMsg,
			in_mapLinkRules, ui64RuleId, ptrRule);
		if (0 == iRes)
		{
			io_OrderMsg->tradePolicy.ui64RuleId = ui64RuleId;
			io_OrderMsg->tradePolicy.ptrRule_Sz = ptrRule;

			return 0;
		}
	}

	return -1;
}
