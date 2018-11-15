#include "tool_json/RapidJsonHelper_tgw.h"

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"

#include "util/EzLog.h"

RapidJsonHelper_tgw::RapidJsonHelper_tgw( void )
{
}

RapidJsonHelper_tgw::~RapidJsonHelper_tgw( void )
{
}

/*
尝试由rapidjson::Value取得object value
Return :
0 -- 获取成功
1 -- 成员不存在，获取失败
-1 -- 其他的获取异常失败
*/
int RapidJsonHelper_tgw::GetObjectValue( rapidjson::Document& jsonValue,
	const string& strKey, rapidjson::Value& out_jsValue )
{
	static const string ftag( "RapidJsonHelper_tgw::GetObjectFromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			//EzLog::e( ftag, "解析错误[" + strKey + "]" );
			return 1;
		}

		out_jsValue = jsonValue[strKey.c_str()];


		return 0;
	}
	catch ( exception& e )
	{
		EzLog::ex( ftag, e );

		return -1;
	}


	return 0;
}

/*
尝试由rapidjson::Value取得string
Return :
0 -- 获取成功
1 -- 成员不存在，获取失败
-1 -- 其他的获取异常失败
*/
int RapidJsonHelper_tgw::GetStringFromJsonValue( rapidjson::Document& jsonValue,
	const string& strKey, string& out_strValue )
{
	static const string ftag( "RapidJsonHelper_tgw::GetStringFromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			//EzLog::e( ftag, "解析错误[" + strKey + "]" );
			return 1;
		}

		rapidjson::Value &jsValue = jsonValue[strKey.c_str()];

		if ( jsValue.IsArray() )
		{
			rapidjson::StringBuffer sbBuf;
			rapidjson::Writer<rapidjson::StringBuffer> jWriter( sbBuf );
			jsValue.Accept( jWriter );

			out_strValue = sbBuf.GetString();
		}
		else
		{
			out_strValue = jsValue.GetString();
		}

		return 0;
	}
	catch ( exception& e )
	{
		EzLog::ex( ftag, e );

		return -1;
	}


	return 0;
}

/*
尝试由rapidjson::Value取得其字符串值
Return :
0 -- 成功
*/
int RapidJsonHelper_tgw::GetString(rapidjson::Value& jsonValue, std::string& out_strValue)
{
	static const string ftag("RapidJsonHelper_tgw::GetString()");

	try
	{
		if (jsonValue.IsNull())
		{
			out_strValue = "IsNull()";
			return 0;
		}

		rapidjson::StringBuffer sbBuf;
		rapidjson::Writer<rapidjson::StringBuffer> jWriter(sbBuf);
		jsonValue.Accept(jWriter);

		out_strValue = sbBuf.GetString();

		return 0;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

// 由rapidjson::Value取得int
int RapidJsonHelper_tgw::GetIntFromJsonValue( const rapidjson::Document& jsonValue,
	const string& strKey, int& out_iValue )
{
	static const string ftag( "RapidJsonHelper_tgw::GetIntFromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			EzLog::e( ftag, "解析错误[" + strKey + "];源-" );
			return -2;
		}

		string strValue = jsonValue[strKey.c_str()].GetString();
		int iRes = Tgw_StringUtil::String2Int_atoi( strValue, out_iValue );

		return iRes;
	}
	catch ( exception& e )
	{
		EzLog::ex( ftag, e );

		return -1;
	}
}

// 由rapidjson::Value取得long
int RapidJsonHelper_tgw::GetLongFromJsonValue( const rapidjson::Document& jsonValue,
	const string& strKey, long& out_lValue )
{
	static const string ftag( "RapidJsonHelper_tgw::GetLongFromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			EzLog::e( ftag, "解析错误[" + strKey + "];源-" );
			return -2;
		}

		string strValue = jsonValue[strKey.c_str()].GetString();
		int iRes = Tgw_StringUtil::String2Long_atol( strValue, out_lValue );

		return iRes;
	}
	catch ( exception& e )
	{
		EzLog::ex( ftag, e );

		return -1;
	}
}

// 由rapidjson::Value取得UINT64Money(单位为厘)
int RapidJsonHelper_tgw::GetUINT64MoneyFromJsonValue( const rapidjson::Document& jsonValue,
	const string& strKey, uint64_t& out_ui64Value )
{
	static const string ftag( "RapidJsonHelper_tgw::GetUINT64MoneyFromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			EzLog::e( ftag, "解析错误[" + strKey + "];源-" + jsonValue.GetString() );
			return -2;
		}

		string strValue = jsonValue[strKey.c_str()].GetString();
		uint64_t ui64Money = 0;
		int iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64( strValue, ui64Money );

		if ( 0 != iRes )
		{
			string strDebug( "转换" );
			strDebug += strValue;
			strDebug += "为uint64_t失败";
			EzLog::e(ftag, strDebug);

			return -2;
		}

		out_ui64Value = ui64Money;

		return 0;
	}
	catch ( exception& e )
	{
		EzLog::e( ftag, "解析错误[" + strKey + "];源-" + jsonValue.GetString() );
		EzLog::ex( ftag, e );

		return -1;
	}
}

// 由rapidjson::Value取得uint64_t
int RapidJsonHelper_tgw::GetUINT64FromJsonValue( const rapidjson::Document& jsonValue,
	const string& strKey, uint64_t& out_ui64Value )
{
	static const string ftag( "RapidJsonHelper_tgw::GetUINT64FromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			EzLog::e( ftag, "解析错误[" + strKey + "];源-" + jsonValue.GetString() );
			return -2;
		}

		uint64_t ui64Money = 0;

		if ( jsonValue[strKey.c_str()].IsString() )
		{
			string strValue = jsonValue[strKey.c_str()].GetString();

			int iRes = Tgw_StringUtil::String2UInt64_strtoui64( strValue, ui64Money );

			if ( 0 != iRes )
			{
				string strDebug( "转换" );
				strDebug += strValue;
				strDebug += "为uint64_t失败";
				EzLog::e( ftag, strDebug );

				return -2;
			}
		}
		else if ( jsonValue[strKey.c_str()].IsInt() || jsonValue[strKey.c_str()].IsInt64() )
		{
			ui64Money = jsonValue[strKey.c_str()].GetUint64();
		}
		else
		{
			// 不是string 或 int
			return -1;
		}

		out_ui64Value = ui64Money;

		return 0;
	}
	catch ( exception& e )
	{
		EzLog::e( ftag, "解析错误[" + strKey + "];源-" + jsonValue.GetString() );
		EzLog::ex( ftag, e );

		return -1;
	}
}

// 由rapidjson::Value取得double
int RapidJsonHelper_tgw::GetDoubleFromJsonValue( const rapidjson::Document& jsonValue,
	const string& strKey, double& out_dbValue )
{
	static const string ftag( "RapidJsonHelper_tgw::GetStringFromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			EzLog::e( ftag, "解析错误[" + strKey + "];源-" + jsonValue.GetString() );
			return -2;
		}

		string strValue = jsonValue[strKey.c_str()].GetString();
		int iRes = Tgw_StringUtil::String2Double_atof( strValue, out_dbValue );

		return iRes;
	}
	catch ( exception& e )
	{
		EzLog::ex( ftag, e );

		return -1;
	}
}
