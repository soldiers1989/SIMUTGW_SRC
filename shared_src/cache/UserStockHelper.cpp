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
����ʱ��ջ���
*/
void UserStockHelper::DayEndCleanUp(void)
{
	boost::unique_lock<boost::mutex> Locker(simutgw::g_mapUStockMutex);

	simutgw::g_mapUserStock.clear();
}

/*
���׺�����û��ɷ�
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
�����ɹ�������û��ɷ�
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

//------------------------------�ɷ���������ɷ�--------------------------------//
/*
������ͨ��Ʊ�ɷ�

Param:
out_ui64etf -- Դ��etf��صĹɷ�����
out_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0	-- ����ɹ�
-1	-- ����ʧ��
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
		std::string strDebug("δ�ҵ�account_id=");
		strDebug += strAccount;
		strDebug += ",stock_id=";
		strDebug += strStockID;
		strDebug += "�Ĺɷ���Ϣ";
		EzLog::i(strTag, strDebug);

		return -1;
	}

	iRes = simutgw::g_mapUserStock[strAccount][strStockID]->SellFroze(ui64Froze, out_ui64etf, out_ui64avl);

	return iRes;
}

/*
���������ͨ��Ʊ�ɷ�

Param:
in_ui64etf -- Դ��etf��صĹɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0	-- �������ɹ�
-1	-- �������ʧ��
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
ȷ����ͨ�ɷݽ��׶���Ӷ���Ĳ��ֿ۳���Ӧ���
Param:
in_ui64etf -- Դ��etf��صĹɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0 -- ȷ�ϳɹ�
-1 -- ȷ��ʧ��
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

//------------------------------etf��������etf�ݶ�--------------------------------//
/*
����etf�ɷ�
Param:
out_ui64etf -- Դ���깺�ķݶ�
out_ui64avl -- Դ�ڿ���etf�ݶ�

Return:
0	-- ����ɹ�
-1	-- ����ʧ��
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
�������etf�ɷ�
Param:
in_ui64etf -- Դ���깺�ķݶ�
in_ui64avl -- Դ�ڿ���etf�ݶ�

Return:
0	-- �������ɹ�
-1	-- �������ʧ��
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
ȷ��etf���׶���Ӷ���Ĳ��ֿ۳���Ӧ���
Param:
in_ui64etf -- Դ���깺�ķݶ�
in_ui64avl -- Դ�ڿ���etf�ݶ�

Return:
0 -- ȷ�ϳɹ�
-1 -- ȷ��ʧ��
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

//------------------------------etf�깺����ɷֹ�--------------------------------//
/*
�깺etf�����гɷֹɹɷ��Ƿ��㹻
Param:
ui64EtfQty -- ETF�깺����
out_ui64CashCompnents -- �ֽ�����ܽ��

Return:
0	-- �㹻
-1	-- ����
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
			// �����ֽ����
			//dMaxCash -= ui64Times * ComponentInfo.ui64MoneyCreationCashSubstitute;
			if (0 > dMaxCash)
			{
				// ����ֽ�����������
				string strDebug("ETF:");
				strDebug += ptrEtf->strSecurityID;
				strDebug += "����ֽ�����������";

				EzLog::e(strTag, strDebug);
				return -1;
			}
		}
		else
		{
			string strStockID = ComponentInfo.strUnderlyingSecurityID;

			// ���ҹɷ���Ϣ
			enum StockHelper::StockType stkType;
			int iRes = LookUp(strAccount, strStockID, stkType);
			if (0 != iRes)
			{
				string strDebug("ETF:");
				strDebug += ptrEtf->strSecurityID;
				strDebug += "�����йɷ�:";
				strDebug += strStockID;

				EzLog::e(strTag, strDebug);
				return -1;
			}

			// �ٲ�ѯ����������
			uint64_t ui64Max = 0;
			iRes = simutgw::g_mapUserStock[strAccount][strStockID]->CreationQuery(
				ui64Times*ComponentInfo.ui64ComponentShare, ui64Max);
			if (0 != iRes)
			{
				// ���ùɷݲ���
				if (simutgw::Only_Security == ComponentInfo.iSubstituteFlag)
				{
					// ֻ���ùɷ�
					string strDebug("ETF:");
					strDebug += ptrEtf->strSecurityID;
					strDebug += "���йɷ�:";
					strDebug += strStockID;
					strDebug += "����";

					EzLog::e(strTag, strDebug);
					return -1;
				}
				else
				{
					// ���㲿�����ֽ����

					// �Ȳ����Ʊ����۸�
					uint64_t ui64Zrsp;
					string strTpbz;
					iRes = MarketInfoHelper::GetCurrStaticQuot(strStockID, ui64Zrsp, strTpbz);
					if (0 != iRes)
					{
						// ������
						string strDebug("ETF:");
						strDebug += ptrEtf->strSecurityID;
						strDebug += "�ɷֹɹɷ�:";
						strDebug += strStockID;
						strDebug += "�����̼�";

						EzLog::e(strTag, strDebug);
						return -1;
					}

					if (0 != strTpbz.compare("F"))
					{
						// ͣ��
						string strDebug("ETF:");
						strDebug += ptrEtf->strSecurityID;
						strDebug += "�ɷֹɹɷ�:";
						strDebug += strStockID;
						strDebug += "ͣ��";

						EzLog::e(strTag, strDebug);
						return -1;
					}

					dMaxCash -= (ui64Times*ComponentInfo.ui64ComponentShare - ui64Max)*ui64Zrsp;
					//* (1 + ComponentInfo.dPremiumRatio);
					if (0 > dMaxCash)
					{
						// ����ֽ�����������
						string strDebug("ETF:");
						strDebug += ptrEtf->strSecurityID;
						strDebug += "����ֽ�����������";

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

	// �ٶ���ÿһֻ�ɷֹ�
	for (size_t st = 0; st < ptrEtf->vecComponents.size(); ++st)
	{
		struct simutgw::SzETFComponent &ComponentInfo = ptrEtf->vecComponents[st];
		std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent> ptrFrozeComponentInfo(
			new struct simutgw::EtfCrt_FrozeComponent);

		ptrFrozeComponentInfo->strSecurityID = ComponentInfo.strUnderlyingSecurityID;

		if (simutgw::Only_Cash == ComponentInfo.iSubstituteFlag)
		{
			// �����ֽ����
			ptrFrozeComponentInfo->ui64avl_count = ptrFrozeComponentInfo->ui64act_pch_count = 0;
			ptrFrozeComponentInfo->ui64Cash = ui64Times * ComponentInfo.ui64mCreationCashSubstitute;
		}
		else
		{
			string strStockID = ComponentInfo.strUnderlyingSecurityID;

			// ���ҹɷ���Ϣ
			enum StockHelper::StockType stkType;
			LookUp(strAccount, strStockID, stkType);

			// �ٲ�ѯ����������
			int iRes = simutgw::g_mapUserStock[strAccount][strStockID]->CreationFroze(
				ui64Times*ComponentInfo.ui64ComponentShare, ptrFrozeComponentInfo->ui64act_pch_count,
				ptrFrozeComponentInfo->ui64avl_count);
			if (0 != iRes)
			{
				// ���ùɷݲ���
				if (simutgw::Security_And_Cash == ComponentInfo.iSubstituteFlag)
				{
					// ���������ֽ����

					// �Ȳ����Ʊ����۸�
					uint64_t ui64Zrsp;
					string strTpbz;
					iRes = MarketInfoHelper::GetCurrStaticQuot(strStockID, ui64Zrsp, strTpbz);
					if (0 != iRes)
					{
						string strDebug("ETF: stockId=");
						strDebug += strStockID;
						strDebug += "�޿�������";

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
�����깺etf�ĳɷֹɹɷ�

Return:
0	-- ����ɹ�
-1	-- ����ʧ��
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
��������깺etf�ĳɷֹɹɷ�

Param:
in_ui64act -- Դ�ھ�������ɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0	-- �������ɹ�
-1	-- �������ʧ��
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
ȷ���깺etf�ĳɷֹɽ��׶���Ӷ���Ĳ��ֿ۳���Ӧ���
Param:
in_ui64act -- Դ�ھ�������ɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0 -- ȷ�ϳɹ�
-1 -- ȷ��ʧ��
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

//------------------------------etf��ض���etf�ݶ�--------------------------------//
/*
�������etf�ķݶ�

Return:
0	-- ����ɹ�
-1	-- ����ʧ��
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
����������etf�ķݶ�

Param:
in_ui64act -- Դ�ھ�������ɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0	-- �������ɹ�
-1	-- �������ʧ��
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
ȷ�����etf�ķݶ���Ӷ���Ĳ��ֿ۳���Ӧ���
Param:
in_ui64act -- Դ�ھ�������ɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0 -- ȷ�ϳɹ�
-1 -- ȷ��ʧ��
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

//------------------------------���ӹɷ�--------------------------------//
/*
�����������ӣ���ͨ�ɷݡ�etf�ݶ

Return:
0 -- �ɹ�
-1 -- ʧ��
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
�깺��������(etf�ݶ�)

Return:
0 -- �ɹ�
-1 -- ʧ��
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
���etf�ݶ�ʱ����(�ɷֹɹɷ�)

Return:
0 -- �ɹ�
-1 -- ʧ��
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
�����û��ɷ���Ϣ
���ڴ����У����سɹ�
���ڴ����ޣ����ѯ���ݿ⣬�м��ؽ��ڴ沢���سɹ���û�з���ʧ��

Return:
0 -- �ɹ�
-1 -- ʧ��
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
�����ݿ���عɷ���Ϣ

Return:
0 -- �ɹ�
-1 -- ʧ��
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
		std::string strDebug("δ�ҵ�account_id=");
		strDebug += strAccount;
		strDebug += ",stock_id=";
		strDebug += strStockID;
		strDebug += "�Ĺɷ���Ϣ";
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
	//	strDebug += "�ĸ��ɷ�֮�ͺ͹ɷ������ֶ�ֵ����";
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
���׺�����û��ɷ�--��ͨ����(����etf����)
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
				// 3 -- ��ͨģʽ ��¼���ݿ�
				iRes = StockCacheSynHelper::SellSyn(io_order->strAccountId, io_order->strStockID);
			}
		}
		else if (0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == io_order->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
		{
			iRes = UserStockHelper::AddAct(io_order->strAccountId, io_order->strStockID, io_order->ui64Orderqty_matched);

			if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
			{
				// 3 -- ��ͨģʽ ��¼���ݿ�
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
			// 3 -- ��ͨģʽ ��¼���ݿ�
			iRes = StockCacheSynHelper::SellEtfSyn(io_order->strAccountId, io_order->strStockID);
		}
		break;

	case simutgw::TADE_TYPE::etf_buy:
		iRes = UserStockHelper::AddAct(io_order->strAccountId, io_order->strStockID, io_order->ui64Orderqty_matched);

		if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
		{
			// 3 -- ��ͨģʽ ��¼���ݿ�
			iRes = StockCacheSynHelper::AddEtfSyn(io_order->strAccountId, io_order->strStockID);
		}
		break;

	default:
		break;
	}

	if (0 != iRes)
	{
		std::string strDebug("�ɷݸ���ʧ��,account_id=");
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
���׺�����û��ɷ�--ETF����
*/
int UserStockHelper::UpdateETFCrtRdp(std::shared_ptr<struct simutgw::OrderMessage>& io_order)
{
	static const std::string strTag("UserStockHelper::UpdateETFCrtRdp() ");

	switch (io_order->iTrade_type)
	{
	case simutgw::TADE_TYPE::etf_crt:
		// ���깺
	{
		int iRes = 0;
		// �ȼ�ȥ�ɷֹɵĹɷ�
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
					// 3 -- ��ͨģʽ ��¼���ݿ�
					StockCacheSynHelper::CrtComponentSyn(io_order->strAccountId, ptrTemp->strSecurityID);
				}
			}
			RecordMatchTableHelper::RecordCompnentMatchInfo(io_order, ptrTemp);
		}

		if (io_order->tradePolicy.bCheck_Assets)
		{
			// �ټ����깺��etf�ɷ�
			iRes = AddCrt(io_order->strAccountId, io_order->strStockID, io_order->ui64Orderqty_matched);
			if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
			{
				// 3 -- ��ͨģʽ ��¼���ݿ�
				StockCacheSynHelper::CrtEtfSyn(io_order->strAccountId, io_order->strStockID);
			}
		}

		break;
	}

	case simutgw::TADE_TYPE::etf_rdp:
		// �����
	{
		// �ȼ�����ز��ֵĹɷ�
		int iRes = 0;
		if (io_order->tradePolicy.bCheck_Assets)
		{
			iRes = RedemptionConfirm(io_order->strAccountId, io_order->strStockID,
				io_order->rdpETFCps.ui64Act_pch, io_order->rdpETFCps.ui64Avl);

			if (simutgw::SysRunMode::NormalMode == io_order->tradePolicy.iRunMode)
			{
				// 3 -- ��ͨģʽ ��¼���ݿ�
				StockCacheSynHelper::RdpEtfSyn(io_order->strAccountId, io_order->strStockID);
			}
		}

		// �����ӳɷֹɵĹɷ�
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
					// 3 -- ��ͨģʽ ��¼���ݿ�
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
����������û��ɷ�--��ͨ����(����etf����)
*/
int UserStockHelper::UpdateCancel(std::shared_ptr<struct simutgw::OrderMessage>& io_order)
{
	static const std::string strTag("UserStockHelper::UpdateCancel() ");

	if (!io_order->tradePolicy.bCheck_Assets)
	{
		return 0;
	}

	// ʣ����Ϊ0
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
		std::string strDebug("�ɷݸ���ʧ��,account_id=");
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