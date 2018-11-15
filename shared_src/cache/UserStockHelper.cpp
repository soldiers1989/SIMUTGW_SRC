#include "UserStockHelper.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw_config/g_values_sys_run_config.h"

#include "cache/UserStockVolume.h"
#include "simutgw/db_oper/StockCacheSynHelper.h"
#include "simutgw/db_oper/RecordMatchTableHelper.h"
#include "simutgw/db_oper/DbUserInfoAsset.h"

#include "tool_string/Tgw_StringUtil.h"

#include "quotation/MarketInfoHelper.h"


UserStockHelper::UserStockHelper()
{
}


UserStockHelper::~UserStockHelper()
{
}

/*
日终时清空缓存
*/
void UserStockHelper::DayEndCleanUp(void)
{
	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	simutgw::g_mapUserStock.clear();
}

/*
交易后更新用户股份
*/
int UserStockHelper::UpdateAfterTrade(std::shared_ptr<struct simutgw::OrderMessage>& io_order)
{
	// static const std::string ftag("UserStockHelper::UpdateAfterTrade() ");

	int iRes = 0;
	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);
	if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		if (0 == io_order->strApplID.compare("010"))
		{
			iRes = UpdateTrade(io_order);
		}
		else if (0 == io_order->strApplID.compare("120"))
		{
			iRes = UpdateETFCrtRdp(io_order);
		}
	}
	else if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		iRes = UpdateTrade(io_order);
	}

	return iRes;
}

/*
撤单成功后更新用户股份
*/
int UserStockHelper::UpdateAfterCancel(std::shared_ptr<struct simutgw::OrderMessage>& io_order)
{
	// static const std::string ftag("UserStockHelper::UpdateAfterCancel() ");

	int iRes = 0;
	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);
	if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		if (0 == io_order->strApplID.compare("010"))
		{
			iRes = UpdateCancel(io_order);
		}
		else if (0 == io_order->strApplID.compare("120"))
		{

		}
	}
	else if (0 == io_order->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		iRes = UpdateCancel(io_order);
	}


	return iRes;
}

//------------------------------股份卖出冻结股份--------------------------------//
/*
冻结普通股票股份

Param:
out_ui64etf -- 源于etf赎回的股份数量
out_ui64avl -- 源于可用股份数量

Return:
0	-- 冻结成功
-1	-- 冻结失败
*/
int UserStockHelper::SellFroze(const std::string& strAccount, const std::string& strStockID,
	uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl)
{
	static const std::string strTag("UserStockHelper::SellFroze() ");

	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		std::string strDebug("未找到account_id=");
		strDebug += strAccount;
		strDebug += ",stock_id=";
		strDebug += strStockID;
		strDebug += "的股份信息";
		EzLog::i(strTag, strDebug);

		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->SellFroze(ui64Froze, out_ui64etf, out_ui64avl);

	return iRes;
}

/*
解除冻结普通股票股份

Param:
in_ui64etf -- 源于etf赎回的股份数量
in_ui64avl -- 源于可用股份数量

Return:
0	-- 解除冻结成功
-1	-- 解除冻结失败
*/
int UserStockHelper::SellDeFroze(const std::string& strAccount, const std::string& strStockID,
	uint64_t in_ui64etf, uint64_t in_ui64avl)
{
	// static const std::string ftag("UserStockHelper::SellDeFroze()");

	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->SellDeFroze(in_ui64etf, in_ui64avl);

	return iRes;
}

/*
确认普通股份交易额，将从冻结的部分扣除相应额度
Param:
in_ui64etf -- 源于etf赎回的股份数量
in_ui64avl -- 源于可用股份数量

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int UserStockHelper::SellConfirm(const std::string& strAccount, const std::string& strStockID,
	uint64_t in_ui64etf, uint64_t in_ui64avl)
{
	{
		// static const std::string ftag("UserStockHelper::SellConfirm()");

		//boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

		enum StockHelper::StockType stkType;
		int iRes = LookUp(strAccount, strStockID, stkType);
		if (0 != iRes)
		{
			return -1;
		}

		iRes = simutgw::g_mapUserStock[strAccount][strStockID]->SellConfirm(in_ui64etf, in_ui64avl);

		return iRes;
	}
}

//------------------------------etf卖出冻结etf份额--------------------------------//
/*
冻结etf股份
Param:
out_ui64etf -- 源于申购的份额
out_ui64avl -- 源于可用etf份额

Return:
0	-- 冻结成功
-1	-- 冻结失败
*/
int UserStockHelper::SellEtfFroze(const std::string& strAccount, const std::string& strStockID,
	uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl)
{
	// static const std::string ftag("UserStockHelper::SellEtfFroze()");

	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->SellEtfFroze(ui64Froze, out_ui64etf, out_ui64avl);

	return iRes;
}

/*
解除冻结etf股份
Param:
in_ui64etf -- 源于申购的份额
in_ui64avl -- 源于可用etf份额

Return:
0	-- 解除冻结成功
-1	-- 解除冻结失败
*/
int UserStockHelper::SellEtfDeFroze(const std::string& strAccount, const std::string& strStockID,
	uint64_t in_ui64etf, uint64_t in_ui64avl)
{
	// static const std::string ftag("UserStockHelper::SellEtfDeFroze()");

	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->SellEtfDeFroze(in_ui64etf, in_ui64avl);

	return iRes;
}

/*
确认etf交易额，将从冻结的部分扣除相应额度
Param:
in_ui64etf -- 源于申购的份额
in_ui64avl -- 源于可用etf份额

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int UserStockHelper::SellEtfConfirm(const std::string& strAccount, const std::string& strStockID,
	uint64_t in_ui64etf, uint64_t in_ui64avl)
{
	// static const std::string ftag("UserStockHelper::SellEtfConfirm()");

	//boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->SellEtfConfirm(in_ui64etf, in_ui64avl);

	return iRes;
}

//------------------------------etf申购冻结成分股--------------------------------//
/*
申购etf的所有成分股股份是否足够
Param:
ui64EtfQty -- ETF申购数量
out_ui64CashCompnents -- 现金替代总金额

Return:
0	-- 足够
-1	-- 不足
*/
int UserStockHelper::CreationQuery(const std::string& strAccount, const uint64_t ui64EtfQty,
	const std::shared_ptr<struct simutgw::SzETF>& ptrEtf,
	std::vector<std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent>> &io_vecFrozeCompnent)
{
	static const std::string strTag("UserStockHelper::CreationQuery()");

	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	uint64_t ui64Times = ui64EtfQty / ptrEtf->ui64CreationRedemptionUnit;
	double dMaxCash = ui64EtfQty * ptrEtf->ui64CreationRedemptionUnit * ptrEtf->ui64mNAVperCU *
		ptrEtf->dMaxCashRatio;

	for (size_t st = 0; st < ptrEtf->vecComponents.size(); ++st)
	{
		struct simutgw::SzETFComponent &ComponentInfo = ptrEtf->vecComponents[st];
		if (simutgw::Only_Cash == ComponentInfo.iSubstituteFlag)
		{
			// 必须现金替代
			//dMaxCash -= ui64Times * ComponentInfo.ui64MoneyCreationCashSubstitute;
			if (0 > dMaxCash)
			{
				// 最大现金替代金额用完
				string strDebug("ETF:");
				strDebug += ptrEtf->strSecurityID;
				strDebug += "最大现金替代金额用完";

				EzLog::e(strTag, strDebug);
				return -1;
			}
		}
		else
		{
			string strStockID = ComponentInfo.strUnderlyingSecurityID;

			// 先找股份信息
			enum StockHelper::StockType stkType;
			int iRes = LookUp(strAccount, strStockID, stkType);
			if (0 != iRes)
			{
				string strDebug("ETF:");
				strDebug += ptrEtf->strSecurityID;
				strDebug += "不持有股份:";
				strDebug += strStockID;

				EzLog::e(strTag, strDebug);
				return -1;
			}

			// 再查询最大可用数量
			uint64_t ui64Max = 0;
			iRes = simutgw::g_mapUserStock[strAccount][strStockID]->CreationQuery(
				ui64Times*ComponentInfo.ui64ComponentShare, ui64Max);
			if (0 != iRes)
			{
				// 可用股份不足
				if (simutgw::Only_Security == ComponentInfo.iSubstituteFlag)
				{
					// 只能用股份
					string strDebug("ETF:");
					strDebug += ptrEtf->strSecurityID;
					strDebug += "持有股份:";
					strDebug += strStockID;
					strDebug += "不足";

					EzLog::e(strTag, strDebug);
					return -1;
				}
				else
				{
					// 不足部分用现金替代

					// 先查出股票行情价格
					uint64_t ui64Zrsp;
					string strTpbz;
					iRes = MarketInfoHelper::GetCurrStaticQuot(strStockID, ui64Zrsp, strTpbz);
					if (0 != iRes)
					{
						// 无行情
						string strDebug("ETF:");
						strDebug += ptrEtf->strSecurityID;
						strDebug += "成分股股份:";
						strDebug += strStockID;
						strDebug += "无收盘价";

						EzLog::e(strTag, strDebug);
						return -1;
					}

					if (0 != strTpbz.compare("F"))
					{
						// 停牌
						string strDebug("ETF:");
						strDebug += ptrEtf->strSecurityID;
						strDebug += "成分股股份:";
						strDebug += strStockID;
						strDebug += "停牌";

						EzLog::e(strTag, strDebug);
						return -1;
					}

					dMaxCash -= (ui64Times*ComponentInfo.ui64ComponentShare - ui64Max)*ui64Zrsp;
					//* (1 + ComponentInfo.dPremiumRatio);
					if (0 > dMaxCash)
					{
						// 最大现金替代金额用完
						string strDebug("ETF:");
						strDebug += ptrEtf->strSecurityID;
						strDebug += "最大现金替代金额用完";

						EzLog::e(strTag, strDebug);
						return -1;
					}
				}
			}
			else
			{

			}
		}
	}

	// 再冻结每一只成分股
	for (size_t st = 0; st < ptrEtf->vecComponents.size(); ++st)
	{
		struct simutgw::SzETFComponent &ComponentInfo = ptrEtf->vecComponents[st];
		std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent> ptrFrozeComponentInfo(
			new struct simutgw::EtfCrt_FrozeComponent);

		ptrFrozeComponentInfo->strSecurityID = ComponentInfo.strUnderlyingSecurityID;

		if (simutgw::Only_Cash == ComponentInfo.iSubstituteFlag)
		{
			// 必须现金替代
			ptrFrozeComponentInfo->ui64avl_count = ptrFrozeComponentInfo->ui64act_pch_count = 0;
			ptrFrozeComponentInfo->ui64Cash = ui64Times * ComponentInfo.ui64mCreationCashSubstitute;
		}
		else
		{
			string strStockID = ComponentInfo.strUnderlyingSecurityID;

			// 先找股份信息
			enum StockHelper::StockType stkType;
			LookUp(strAccount, strStockID, stkType);

			// 再查询最大可用数量
			int iRes = simutgw::g_mapUserStock[strAccount][strStockID]->CreationFroze(
				ui64Times*ComponentInfo.ui64ComponentShare, ptrFrozeComponentInfo->ui64act_pch_count,
				ptrFrozeComponentInfo->ui64avl_count);
			if (0 != iRes)
			{
				// 可用股份不足
				if (simutgw::Security_And_Cash == ComponentInfo.iSubstituteFlag)
				{
					// 不足是用现金替代

					// 先查出股票行情价格
					uint64_t ui64Zrsp;
					string strTpbz;
					iRes = MarketInfoHelper::GetCurrStaticQuot(strStockID, ui64Zrsp, strTpbz);
					if (0 != iRes)
					{
						string strDebug("ETF: stockId=");
						strDebug += strStockID;
						strDebug += "无可用行情";

						EzLog::e(strTag, strDebug);
						return -1;
					}

					ptrFrozeComponentInfo->ui64Cash = (ui64Times*ComponentInfo.ui64ComponentShare -
						ptrFrozeComponentInfo->ui64act_pch_count - ptrFrozeComponentInfo->ui64avl_count) *
						(uint64_t)(ui64Zrsp * (1 + ComponentInfo.dPremiumRatio));
				}
			}
			else
			{
				ptrFrozeComponentInfo->ui64Cash = 0;
			}
		}

		io_vecFrozeCompnent.push_back(ptrFrozeComponentInfo);
	}


	return 0;
}

/*
冻结申购etf的成分股股份

Return:
0	-- 冻结成功
-1	-- 冻结失败
*/
int UserStockHelper::CreationFroze(const std::string& strAccount, const std::string& strStockID,
	uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl)
{
	// static const std::string ftag("UserStockHelper::CreationFroze()");

	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->CreationFroze(ui64Froze, out_ui64act, out_ui64avl);

	return iRes;
}

/*
解除冻结申购etf的成分股股份

Param:
in_ui64act -- 源于竞价买入股份数量
in_ui64avl -- 源于可用股份数量

Return:
0	-- 解除冻结成功
-1	-- 解除冻结失败
*/
int UserStockHelper::CreationDeFroze(const std::string& strAccount, const std::string& strStockID,
	uint64_t in_ui64act, uint64_t in_ui64avl)
{
	// static const std::string ftag("UserStockHelper::CreationDeFroze()");

	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->CreationDeFroze(in_ui64act, in_ui64avl);

	return iRes;
}

/*
确认申购etf的成分股交易额，将从冻结的部分扣除相应额度
Param:
in_ui64act -- 源于竞价买入股份数量
in_ui64avl -- 源于可用股份数量

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int UserStockHelper::CreationConfirm(const std::string& strAccount, const std::string& strStockID,
	uint64_t in_ui64act, uint64_t in_ui64avl)
{
	// static const std::string ftag("UserStockHelper::CreationConfirm()");

	//boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->CreationConfirm(in_ui64act, in_ui64avl);

	return iRes;
}

//------------------------------etf赎回冻结etf份额--------------------------------//
/*
冻结赎回etf的份额

Return:
0	-- 冻结成功
-1	-- 冻结失败
*/
int UserStockHelper::RedemptionFroze(const std::string& strAccount, const std::string& strStockID,
	uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl)
{
	// static const std::string ftag("UserStockHelper::RedemptionFroze()");

	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->RedemptionFroze(ui64Froze, out_ui64act, out_ui64avl);

	return iRes;
}

/*
解除冻结赎回etf的份额

Param:
in_ui64act -- 源于竞价买入股份数量
in_ui64avl -- 源于可用股份数量

Return:
0	-- 解除冻结成功
-1	-- 解除冻结失败
*/
int UserStockHelper::RedemptionDeFroze(const std::string& strAccount, const std::string& strStockID,
	uint64_t in_ui64act, uint64_t in_ui64avl)
{
	// static const std::string ftag("UserStockHelper::RedemptionDeFroze()");

	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->RedemptionDeFroze(in_ui64act, in_ui64avl);

	return iRes;
}

/*
确认赎回etf的份额，将从冻结的部分扣除相应额度
Param:
in_ui64act -- 源于竞价买入股份数量
in_ui64avl -- 源于可用股份数量

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int UserStockHelper::RedemptionConfirm(const std::string& strAccount, const std::string& strStockID,
	uint64_t in_ui64act, uint64_t in_ui64avl)
{
	// static const std::string ftag("UserStockHelper::RedemptionConfirm()");

	//boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->RedemptionConfirm(in_ui64act, in_ui64avl);

	return iRes;
}

//------------------------------增加股份--------------------------------//
/*
竞价买入增加（普通股份、etf份额）

Return:
0 -- 成功
-1 -- 失败
*/
int UserStockHelper::AddAct(const std::string& strAccount, const std::string& strStockID, uint64_t ui64StpNum)
{
	// static const std::string ftag("UserStockHelper::AddAct()");

	//boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		auto it = simutgw::g_mapUserStock.find(strAccount);
		if (simutgw::g_mapUserStock.end() == it)
		{
			std::map<string, std::shared_ptr<UserStockVolume>> mapStk;
			simutgw::g_mapUserStock.insert(make_pair(strAccount, mapStk));
		}

		std::shared_ptr<UserStockVolume> ptrUserStock(new UserStockVolume(false, 0, 0, 0, 0, 0, 0, 0));
		simutgw::g_mapUserStock[strAccount].insert(make_pair(strStockID, ptrUserStock));
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->AddAct(ui64StpNum);

	return iRes;
}

/*
申购买入增加(etf份额)

Return:
0 -- 成功
-1 -- 失败
*/
int UserStockHelper::AddCrt(const std::string& strAccount, const std::string& strStockID, uint64_t ui64CrtNum)
{
	// static const std::string ftag("UserStockHelper::AddCrt()");

	//boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		auto it = simutgw::g_mapUserStock.find(strAccount);
		if (simutgw::g_mapUserStock.end() == it)
		{
			std::map<string, std::shared_ptr<UserStockVolume>> mapStk;
			simutgw::g_mapUserStock.insert(make_pair(strAccount, mapStk));
		}

		std::shared_ptr<UserStockVolume> ptrUserStock(new UserStockVolume(false, 0, 0, 0, 0, 0, 0, 0));
		simutgw::g_mapUserStock[strAccount].insert(make_pair(strStockID, ptrUserStock));
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->AddCrt(ui64CrtNum);

	return iRes;
}

/*
赎回etf份额时增加(成分股股份)

Return:
0 -- 成功
-1 -- 失败
*/
int UserStockHelper::AddRdp(const std::string& strAccount, const std::string& strStockID, uint64_t ui64RdpNum)
{
	// static const std::string ftag("UserStockHelper::AddRdp()");

	//boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	enum StockHelper::StockType stkType;
	int iRes = LookUp(strAccount, strStockID, stkType);
	if (0 != iRes)
	{
		auto it = simutgw::g_mapUserStock.find(strAccount);
		if (simutgw::g_mapUserStock.end() == it)
		{
			std::map<string, std::shared_ptr<UserStockVolume>> mapStk;
			simutgw::g_mapUserStock.insert(make_pair(strAccount, mapStk));
		}

		std::shared_ptr<UserStockVolume> ptrUserStock(new UserStockVolume(false, 0, 0, 0, 0, 0, 0, 0));
		simutgw::g_mapUserStock[strAccount].insert(make_pair(strStockID, ptrUserStock));
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->AddRdp(ui64RdpNum);

	return iRes;
}

/*
查找用户股份信息
若内存中有，返回成功
若内存中无，则查询数据库，有加载进内存并返回成功，没有返回失败

Return:
0 -- 成功
-1 -- 失败
*/
int UserStockHelper::LookUp(const std::string& strAccount, const std::string& strStockID,
enum StockHelper::StockType& out_stkType)
{
	// static const std::string ftag("UserStockHelper::LookUp() ");

	bool bExist = false;
	auto it = simutgw::g_mapUserStock.find(strAccount);
	if (simutgw::g_mapUserStock.end() != it)
	{
		auto itStk = it->second.find(strStockID);
		if (it->second.end() != itStk)
		{
			bExist = true;
		}
	}

	if (bExist)
	{
		return 0;
	}

	int iRes = LoadUserStockFromDBAndStore(strAccount, strStockID);

	return iRes;
}

/*
从数据库加载股份信息

Return:
0 -- 成功
-1 -- 失败
*/
int UserStockHelper::LoadUserStockFromDBAndStore(const std::string& strAccount, const std::string& strStockID)
{
	static const std::string strTag("UserStockHelper::LoadUserStockFromDBAndStore() ");

	struct UserStockInfo usrStkInfo;
	usrStkInfo.strAccount = strAccount;
	usrStkInfo.strStockID = strStockID;

	int iRes = DbUserInfoAsset::LoadUserStockFromDB(strAccount, strStockID, usrStkInfo);
	if (0 != iRes)
	{
		std::string strDebug("未找到account_id=");
		strDebug += strAccount;
		strDebug += ",stock_id=";
		strDebug += strStockID;
		strDebug += "的股份信息";
		EzLog::e(strTag, strDebug);

		return -1;
	}

	//if (usrStkInfo.ui64stkBalance != (usrStkInfo.ui64stk_act_pch_Balance +
	//	usrStkInfo.ui64stk_stp_pch_Balance + usrStkInfo.ui64stk_etf_rdp_Balance +
	//	usrStkInfo.ui64stk_crt_Balance + usrStkInfo.ui64stk_avl_Balance))
	//{
	//	std::string strDebug("account_id=");
	//	strDebug += strAccount;
	//	strDebug += ",stock_id=";
	//	strDebug += strStockID;
	//	strDebug += "的各股份之和和股份数量字段值不等";
	//	EzLog::e(strTag, strDebug);

	//	return -1;
	//}

	std::shared_ptr<UserStockVolume> ptrUserStock(new UserStockVolume(true,
		usrStkInfo.ui64stkBalance, usrStkInfo.ui64stk_act_pch_Balance,
		usrStkInfo.ui64stk_stp_pch_Balance, usrStkInfo.ui64stk_etf_rdp_Balance,
		usrStkInfo.ui64stk_crt_Balance, usrStkInfo.ui64stk_avl_Balance,
		usrStkInfo.ui64stk_last_Balance));

	auto it = simutgw::g_mapUserStock.find(strAccount);
	if (simutgw::g_mapUserStock.end() == it)
	{
		std::map<string, std::shared_ptr<UserStockVolume>> mapStk;
		simutgw::g_mapUserStock.insert(make_pair(strAccount, mapStk));
	}

	simutgw::g_mapUserStock[strAccount].insert(make_pair(strStockID, ptrUserStock));

	return 0;
}

/*
交易后更新用户股份--普通交易(包括etf买卖)
*/
int UserStockHelper::UpdateTrade(std::shared_ptr<struct simutgw::OrderMessage>& io_order)
{
	static const std::string strTag("UserStockHelper::UpdateTrade() ");

	if (!io_order->tradePolicy.bCheck_Assets)
	{
		return 0;
	}

	enum StockHelper::StockType type;
	StockHelper::GetStockType(io_order->strStockID, type);

	int iRes = 0;

	uint64_t ui64Fst, ui64Snd, ui64FstBefore, ui64SndBefore;
	switch (io_order->iTrade_type)
	{
	case simutgw::TADE_TYPE::a_trade:
	case simutgw::TADE_TYPE::b_trade:
		if (0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
		{
			ui64FstBefore = io_order->sellCps.ui64Etf_rdp;
			ui64SndBefore = io_order->sellCps.ui64Avl;
			if (io_order->ui64Orderqty_matched <= (io_order->sellCps.ui64Etf_rdp + io_order->sellCps.ui64Avl))
			{
				if (io_order->ui64Orderqty_matched <= io_order->sellCps.ui64Etf_rdp)
				{
					ui64Fst = io_order->ui64Orderqty_matched;
					ui64Snd = 0;
					io_order->sellCps.ui64Etf_rdp -= ui64Fst;
				}
				else
				{
					ui64Fst = io_order->sellCps.ui64Etf_rdp;
					ui64Snd = io_order->ui64Orderqty_matched - ui64Fst;
					io_order->sellCps.ui64Etf_rdp = 0;
					io_order->sellCps.ui64Avl -= ui64Snd;
				}
			}
			else
			{
				ui64Fst = io_order->sellCps.ui64Etf_rdp;
				ui64Snd = io_order->sellCps.ui64Avl;

				io_order->sellCps.ui64Etf_rdp = 0;
				io_order->sellCps.ui64Avl = 0;
			}
			iRes = UserStockHelper::SellConfirm(io_order->strAccountId, io_order->strStockID, ui64Fst, ui64Snd);

			if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
			{
				// 3 -- 普通模式 记录数据库
				iRes = StockCacheSynHelper::SellSyn(io_order->strAccountId, io_order->strStockID);
			}
		}
		else if (0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
		{
			iRes = UserStockHelper::AddAct(io_order->strAccountId, io_order->strStockID, io_order->ui64Orderqty_matched);

			if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
			{
				// 3 -- 普通模式 记录数据库
				iRes = StockCacheSynHelper::AddSyn(io_order->strAccountId, io_order->strStockID);
			}
		}

		break;

	case simutgw::TADE_TYPE::etf_sell:

		ui64FstBefore = io_order->sellETFCps.ui64Etf_crt;
		ui64SndBefore = io_order->sellETFCps.ui64Avl;
		if (io_order->ui64Orderqty_matched < (io_order->sellETFCps.ui64Etf_crt + io_order->sellETFCps.ui64Avl))
		{
			if (io_order->ui64Orderqty_matched < io_order->sellETFCps.ui64Etf_crt)
			{
				ui64Fst = io_order->ui64Orderqty_matched;
				ui64Snd = 0;
				io_order->sellETFCps.ui64Etf_crt -= ui64Fst;
			}
			else
			{
				ui64Fst = io_order->sellETFCps.ui64Etf_crt;
				ui64Snd = io_order->ui64Orderqty_matched - ui64Fst;
				io_order->sellETFCps.ui64Etf_crt = 0;
				io_order->sellETFCps.ui64Avl -= ui64Snd;
			}
		}
		else
		{
			ui64Fst = io_order->sellCps.ui64Etf_rdp;
			ui64Snd = io_order->sellCps.ui64Avl;

			io_order->sellETFCps.ui64Etf_crt = 0;
			io_order->sellETFCps.ui64Avl = 0;
		}
		iRes = UserStockHelper::SellEtfConfirm(io_order->strAccountId, io_order->strStockID, ui64Fst, ui64Snd);

		if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
		{
			// 3 -- 普通模式 记录数据库
			iRes = StockCacheSynHelper::SellEtfSyn(io_order->strAccountId, io_order->strStockID);
		}
		break;

	case simutgw::TADE_TYPE::etf_buy:
		iRes = UserStockHelper::AddAct(io_order->strAccountId, io_order->strStockID, io_order->ui64Orderqty_matched);

		if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
		{
			// 3 -- 普通模式 记录数据库
			iRes = StockCacheSynHelper::AddEtfSyn(io_order->strAccountId, io_order->strStockID);
		}
		break;

	default:
		break;
	}

	if (0 != iRes)
	{
		std::string strDebug("股份更新失败,account_id=");
		strDebug += io_order->strAccountId;
		strDebug += ",stock_id=";
		strDebug += io_order->strStockID;
		strDebug += ",cliordid=";
		strDebug += io_order->strClordid;

		EzLog::e(strTag, strDebug);

		return -1;
	}

	return 0;
}

/*
交易后更新用户股份--ETF申赎
*/
int UserStockHelper::UpdateETFCrtRdp(std::shared_ptr<struct simutgw::OrderMessage>& io_order)
{
	static const std::string strTag("UserStockHelper::UpdateETFCrtRdp() ");

	switch (io_order->iTrade_type)
	{
	case simutgw::TADE_TYPE::etf_crt:
		// 是申购
	{
		int iRes = 0;
		// 先减去成分股的股份
		std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent> ptrTemp(new struct simutgw::EtfCrt_FrozeComponent);
		for (size_t st = 0; st < io_order->vecFrozeComponent.size(); ++st)
		{
			ptrTemp = io_order->vecFrozeComponent[st];
			if (0 == ptrTemp->strSecurityID.compare("159900"))
			{
				continue;
			}

			if (io_order->tradePolicy.bCheck_Assets)
			{
				iRes = CreationConfirm(io_order->strAccountId, ptrTemp->strSecurityID,
					ptrTemp->ui64act_pch_count, ptrTemp->ui64avl_count);

				if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
				{
					// 3 -- 普通模式 记录数据库
					StockCacheSynHelper::CrtComponentSyn(io_order->strAccountId, ptrTemp->strSecurityID);
				}
			}
			RecordMatchTableHelper::RecordCompnentMatchInfo(io_order, ptrTemp);
		}

		if (io_order->tradePolicy.bCheck_Assets)
		{
			// 再加上申购的etf股份
			iRes = AddCrt(io_order->strAccountId, io_order->strStockID, io_order->ui64Orderqty_matched);
			if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
			{
				// 3 -- 普通模式 记录数据库
				StockCacheSynHelper::CrtEtfSyn(io_order->strAccountId, io_order->strStockID);
			}
		}

		break;
	}

	case simutgw::TADE_TYPE::etf_rdp:
		// 是赎回
	{
		// 先减少赎回部分的股份
		int iRes = 0;
		if (io_order->tradePolicy.bCheck_Assets)
		{
			iRes = RedemptionConfirm(io_order->strAccountId, io_order->strStockID,
				io_order->rdpETFCps.ui64Act_pch, io_order->rdpETFCps.ui64Avl);

			if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
			{
				// 3 -- 普通模式 记录数据库
				StockCacheSynHelper::RdpEtfSyn(io_order->strAccountId, io_order->strStockID);
			}
		}

		// 再增加成分股的股份
		std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent> ptrTemp(new struct simutgw::EtfCrt_FrozeComponent);
		for (size_t st = 0; st < io_order->vecFrozeComponent.size(); ++st)
		{
			ptrTemp = io_order->vecFrozeComponent[st];
			if (0 == ptrTemp->strSecurityID.compare("159900"))
			{
				continue;
			}

			if (io_order->tradePolicy.bCheck_Assets)
			{
				iRes = AddRdp(io_order->strAccountId, ptrTemp->strSecurityID,
					ptrTemp->ui64rdp_count);

				if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
				{
					// 3 -- 普通模式 记录数据库
					StockCacheSynHelper::RdpComponentSyn(io_order->strAccountId, ptrTemp->strSecurityID);
				}
			}
			RecordMatchTableHelper::RecordCompnentMatchInfo(io_order, ptrTemp);
		}
		break;
	}

	default:
		break;
	}

	return 0;
}

/*
撤单后更新用户股份--普通交易(包括etf买卖)
*/
int UserStockHelper::UpdateCancel(std::shared_ptr<struct simutgw::OrderMessage>& io_order)
{
	static const std::string strTag("UserStockHelper::UpdateCancel() ");

	if (!io_order->tradePolicy.bCheck_Assets)
	{
		return 0;
	}

	// 剩余量为0
	if (0 == io_order->ui64LeavesQty)
	{
		return 0;
	}

	enum StockHelper::StockType type;
	StockHelper::GetStockType(io_order->strStockID, type);

	int iRes = 0;

	switch (io_order->iTrade_type)
	{
	case simutgw::TADE_TYPE::a_trade:
	case simutgw::TADE_TYPE::b_trade:
		if (0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
		{
			iRes = simutgw::g_mapUserStock[io_order->strAccountId][io_order->strStockID]->SellDeFroze(
				io_order->sellCps.ui64Etf_rdp, io_order->sellCps.ui64Avl);
		}
		else if (0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
		{
			//???
		}

		break;

	case simutgw::TADE_TYPE::etf_sell:
		iRes = simutgw::g_mapUserStock[io_order->strAccountId][io_order->strStockID]->SellEtfDeFroze(
			io_order->sellETFCps.ui64Etf_crt, io_order->sellETFCps.ui64Avl);

		break;

	case simutgw::TADE_TYPE::etf_buy:

		break;

	default:
		break;
	}

	if (0 != iRes)
	{
		std::string strDebug("股份更新失败,account_id=");
		strDebug += io_order->strAccountId;
		strDebug += ",stock_id=";
		strDebug += io_order->strStockID;
		strDebug += ",cliordid=";
		strDebug += io_order->strClordid;

		EzLog::e(strTag, strDebug);

		return -1;
	}

	return 0;
}