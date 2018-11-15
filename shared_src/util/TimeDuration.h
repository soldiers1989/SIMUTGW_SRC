#ifndef __TIME_DURATION_H__
#define __TIME_DURATION_H__

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include <stdint.h>

/*
统计程序运行时间的类
*/
class TimeDuration
{
	//
	// Members
	//
protected:
#ifdef _MSC_VER
	// 开始时间戳
	LARGE_INTEGER m_listart;

	// 结束时间戳
	LARGE_INTEGER m_liend;

	// 频率
	LARGE_INTEGER m_lifreq;
#else
	// 开始时间戳
	timeval m_tvstart;
	int64_t m_i64start;

	// 结束时间戳
	timeval m_tvend;
	int64_t m_i64end;
#endif

	//
	// Functions
	//
public:
	TimeDuration(void);
	virtual ~TimeDuration(void);

	// 记录开始时间
	void Begin(void);

	// 记录结束时间，精确到ms
	long long End_ms(long long& out_llLastTime);

	// 记录结束时间，精确到us
	long long End_us(long long& out_llLastTime);
};

#endif