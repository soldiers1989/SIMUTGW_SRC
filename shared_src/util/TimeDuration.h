#ifndef __TIME_DURATION_H__
#define __TIME_DURATION_H__

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include <stdint.h>

/*
ͳ�Ƴ�������ʱ�����
*/
class TimeDuration
{
	//
	// Members
	//
protected:
#ifdef _MSC_VER
	// ��ʼʱ���
	LARGE_INTEGER m_listart;

	// ����ʱ���
	LARGE_INTEGER m_liend;

	// Ƶ��
	LARGE_INTEGER m_lifreq;
#else
	// ��ʼʱ���
	timeval m_tvstart;
	int64_t m_i64start;

	// ����ʱ���
	timeval m_tvend;
	int64_t m_i64end;
#endif

	//
	// Functions
	//
public:
	TimeDuration(void);
	virtual ~TimeDuration(void);

	// ��¼��ʼʱ��
	void Begin(void);

	// ��¼����ʱ�䣬��ȷ��ms
	long long End_ms(long long& out_llLastTime);

	// ��¼����ʱ�䣬��ȷ��us
	long long End_us(long long& out_llLastTime);
};

#endif