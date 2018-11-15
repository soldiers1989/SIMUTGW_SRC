#include "StgwFixUtil.h"
#include "util/EzLog.h"

// ȡnopartyids�ظ�������ݣ������˺š�ϯλ��Ӫҵ������
// @param message fix��Ϣ
// @param out_strSeat ȡ�õ�ϯλ
// @param out_strAccount ȡ�õ��˺�
// @param out_strMarket_branchid ȡ�õ�Ӫҵ������
// @return
// 0 -- �ɹ�
// -1 -- ʧ��
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
			// û���ظ���
			return -1;
		}

		// �ظ���Ԫ�ظ���
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
				//�õ�֤ȯ�˺�
				out_strAccount = strPartyid;
			}
			else if ((strPartyidsource == "C") && (strPartyRole == "1"))
			{
				//�õ�֤ȯϯλ
				out_strSeat = strPartyid;
			}
			else if ((strPartyidsource == "D") && (strPartyRole == "4001"))
			{
				//�õ�Ӫҵ������
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
		EzLog::e(ftag, "�����쳣");

		return -1;
	}

	return 0;
}

// ȡ�ֶ�����
// @param message fix��Ϣ
// @param field �ֶ�key
// @param strValue ����
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
// @param fixReport fix�ر�
// @param field �ֶ�key
// @param strValue ����
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
