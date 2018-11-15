#include "SettleUtil.h"

#include <memory>

#include "tool_string/Tgw_StringUtil.h"

namespace SettleUtil
{
	/*
	securityid �Ƿ�������ETF����

	@return:
	true -- ������ETF����
	false -- ����
	*/
	bool IsSzEtf(const std::string& strSecurityId)
	{
		int iSecurityId = 0;
		Tgw_StringUtil::String2Int_atoi(strSecurityId, iSecurityId);

		// �� 159901 �� 159999֮��
		if (159901 <= iSecurityId && 159999 >= iSecurityId)
		{
			return true;
		}

		return false;
	}

};