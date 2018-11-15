#ifndef __STGW_MOCKER_RESPONSE_H__
#define __STGW_MOCKER_RESPONSE_H__

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "simutgw/stgw_fix_acceptor/StgwApplication.h"

/*
模拟回报类，根据配置json文件发送回报
*/
class StgwMockerResponse
{
	//
	// member
	//
private:
	// 配置中的回报name
	static const std::string strResponseKeyName;

	//
	// function
	//
public:
	StgwMockerResponse();
	virtual ~StgwMockerResponse();

	/*
	根据消息处理回报
	@param message 收到的委托消息
	@return
	0 -- 成功
	-1 -- 失败
	*/
	static int Response(const FIX::Message& message);

private:
	/*
	解析json配置并回回报
	@param msessage 收到的委托消息
	@param resValue json格式的回报配置
	@return
	0 -- 成功
	-1 -- 失败
	*/
	static int ParseConfigResponse(const FIX::Message& message, rapidjson::Value& resValue);

	/*
	替换json配置回回报
	@param msessage 收到的委托消息
	@param resValue json格式的回报配置
	@return
	0 -- 成功
	-1 -- 失败
	*/
	static int ReplaceConfig_SendResponse(const FIX::Message& message, rapidjson::Value& resValue);

	/*
	替换json配置
	@param msessage 收到的委托消息
	@param fixReport 回报消息
	@param resValue json格式的回报配置
	@return
	0 -- 成功
	-1 -- 失败
	*/
	static int ReplaceConfig(const FIX::Message& message,
		FIX::FieldMap& fixReport,
		rapidjson::Value& resValue,
		const std::string& strSeat,
		const std::string& strAccount);

	// set field
	static int SetField(FIX::FieldMap& fixReport,
		int field, const std::string& strValue);

	/*
	取的席位、账号、营业部代码
	@param msessage 收到的委托消息
	@param strSeat 席位
	@param strAccount 账号
	*/
	static int GetSeat_Account(const FIX::Message& message,
		std::string& strSeat,
		std::string& strAccount);
};

#endif