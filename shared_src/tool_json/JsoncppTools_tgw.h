#ifndef __JSON_TOOLS_TGW_H__
#define __JSON_TOOLS_TGW_H__

#include "json/json.h"
#include "tool_string/Tgw_StringUtil.h"

using namespace std;

class JsoncppTools_tgw
{
	//
	// Members
	//

	//
	// Functions
	//

public:
	//JsonTools_tgw(void);
	//virtual ~JsonTools_tgw(void);


	// 由Json::Value取得Json::Value
	static int GetJsonValueFromJsonValue(const Json::Value& jsonValue,
		const string& strKey, Json::Value& out_jsvValue);

	// 由Json::Value取得string
	static int GetStringFromJsonValue(const Json::Value& jsonValue,
		const string& strKey, string& out_strValue);

	// 由Json::Value取得long
	static int GetLongFromJsonValue(const Json::Value& jsonValue,
		const string& strKey, long& out_lValue);

	// 由Json::Value取得UINT64Money(单位为厘)
	static int GetUINT64MoneyFromJsonValue(const Json::Value& jsonValue,
		const string& strKey, uint64_t& out_ui64Value);

	// 由Json::Value取得uint64_t
	static int GetUINT64FromJsonValue(const Json::Value& jsonValue,
		const string& strKey, uint64_t& out_ui64Value);

	// 由Json::Value取得double
	static int GetDoubleFromJsonValue(const Json::Value& jsonValue,
		const string& strKey, double& out_dbValue);

};

#endif