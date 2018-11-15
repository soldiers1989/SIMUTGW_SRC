#ifndef __STOCK_CACHE_SYN_HELPER_H__
#define __STOCK_CACHE_SYN_HELPER_H__

#include <string>

/*
	内存缓存的股份信息同步到数据库
	发生在股份确认之后
*/
class StockCacheSynHelper
{
	//
	// member
	//
private:

	//
	// function
	//
public:
	StockCacheSynHelper();
	virtual ~StockCacheSynHelper();

	/*
	内存与DB股份信息同步
	普通股份卖出
	*/
	static int SellSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	内存与DB股份信息同步
	etf卖出
	*/
	static int SellEtfSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	内存与DB股份信息同步
	普通股份买入
	*/
	static int AddSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	内存与DB股份信息同步
	etf买入
	*/
	static int AddEtfSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	内存与DB股份信息同步
	申购ETF增加
	*/
	static int CrtEtfSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	内存与DB股份信息同步
	申购成分股减少
	*/
	static int CrtComponentSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	内存与DB股份信息同步
	赎回ETF减少
	*/
	static int RdpEtfSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	内存与DB股份信息同步
	赎回成分股增加
	*/
	static int RdpComponentSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	执行更新sql语句
	*/
	static int ExcUpdateSql(const std::string& strUpdate);
};

#endif