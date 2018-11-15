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
TGW自用String工具类
*/
namespace Tgw_StringUtil
{
	using namespace std;

	//
	// Functions
	//

	/*
	安全的将String转换为Int
	Return:
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int String2Int_atoi(const string& in_strInt, int& out_int);


	/*
	安全的将String转换为Long
	Return:
	0 -- 转换成功
	非0 -- 转换失败
	*/
	long String2Long_atol(const string& in_strInt, long& out_long);

	/*
	安全的将String(正数，有可能带小数)转换为uint64_t的金额(单位为厘 -- 分的十分之一)
	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int String2UInt64MoneyInLi_strtoui64(const string& in_strInt, uint64_t& out_ui64Int);

	/*
	安全的将String(正数，有可能带小数)转换为uint64_t的正整数(不含小数点)
	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int String2UInt64_strtoui64(const string& in_strInt, uint64_t& out_ui64Int);

	/*
	安全的将String(整数，有可能带小数)转换为INT64的整数(不含小数点)
	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int String2Int64_atoi64(const string& in_strInt, int64_t& out_i64Int);

	/*
	安全的将String转换为Double
	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int String2Double_atof(const string& in_strInt, double& out_double);

	// 循环替换字符串中旧字符串为新值
	string& replace_all(string& str, const string& old_value, const string& new_value);

	/*
	将以厘(分的十分之一)为单位的数值转为以元为单位的带小数点字符串
	Params :
	uint64_t ui64MoneyLi :
	以厘(分的十分之一)为单位的数值.

	string& out_strLi :
	以元为单位的带小数点字符串.

	unsigned int uiDecimalLen :
	小数点后数字长度.

	Return :
	string& out_strLi :
	以元为单位的带小数点字符串.
	*/
	string& iLiToStr(uint64_t ui64MoneyLi, string& out_strLi, unsigned int uiDecimalLen);

	/*
	将以厘(分的十分之一)为单位的字符串转为以元为单位的double值
	Params :

	string& out_strLi :
	以厘(分的十分之一)为单位的字符串.

	double &io_dValue
	以元为单位的double值

	unsigned int uiDecimalLen :
	小数点后数字长度.

	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int StringLiToDouble(const string& in_strLi, double &io_dValue, unsigned int in_uiDecimalLen);

	/*
	将浮点字值转为字符串值
	Params :

	const double &in_dValue :
	double值

	string& out_strValue :
	字符串.

	unsigned int uiDecimalLen :
	小数点后数字长度.

	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int DoubleToString(const double &in_dValue, string& out_strValue, unsigned int uiDecimalLen);

	/*
	将以厘(分的十分之一)为单位的字符串转为以元为单位的字符串
	Params :

	string& out_strLi :
	以厘(分的十分之一)为单位的字符串.

	string &io_strValue
	以元为单位的带小数点字符串

	unsigned int uiDecimalLen :
	小数点后数字长度.

	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int StringLiToStringYuan(const string& in_strLi, string &io_strValue, unsigned int in_uiDecimalLen);

	/*
	获取JSON对象的内容字符串

	@return 0 -- 成功
	@return -1 -- 失败
	*/
	int GetJsonString(rapidjson::Value& in_jsObj, std::string& out_strJsContent);
}

#endif