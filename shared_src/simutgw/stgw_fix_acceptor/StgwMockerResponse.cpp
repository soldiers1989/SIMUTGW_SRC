#include "StgwMockerResponse.h"
#include "simutgw/stgw_fix_acceptor/StgwFixUtil.h"

#include "tool_string/TimeStringUtil.h"

#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/stgw_config/g_values_biz.h"

// �����еĻر�name
const std::string StgwMockerResponse::strResponseKeyName("response");

StgwMockerResponse::StgwMockerResponse()
{
}


StgwMockerResponse::~StgwMockerResponse()
{
}

/*
������Ϣ����ر�
@param message �յ���ί����Ϣ
@return
0 -- �ɹ�
-1 -- ʧ��
*/
int StgwMockerResponse::Response(const FIX::Message& message)
{
	static const std::string ftag("StgwMockerResponse::Response() ");

	int iRes = 0;
	std::string strApplId;
	iRes = StgwFixUtil::GetStringField(message, FIX::FIELD::ApplID, strApplId);
	if (0 != iRes)
	{
		EzLog::e(ftag, "��Ϣȱ��ApplID�ֶΣ��޷�����Ӧ�Ļر�����");
		return -1;
	}

	std::string strResponse;
	// �Ȳ���û�и�applid��Ӧ��json�ر�
	iRes = simutgw::g_mockerResponses.GetResponse(strApplId, strResponse);
	if (0 != iRes)
	{
		// û�и�����
		std::string strError(strApplId);
		strError += ".json������";
		EzLog::e(ftag, strError);
		return -1;
	}

	// �ҵ���Ӧ�Ļر�json value
	// �Ƚ���json����
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
	if ((doc.Parse<0>(strResponse.c_str())).HasParseError() || !doc.IsArray())
	{
		//  ����ʧ��
		std::string strError(strApplId);
		strError += ".json���Ǹ�ʽ����ȷ";
		EzLog::e(ftag, strError);
		return -1;
	}

	std::string strKey, strValue, strMsgValue;
	int iField = 0;

	// �ҵ�ƥ����ͬ��json value
	rapidjson::Document::ValueIterator iterValue = doc.Begin();
	for (; doc.End() != iterValue; ++iterValue)
	{
		bool bFound = true;
		rapidjson::Document::MemberIterator iterMember = iterValue->MemberBegin();
		for (; iterMember != iterValue->MemberEnd(); ++iterMember)
		{
			strKey = iterMember->name.GetString();
			if (0 == strKey.compare(strResponseKeyName))
			{
				// ��response����
				continue;
			}

			if (!iterMember->value.IsString())
			{
				// value����string����ʽ������
				std::string strError("�ر�ɸѡ�ֶ�[");
				strError += strKey;
				strError += "]��ʽ����ȷ���������������ļ���=";
				strError += strApplId;
				strError += ".json]";

				EzLog::e(ftag, strError);
				continue;
			}
			strValue = iterMember->value.GetString();

			// strKeyת��ΪField
			Tgw_StringUtil::String2Int_atoi(strKey, iField);

			if (0 == StgwFixUtil::GetStringField(message, iField, strMsgValue))
			{
				// ȡֵ�ɹ�
				if (0 != strValue.compare(strMsgValue))
				{
					// �����������
					bFound = false;
					break;
				}
			}
			else
			{
				// ȡֵʧ��
				std::string strError("�ر�ɸѡ�ֶ�[");
				strError += strKey;
				strError += "]����ί����Ϣ�У��������������ļ���=";
				strError += strApplId;
				strError += ".json]";

				EzLog::e(ftag, strError);
				continue;
			}
		}

		if (bFound)
		{
			break;
		}
	}

	if (doc.End() != iterValue)
	{
		return ParseConfigResponse(message, *iterValue);
	}

	return -1;
}

/*
����json���ò��ػر�
@param msessage �յ���ί����Ϣ
@param resValue json��ʽ�Ļر�����
@return
0 -- �ɹ�
-1 -- ʧ��
*/
int StgwMockerResponse::ParseConfigResponse(const FIX::Message& message, rapidjson::Value& resValue)
{
	static const std::string ftag("StgwMockerResponse::ParseConfigResponse() ");

	std::string strApplId;
	StgwFixUtil::GetStringField(message, FIX::FIELD::ApplID, strApplId);
	rapidjson::Document::MemberIterator iterMember = resValue.FindMember(strResponseKeyName.c_str());
	if (resValue.MemberEnd() == iterMember)
	{
		// ��response
		std::string strError("�ļ�[");
		strError += strApplId;
		strError += ".json]response�ֶ�ȱʧ";
		EzLog::e(ftag, strError);
		return -1;
	}

	// response��value��һ��array
	if (!(iterMember->value).IsArray())
	{
		std::string strError("�ļ�[");
		strError += strApplId;
		strError += ".json]response ����Array";
		EzLog::e(ftag, strError);
		return -1;
	}

	rapidjson::Value &rspArray = iterMember->value;
	rapidjson::Document::ValueIterator iterArray = rspArray.Begin();
	for (; rspArray.End() != iterArray; ++iterArray)
	{
		ReplaceConfig_SendResponse(message, *iterArray);
	}

	return 0;
}

/*
�滻json���ûػر�
@param msessage �յ���ί����Ϣ
@param resValue json��ʽ�Ļر�����
@return
0 -- �ɹ�
-1 -- ʧ��
*/
int StgwMockerResponse::ReplaceConfig_SendResponse(const FIX::Message& message, rapidjson::Value& resValue)
{
	static const std::string ftag("StgwMockerResponse::ReplaceConfig_SendResponse() ");

	try
	{
		FIX::Message fixReport(message);
	
		// �Ƚ���senderID��targetID
		SetField(fixReport, FIX::FIELD::TargetCompID, message.getHeader().getField(FIX::FIELD::SenderCompID));
		SetField(fixReport, FIX::FIELD::SenderCompID, message.getHeader().getField(FIX::FIELD::TargetCompID));

		// �����reportindex
		string strValue = message.getHeader().getField(FIX::FIELD::SenderCompID);
		uint64_t ui64Num = 0;
		if (simutgw::g_mapSzConns.end() != simutgw::g_mapSzConns.find(strValue))
		{
			ui64Num = simutgw::g_mapSzConns[strValue].GetRptIdex();
		}
		sof_string::itostr(ui64Num, strValue);
		SetField(fixReport, FIX::FIELD::ReportIndex, strValue);

		// ϯλ �˺�
		std::string strSeat, strAccount;
		GetSeat_Account(message, strSeat, strAccount);

		// ��ɻر�
		ReplaceConfig(message, fixReport, resValue, strSeat, strAccount);

		simutgw::g_fixaccptor.SendMsg(fixReport);
	}
	catch (std::exception& e)
	{
		EzLog::ex(ftag, e);
	}
	catch (...)
	{
		EzLog::e(ftag, "�����쳣");
	}

	return 0;
}

/*
�滻json����
@param msessage �յ���ί����Ϣ
@param fixReport �ر���Ϣ
@param resValue json��ʽ�Ļر�����
@return
0 -- �ɹ�
-1 -- ʧ��
*/
int StgwMockerResponse::ReplaceConfig(const FIX::Message& message,
	FIX::FieldMap& fixReport,
	rapidjson::Value& resValue,
	const std::string& strSeat,
	const std::string& strAccount)
{
	static const std::string ftag("StgwMockerResponse::ReplaceConfig() ");
	static const std::string strUniqueTag("$UNIQUE");
	static const std::string strSeatTag("$SEAT");
	static const std::string strAccountTag("$ACCOUNT");

	// �滻����Ԫ��
	rapidjson::Document::MemberIterator iter = resValue.MemberBegin();
	int iField = 0;
	std::string strField, strValue;
	for (; iter != resValue.MemberEnd(); ++iter)
	{
		strField.clear();
		strValue.clear();

		// ת��Ϊ����
		strField = iter->name.GetString();
		Tgw_StringUtil::String2Int_atoi(strField, iField);

		if (iter->value.IsString())
		{
			strValue = iter->value.GetString();
			size_t iLength = strValue.length();

			if (iLength == 0)
			{
				continue;
			}
			// �ж��Ƿ��������ַ�
			if (strValue[0] == '$')
			{
				if (0 == strValue.compare(strUniqueTag))
				{
					// ����Ψһ
					TimeStringUtil::ExRandom15(strValue);
					SetField(fixReport, iField, strValue);
				}
				else if (0 == strValue.compare(strSeatTag) )
				{
					// ϯλ
					SetField(fixReport, iField, strSeat);
				}
				else if (0 == strValue.compare(strAccountTag))
				{
					// �˺�
					SetField(fixReport, iField, strAccount);
				}
				else if (iLength >= 3 && 0 == strValue.substr(0, 3).compare("$ID"))
				{
					// ��ί����Ϣ���ֶ� ��$ID11����ʾ11�ֶ�
					int iOrderField = 0;
					Tgw_StringUtil::String2Int_atoi(strValue.substr(3, strValue.length()-3), iOrderField);
					std::string strOrderValue = message.getField(iOrderField);

					SetField(fixReport, iField, strOrderValue);
				}
				else
				{
					// ��ʽ����
					std::string strError("value��ʽ����[");
					strError += strValue;
					EzLog::e(ftag, strError);

					return -1;
				}
			}
			else
			{
				// �������ַ�
				SetField(fixReport, iField, strValue);
			}
		}
		else if (iter->value.IsArray())
		{
			rapidjson::Document::ValueIterator iterValue = iter->value.Begin();
			for (; iterValue != iter->value.End(); ++iterValue)
			{
				// ����Array��Value
				rapidjson::Document::MemberIterator iterMember = iterValue->MemberBegin();
				std::string strFieldSub = iterMember->name.GetString();
				int iFieldSub = 0;
				Tgw_StringUtil::String2Int_atoi(strFieldSub, iFieldSub);
				
				FIX::Group group(iField, iFieldSub);
				ReplaceConfig(message, group, *iterValue, strSeat, strAccount);

				fixReport.addGroup(iField, group);
			}
		}
		else
		{

		}
	}

	return 0;
}

// set field
int StgwMockerResponse::SetField(FIX::FieldMap& fixReport,
	int field, const std::string& strValue)
{
	FIX::Message& msg = static_cast<FIX::Message&>(fixReport);
	StgwFixUtil::SetField(msg, field, strValue);

	return 0;
}

/*
ȡ��ϯλ���˺š�Ӫҵ������
@param msessage �յ���ί����Ϣ
@param strSeat ϯλ
@param strAccount �˺�
*/
int StgwMockerResponse::GetSeat_Account(const FIX::Message& message,
	std::string& strSeat,
	std::string& strAccount)
{
	static const std::string ftag("StgwMockerResponse::GetNopartyIds() ");

	try
	{
		FIX::FieldMap& msg = (FIX::FieldMap&)message;
		
		std::string strMarket_branchId;
		return StgwFixUtil::GetNopartyIds(msg, strSeat, strAccount, strMarket_branchId);
	}
	catch (exception& e){
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}