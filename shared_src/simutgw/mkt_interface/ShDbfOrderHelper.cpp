#include "ShDbfOrderHelper.h"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "simutgw/msg_biz/ProcCancelOrder.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "tool_string/sof_string.h"

#include "order/StockOrderHelper.h"
#include "simutgw/order/OrderMemoryStoreFactory.h"
#include "simutgw/db_oper/RecordNewOrderHelper.h"

#include "simutgw_config/g_values_sys_run_config.h"

#include "config/conf_mssql_table.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "simutgw/biz/AppId_010_OrderValide.h"
#include "simutgw/biz/PreTrade.h"

#include "ShConn_DbfOrder.h"

/*
�Ϻ�ί������ת����FIXЭ���ʽ
*/
int ShDbfOrderHelper::SHOrderToFix(OTLConn40240& otlConn, otl_stream& in_stream,
	vector<std::shared_ptr<struct simutgw::OrderMessage>> &io_vecOrder,
	const string &strSessionid, const struct TradePolicyCell& in_policy)
{
	static const string strTag("ShDbfOrderHelper::SHOrderToFix() ");

	vector<string> vecRecNum;
	string strKey, strDate, strTime;
	std::map<std::string, struct OTLConn_DF::DataInRow> mapRowData;
	while (otlConn.FetchNextRow(&in_stream, mapRowData))
	{
		vecRecNum.push_back(mapRowData["rec_num"].strValue);

		//����Arrary�е�һ��order
		std::shared_ptr<struct simutgw::OrderMessage> shOrder(new struct simutgw::OrderMessage);

		// д�����
		shOrder->tradePolicy = in_policy;

		// ί���յ�ʱ��
		shOrder->tRcvTime = time(NULL);

		rapidjson::Document::AllocatorType& jsAlloc = shOrder->jsOrder.GetAllocator();

		// rec_num ��¼��ţ�����������Ψһ
		shOrder->strMsgSeqNum = mapRowData["rec_num"].strValue;
		shOrder->jsOrder.AddMember("rec_num", rapidjson::Value(mapRowData["rec_num"].strValue.c_str(), jsAlloc), jsAlloc);

		if (0 == simutgw::g_mapShConns[strSessionid].GetRecNum())//(0 == simutgw::g_iRec_Num)
		{
			//Tgw_StringUtil::String2UInt64_strtoui64(shOrder->strMsgSeqNum, simutgw::g_iRec_Num);
			uint64_t ui64Value = 0;
			Tgw_StringUtil::String2UInt64_strtoui64(shOrder->strMsgSeqNum, ui64Value);
			simutgw::g_mapShConns[strSessionid].SetRecNum(ui64Value + 1);
		}
		//EzLog::Out("rec_num: ", info, (int)simutgw::g_iRec_Num);

		//date д������ YYYYMMDD
		strDate = mapRowData["date"].strValue;
		shOrder->jsOrder.AddMember("date", rapidjson::Value(mapRowData["date"].strValue.c_str(), jsAlloc), jsAlloc);

		//time д��ʱ�� HH:MM:SS
		strTime = mapRowData["time"].strValue;
		shOrder->jsOrder.AddMember("time", rapidjson::Value(mapRowData["time"].strValue.c_str(), jsAlloc), jsAlloc);

		//reff ��Ա�ڲ�������
		shOrder->strClordid = mapRowData["reff"].strValue;
		shOrder->jsOrder.AddMember("reff", rapidjson::Value(mapRowData["reff"].strValue.c_str(), jsAlloc), jsAlloc);

		//acc ֤ȯ�˺�
		shOrder->strAccountId = mapRowData["acc"].strValue;
		shOrder->jsOrder.AddMember("acc", rapidjson::Value(mapRowData["acc"].strValue.c_str(), jsAlloc), jsAlloc);

		//stock ֤ȯ����
		shOrder->strStockID = mapRowData["stock"].strValue;
		shOrder->jsOrder.AddMember("stock", rapidjson::Value(mapRowData["stock"].strValue.c_str(), jsAlloc), jsAlloc);

		//bs ��������
		shOrder->strSide = mapRowData["bs"].strValue;
		if (0 == shOrder->strSide.compare("b"))
		{
			shOrder->strSide = "B";
		}
		else if (0 == shOrder->strSide.compare("s"))
		{
			shOrder->strSide = "S";
		}

		shOrder->jsOrder.AddMember("bs", rapidjson::Value(mapRowData["bs"].strValue.c_str(), jsAlloc), jsAlloc);

		//price �걨�۸�
		shOrder->strOrderPrice = mapRowData["price"].strValue;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(shOrder->strOrderPrice, shOrder->ui64mOrderPrice);
		sof_string::itostr(shOrder->ui64mOrderPrice, shOrder->strOrderPrice);

		shOrder->jsOrder.AddMember("price", rapidjson::Value(mapRowData["price"].strValue.c_str(), jsAlloc), jsAlloc);

		//qty �걨����
		shOrder->strOrderqty_origin = mapRowData["qty"].strValue;
		Tgw_StringUtil::String2UInt64_strtoui64(shOrder->strOrderqty_origin, shOrder->ui64Orderqty_origin);

		shOrder->jsOrder.AddMember("qty", rapidjson::Value(mapRowData["qty"].strValue.c_str(), jsAlloc), jsAlloc);

		//leavesqty
		shOrder->strLeavesQty = shOrder->strOrderqty_origin;
		//"status"))
		// nothing ����״̬����R����r����ʾ�ü�¼��û�з��ͣ���P����ʾ�ѷ����Ͻ�����̨

		//owflag �������ͱ�־
		shOrder->strOrdType = mapRowData["owflag"].strValue;
		shOrder->jsOrder.AddMember("owflag", rapidjson::Value(mapRowData["owflag"].strValue.c_str(), jsAlloc), jsAlloc);

		//ordrec �������
		shOrder->strOrigClordid = mapRowData["ordrec"].strValue;
		shOrder->jsOrder.AddMember("ordrec", rapidjson::Value(mapRowData["ordrec"].strValue.c_str(), jsAlloc), jsAlloc);

		//firmid B�ɽ����Ա���룬����A��Ͷ����ȡֵ������
		shOrder->strConfirmID = mapRowData["firmid"].strValue;
		shOrder->jsOrder.AddMember("firmid", rapidjson::Value(mapRowData["firmid"].strValue.c_str(), jsAlloc), jsAlloc);

		//branchid Ӫҵ������
		shOrder->strMarket_branchid = mapRowData["branchid"].strValue;
		shOrder->jsOrder.AddMember("branchid", rapidjson::Value(mapRowData["branchid"].strValue.c_str(), jsAlloc), jsAlloc);

		//"checkord"
		//nothing У����,������

		// �Ϻ�A��
		shOrder->strTrade_market = simutgw::TRADE_MARKET_SH;

		// transacttime YYYYMMDD-HH:MM:SS.sss
		shOrder->strTransactTime = strDate;
		shOrder->strTransactTime += "-";
		shOrder->strTransactTime += strTime;
		shOrder->strTransactTime += ".000";

		// `oper_time` YYYY-MM-DD HH:MM:SS
		TimeStringUtil::GetCurrTimeInTradeType(shOrder->strOper_time);

		//seesionid
		shOrder->strSessionId = strSessionid;

		//msgtype
		if (0 == shOrder->strOrdType.compare("WTH"))
		{
			shOrder->strMsgType = "F";

			//����ʱ����Ҫ����reff��ordrec
			string strValue = shOrder->strOrigClordid;
			shOrder->strOrigClordid = shOrder->strClordid;
			shOrder->strClordid = strValue;
		}
		else
		{
			shOrder->strMsgType = "D";
		}

		// web����ģʽ�²���Ҫ��ѯ
		if (simutgw::WebManMode::WebMode == simutgw::g_iWebManMode)
		{
			// �жϹ�Ʊ�ĳɽ����ù���
			StockOrderHelper::GetOrderMatchRule(shOrder, shOrder->tradePolicy.mapLinkRules);
		}

		// �ж��Ƿ����й���
		if (0 == shOrder->tradePolicy.ui64RuleId)
		{
			// �޳ɽ����ù���ʱ�����뽻����
			// �жϹ�Ʊ��������
			int iRes = StockOrderHelper::CheckOrder_TradeType(shOrder);
			if (0 != iRes)
			{
				// ��ȡ����ί��
				ShConn_DbfOrder::SetOrdwthStatus(strSessionid, shOrder->strMsgSeqNum, "P");

				shOrder->enMatchType = simutgw::ErrorMatch;
				shOrder->bDataError = true;

				// д��ر�����
				simutgw::g_outMsg_buffer.PushBack(shOrder);

				continue;
			}
		}

		io_vecOrder.push_back(shOrder);

	} // while (valIter != doc["tables"].End())

	ShConn_DbfOrder::Set_MutilOrd_Status(strSessionid, vecRecNum);

	return 0;
}

/*
����Ϻ�ί���Ƿ���֧�ֵ�ҵ������

Return:
0 -- ֧��
-1 -- ��֧��
*/
int ShDbfOrderHelper::Validate_SH_Order(std::shared_ptr<struct simutgw::OrderMessage> &shOrder)
{
	static const string strTag("ShDbfOrderHelper::Validate_SH_Order() ");

	int iRes = 0;

	//�ų�һЩ�ǽ���ҵ��
	if (0 == shOrder->strAccountId.compare("799988") && 0 == shOrder->strConfirmID.compare("939988"))
	{
		shOrder->bDataError = true;
		shOrder->enMatchType = simutgw::ErrorMatch;
		shOrder->strErrorCode = "269";
		iRes = -1;
	}
	else if (0 == shOrder->strStockID.compare("799999") && 1 == shOrder->ui64mOrderPrice &&
		1 == shOrder->ui64Orderqty_origin)
	{
		// ָ���Ǽ�
		shOrder->bDataError = true;
		shOrder->enMatchType = simutgw::ErrorMatch;
		shOrder->strErrorCode = "269";
		iRes = -1;
	}
	else if (shOrder->strStockID.empty() && shOrder->strAccountId.empty() && shOrder->strClordid.empty())
	{
		//nothing
		shOrder->bDataError = true;
		shOrder->enMatchType = simutgw::ErrorMatch;
		shOrder->strErrorCode = "269";
		iRes = -1;
	}
	else
	{

	}

	return iRes;
}