#include "tool_json/JsoncppTools_tgw.h"

#include "util/EzLog.h"

/*
JsoncppTools_tgw::JsoncppTools_tgw(void)
{
}

JsoncppTools_tgw::~JsoncppTools_tgw(void)
{
}
*/

// 由Json::Value取得Json::Value
int JsoncppTools_tgw::GetJsonValueFromJsonValue( const Json::Value& jsonValue,
												const string& strKey, Json::Value& out_jsvValue )
{
	static const string ftag("JsoncppTools_tgw::GetJsonValueFromJsonValue()");

	try
	{
		bool bRes = jsonValue.isMember(strKey.c_str());

		if( !bRes )
		{
			EzLog::e( ftag, "解析错误[" + strKey + "];源-" );
			return -2;
		}		

		Json::Value jsValue = jsonValue[strKey.c_str()];

		out_jsvValue = jsValue;

		return 0;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
}

// 由Json::Value取得string
int JsoncppTools_tgw::GetStringFromJsonValue( const Json::Value& jsonValue,
											 const string& strKey, string& out_strValue )
{
	static const string ftag("JsoncppTools_tgw::GetStringFromJsonValue()");

	try
	{
		bool bRes = jsonValue.isMember(strKey.c_str());

		if( !bRes )
		{
			EzLog::e( ftag, "解析错误[" + strKey + "];源-" );
			return -2;
		}		

		Json::Value jsValue = jsonValue[strKey.c_str()];

		if( jsValue.isArray() )
		{
			out_strValue = jsValue.toStyledString();
		}
		else
		{

			out_strValue = jsValue.asString();
		}

		return 0;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

// 由Json::Value取得long
int JsoncppTools_tgw::GetLongFromJsonValue( const Json::Value& jsonValue,
										   const string& strKey, long& out_lValue )
{
	static const string ftag("JsoncppTools_tgw::GetLongFromJsonValue()");

	try
	{
		bool bRes = jsonValue.isMember(strKey.c_str());

		if( !bRes )
		{
			EzLog::e( ftag, "解析错误[" + strKey + "];源-" );
			return -2;
		}

		string strValue = jsonValue[strKey.c_str()].asString();
		int iRes = Tgw_StringUtil::String2Long_atol( strValue, out_lValue );

		return iRes;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
}

// 由Json::Value取得UINT64Money(单位为厘)
int JsoncppTools_tgw::GetUINT64MoneyFromJsonValue( const Json::Value& jsonValue,
												  const string& strKey, uint64_t& out_ui64Value )
{
	static const string ftag("JsoncppTools_tgw::GetUINT64MoneyFromJsonValue()");

	try
	{
		bool bRes = jsonValue.isMember(strKey.c_str());

		if( !bRes )
		{
			EzLog::e( ftag, "解析错误[" + strKey + "];源-" + jsonValue.asString() );
			return -2;
		}

		string strValue = jsonValue[strKey.c_str()].asString();
		uint64_t ui64Money = 0;
		int iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64( strValue, ui64Money );

		if( 0 != iRes )
		{
			string strDebug("转换");
			strDebug += strValue;
			strDebug += "为uint64_t失败";
			EzLog::e(ftag, strDebug);

			return -2;
		}

		out_ui64Value = ui64Money;

		return 0;
	}
	catch (exception& e)
	{		
		EzLog::e( ftag, "解析错误[" + strKey + "];源-" + jsonValue.asString() );
		EzLog::ex(ftag, e);

		return -1;
	}
}

// 由Json::Value取得uint64_t
int JsoncppTools_tgw::GetUINT64FromJsonValue( const Json::Value& jsonValue,
											 const string& strKey, uint64_t& out_ui64Value )
{
	static const string ftag("JsoncppTools_tgw::GetUINT64FromJsonValue()");

	try
	{
		bool bRes = jsonValue.isMember(strKey.c_str());

		if( !bRes )
		{
			EzLog::e( ftag, "解析错误[" + strKey + "];源-" + jsonValue.asString() );
			return -2;
		}

		string strValue = jsonValue[strKey.c_str()].asString();
		uint64_t ui64Money = 0;
		int iRes = Tgw_StringUtil::String2UInt64_strtoui64( strValue, ui64Money );

		if( 0 != iRes )
		{
			string strDebug("转换");
			strDebug += strValue;
			strDebug += "为uint64_t失败";
			EzLog::e(ftag, strDebug);

			return -2;
		}

		out_ui64Value = ui64Money;

		return 0;
	}
	catch (exception& e)
	{		
		EzLog::e( ftag, "解析错误[" + strKey + "];源-" + jsonValue.asString() );
		EzLog::ex(ftag, e);

		return -1;
	}
}

// 由Json::Value取得double
int JsoncppTools_tgw::GetDoubleFromJsonValue( const Json::Value& jsonValue,
											 const string& strKey, double& out_dbValue )
{
	static const string ftag("GetStringFromJsonValue()");

	try
	{
		bool bRes = jsonValue.isMember(strKey.c_str());

		if( !bRes )
		{
			EzLog::e( ftag, "解析错误[" + strKey + "];源-" + jsonValue.asString() );
			return -2;
		}

		string strValue = jsonValue[strKey.c_str()].asString();
		int iRes = Tgw_StringUtil::String2Double_atof( strValue, out_dbValue );

		return iRes;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
}
