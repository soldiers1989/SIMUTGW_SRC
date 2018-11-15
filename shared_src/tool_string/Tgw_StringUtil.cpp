#include "Tgw_StringUtil.h"

#ifdef _MSC_VER

#else

#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctime>
#include <math.h>
#include <stdint.h>

#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/algorithm.hpp>
#include "boost/algorithm/string.hpp"
#include "boost/random.hpp"

#include "util/EzLog.h"


using namespace std;

namespace Tgw_StringUtil
{
	/*
	安全的将String转换为Int
	Return:
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int String2Int_atoi(const string& in_strInt, int& out_int)
	{
		static const string ftag("String2Int_atoi()");

		if (in_strInt.empty() || 0 == in_strInt.length())
		{
			out_int = 0;
			return 0;
		}

		try
		{
			out_int = atoi(in_strInt.c_str());
			return 0;
		}
		catch (exception& e)
		{
			EzLog::ex(ftag, e);
			return -1;
		}

	}

	/*
	安全的将String转换为Long
	Return:
	0 -- 转换成功
	非0 -- 转换失败
	*/
	long String2Long_atol(const string& in_strInt, long& out_long)
	{
		static const string ftag("String2Long_atol()");

		if (in_strInt.empty() || 0 == in_strInt.length())
		{
			out_long = 0;
			return 0;
		}


		try
		{
			out_long = atol(in_strInt.c_str());
		}
		catch (exception& e)
		{
			std::string strDebug("[");
			strDebug += in_strInt;
			strDebug += "]";
			EzLog::e(ftag, strDebug);
			EzLog::ex(ftag, e);
			return -1;
		}

		return 0;


	}

	/*
	安全的将String(正数，有可能带小数)转换为uint64_t的金额(单位为厘 -- 分的十分之一)
	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int String2UInt64MoneyInLi_strtoui64(const string& in_strInt, uint64_t& out_ui64Int)
	{
		static const string ftag("String2UInt64MoneyInLi_strtoui64()");
		static const char DOT[] = { "." };

		if (in_strInt.empty() || 0 == in_strInt.length())
		{
			out_ui64Int = 0;
			return 0;
		}

		try
		{
			// 小数点前的数值
			string strBeforePoint;
			uint64_t ui64BeforePoint = 0;
			// 小数点后的数值
			string strAfterPoint;
			uint64_t ui64AfterPoint = 0;

			// 带小数点的浮点数

			vector<string> vectSegement;
			// boost::is_any_of这里相当于分割规则了
			boost::split(vectSegement, in_strInt, boost::is_any_of(DOT));
			if (0 == vectSegement.size())
			{
				string strDebug(in_strInt);
				strDebug += "分割'.'错误";
				EzLog::e(ftag, strDebug);

				return -1;
			}
			else if (1 == vectSegement.size())
			{
				strBeforePoint = vectSegement[0];
				if (0 == strBeforePoint.length())
				{
					ui64BeforePoint = 0;
				}
				else
				{
					ui64BeforePoint = strtoull(strBeforePoint.c_str(), NULL, 10);
				}
				ui64AfterPoint = 0;
			}
			else if (2 <= vectSegement.size())
			{
				strBeforePoint = vectSegement[0];
				strAfterPoint = vectSegement[1];

				if (0 == strBeforePoint.length())
				{
					ui64BeforePoint = 0;
				}
				else
				{
					ui64BeforePoint = strtoull(strBeforePoint.c_str(), NULL, 10);
				}

				size_t stSizeAfterPoint = strAfterPoint.length();
				if (0 == stSizeAfterPoint)
				{
					ui64AfterPoint = 0;
				}
				else
				{
					// 小数点后第一位
					string strDigit1("0");
					int iDigit1 = 0;
					// 小数点后第二位
					string strDigit2("0");
					int iDigit2 = 0;
					// 小数点后第三位
					string strDigit3("0");
					int iDigit3 = 0;

					// .*****
					if (3 <= stSizeAfterPoint)
					{
						strDigit1 = strAfterPoint.substr(0, 1);
						strDigit2 = strAfterPoint.substr(1, 1);
						strDigit3 = strAfterPoint.substr(2, 1);
					}
					else if (2 <= stSizeAfterPoint)
					{
						strDigit1 = strAfterPoint.substr(0, 1);
						strDigit2 = strAfterPoint.substr(1, 1);
					}
					else
					{
						strDigit1 = strAfterPoint.substr(0, 1);
					}
					iDigit1 = atoi(strDigit1.c_str());
					iDigit2 = atoi(strDigit2.c_str());
					iDigit3 = atoi(strDigit3.c_str());

					ui64AfterPoint = iDigit1 * 100 + iDigit2 * 10 + iDigit3;
				}

			}

			out_ui64Int = ui64BeforePoint * 1000 + ui64AfterPoint;
		}
		catch (exception& e)
		{
			EzLog::ex(ftag, e);
			return -1;
		}

		return 0;
	}	

	/*
	安全的将String(正数，有可能带小数)转换为uint64_t的正整数(不含小数点)
	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int String2UInt64_strtoui64(const string& in_strInt, uint64_t& out_ui64Int)
	{
		static const string ftag("String2UInt64_strtoui64()");

		if (in_strInt.empty())
		{
			out_ui64Int = 0;
			return 0;
		}

		try
		{
			out_ui64Int = strtoll(in_strInt.c_str(), NULL, 10);
		}
		catch (exception& e)
		{
			EzLog::ex(ftag, e);
			return -1;
		}
		return 0;
	}

	/*
	安全的将String(整数，有可能带小数)转换为INT64的整数(不含小数点)
	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int String2Int64_atoi64(const string& in_strInt, int64_t& out_i64Int)
	{
		static const string ftag("String2Int64_atoi64()");

		if (in_strInt.empty())
		{
			out_i64Int = 0;
			return 0;
		}

		try
		{
			out_i64Int = strtoll(in_strInt.c_str(), NULL, 10);

		}
		catch (exception& e)
		{
			EzLog::ex(ftag, e);
			return -1;
		}
		return 0;
	}

	/*
	安全的将String转换为Double
	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int String2Double_atof(const string& in_strInt, double& out_double)
	{
		static const string ftag("String2Double_atof()");

		if (in_strInt.empty())
		{
			out_double = 0;
			return 0;
		}

		try
		{
			out_double = atof(in_strInt.c_str());
			return 0;
		}
		catch (exception& e)
		{
			EzLog::ex(ftag, e);
			return -1;
		}
	}

	// 循环替换字符串中旧字符串为新值
	string& replace_all(string&   str, const   string&   old_value, const   string&   new_value)
	{
		while (true)
		{
			string::size_type pos(0);
			if (string::npos != (pos = str.find(old_value)))
			{
				str.replace(pos, old_value.length(), new_value);
			}
			else
			{
				break;
			}
		}

		return str;
	}

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
	string& iLiToStr(uint64_t ui64MoneyLi, string& out_strLi, unsigned int uiDecimalLen)
	{
		if (0 == ui64MoneyLi)
		{
			switch (uiDecimalLen)
			{
			case 0:
				out_strLi = "0";
				break;

			case 2:
				out_strLi = "0.00";
				break;

			case 3:
				out_strLi = "0.000";
				break;

			case 4:
				out_strLi = "0.0000";
				break;

			case 6:
				out_strLi = "0.000000";
				break;

			default:
				out_strLi = "0";
				break;
			}
			return out_strLi;
		}

		uint64_t ui64Yuan = ui64MoneyLi / 1000;
		const uint64_t cui64Li = ui64MoneyLi % 1000;
		uint64_t ui64AfterPoint = cui64Li;


		// 元数值
		string strYuan;
		// 小数点后数值
		string strLi = "";
		// 小数点后数值临时变量
		string strLiPart;

		sof_string::itostr(ui64Yuan, strYuan);

		switch (uiDecimalLen)
		{
		case 0:
			break;

		case 1:
			// 过滤后2位
			ui64AfterPoint = cui64Li / 100;
			sof_string::itostr(ui64AfterPoint, strLiPart);

			strLi = ".";
			strLi += strLiPart;
			break;

		case 2:
			// 过滤后2位
			ui64AfterPoint = cui64Li / 10;
			sof_string::itostr(ui64AfterPoint, strLiPart);

			if (10 <= ui64AfterPoint)
			{
				strLi = ".";
				strLi += strLiPart;
			}
			else
			{
				strLi = ".0";
				strLi += strLiPart;
			}
			break;

		default:
			// 过滤后3位			
			sof_string::itostr(ui64AfterPoint, strLiPart);

			if (100 <= ui64AfterPoint)
			{
				strLi = ".";
				strLi += strLiPart;
			}
			else if (10 <= ui64AfterPoint)
			{
				strLi = ".0";
				strLi += strLiPart;
			}
			else
			{
				strLi = ".00";
				strLi += strLiPart;
			}
			break;
		}

		out_strLi = strYuan;
		out_strLi += strLi;

		switch (uiDecimalLen)
		{
		case 4:
			out_strLi += "0";
			break;

		case 6:
			out_strLi += "000";
			break;

		default:
			break;
		}

		return out_strLi;
	}

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
	int StringLiToDouble(const string& in_strLi, double &io_dValue, unsigned int uiDecimalLen)
	{
		static const string ftag("StringLiToDouble()");

		int64_t i64Int = 0;

		int iRes = String2Int64_atoi64(in_strLi, i64Int);
		if (0 != iRes)
		{
			string strDebug("String2Int64_atoi64 param[");
			strDebug += in_strLi;
			strDebug += "] failed";
			EzLog::e(ftag, strDebug);
			return -1;
		}

		int64_t i64Yuan = 0;
		long lDecimal = 0;
		long ulDecimalByLen = 0;

		// 获取元
		i64Yuan = i64Int / 1000;
		// 获取以厘为精度的小数
		lDecimal = i64Int % 1000;

		switch (uiDecimalLen)
		{
		case 1:
			ulDecimalByLen = lDecimal / 100;
			io_dValue = i64Yuan + (double)ulDecimalByLen / 10;
			break;

		case 2:
			ulDecimalByLen = lDecimal / 10;
			io_dValue = i64Yuan + (double)ulDecimalByLen / 100;
			break;

		default:
			ulDecimalByLen = lDecimal;
			io_dValue = i64Yuan + (double)ulDecimalByLen / 1000;
			break;
		}

		return 0;
	}

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
	int DoubleToString(const double &in_dValue, string& out_strValue, unsigned int uiDecimalLen)
	{
		int64_t iBeforePoint = (int64_t)in_dValue;
		int64_t iAfterPoint = 0;
		string strBeforePoint;
		string strAfterPoint;

		sof_string::itostr(iBeforePoint, strBeforePoint);

		out_strValue = strBeforePoint;		

		switch (uiDecimalLen)
		{
		case 1:
			iAfterPoint = in_dValue * 10;
			iAfterPoint %= 10;
			if (0 == iAfterPoint)
			{
				out_strValue += ".0";
			}
			else
			{
				out_strValue += ".";
				sof_string::itostr(iAfterPoint, strAfterPoint);
				out_strValue += strAfterPoint;
			}
			break;

		case 2:
			iAfterPoint = in_dValue * 100;
			iAfterPoint %= 100;
			if (0 == iAfterPoint)
			{
				out_strValue += ".00";
			}
			else
			{
				out_strValue += ".";
				sof_string::itostr(iAfterPoint, strAfterPoint);
				out_strValue += strAfterPoint;
			}
			break;

		case 3:
			iAfterPoint = in_dValue * 1000;
			iAfterPoint %= 1000;
			if (0 == iAfterPoint)
			{
				out_strValue += ".000";
			}
			else
			{
				out_strValue += ".";
				sof_string::itostr(iAfterPoint, strAfterPoint);
				out_strValue += strAfterPoint;
			}
			break;

		default:
			iAfterPoint = in_dValue * 1000;
			iAfterPoint %= 1000;
			if (0 == iAfterPoint)
			{
				out_strValue += ".000";
			}
			else
			{
				out_strValue += ".";
				sof_string::itostr(iAfterPoint, strAfterPoint);
				out_strValue += strAfterPoint;
			}
			break;
		}

		return 0;
	}

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
	int StringLiToStringYuan(const string& in_strLi, string &io_strValue, unsigned int in_uiDecimalLen)
	{
		static const string ftag("StringLiToStringYuan()");

		//
		int64_t i64Int = 0;

		int iRes = String2Int64_atoi64(in_strLi, i64Int);
		if (0 != iRes)
		{
			string strDebug("String2Int64_atoi64 param[");
			strDebug += in_strLi;
			strDebug += "] failed";
			EzLog::e(ftag, strDebug);
			return -1;
		}

		// 正整数
		uint64_t ui64Int = 0;
		// 是否是正数
		// true -- 正
		// false -- 负
		bool bIsPositive = true;

		if (0 == i64Int)
		{
			ui64Int = 0;
		}
		else if (0 > i64Int)
		{
			ui64Int = 0 - i64Int;
			bIsPositive = false;
		}
		else
		{
			ui64Int = i64Int;
		}

		// 分解大数的整数和小数部分
		uint64_t ui64Yuan = 0;
		unsigned int uiDecimal = 0;
		unsigned int uiDecimalByLen = 0;
		std::string strYuan;

		// 获取元
		ui64Yuan = ui64Int / 1000;
		// 获取以厘为精度的小数
		uiDecimal = ui64Int % 1000;

		sof_string::itostr(ui64Yuan, strYuan);

		// 给符号位
		if (!bIsPositive)
		{
			io_strValue = "-";
		}
		else
		{
			io_strValue = "";
		}

		// 无小数位
		if (0 == in_uiDecimalLen)
		{
			io_strValue += strYuan;
			return 0;
		}

		// 有小数位
		io_strValue += strYuan;
		io_strValue += ".";

		// 循环用余数
		unsigned int uiDecimalBase = uiDecimal;
		string strTrans;
		// 位数阶乘
		int iTenLv = 1000;

		for (unsigned int i = 1; i <= in_uiDecimalLen; ++i)
		{
			if (3 >= i)
			{
				iTenLv /= 10;

				// 得到首数
				uiDecimalByLen = uiDecimalBase / iTenLv;
				// 得到余数
				uiDecimalBase = uiDecimalBase % iTenLv;

				io_strValue += sof_string::itostr(uiDecimalByLen, strTrans);
			}
			else
			{
				io_strValue += "0";
			}
		}

		return 0;
	}

	/*
	获取JSON对象的内容字符串

	@return 0 -- 成功
	@return -1 -- 失败
	*/
	int GetJsonString(rapidjson::Value& in_jsObj, std::string& out_strJsContent)
	{
		try
		{
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			in_jsObj.Accept(writer);
			out_strJsContent = buffer.GetString();

			return 0;
		}
		catch (exception& e)
		{
			std::cout << e.what() << std::endl;
			return -1;
		}
	}
}