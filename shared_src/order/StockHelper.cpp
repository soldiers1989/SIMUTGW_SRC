#include "StockHelper.h"

#include "util/EzLog.h"

#include "order/define_order_msg.h"
#include "config/conf_msg.h"

using namespace std;

namespace StockHelper
{
	/*
	获取股票类型
	普通股票或者ETF

	Return:
	0 -- 成功
	-1 -- 失败
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
	取得股票的市场
	深圳或上海
	*/
	string& GetStockTradeMarket(const string& strStockID, string& out_strTradeMarket)
	{
		static const string strTag("StockOrderHelper::GetStockTradeMarket() ");

		out_strTradeMarket.clear();
		if (strStockID.length() != 6)
		{
			// 股票代码为6位，其他的不合法
			string strError("StockId[");
			strError += strStockID;
			strError += "] illeagal";

			EzLog::e(strTag, strError);
			return out_strTradeMarket;
		}
		/*
		*00开头的股票是上证A股，属于大盘股，其中6006开头的股票是最早上市的股票，6016开头的股票为大盘蓝筹股;
		*900开头的股票是上证B股;
		*000开头的股票是深证A股，001、002开头的股票也都属于深证A股，其中002开头的股票是深证A股中小企业股票;
		*200开头的股票是深证B股;
		*300开头的股票是创业板股票;(只有深市)
		*400开头的股票是三板市场股票。
		*/
		string strHead = strStockID.substr(0, 3);
		string strFundHead = strStockID.substr(0, 2); //基金代码

		if (0 == strHead.compare("000") || 0 == strHead.compare("001")
			|| 0 == strHead.compare("002") || 0 == strHead.compare("300")
			|| 0 == strFundHead.compare("15") || 0 == strFundHead.compare("16")
			|| 0 == strFundHead.compare("18") || 0 == strHead.compare("200"))
		{
			// 深圳
			out_strTradeMarket = simutgw::TRADE_MARKET_SZ;
		}
		else if (0 == strHead.compare("600") || 0 == strHead.compare("601")
			|| 0 == strHead.compare("603") || 0 == strFundHead.compare("50")
			|| 0 == strFundHead.compare("51") || 0 == strHead.compare("900"))
		{
			// 上海	
			out_strTradeMarket = simutgw::TRADE_MARKET_SH;
		}
		else
		{
			// 未支持
			string strError("StockId[");
			strError += strStockID;
			strError += "] illegal";

			EzLog::e(strTag, strError);
			return out_strTradeMarket;
		}

		return out_strTradeMarket;
	}
}

