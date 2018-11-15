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
	��ȫ�Ľ�Stringת��ΪInt
	Return:
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
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
	��ȫ�Ľ�Stringת��ΪLong
	Return:
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
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
	��ȫ�Ľ�String(�������п��ܴ�С��)ת��Ϊuint64_t�Ľ��(��λΪ�� -- �ֵ�ʮ��֮һ)
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
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
			// С����ǰ����ֵ
			string strBeforePoint;
			uint64_t ui64BeforePoint = 0;
			// С��������ֵ
			string strAfterPoint;
			uint64_t ui64AfterPoint = 0;

			// ��С����ĸ�����

			vector<string> vectSegement;
			// boost::is_any_of�����൱�ڷָ������
			boost::split(vectSegement, in_strInt, boost::is_any_of(DOT));
			if (0 == vectSegement.size())
			{
				string strDebug(in_strInt);
				strDebug += "�ָ�'.'����";
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
					// С������һλ
					string strDigit1("0");
					int iDigit1 = 0;
					// С�����ڶ�λ
					string strDigit2("0");
					int iDigit2 = 0;
					// С��������λ
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
	��ȫ�Ľ�String(�������п��ܴ�С��)ת��Ϊuint64_t��������(����С����)
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
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
	��ȫ�Ľ�String(�������п��ܴ�С��)ת��ΪINT64������(����С����)
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
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
	��ȫ�Ľ�Stringת��ΪDouble
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
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

	// ѭ���滻�ַ����о��ַ���Ϊ��ֵ
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


		// Ԫ��ֵ
		string strYuan;
		// С�������ֵ
		string strLi = "";
		// С�������ֵ��ʱ����
		string strLiPart;

		sof_string::itostr(ui64Yuan, strYuan);

		switch (uiDecimalLen)
		{
		case 0:
			break;

		case 1:
			// ���˺�2λ
			ui64AfterPoint = cui64Li / 100;
			sof_string::itostr(ui64AfterPoint, strLiPart);

			strLi = ".";
			strLi += strLiPart;
			break;

		case 2:
			// ���˺�2λ
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
			// ���˺�3λ			
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

		// ��ȡԪ
		i64Yuan = i64Int / 1000;
		// ��ȡ����Ϊ���ȵ�С��
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

		// ������
		uint64_t ui64Int = 0;
		// �Ƿ�������
		// true -- ��
		// false -- ��
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

		// �ֽ������������С������
		uint64_t ui64Yuan = 0;
		unsigned int uiDecimal = 0;
		unsigned int uiDecimalByLen = 0;
		std::string strYuan;

		// ��ȡԪ
		ui64Yuan = ui64Int / 1000;
		// ��ȡ����Ϊ���ȵ�С��
		uiDecimal = ui64Int % 1000;

		sof_string::itostr(ui64Yuan, strYuan);

		// ������λ
		if (!bIsPositive)
		{
			io_strValue = "-";
		}
		else
		{
			io_strValue = "";
		}

		// ��С��λ
		if (0 == in_uiDecimalLen)
		{
			io_strValue += strYuan;
			return 0;
		}

		// ��С��λ
		io_strValue += strYuan;
		io_strValue += ".";

		// ѭ��������
		unsigned int uiDecimalBase = uiDecimal;
		string strTrans;
		// λ���׳�
		int iTenLv = 1000;

		for (unsigned int i = 1; i <= in_uiDecimalLen; ++i)
		{
			if (3 >= i)
			{
				iTenLv /= 10;

				// �õ�����
				uiDecimalByLen = uiDecimalBase / iTenLv;
				// �õ�����
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
	��ȡJSON����������ַ���

	@return 0 -- �ɹ�
	@return -1 -- ʧ��
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