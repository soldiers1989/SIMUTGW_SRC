#ifndef __STOCK_HELPER_H__
#define __STOCK_HELPER_H__

#include <string>
#include <stdint.h>


namespace StockHelper
{
	// 股票类型，普通股票或etf
	enum StockType
	{
		Ordinary = 1,//普通股票
		Etf = 2 //ETF
	};

	/*
	获取股票类型
	普通股票或者ETF

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int GetStockType(const std::string& strStockID, enum StockHelper::StockType& out_stkType);

	/*
	取得股票的市场
	深圳或上海
	*/
	std::string& GetStockTradeMarket(const std::string& strStockID, std::string& out_strTradeMarket);
}


#endif