#ifndef __STOCK_CACHE_SYN_HELPER_H__
#define __STOCK_CACHE_SYN_HELPER_H__

#include <string>

/*
	�ڴ滺��Ĺɷ���Ϣͬ�������ݿ�
	�����ڹɷ�ȷ��֮��
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
	�ڴ���DB�ɷ���Ϣͬ��
	��ͨ�ɷ�����
	*/
	static int SellSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	�ڴ���DB�ɷ���Ϣͬ��
	etf����
	*/
	static int SellEtfSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	�ڴ���DB�ɷ���Ϣͬ��
	��ͨ�ɷ�����
	*/
	static int AddSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	�ڴ���DB�ɷ���Ϣͬ��
	etf����
	*/
	static int AddEtfSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	�ڴ���DB�ɷ���Ϣͬ��
	�깺ETF����
	*/
	static int CrtEtfSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	�ڴ���DB�ɷ���Ϣͬ��
	�깺�ɷֹɼ���
	*/
	static int CrtComponentSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	�ڴ���DB�ɷ���Ϣͬ��
	���ETF����
	*/
	static int RdpEtfSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	�ڴ���DB�ɷ���Ϣͬ��
	��سɷֹ�����
	*/
	static int RdpComponentSyn(const std::string& strAccount, const std::string& strStockID);

	/*
	ִ�и���sql���
	*/
	static int ExcUpdateSql(const std::string& strUpdate);
};

#endif