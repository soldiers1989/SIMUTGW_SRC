#include "StockHelper.h"

#include "util/EzLog.h"

#include "order/define_order_msg.h"
#include "config/conf_msg.h"

using namespace std;

namespace StockHelper
{
	/*
	��ȡ��Ʊ����
	��ͨ��Ʊ����ETF

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int GetStockType(const std::string& strStockID, enum StockHelper::StockType& out_stkType)
	{
		if (6 != strStockID.length())
		{
			return -1;
		}

		std::string strHead = strStockID.substr(0, 2);
		if (0 == strHead.compare("15") || 0 == strHead.compare("16")
			|| 0 == strHead.compare("18") || 0 == strHead.compare("50")
			|| 0 == strHead.compare("51"))
		{
			out_stkType = StockHelper::Etf;
		}
		else
		{
			out_stkType = StockHelper::Ordinary;
		}

		return 0;
	}

	/*
	ȡ�ù�Ʊ���г�
	���ڻ��Ϻ�
	*/
	string& GetStockTradeMarket(const string& strStockID, string& out_strTradeMarket)
	{
		static const string strTag("StockOrderHelper::GetStockTradeMarket() ");

		out_strTradeMarket.clear();
		if (strStockID.length() != 6)
		{
			// ��Ʊ����Ϊ6λ�������Ĳ��Ϸ�
			string strError("StockId[");
			strError += strStockID;
			strError += "] illeagal";

			EzLog::e(strTag, strError);
			return out_strTradeMarket;
		}
		/*
		*00��ͷ�Ĺ�Ʊ����֤A�ɣ����ڴ��̹ɣ�����6006��ͷ�Ĺ�Ʊ���������еĹ�Ʊ��6016��ͷ�Ĺ�ƱΪ���������;
		*900��ͷ�Ĺ�Ʊ����֤B��;
		*000��ͷ�Ĺ�Ʊ����֤A�ɣ�001��002��ͷ�Ĺ�ƱҲ��������֤A�ɣ�����002��ͷ�Ĺ�Ʊ����֤A����С��ҵ��Ʊ;
		*200��ͷ�Ĺ�Ʊ����֤B��;
		*300��ͷ�Ĺ�Ʊ�Ǵ�ҵ���Ʊ;(ֻ������)
		*400��ͷ�Ĺ�Ʊ�������г���Ʊ��
		*/
		string strHead = strStockID.substr(0, 3);
		string strFundHead = strStockID.substr(0, 2); //�������

		if (0 == strHead.compare("000") || 0 == strHead.compare("001")
			|| 0 == strHead.compare("002") || 0 == strHead.compare("300")
			|| 0 == strFundHead.compare("15") || 0 == strFundHead.compare("16")
			|| 0 == strFundHead.compare("18") || 0 == strHead.compare("200"))
		{
			// ����
			out_strTradeMarket = simutgw::TRADE_MARKET_SZ;
		}
		else if (0 == strHead.compare("600") || 0 == strHead.compare("601")
			|| 0 == strHead.compare("603") || 0 == strFundHead.compare("50")
			|| 0 == strFundHead.compare("51") || 0 == strHead.compare("900"))
		{
			// �Ϻ�	
			out_strTradeMarket = simutgw::TRADE_MARKET_SH;
		}
		else
		{
			// δ֧��
			string strError("StockId[");
			strError += strStockID;
			strError += "] illegal";

			EzLog::e(strTag, strError);
			return out_strTradeMarket;
		}

		return out_strTradeMarket;
	}
}

