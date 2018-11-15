#include "StringEncodeUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#ifdef _MSC_VER
#include <codecvt>
#else
#include <iconv.h>
#endif
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/algorithm.hpp>
#include "boost/algorithm/string.hpp"
#include "boost/random.hpp"

#include "util/EzLog.h"

using namespace std;

namespace StringEncodeUtil
{
#ifdef _MSC_VER
	/*
	将utf8编码的字符串转为gbk
	*/
	string& UtfToGbk(const string& in_strSrc, string& out_strDest)
	{
		static const string ftag("UtfToGbk() ");
		int len = MultiByteToWideChar(CP_UTF8, 0, in_strSrc.c_str(), -1, NULL, 0);

		wchar_t* wstrSrc;
		try
		{
			wstrSrc = new wchar_t[len + 1];
		}
		catch (std::bad_alloc& e)
		{
			EzLog::ex(ftag, e);
			wstrSrc = 0;
			out_strDest = "";
			return out_strDest;
		}

		memset(wstrSrc, 0, len + 1);

		MultiByteToWideChar(CP_UTF8, 0, in_strSrc.c_str(), -1, wstrSrc, len);

		len = WideCharToMultiByte(CP_ACP, 0, wstrSrc, -1, NULL, 0, NULL, NULL);
		char* strDest;
		try
		{
			strDest = new char[len + 1];
		}
		catch (std::bad_alloc& e)
		{
			if (wstrSrc)
			{
				delete[] wstrSrc;
			}

			EzLog::ex(ftag, e);
			strDest = 0;
			out_strDest = "";
			return out_strDest;
		}

		memset(strDest, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, wstrSrc, -1, strDest, len, NULL, NULL);

		out_strDest = strDest;

		if (wstrSrc)
		{
			delete[] wstrSrc;
			wstrSrc = NULL;
		}

		if (strDest)
		{
			delete[] strDest;
			strDest = NULL;
		}

		return out_strDest;
	}

	/*
	将gbk编码的字符串转为utf8
	*/
	string& GbkToUtf(const string& in_strSrc, string& out_strDest)
	{
		static const string ftag("GbkToUtf() ");

		int len = MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)in_strSrc.c_str(), -1, NULL, 0);
		wchar_t* wstrSrc = nullptr;
		try
		{
			wstrSrc = new wchar_t[len + 1];
		}
		catch (std::bad_alloc& e)
		{
			EzLog::ex(ftag, e);
			wstrSrc = 0;
			out_strDest = "";
			return out_strDest;
		}

		memset(wstrSrc, 0, len);
		MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)in_strSrc.c_str(), -1, wstrSrc, len);

		len = WideCharToMultiByte(CP_UTF8, 0, wstrSrc, -1, NULL, 0, NULL, NULL);
		char* strDest = nullptr;
		try
		{
			strDest = new char[len + 1];
		}
		catch (std::bad_alloc& e)
		{
			if (wstrSrc)
			{
				delete[] wstrSrc;
			}

			EzLog::ex(ftag, e);
			strDest = 0;
			out_strDest = "";
			return out_strDest;
		}

		memset(strDest, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, wstrSrc, -1, strDest, len, NULL, NULL);

		out_strDest = strDest;

		if (wstrSrc)
		{
			delete[] wstrSrc;
			wstrSrc = NULL;
		}

		if (strDest)
		{
			delete[] strDest;
			strDest = NULL;
		}

		return out_strDest;
	}
#else
	int code_convert(const char *from_charset, const char *to_charset,
		char *inbuf, size_t in_inlen,
		char *outbuf, size_t in_outlen)
	{
		static const string ftag("code_convert() ");
		iconv_t cd;
		char **pin = &inbuf;
		char **pout = &outbuf;

		cd = iconv_open(to_charset, from_charset);
		if (0 == cd)
		{
			string strTran;
			string strDebug("iconv_open to_charset=[");
			strDebug += to_charset;
			strDebug += "] from_charset=";
			strDebug += from_charset;
			strDebug += "] errno=";
			int iError = errno;
			strDebug += sof_string::itostr(iError, strTran);
			EzLog::e(ftag, strDebug);

			return -1;
		}

		// memset(outbuf, 0, outlen);

		// 由于iconv()函数会修改指针，所以要保存源指针
		size_t inlen = in_inlen;
		size_t outlen = in_outlen;
		/*
		 The iconv() function converts one multibyte character at a time, and
		 for each character conversion it increments *inbuf and decrements
		 *inbytesleft by the number of converted input bytes, it increments
		 *outbuf and decrements *outbytesleft by the number of converted
		 output bytes, and it updates the conversion state contained in cd.
		 */
		int iRes = iconv(cd, pin, &inlen, pout, &outlen);
		if (-1 == iRes)
		{
			string strTran;
			string strDebug("iconv errno=");
			int iError = errno;
			strDebug += sof_string::itostr(iError, strTran);
			if (E2BIG == iError)
			{
				strDebug += ", [E2BIG]";
			}
			else if(EILSEQ == iError)
			{
				strDebug += ", [EILSEQ]";
			}
			else if(EINVAL == iError)
			{
				strDebug += ", [EINVAL]";
			}			
			strDebug += ", inbytesleft=";
			strDebug += sof_string::itostr(inlen, strTran);
			strDebug += ", outbytesleft=";
			strDebug += sof_string::itostr(outlen, strTran);
			EzLog::e(ftag, strDebug);

			return -1;
		}

		iconv_close(cd);

		// null-terminate, so the outbuffer must have extra len!!!
		*pout = '\0';

		return 0;
	}

	/*
	将utf8编码的字符串转为gbk
	*/
	string& UtfToGbk(const string& in_strSrc, string& out_strDest)
	{
		static const string ftag("UtfToGbk() ");

		static const char cszUtf8[] = "utf-8";
		static const char cszGb2312[] = "gb2312";

		size_t iSrcLen = in_strSrc.length();
		if (0 == iSrcLen)
		{
			out_strDest = "";
			return out_strDest;
		}

		char* pszSrc = new char[iSrcLen + 1];
		if (NULL == pszSrc)
		{
			string strTran;
			string strDebug("new char failed errno=");
			int iError = errno;
			strDebug += sof_string::itostr(iError, strTran);
			EzLog::e(ftag, strDebug);

			out_strDest = "";
			return out_strDest;
		}

		char* pszDest = new char[iSrcLen + 1];
		if (NULL == pszDest)
		{
			if (NULL != pszSrc)
			{
				delete[] pszSrc;
				pszSrc = NULL;
			}

			string strTran;
			string strDebug("new char failed errno=");
			int iError = errno;
			strDebug += sof_string::itostr(iError, strTran);
			EzLog::e(ftag, strDebug);

			out_strDest = "";
			return out_strDest;
		}

		memset(pszDest, '\0', iSrcLen + 1);

		// 由于iconv()函数会修改指针，所以要保存源指针
		char* pszSrc_param = pszSrc;
		char* pszDest_param = pszDest;
		size_t iSrcLen_param = iSrcLen;
		size_t iDestLen_param = iSrcLen;

		// copy source string to char[]
		size_t i = 0;
		for (i = 0; i < iSrcLen; ++i)
		{
			pszSrc[i] = in_strSrc[i];
		}

		// null terminated
		pszSrc[iSrcLen] = '\0';

		int iRes = code_convert(cszUtf8, cszGb2312, pszSrc_param, iSrcLen_param, pszDest_param, iDestLen_param);
		if (0 != iRes)
		{
			if (NULL != pszSrc)
			{
				delete[] pszSrc;
				pszSrc = NULL;
			}

			if (NULL != pszDest)
			{
				delete[] pszDest;
				pszDest = NULL;
			}

			string strDebug("src=");
			strDebug += in_strSrc;
			EzLog::e(ftag, strDebug);

			out_strDest = "";

			return out_strDest;
		}

		out_strDest = pszDest;

		if (NULL != pszSrc)
		{
			delete[] pszSrc;
			pszSrc = NULL;
		}

		if (NULL != pszDest)
		{
			delete[] pszDest;
			pszDest = NULL;
		}

		return out_strDest;
	}

	/*
	将gbk编码的字符串转为utf8
	*/
	string& GbkToUtf(const string& in_strSrc, string& out_strDest)
	{
		static const string ftag("GbkToUtf() ");

		static const char cszUtf8[] = "utf-8";
		static const char cszGb2312[] = "gb2312";

		size_t iSrcLen = in_strSrc.length();
		if (0 == iSrcLen)
		{
			out_strDest = "";
			return out_strDest;
		}

		char* pszSrc = new char[iSrcLen + 1];
		if (NULL == pszSrc)
		{
			string strTran;
			string strDebug("new char failed errno=");
			int iError = errno;
			strDebug += sof_string::itostr(iError, strTran);
			EzLog::e(ftag, strDebug);

			out_strDest = "";
			return out_strDest;
		}

		char* pszDest = new char[iSrcLen + 1];
		if (NULL == pszDest)
		{
			if (NULL != pszSrc)
			{
				delete[] pszSrc;
				pszSrc = NULL;
			}

			string strTran;
			string strDebug("new char failed errno=");
			int iError = errno;
			strDebug += sof_string::itostr(iError, strTran);
			EzLog::e(ftag, strDebug);

			out_strDest = "";
			return out_strDest;
		}

		memset(pszDest, '\0', iSrcLen + 1);

		// 由于iconv()函数会修改指针，所以要保存源指针
		char* pszSrc_param = pszSrc;
		char* pszDest_param = pszDest;
		size_t iSrcLen_param = iSrcLen;
		size_t iDestLen_param = iSrcLen;

		// copy source string to char[]
		size_t i = 0;
		for (i = 0; i < iSrcLen; ++i)
		{
			pszSrc[i] = in_strSrc[i];
		}

		// null terminated
		pszSrc[iSrcLen] = '\0';

		int iRes = code_convert(cszGb2312, cszUtf8, pszSrc_param, iSrcLen_param, pszDest_param, iDestLen_param);
		if (0 != iRes)
		{
			if (NULL != pszSrc)
			{
				delete[] pszSrc;
				pszSrc = NULL;
			}

			if (NULL != pszDest)
			{
				delete[] pszDest;
				pszDest = NULL;
			}

			string strDebug("src=");
			strDebug += in_strSrc;
			EzLog::e(ftag, strDebug);

			out_strDest = "";

			return out_strDest;
		}

		out_strDest = pszDest;

		if (NULL != pszSrc)
		{
			delete[] pszSrc;
			pszSrc = NULL;
		}

		if (NULL != pszDest)
		{
			delete[] pszDest;
			pszDest = NULL;
}

		return out_strDest;
	}
#endif
}