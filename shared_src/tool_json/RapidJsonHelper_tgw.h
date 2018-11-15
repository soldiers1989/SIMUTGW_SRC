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
	������rapidjson::Valueȡ��object value
	Return :
	0 -- ��ȡ�ɹ�
	1 -- ��Ա�����ڣ���ȡʧ��
	-1 -- �����Ļ�ȡ�쳣ʧ��
	*/
	static int GetObjectValue(rapidjson::Document& jsonValue,
		const std::string& strKey, rapidjson::Value& out_jsValue);

	// ��rapidjson::Valueȡ��rapidjson::Value
	static int GetJsonValueFromJsonValue(rapidjson::Document& jsonValue,
		const std::string& strKey, rapidjson::Value& out_jsvValue);

	/*
	��rapidjson::Valueȡ��string
	Return :
	0 -- ��ȡ�ɹ�
	1 -- ��Ա�����ڣ���ȡʧ��
	-1 -- �����Ļ�ȡ�쳣ʧ��
	*/
	static int GetStringFromJsonValue(rapidjson::Document& jsonValue,
		const std::string& strKey, std::string& out_strValue);

	/*
	������rapidjson::Valueȡ�����ַ���ֵ
	Return :
	0 -- �ɹ�
	*/
	static int GetString(rapidjson::Value& jsonValue, std::string& out_strValue);

	// ��rapidjson::Valueȡ��int
	static int GetIntFromJsonValue(const rapidjson::Document& jsonValue,
		const std::string& strKey, int& out_iValue);

	// ��rapidjson::Valueȡ��long
	static int GetLongFromJsonValue(const rapidjson::Document& jsonValue,
		const std::string& strKey, long& out_lValue);

	// ��rapidjson::Valueȡ��UINT64Money(��λΪ��)
	static int GetUINT64MoneyFromJsonValue(const rapidjson::Document& jsonValue,
		const std::string& strKey, uint64_t& out_ui64Value);

	// ��rapidjson::Valueȡ��uint64_t
	static int GetUINT64FromJsonValue(const rapidjson::Document& jsonValue,
		const std::string& strKey, uint64_t& out_ui64Value);

	// ��rapidjson::Valueȡ��double
	static int GetDoubleFromJsonValue(const rapidjson::Document& jsonValue,
		const std::string& strKey, double& out_dbValue);

private:
	// ��ֹʵ������
	RapidJsonHelper_tgw(void);
	virtual ~RapidJsonHelper_tgw(void);
};

#endif