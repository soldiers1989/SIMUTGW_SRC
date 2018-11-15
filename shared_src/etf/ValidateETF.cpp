#include "ValidateETF.h"
#include "util/EzLog.h"

ValidateETF::ValidateETF()
{
}


ValidateETF::~ValidateETF()
{
}


/*
校验成分股信息
Return:
0 -- 合法
-1 -- 非法
*/
int ValidateETF::Validate_ETFComponent_Info(struct simutgw::SzETFComponent& cpn)
{
	static const std::string strTag("ValidateETF::Validate_ETFComponent_Info() ");

	if (cpn.strUnderlyingSecurityID.empty())
	{
		EzLog::e(strTag, "成分股代码为空");
		return -1;
	}

	if (0 == cpn.strUnderlyingSecurityID.compare("159900"))
	{
		//虚拟成份证券 159900（用于现金替代）的信息，该虚拟成份证券
		//“ 现金替代标志”字段为“必须现金替代”，“ 证券代码源”字段为“ 102”，表示该虚
		//拟成份证券是深市证券，在申购赎回时，对该虚拟成份证券必须使用现金替代。
		if (simutgw::Only_Cash != cpn.iSubstituteFlag)
		{
			EzLog::e(strTag, "成分股159900现金替代比例有误");
			return -1;
		}

		if (0 == cpn.ui64mCreationCashSubstitute)
		{
			EzLog::e(strTag, "成分股159900替代金额有误");
			return -1;
		}

		return 0;
	}

	// 根据是否可用现金替代进行判断
	if (simutgw::Only_Security == cpn.iSubstituteFlag)
	{
		// 不能用现金替代 
		if (0 == cpn.ui64ComponentShare)
		{
			// 成分股数量为0
			EzLog::e(strTag, "成分股数量为0");
			return -1;
		}
	}
	else if (simutgw::Security_And_Cash == cpn.iSubstituteFlag)
	{
		// 可用现金替代
		if (0 == cpn.ui64ComponentShare)
		{
			// 成分股数量为0
			EzLog::e(strTag, "成分股数量为0");
			return -1;
		}

		if (0 == cpn.dPremiumRatio)
		{
			// 现金替代比例为0
			EzLog::e(strTag, "成分股现金替代比例为0");
			return -1;
		}
	}
	else if (simutgw::Only_Cash == cpn.iSubstituteFlag)
	{
		// 只能用现金替代
		if (0 == cpn.ui64mCreationCashSubstitute ||
			0 == cpn.ui64mRedemptionCashSubstitute)
		{
			// 替代总金额为0
			EzLog::e(strTag, "成分股替代金额为0");
			return -1;
		}
	}

	return 0;
}

/*
校验ETF除成分股之外的信息
Return:
0 -- 合法
-1 -- 非法
*/
int ValidateETF::Validate_ETF_Info(const std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	static const std::string strTag("ValidateETF::Validate_ETF_Info() ");

	if (ptrEtf->strSecurityID.empty())
	{
		EzLog::e(strTag, "证券代码为空");
		return -1;
	}

	if (0 == ptrEtf->ui64CreationRedemptionUnit)
	{
		EzLog::e(strTag, "最小申购赎回单位为0");
		return -1;
	}

	if (0 == ptrEtf->ui64CreationRedemptionUnit)
	{
		EzLog::e(strTag, "最小申购赎回单位为0");
		return -1;
	}

	if (ptrEtf->ui64TotalRecordNum != ptrEtf->vecComponents.size())
	{
		EzLog::e(strTag, "成分股数量和实际成分股数量不等");
		return -1;
	}

	return 0;
}