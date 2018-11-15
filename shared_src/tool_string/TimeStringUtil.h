#ifndef __TIME_STRING_UTIL_H__
#define __TIME_STRING_UTIL_H__

#ifdef _MSC_VER
#include <Windows.h>
#else

#endif

#include <time.h>
#include <mutex>
#include <stdint.h>

#include "boost/date_time.hpp"


/*
TimeתString������
*/
namespace TimeStringUtil
{
	using namespace std;

	//
	// Members
	//
	extern mutex mMutexExecID;
	extern uint64_t ui64ExecID;

	//
	// Functions
	//

	/*
	�������װSeed
	*/
	void FeedRandSeed(void);

	/*
	ʮλʱ�䣨��ȷ���룩 + β�������
	*/
	string& Random15(string& out_strRandom);

	/*
	ʮλʱ�䣨��ȷ���룩 + β�������
	*/
	string& ExRandom15(string& out_strRandom);

	/*
	��ȫ�Ľ�"YYYY-MM-DD HH:MM:SS.sss"ת��Ϊboost time
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int ConvertStringToPTime(const string& dateTimeString, boost::posix_time::ptime& out_ptime);

	/*
	�Ƚ�����string��ʱ��
	Param :
	INT64& out_i64TimeDiff = End - Begin
	Return :
	0 -- ת���ɹ�
	��0 -- ת��ʧ��
	*/
	int CompareTime(const string& in_strBegin, const string& in_strEnd, int64_t& out_i64TimeDiff);

	/*
	������ʱ�����ΪFixmsg�ĸ�ʽ 20170512-16:11:27.751
	*/
	string& GetCurrTimeInFixmsgType(string& out_time);

	/*
	������ʱ�����Ϊtrade_time�ĸ�ʽ 2017-05-12 16:11:27
	*/
	string& GetCurrTimeInTradeType(string& out_time);

	/*
	�õ���ǰ��ʱ��� int string ���ʽ
	*/
	string& GetTimeStamp_intstr(string& out_time);

	/*
	�õ���ǰ��ʱ���
	*/
	unsigned long GetTimeStamp(void);

	/*
	�õ�ָ����ʱ�������ȷ����
	@param time_t& out_tTime : time_t��ʽ��ʱ��
	@param std::string& out_sTime : Time in YYYY-MM-DD HH:mm:ss
	*/
	int GetTimeStamp(time_t& out_tTime, std::string& out_sTime);

	/*
	������ʱ�����ΪBinary LocalTimeStamp Int64�ĸ�ʽ YYYYMMDDHHMMSSsss(����)
	*/
	int64_t& GetCurrTime_BinaryLocalTimeStamp_Type(int64_t& out_time);

	/*
	������ʱ�����Ϊ YYYYMMDD �ĸ�ʽ 20170512

	@param string& out_time : �����ʱ�䣬YYYYMMDD
	@param  const int iDayFromToday : �ͽ���ȵ�ƫ������

	*/
	string& GetTimeIn_YYYYMMDD(string& out_time, const long iDayFromToday);


	/*
	������ʱ�����Ϊ HH:MM:SS �ĸ�ʽ

	@param string& out_time : �����ʱ�䣬HH:MM:SS
	*/
	string& GetTimeIn_Time_C8(string& out_time);

	/*
	������ʱ�����Ϊ HHMMSS �ĸ�ʽ

	@param string& out_time : �����ʱ�䣬HHMMSS
	*/
	string& GetTimeIn_Time_C6(string& out_time);
}

#endif