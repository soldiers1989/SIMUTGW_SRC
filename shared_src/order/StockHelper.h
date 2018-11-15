#ifndef __STOCK_HELPER_H__
#define __STOCK_HELPER_H__

#include <string>
#include <stdint.h>


namespace StockHelper
{
	// ��Ʊ���ͣ���ͨ��Ʊ��etf
	enum StockType
	{
		Ordinary = 1,//��ͨ��Ʊ
		Etf = 2 //ETF
	};

	/*
	��ȡ��Ʊ����
	��ͨ��Ʊ����ETF

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int GetStockType(const std::string& strStockID, enum StockHelper::StockType& out_stkType);

	/*
	ȡ�ù�Ʊ���г�
	���ڻ��Ϻ�
	*/
	std::string& GetStockTradeMarket(const std::string& strStockID, std::string& out_strTradeMarket);
}


#endif