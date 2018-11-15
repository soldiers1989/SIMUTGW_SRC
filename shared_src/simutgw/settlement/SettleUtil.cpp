#include "SettleUtil.h"

#include <memory>

#include "tool_string/Tgw_StringUtil.h"

namespace SettleUtil
{
	/*
	securityid 是否是深圳ETF代码

	@return:
	true -- 是深圳ETF代码
	false -- 不是
	*/
	bool IsSzEtf(const std::string& strSecurityId)
	{
		int iSecurityId = 0;
		Tgw_StringUtil::String2Int_atoi(strSecurityId, iSecurityId);

		// 在 159901 和 159999之间
		if (159901 <= iSecurityId && 159999 >= iSecurityId)
		{
			return true;
		}

		return false;
	}

};