#include "StgwFixUtil.h"
#include "util/EzLog.h"

// 取nopartyids重复组的数据，包括账号、席位和营业部代码
// @param message fix消息
// @param out_strSeat 取得的席位
// @param out_strAccount 取得的账号
// @param out_strMarket_branchid 取得的营业部代码
// @return
// 0 -- 成功
// -1 -- 失败
int StgwFixUtil::GetNopartyIds(const FIX::FieldMap& message,
	std::string& out_strSeat,
	std::string& out_strAccount,
	std::string& out_strMarket_branchid)
{
	static const std::string ftag("StgwFixUtil::GetNopartyIds() ");

	try
	{
		if (!message.hasGroup(FIX::FIELD::NoPartyIDs))
		{
			// 没有重复组
			return -1;
		}

		// 重复组元素个数
		size_t num = message.groupCount(FIX::FIELD::NoPartyIDs);

		std::string strPartyid, strPartyidsource, strPartyRole;
		for (int i = 1; i <= num; ++i)
		{
			FIX::FieldMap group;
			message.getGroup(i, FIX::FIELD::NoPartyIDs, group);
			if (group.isEmpty())
			{
				continue;
			}

			strPartyid.clear();
			strPartyidsource.clear();
			strPartyRole.clear();

			if (group.isSetField(FIX::FIELD::PartyIDSource))
			{
				strPartyidsource = group.getField(FIX::FIELD::PartyIDSource);
			}

			if (group.isSetField(FIX::FIELD::PartyRole))
			{
				strPartyRole = group.getField(FIX::FIELD::PartyRole);
			}

			if (group.isSetField(FIX::FIELD::PartyID))
			{
				strPartyid = group.getField(FIX::FIELD::PartyID);
			}

			if ((strPartyidsource == "5") && (strPartyRole == "5"))
			{
				//得到证券账号
				out_strAccount = strPartyid;
			}
			else if ((strPartyidsource == "C") && (strPartyRole == "1"))
			{
				//得到证券席位
				out_strSeat = strPartyid;
			}
			else if ((strPartyidsource == "D") && (strPartyRole == "4001"))
			{
				//得到营业部代码
				out_strMarket_branchid = strPartyid;
			}
			else
			{
				//nothing
			}
		}
	}
	catch (std::exception& e){
		EzLog::ex(ftag, e);

		return -1;
	}
	catch (...){
		EzLog::e(ftag, "捕获异常");

		return -1;
	}

	return 0;
}

// 取字段内容
// @param message fix消息
// @param field 字段key
// @param strValue 内容
int StgwFixUtil::GetStringField(const FIX::Message& message,
	int field, std::string& strValue)
{
	static const std::string ftag("StgwFixUtil::GetStringField() ");

	try
	{
		if (message.isHeaderField(field))
		{
			strValue = message.getHeader().getField(field);
		}
		else if (message.isTrailerField(field))
		{
			strValue = message.getTrailer().getField(field);
		}
		else
		{
			strValue = message.getField(field);
		}
	}
	catch (std::exception& e)
	{
		// EzLog::ex(ftag, e);

		return -1;
	}
	catch (...)
	{
		return -1;
	}

	return 0;
}

// set field
// @param fixReport fix回报
// @param field 字段key
// @param strValue 内容
int StgwFixUtil::SetField(FIX::Message& fixReport,
	int field, const std::string& strValue)
{
	if (fixReport.isHeaderField(field))
	{
		fixReport.getHeader().setField(field, strValue);
	}
	else if (fixReport.isTrailerField(field))
	{
		fixReport.getTrailer().setField(field, strValue);
	}
	else
	{
		fixReport.setField(field, strValue);
	}

	return 0;
}

// set field
int StgwFixUtil::SetField(FIX::FieldMap& fixReport,
	int field, const std::string& strValue)
{
	FIX::Message& msg = static_cast<FIX::Message&>(fixReport);
	SetField(msg, field, strValue);

	return 0;
}
