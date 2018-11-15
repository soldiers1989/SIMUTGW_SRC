#include "TimeStringUtil.h"

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

#include "tool_string/Tgw_StringUtil.h"

using namespace std;

namespace TimeStringUtil
{
	mutex mMutexExecID;
	uint64_t ui64ExecID = 0;

	/*
	�������װSeed
	*/
	void FeedRandSeed(void)
	{
		srand((int)time(NULL));
	}

	/*
	ʮλʱ�䣨��ȷ���룩 + β�������
	*/
	string& Random15(string& out_strRandom)
	{
		// β���������λ��
		static const unsigned int uiRandomTailLen = 100000;
		time_t now = 0;
		int unixTime = (int)time(&now);

		boost::mt19937 generator;
		boost::uniform_real <> distribution(1, uiRandomTailLen);

		boost::mt19937::result_type random_seed = static_cast<int>(clock());
		generator.seed(random_seed);
		int iRandom = (int)distribution(generator);

		//// ���������		
		//int iRandom = rand() % uiRandomTailLen;

		// β�����������λ��
		string strRandTail;
		// ������ַ�����
		string strRandNum;
		sof_string::itostr(iRandom, strRandNum);

		if (100 <= iRandom)
		{
			if (10000 <= iRandom)
			{
				// 5λ
				// ���ò�λ
				strRandTail = strRandNum;
			}
			else if (1000 <= iRandom)
			{
				// 4λ
				strRandTail = "0";
				strRandTail += strRandNum;
			}
			else
			{
				// 3λ
				strRandTail = "00";
				strRandTail += strRandNum;
			}
		}
		else
		{
			if (10 <= iRandom)
			{
				// 2λ
				strRandTail = "000";
				strRandTail += strRandNum;
			}
			else
			{
				// 1λ
				strRandTail = "0000";
				strRandTail += strRandNum;
			}
		}

		sof_string::itostr(unixTime, out_strRandom);

		out_strRandom += strRandTail;

		return out_strRandom;
	}

	/*
	ʮλʱ�䣨��ȷ���룩 + β�������
	*/
	string& ExRandom15(string& out_strRandom)
	{
		unique_lock<mutex> Locker(mMutexExecID);

		if (0 == ui64ExecID)
		{
			// δ��ʼ��
			string strID;
			Random15(strID);
			Tgw_StringUtil::String2UInt64_strtoui64(strID, ui64ExecID);
		}

		sof_string::itostr(ui64ExecID, out_strRandom);
		++ui64ExecID;

		return out_strRandom;
	}

	/*
	��ȫ�Ľ�"YYYY-MM-DD HH:MM:SS.sss"ת��Ϊboost time
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int ConvertStringToPTime(const string& dateTimeString, boost::posix_time::ptime& out_ptime)
	{
		static const string ftag("ConvertStringToPTime()");

		try
		{
			out_ptime = boost::posix_time::time_from_string(dateTimeString);

			return 0;

		}
		catch (exception& e)
		{
			EzLog::ex(ftag, e);
			return -1;
		}

		return -2;
	}

	/*
	�Ƚ�����string��ʱ��
	Param :
	INT64& out_i64TimeDiff = End - Begin
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int CompareTime(const string& in_strBegin, const string& in_strEnd, int64_t& out_i64TimeDiff)
	{
		static const string ftag("CompareTime()");

		boost::posix_time::ptime ptBegin;
		boost::posix_time::ptime ptEnd;

		try
		{
			int iRes = ConvertStringToPTime(in_strBegin, ptBegin);

			if (0 != iRes)
			{
				return -1;
			}

			iRes = ConvertStringToPTime(in_strEnd, ptEnd);

			if (0 != iRes)
			{
				return -1;
			}

			boost::posix_time::time_duration tmDura = ptEnd - ptBegin;

			out_i64TimeDiff = tmDura.total_milliseconds();

			return 0;

		}
		catch (exception& e)
		{
			EzLog::ex(ftag, e);
			return -1;
		}
	}

	/*
	������ʱ�����ΪFixmsg�ĸ�ʽ 20170512-16:11:27.751
	*/
	string& GetCurrTimeInFixmsgType(string& out_time)
	{
		//��ǰʱ��
		// Fix message format : 20170512-16:11:27.751

		time_t rawtime = 0;

		time(&rawtime);
		struct tm * timeinfo = localtime(&rawtime);

		char buffer[256];
#ifdef _MSC_VER
		sprintf_s(buffer, sizeof(buffer), "%04d%02d%02d-%02d:%02d:%02d.000",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
			timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
			timeinfo->tm_sec);
#else
		snprintf(buffer, sizeof(buffer), "%04d%02d%02d-%02d:%02d:%02d.000",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
			timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
			timeinfo->tm_sec);
#endif


		/*
		SYSTEMTIME  time;
		GetLocalTime(&time);
		char buffer[256];
		sprintf_s(buffer, sizeof(buffer), "%04d%02d%02d-%02d:%02d:%02d.%03d",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
		*/


		out_time = buffer;

		return out_time;
	}

	/*
	������ʱ�����Ϊtrade_time�ĸ�ʽ 2017-05-12 16:11:27
	*/
	string& GetCurrTimeInTradeType(string& out_time)
	{
		//��ǰʱ��
		// trade_time�ĸ�ʽ 2017-05-12 16:11:27

		time_t rawtime = 0;
		time(&rawtime);
		struct tm * timeinfo = localtime(&rawtime);

		char buffer[256];
#ifdef _MSC_VER
		sprintf_s(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
			timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
			timeinfo->tm_sec);
#else
		snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
			timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
			timeinfo->tm_sec);
#endif				

		/*
		SYSTEMTIME  time;
		GetLocalTime(&time);
		char buffer[256];
		sprintf_s(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
		*/
		out_time = buffer;

		return out_time;
	}

	/*
	�õ���ǰ��ʱ��� int string ���ʽ
	*/
	string& GetTimeStamp_intstr(string& out_time)
	{
		time_t t = time(NULL);

		uint64_t j = static_cast<unsigned long>(t);

		return sof_string::itostr(j, out_time);
	}

	/*
	�õ���ǰ��ʱ���
	*/
	unsigned long GetTimeStamp(void)
	{
		time_t t = time(NULL);

		unsigned long j = static_cast<unsigned long>(t);

		return j;
	}

	/*
	�õ�ָ����ʱ�������ȷ����
	@param time_t& out_tTime : time_t��ʽ��ʱ��
	@param std::string& out_sTime : Time in YYYY-MM-DD HH:mm:ss
	*/
	int GetTimeStamp(time_t& out_tTime, std::string& out_sTime)
	{
		out_tTime = time(NULL);

		time_t rawtime = 0;
		time(&rawtime);
		struct tm * timeinfo = localtime(&rawtime);

		char buffer[256];
#ifdef _MSC_VER
		sprintf_s(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
			timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
			timeinfo->tm_sec);
#else
		snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
			timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
			timeinfo->tm_sec);
#endif		
		/*
		SYSTEMTIME time;
		GetLocalTime(&time);
		char buffer[256];
		sprintf_s(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
		*/

		out_sTime = buffer;

		return 0;
	}

	/*
	������ʱ�����ΪBinary LocalTimeStamp Int64�ĸ�ʽ YYYYMMDDHHMMSSsss(����)
	*/
	int64_t& GetCurrTime_BinaryLocalTimeStamp_Type(int64_t& out_time)
	{
		static const int64_t ciTen13 = 100000LL * 100000LL * 1000LL;
		static const int64_t ciTen11 = 100000LL * 100000LL * 10LL;
		static const int64_t ciTen9 = 100000LL * 10000LL;
		static const int64_t ciTen7 = 100000LL * 100LL;
		static const int64_t ciTen5 = 100000LL;
		static const int64_t ciTen3 = 10000LL;

		//��ǰʱ��
		// Binary LocalTimeStamp Int64�ĸ�ʽ YYYYMMDDHHmmSSsss(����)
		// YYYY = 0000-9999, MM = 01-12, DD = 01-31, HH = 00-23,
		// mm = 00-59, SS = 00-60(��), sss=000-999(����)

		time_t rawtime = 0;
		time(&rawtime);
		struct tm * timeinfo = localtime(&rawtime);

		// YYYY MM DD HH mm SS sss
		// 7654 32 10 98 76 54 321

		int64_t iTmp = timeinfo->tm_year + 1900;
		iTmp *= ciTen13;
		out_time += iTmp;

		iTmp = timeinfo->tm_mon + 1;
		iTmp *= ciTen11;
		out_time += iTmp;

		iTmp = timeinfo->tm_mday;
		iTmp *= ciTen9;
		out_time += iTmp;

		iTmp = timeinfo->tm_hour;
		iTmp *= ciTen7;
		out_time += iTmp;

		iTmp = timeinfo->tm_min;
		iTmp *= ciTen5;
		out_time += iTmp;

		iTmp = timeinfo->tm_sec;
		iTmp *= ciTen3;
		out_time += iTmp;

		return out_time;
	}

	/*
	������ʱ�����Ϊ YYYYMMDD �ĸ�ʽ 20170512

	@param string& out_time : �����ʱ�䣬YYYYMMDD
	@param  const int iDayFromToday : �ͽ���ȵ�ƫ������

	*/
	string& GetTimeIn_YYYYMMDD(string& out_time, const long iDayFromToday)
	{
		//��ǰʱ��
		// trade_time�ĸ�ʽ 2017-05-12 16:11:27

		time_t rawtime = 0;
		time(&rawtime);

		rawtime += iDayFromToday * 24 * 3600;
		struct tm * timeinfo = localtime(&rawtime);

		char buffer[256];
#ifdef _MSC_VER
		sprintf_s(buffer, sizeof(buffer), "%04d%02d%02d",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
			timeinfo->tm_mday);
#else
		snprintf(buffer, sizeof(buffer), "%04d%02d%02d",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
			timeinfo->tm_mday);
#endif
		out_time = buffer;

		return out_time;
	}

	/*
	������ʱ�����Ϊ HH:MM:SS �ĸ�ʽ

	@param string& out_time : �����ʱ�䣬HH:MM:SS
	*/
	string& GetTimeIn_Time_C8(string& out_time)
	{
		//��ǰʱ��
		time_t rawtime = 0;
		time(&rawtime);

		struct tm * timeinfo = localtime(&rawtime);

		char buffer[256];
#ifdef _MSC_VER
		sprintf_s(buffer, sizeof(buffer), "%02d:%02d:%02d",
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
#else
		snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
#endif				
		out_time = buffer;

		return out_time;
	}

	/*
	������ʱ�����Ϊ HHMMSS �ĸ�ʽ

	@param string& out_time : �����ʱ�䣬HHMMSS
	*/
	string& GetTimeIn_Time_C6(string& out_time)
	{
		//��ǰʱ��
		time_t rawtime = 0;
		time(&rawtime);

		struct tm * timeinfo = localtime(&rawtime);

		char buffer[256];
#ifdef _MSC_VER
		sprintf_s(buffer, sizeof(buffer), "%02d%02d%02d",
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
#else
		snprintf(buffer, sizeof(buffer), "%02d%02d%02d",
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
#endif				
		out_time = buffer;

		return out_time;
	}
}