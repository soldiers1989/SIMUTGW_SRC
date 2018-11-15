#include "StgwMockerResponse.h"
#include "simutgw/stgw_fix_acceptor/StgwFixUtil.h"

#include "tool_string/TimeStringUtil.h"

#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/stgw_config/g_values_biz.h"

// 配置中的回报name
const std::string StgwMockerResponse::strResponseKeyName("response");

StgwMockerResponse::StgwMockerResponse()
{
}


StgwMockerResponse::~StgwMockerResponse()
{
}

/*
根据消息处理回报
@param message 收到的委托消息
@return
0 -- 成功
-1 -- 失败
*/
int StgwMockerResponse::Response(const FIX::Message& message)
{
	static const std::string ftag("StgwMockerResponse::Response() ");

	int iRes = 0;
	std::string strApplId;
	iRes = StgwFixUtil::GetStringField(message, FIX::FIELD::ApplID, strApplId);
	if (0 != iRes)
	{
		EzLog::e(ftag, "消息缺少ApplID字段，无法读相应的回报配置");
		return -1;
	}

	std::string strResponse;
	// 先查有没有该applid对应的json回报
	iRes = simutgw::g_mockerResponses.GetResponse(strApplId, strResponse);
	if (0 != iRes)
	{
		// 没有该配置
		std::string strError(strApplId);
		strError += ".json不存在";
		EzLog::e(ftag, strError);
		return -1;
	}

	// 找到对应的回报json value
	// 先解析json报文
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
	if ((doc.Parse<0>(strResponse.c_str())).HasParseError() || !doc.IsArray())
	{
		//  解析失败
		std::string strError(strApplId);
		strError += ".json不是格式不正确";
		EzLog::e(ftag, strError);
		return -1;
	}

	std::string strKey, strValue, strMsgValue;
	int iField = 0;

	// 找到匹配相同的json value
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
				// 是response部分
				continue;
			}

			if (!iterMember->value.IsString())
			{
				// value不是string，格式有问题
				std::string strError("回报筛选字段[");
				strError += strKey;
				strError += "]格式不正确，跳过该条件，文件名=";
				strError += strApplId;
				strError += ".json]";

				EzLog::e(ftag, strError);
				continue;
			}
			strValue = iterMember->value.GetString();

			// strKey转化为Field
			Tgw_StringUtil::String2Int_atoi(strKey, iField);

			if (0 == StgwFixUtil::GetStringField(message, iField, strMsgValue))
			{
				// 取值成功
				if (0 != strValue.compare(strMsgValue))
				{
					// 不相等则跳过
					bFound = false;
					break;
				}
			}
			else
			{
				// 取值失败
				std::string strError("回报筛选字段[");
				strError += strKey;
				strError += "]不在委托消息中，跳过该条件，文件名=";
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
解析json配置并回回报
@param msessage 收到的委托消息
@param resValue json格式的回报配置
@return
0 -- 成功
-1 -- 失败
*/
int StgwMockerResponse::ParseConfigResponse(const FIX::Message& message, rapidjson::Value& resValue)
{
	static const std::string ftag("StgwMockerResponse::ParseConfigResponse() ");

	std::string strApplId;
	StgwFixUtil::GetStringField(message, FIX::FIELD::ApplID, strApplId);
	rapidjson::Document::MemberIterator iterMember = resValue.FindMember(strResponseKeyName.c_str());
	if (resValue.MemberEnd() == iterMember)
	{
		// 无response
		std::string strError("文件[");
		strError += strApplId;
		strError += ".json]response字段缺失";
		EzLog::e(ftag, strError);
		return -1;
	}

	// response的value是一个array
	if (!(iterMember->value).IsArray())
	{
		std::string strError("文件[");
		strError += strApplId;
		strError += ".json]response 不是Array";
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
替换json配置回回报
@param msessage 收到的委托消息
@param resValue json格式的回报配置
@return
0 -- 成功
-1 -- 失败
*/
int StgwMockerResponse::ReplaceConfig_SendResponse(const FIX::Message& message, rapidjson::Value& resValue)
{
	static const std::string ftag("StgwMockerResponse::ReplaceConfig_SendResponse() ");

	try
	{
		FIX::Message fixReport(message);
	
		// 先交换senderID和targetID
		SetField(fixReport, FIX::FIELD::TargetCompID, message.getHeader().getField(FIX::FIELD::SenderCompID));
		SetField(fixReport, FIX::FIELD::SenderCompID, message.getHeader().getField(FIX::FIELD::TargetCompID));

		// 再添加reportindex
		string strValue = message.getHeader().getField(FIX::FIELD::SenderCompID);
		uint64_t ui64Num = 0;
		if (simutgw::g_mapSzConns.end() != simutgw::g_mapSzConns.find(strValue))
		{
			ui64Num = simutgw::g_mapSzConns[strValue].GetRptIdex();
		}
		sof_string::itostr(ui64Num, strValue);
		SetField(fixReport, FIX::FIELD::ReportIndex, strValue);

		// 席位 账号
		std::string strSeat, strAccount;
		GetSeat_Account(message, strSeat, strAccount);

		// 组成回报
		ReplaceConfig(message, fixReport, resValue, strSeat, strAccount);

		simutgw::g_fixaccptor.SendMsg(fixReport);
	}
	catch (std::exception& e)
	{
		EzLog::ex(ftag, e);
	}
	catch (...)
	{
		EzLog::e(ftag, "捕获异常");
	}

	return 0;
}

/*
替换json配置
@param msessage 收到的委托消息
@param fixReport 回报消息
@param resValue json格式的回报配置
@return
0 -- 成功
-1 -- 失败
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

	// 替换其他元素
	rapidjson::Document::MemberIterator iter = resValue.MemberBegin();
	int iField = 0;
	std::string strField, strValue;
	for (; iter != resValue.MemberEnd(); ++iter)
	{
		strField.clear();
		strValue.clear();

		// 转换为数字
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
			// 判断是否是特殊字符
			if (strValue[0] == '$')
			{
				if (0 == strValue.compare(strUniqueTag))
				{
					// 数字唯一
					TimeStringUtil::ExRandom15(strValue);
					SetField(fixReport, iField, strValue);
				}
				else if (0 == strValue.compare(strSeatTag) )
				{
					// 席位
					SetField(fixReport, iField, strSeat);
				}
				else if (0 == strValue.compare(strAccountTag))
				{
					// 账号
					SetField(fixReport, iField, strAccount);
				}
				else if (iLength >= 3 && 0 == strValue.substr(0, 3).compare("$ID"))
				{
					// 用委托消息的字段 如$ID11，表示11字段
					int iOrderField = 0;
					Tgw_StringUtil::String2Int_atoi(strValue.substr(3, strValue.length()-3), iOrderField);
					std::string strOrderValue = message.getField(iOrderField);

					SetField(fixReport, iField, strOrderValue);
				}
				else
				{
					// 格式错误
					std::string strError("value格式错误[");
					strError += strValue;
					EzLog::e(ftag, strError);

					return -1;
				}
			}
			else
			{
				// 非特殊字符
				SetField(fixReport, iField, strValue);
			}
		}
		else if (iter->value.IsArray())
		{
			rapidjson::Document::ValueIterator iterValue = iter->value.Begin();
			for (; iterValue != iter->value.End(); ++iterValue)
			{
				// 遍历Array的Value
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
取的席位、账号、营业部代码
@param msessage 收到的委托消息
@param strSeat 席位
@param strAccount 账号
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