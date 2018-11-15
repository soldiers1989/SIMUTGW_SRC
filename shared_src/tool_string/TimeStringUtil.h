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
Time转String工具类
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
	给随机数装Seed
	*/
	void FeedRandSeed(void);

	/*
	十位时间（精确到秒） + 尾部随机数
	*/
	string& Random15(string& out_strRandom);

	/*
	十位时间（精确到秒） + 尾部随机数
	*/
	string& ExRandom15(string& out_strRandom);

	/*
	安全的将"YYYY-MM-DD HH:MM:SS.sss"转换为boost time
	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int ConvertStringToPTime(const string& dateTimeString, boost::posix_time::ptime& out_ptime);

	/*
	比较两个string的时间
	Param :
	INT64& out_i64TimeDiff = End - Begin
	Return :
	0 -- 转换成功
	非0 -- 转换失败
	*/
	int CompareTime(const string& in_strBegin, const string& in_strEnd, int64_t& out_i64TimeDiff);

	/*
	将当期时间输出为Fixmsg的格式 20170512-16:11:27.751
	*/
	string& GetCurrTimeInFixmsgType(string& out_time);

	/*
	将当期时间输出为trade_time的格式 2017-05-12 16:11:27
	*/
	string& GetCurrTimeInTradeType(string& out_time);

	/*
	得到当前的时间戳 int string 秒格式
	*/
	string& GetTimeStamp_intstr(string& out_time);

	/*
	得到当前的时间戳
	*/
	unsigned long GetTimeStamp(void);

	/*
	得到指定的时间戳，精确到秒
	@param time_t& out_tTime : time_t格式的时间
	@param std::string& out_sTime : Time in YYYY-MM-DD HH:mm:ss
	*/
	int GetTimeStamp(time_t& out_tTime, std::string& out_sTime);

	/*
	将当期时间输出为Binary LocalTimeStamp Int64的格式 YYYYMMDDHHMMSSsss(毫秒)
	*/
	int64_t& GetCurrTime_BinaryLocalTimeStamp_Type(int64_t& out_time);

	/*
	将当期时间输出为 YYYYMMDD 的格式 20170512

	@param string& out_time : 输出的时间，YYYYMMDD
	@param  const int iDayFromToday : 和今天比的偏移天数

	*/
	string& GetTimeIn_YYYYMMDD(string& out_time, const long iDayFromToday);


	/*
	将当期时间输出为 HH:MM:SS 的格式

	@param string& out_time : 输出的时间，HH:MM:SS
	*/
	string& GetTimeIn_Time_C8(string& out_time);

	/*
	将当期时间输出为 HHMMSS 的格式

	@param string& out_time : 输出的时间，HHMMSS
	*/
	string& GetTimeIn_Time_C6(string& out_time);
}

#endif