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
������rapidjson::Valueȡ��object value
Return :
0 -- ��ȡ�ɹ�
1 -- ��Ա�����ڣ���ȡʧ��
-1 -- �����Ļ�ȡ�쳣ʧ��
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
			//EzLog::e( ftag, "��������[" + strKey + "]" );
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
������rapidjson::Valueȡ��string
Return :
0 -- ��ȡ�ɹ�
1 -- ��Ա�����ڣ���ȡʧ��
-1 -- �����Ļ�ȡ�쳣ʧ��
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
			//EzLog::e( ftag, "��������[" + strKey + "]" );
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
������rapidjson::Valueȡ�����ַ���ֵ
Return :
0 -- �ɹ�
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

// ��rapidjson::Valueȡ��int
int RapidJsonHelper_tgw::GetIntFromJsonValue( const rapidjson::Document& jsonValue,
	const string& strKey, int& out_iValue )
{
	static const string ftag( "RapidJsonHelper_tgw::GetIntFromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			EzLog::e( ftag, "��������[" + strKey + "];Դ-" );
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

// ��rapidjson::Valueȡ��long
int RapidJsonHelper_tgw::GetLongFromJsonValue( const rapidjson::Document& jsonValue,
	const string& strKey, long& out_lValue )
{
	static const string ftag( "RapidJsonHelper_tgw::GetLongFromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			EzLog::e( ftag, "��������[" + strKey + "];Դ-" );
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

// ��rapidjson::Valueȡ��UINT64Money(��λΪ��)
int RapidJsonHelper_tgw::GetUINT64MoneyFromJsonValue( const rapidjson::Document& jsonValue,
	const string& strKey, uint64_t& out_ui64Value )
{
	static const string ftag( "RapidJsonHelper_tgw::GetUINT64MoneyFromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			EzLog::e( ftag, "��������[" + strKey + "];Դ-" + jsonValue.GetString() );
			return -2;
		}

		string strValue = jsonValue[strKey.c_str()].GetString();
		uint64_t ui64Money = 0;
		int iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64( strValue, ui64Money );

		if ( 0 != iRes )
		{
			string strDebug( "ת��" );
			strDebug += strValue;
			strDebug += "Ϊuint64_tʧ��";
			EzLog::e(ftag, strDebug);

			return -2;
		}

		out_ui64Value = ui64Money;

		return 0;
	}
	catch ( exception& e )
	{
		EzLog::e( ftag, "��������[" + strKey + "];Դ-" + jsonValue.GetString() );
		EzLog::ex( ftag, e );

		return -1;
	}
}

// ��rapidjson::Valueȡ��uint64_t
int RapidJsonHelper_tgw::GetUINT64FromJsonValue( const rapidjson::Document& jsonValue,
	const string& strKey, uint64_t& out_ui64Value )
{
	static const string ftag( "RapidJsonHelper_tgw::GetUINT64FromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			EzLog::e( ftag, "��������[" + strKey + "];Դ-" + jsonValue.GetString() );
			return -2;
		}

		uint64_t ui64Money = 0;

		if ( jsonValue[strKey.c_str()].IsString() )
		{
			string strValue = jsonValue[strKey.c_str()].GetString();

			int iRes = Tgw_StringUtil::String2UInt64_strtoui64( strValue, ui64Money );

			if ( 0 != iRes )
			{
				string strDebug( "ת��" );
				strDebug += strValue;
				strDebug += "Ϊuint64_tʧ��";
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
			// ����string �� int
			return -1;
		}

		out_ui64Value = ui64Money;

		return 0;
	}
	catch ( exception& e )
	{
		EzLog::e( ftag, "��������[" + strKey + "];Դ-" + jsonValue.GetString() );
		EzLog::ex( ftag, e );

		return -1;
	}
}

// ��rapidjson::Valueȡ��double
int RapidJsonHelper_tgw::GetDoubleFromJsonValue( const rapidjson::Document& jsonValue,
	const string& strKey, double& out_dbValue )
{
	static const string ftag( "RapidJsonHelper_tgw::GetStringFromJsonValue()" );

	try
	{
		bool bRes = jsonValue.HasMember( strKey.c_str() );

		if ( !bRes )
		{
			EzLog::e( ftag, "��������[" + strKey + "];Դ-" + jsonValue.GetString() );
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
