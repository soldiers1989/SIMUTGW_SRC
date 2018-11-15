#include "TimeDuration.h"

TimeDuration::TimeDuration(void)
{
#ifdef _MSC_VER
	m_listart.QuadPart = 0;
	m_liend.QuadPart = 0;
	m_lifreq.QuadPart = 0;
#else
	m_i64start=0;
	m_i64end=0;
#endif

}

TimeDuration::~TimeDuration(void)
{
}

// 记录开始时间
void TimeDuration::Begin(void)
{
#ifdef _MSC_VER
	// 获取时钟周期
	QueryPerformanceFrequency(&m_lifreq);

	// 获取时钟计数
	QueryPerformanceCounter(&m_listart);

	//return m_listart.QuadPart;
	return;
#else
	gettimeofday(&m_tvstart, 0);

	m_i64start = m_tvstart.tv_sec*1000*1000 + m_tvstart.tv_usec;
	return;
#endif

}

// 记录结束时间并计算间隔，精确到second
long long TimeDuration::End_ms(long long& out_llLastTime)
{
#ifdef _MSC_VER
	QueryPerformanceCounter(&m_liend);

	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	// To guard against loss-of-precision, we convert
	// to microseconds *before* dividing by ticks-per-second.

	LARGE_INTEGER li_ElapsedTime;
	li_ElapsedTime.QuadPart = m_liend.QuadPart - m_listart.QuadPart;
	li_ElapsedTime.QuadPart *= 1000;
	li_ElapsedTime.QuadPart /= m_lifreq.QuadPart;

	out_llLastTime = li_ElapsedTime.QuadPart;

	return out_llLastTime;
#else
	gettimeofday(&m_tvend, 0);

	m_i64end = m_tvend.tv_sec * 1000 * 1000 + m_tvend.tv_usec;

	out_llLastTime = (m_i64end - m_i64start)/1000;

	return out_llLastTime;
#endif
}

// 记录结束时间并计算间隔，精确到us
long long TimeDuration::End_us(long long& out_llLastTime)
{
#ifdef _MSC_VER
	QueryPerformanceCounter(&m_liend);

	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	// To guard against loss-of-precision, we convert
	// to microseconds *before* dividing by ticks-per-second.

	LARGE_INTEGER li_ElapsedTime;
	li_ElapsedTime.QuadPart = m_liend.QuadPart - m_listart.QuadPart;
	li_ElapsedTime.QuadPart *= 1000000;
	li_ElapsedTime.QuadPart /= m_lifreq.QuadPart;

	out_llLastTime = li_ElapsedTime.QuadPart;

	return out_llLastTime;
#else
	gettimeofday(&m_tvend, 0);

	m_i64end = m_tvend.tv_sec * 1000 * 1000 + m_tvend.tv_usec;

	out_llLastTime = m_i64end - m_i64start;

	return out_llLastTime;
#endif
}