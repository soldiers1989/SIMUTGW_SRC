#ifndef __DB_USERINFO_ASSET_H__
#define __DB_USERINFO_ASSET_H__

#include <string>

#include "order/define_order_msg.h"
#include "order/StockHelper.h"
#include "cache/UserStockHelper.h"

#include "tool_mysql/MysqlConnectionPool.h"

/*
	�������ݿ��� �û���Ϣ �Լ� �û��ʲ� ����
	*/
class DbUserInfoAsset
{
	//
	// member
	//


	//
	// function
	//
public:
	DbUserInfoAsset()
	{ }

	virtual ~DbUserInfoAsset(void)
	{ }

	/*
	�����ݿ��л�ȡ�û�����������

	Return :
	0 -- ��ȡ�ɹ�
	<0 -- ��ȡʧ��
	1 -- ��ȯ����
	*/
	static int GetDbUserData(std::shared_ptr<struct simutgw::OrderMessage>& ptrObj);

	/*
	�����ݿ��л�ȡ�û����ʻ��������

	Return :
	0 -- ��ȡ�ɹ�
	<0 -- ��ȡʧ��
	*/
	static int GetDb_UserAcntInfo(std::shared_ptr<struct simutgw::OrderMessage>& ptrObj);

	/*
	�����ݿ��л�ȡ�û���ETF�ֲ�

	Return :
	0 -- ��ȡ�ɹ�
	<0 -- ��ȡʧ��
	*/
	static int GetDb_User_Etf(std::shared_ptr<struct simutgw::OrderMessage>& ptrObj);

	/*
	�����ݿ��л�ȡ�û��ĵ�ֻ��Ʊ�ĳֲ�

	Return :
	0 -- ��ȡ�ɹ����û����и�ֻ��Ʊ
	1 -- ��ȡʧ�ܣ��û������и�ֻ��Ʊ
	-1 -- ��ȡʧ�ܣ��������͵Ĵ���
	*/
	static int GetDb_UserHoldStock(std::shared_ptr<MySqlCnnC602>& mysqlConn,
		const std::string& in_sAccountId, const std::string& in_sStockId,
		std::shared_ptr<struct simutgw::TradeStock>& out_ptrUserStock);

	/*
	�����ݿ���عɷ���Ϣ

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int LoadUserStockFromDB(const std::string& strAccount, const std::string& strStockID, struct UserStockInfo& out_usrStkInfo);
};

#endif