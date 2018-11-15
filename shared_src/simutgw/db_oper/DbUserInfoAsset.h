#ifndef __DB_USERINFO_ASSET_H__
#define __DB_USERINFO_ASSET_H__

#include <string>

#include "order/define_order_msg.h"
#include "order/StockHelper.h"
#include "cache/UserStockHelper.h"

#include "tool_mysql/MysqlConnectionPool.h"

/*
	处理数据库中 用户信息 以及 用户资产 的类
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
	在数据库中获取用户的余额等数据

	Return :
	0 -- 获取成功
	<0 -- 获取失败
	1 -- 余券不足
	*/
	static int GetDbUserData(std::shared_ptr<struct simutgw::OrderMessage>& ptrObj);

	/*
	在数据库中获取用户的帐户相关数据

	Return :
	0 -- 获取成功
	<0 -- 获取失败
	*/
	static int GetDb_UserAcntInfo(std::shared_ptr<struct simutgw::OrderMessage>& ptrObj);

	/*
	在数据库中获取用户的ETF持仓

	Return :
	0 -- 获取成功
	<0 -- 获取失败
	*/
	static int GetDb_User_Etf(std::shared_ptr<struct simutgw::OrderMessage>& ptrObj);

	/*
	在数据库中获取用户的单只股票的持仓

	Return :
	0 -- 获取成功，用户持有该只股票
	1 -- 获取失败，用户不持有该只股票
	-1 -- 获取失败，其他类型的错误
	*/
	static int GetDb_UserHoldStock(std::shared_ptr<MySqlCnnC602>& mysqlConn,
		const std::string& in_sAccountId, const std::string& in_sStockId,
		std::shared_ptr<struct simutgw::TradeStock>& out_ptrUserStock);

	/*
	从数据库加载股份信息

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int LoadUserStockFromDB(const std::string& strAccount, const std::string& strStockID, struct UserStockInfo& out_usrStkInfo);
};

#endif