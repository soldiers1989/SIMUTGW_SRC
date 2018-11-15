#ifndef __TGW_STRING_UTIL_H__
#define __TGW_STRING_UTIL_H__

#ifdef _MSC_VER
#include <Windows.h>
#else

#endif

#include <time.h>
#include <mutex>
#include <stdint.h>

#include "boost/date_time.hpp"

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

/*
TGW����String������
*/
namespace Tgw_StringUtil
{
	using namespace std;

	//
	// Functions
	//

	/*
	��ȫ�Ľ�Stringת��ΪInt
	Return:
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int String2Int_atoi(const string& in_strInt, int& out_int);


	/*
	��ȫ�Ľ�Stringת��ΪLong
	Return:
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	long String2Long_atol(const string& in_strInt, long& out_long);

	/*
	��ȫ�Ľ�String(�������п��ܴ�С��)ת��Ϊuint64_t�Ľ��(��λΪ�� -- �ֵ�ʮ��֮һ)
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int String2UInt64MoneyInLi_strtoui64(const string& in_strInt, uint64_t& out_ui64Int);

	/*
	��ȫ�Ľ�String(�������п��ܴ�С��)ת��Ϊuint64_t��������(����С����)
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int String2UInt64_strtoui64(const string& in_strInt, uint64_t& out_ui64Int);

	/*
	��ȫ�Ľ�String(�������п��ܴ�С��)ת��ΪINT64������(����С����)
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int String2Int64_atoi64(const string& in_strInt, int64_t& out_i64Int);

	/*
	��ȫ�Ľ�Stringת��ΪDouble
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int String2Double_atof(const string& in_strInt, double& out_double);

	// ѭ���滻�ַ����о��ַ���Ϊ��ֵ
	string& replace_all(string& str, const string& old_value, const string& new_value);

	/*
	������(�ֵ�ʮ��֮һ)Ϊ��λ����ֵתΪ��ԪΪ��λ�Ĵ�С�����ַ���
	Params :
	uint64_t ui64MoneyLi :
	����(�ֵ�ʮ��֮һ)Ϊ��λ����ֵ.

	string& out_strLi :
	��ԪΪ��λ�Ĵ�С�����ַ���.

	unsigned int uiDecimalLen :
	С��������ֳ���.

	Return :
	string& out_strLi :
	��ԪΪ��λ�Ĵ�С�����ַ���.
	*/
	string& iLiToStr(uint64_t ui64MoneyLi, string& out_strLi, unsigned int uiDecimalLen);

	/*
	������(�ֵ�ʮ��֮һ)Ϊ��λ���ַ���תΪ��ԪΪ��λ��doubleֵ
	Params :

	string& out_strLi :
	����(�ֵ�ʮ��֮һ)Ϊ��λ���ַ���.

	double &io_dValue
	��ԪΪ��λ��doubleֵ

	unsigned int uiDecimalLen :
	С��������ֳ���.

	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int StringLiToDouble(const string& in_strLi, double &io_dValue, unsigned int in_uiDecimalLen);

	/*
	��������ֵתΪ�ַ���ֵ
	Params :

	const double &in_dValue :
	doubleֵ

	string& out_strValue :
	�ַ���.

	unsigned int uiDecimalLen :
	С��������ֳ���.

	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int DoubleToString(const double &in_dValue, string& out_strValue, unsigned int uiDecimalLen);

	/*
	������(�ֵ�ʮ��֮һ)Ϊ��λ���ַ���תΪ��ԪΪ��λ���ַ���
	Params :

	string& out_strLi :
	����(�ֵ�ʮ��֮һ)Ϊ��λ���ַ���.

	string &io_strValue
	��ԪΪ��λ�Ĵ�С�����ַ���

	unsigned int uiDecimalLen :
	С��������ֳ���.

	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int StringLiToStringYuan(const string& in_strLi, string &io_strValue, unsigned int in_uiDecimalLen);

	/*
	��ȡJSON����������ַ���

	@return 0 -- �ɹ�
	@return -1 -- ʧ��
	*/
	int GetJsonString(rapidjson::Value& in_jsObj, std::string& out_strJsContent);
}

#endif