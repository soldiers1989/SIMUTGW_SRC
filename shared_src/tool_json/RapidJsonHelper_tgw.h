#ifndef __RAPIDJSON_HELPER_TGW_H__
#define __RAPIDJSON_HELPER_TGW_H__

#include "rapidjson/reader.h"
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

#include <string>
#include <stdint.h>

class RapidJsonHelper_tgw
{

	//
	// Members
	//

	//
	// Functions
	//
public:
	/*
	尝试由rapidjson::Value取得object value
	Return :
	0 -- 获取成功
	1 -- 成员不存在，获取失败
	-1 -- 其他的获取异常失败
	*/
	static int GetObjectValue(rapidjson::Document& jsonValue,
		const std::string& strKey, rapidjson::Value& out_jsValue);

	// 由rapidjson::Value取得rapidjson::Value
	static int GetJsonValueFromJsonValue(rapidjson::Document& jsonValue,
		const std::string& strKey, rapidjson::Value& out_jsvValue);

	/*
	由rapidjson::Value取得string
	Return :
	0 -- 获取成功
	1 -- 成员不存在，获取失败
	-1 -- 其他的获取异常失败
	*/
	static int GetStringFromJsonValue(rapidjson::Document& jsonValue,
		const std::string& strKey, std::string& out_strValue);

	/*
	尝试由rapidjson::Value取得其字符串值
	Return :
	0 -- 成功
	*/
	static int GetString(rapidjson::Value& jsonValue, std::string& out_strValue);

	// 由rapidjson::Value取得int
	static int GetIntFromJsonValue(const rapidjson::Document& jsonValue,
		const std::string& strKey, int& out_iValue);

	// 由rapidjson::Value取得long
	static int GetLongFromJsonValue(const rapidjson::Document& jsonValue,
		const std::string& strKey, long& out_lValue);

	// 由rapidjson::Value取得UINT64Money(单位为厘)
	static int GetUINT64MoneyFromJsonValue(const rapidjson::Document& jsonValue,
		const std::string& strKey, uint64_t& out_ui64Value);

	// 由rapidjson::Value取得uint64_t
	static int GetUINT64FromJsonValue(const rapidjson::Document& jsonValue,
		const std::string& strKey, uint64_t& out_ui64Value);

	// 由rapidjson::Value取得double
	static int GetDoubleFromJsonValue(const rapidjson::Document& jsonValue,
		const std::string& strKey, double& out_dbValue);

private:
	// 禁止实例产生
	RapidJsonHelper_tgw(void);
	virtual ~RapidJsonHelper_tgw(void);
};

#endif