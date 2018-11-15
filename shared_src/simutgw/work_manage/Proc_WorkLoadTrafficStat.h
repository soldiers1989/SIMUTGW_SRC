#ifndef __PROC_WORKLOAD_TRAFFIC_STAT_H__
#define __PROC_WORKLOAD_TRAFFIC_STAT_H__

/*
���й�������ͳ��
*/

#include <list>
#include <stdint.h>
#include <memory>

#include "boost/thread/mutex.hpp"

#include "simutgw_flowwork/FlowWorkBase.h"

#include "tool_string/TimeStringUtil.h"

#include "util/EzLog.h"

namespace simutgw
{
	struct TrafficUnit
	{
		// ������ֵ
		uint64_t cunt;
		// ���ϴ�������ȵĲ�ֵ
		uint64_t incr;
		//
		double rate;

		TrafficUnit()
		{
			cunt = 0;
			incr = 0;
			rate = 0.0;
		}
	};

	struct TrafficGroup
	{
		// ���ڽ��ռ���
		struct TrafficUnit Sz_Recv;
		// ����ȷ�ϼ���
		struct TrafficUnit Sz_Confirm;
		// ����ȫ���ɽ�����
		struct TrafficUnit Sz_MatchAll;
		// ���ڲ��ֳɽ�����
		struct TrafficUnit Sz_MatchPart;
		// ���ڳ�������
		struct TrafficUnit Sz_Cancel;
		// ���ڴ��󵥼���
		struct TrafficUnit Sz_Error;

		// �Ϻ����ռ���
		struct TrafficUnit Sh_Recv;
		// �Ϻ�ȷ�ϼ���
		struct TrafficUnit Sh_Confirm;
		// �Ϻ�ȫ���ɽ�����
		struct TrafficUnit Sh_MatchAll;
		// �Ϻ����ֳɽ�����
		struct TrafficUnit Sh_MatchPart;
		// �Ϻ���������
		struct TrafficUnit Sh_Cancel;
		// �Ϻ����󵥼���
		struct TrafficUnit Sh_Error;

		// ��������
		struct TrafficUnit i_conn_sz_match;
		struct TrafficUnit i_conn_sh_match;

		// �߳��ܳɽ�����
		struct TrafficUnit i_th_totalMatch;
		// �߳��ܴ�������
		struct TrafficUnit i_th_totalProc;

		// ��ǰ������¼ʱ��
		time_t tTime;
		// Time in YYYY-MM-DD HH:mm:ss
		std::string sTime;

		// ����ʱ����
		uint64_t ullTimeGap;

		TrafficGroup()
		{
			ullTimeGap = 0;
			TimeStringUtil::GetTimeStamp(tTime, sTime);
		}

		void Calc(std::shared_ptr<struct simutgw::TrafficGroup> ptrPrevious, const uint64_t ullGap,
			uint64_t iSzRecv, uint64_t iSzCfm, uint64_t iSzMA, uint64_t iSzMP, uint64_t iSzCC, uint64_t iSzErr,
			uint64_t iShRecv, uint64_t iShCfm, uint64_t iShMA, uint64_t iShMP, uint64_t iShCC, uint64_t iShErr)
		{
			// sz
			Sz_Recv.cunt = iSzRecv;
			Sz_Confirm.cunt = iSzCfm;
			Sz_MatchAll.cunt = iSzMA;
			Sz_MatchPart.cunt = iSzMP;
			Sz_Cancel.cunt = iSzCC;
			Sz_Error.cunt = iSzErr;

			i_conn_sz_match.cunt = iSzMA + iSzMP + iSzCC + iSzErr;

			// sh
			Sh_Recv.cunt = iShRecv;
			Sh_Confirm.cunt = iShCfm;
			Sh_MatchAll.cunt = iShMA;
			Sh_MatchPart.cunt = iShMP;
			Sh_Cancel.cunt = iShCC;
			Sh_Error.cunt = iShErr;

			i_conn_sh_match.cunt = iShMA + iShMP + iShCC + iShErr;

			i_th_totalMatch.cunt = i_conn_sz_match.cunt + i_conn_sh_match.cunt;
			i_th_totalProc.cunt = i_conn_sz_match.cunt + iSzCfm + i_conn_sh_match.cunt + iShCfm;

			// �����ֵ
			if (nullptr != ptrPrevious)
			{
				/*
				Sz_MatchAll.incr = iSzMA - ptrPrevious->Sz_MatchAll.cunt;
				Sz_MatchPart.incr = iSzMP - ptrPrevious->Sz_MatchPart.cunt;
				Sz_Cancel.incr = iSzCC - ptrPrevious->Sz_Cancel.cunt;
				Sz_Error.incr = iSzErr - ptrPrevious->Sz_Error.cunt;
				Sz_Confirm.incr = iSzCfm - ptrPrevious->Sz_Confirm.cunt;
				*/

				i_conn_sz_match.incr = i_conn_sz_match.cunt - ptrPrevious->i_conn_sz_match.cunt;
				/*
				Sh_MatchAll.incr = iShMA - ptrPrevious->Sh_MatchAll.cunt;
				Sh_MatchPart.incr = iShMP - ptrPrevious->Sh_MatchPart.cunt;
				Sh_Cancel.incr = iShCC - ptrPrevious->Sh_Cancel.cunt;
				Sh_Error.incr = iShErr - ptrPrevious->Sh_Error.cunt;
				Sh_Confirm.incr = iShCfm - ptrPrevious->Sh_Confirm.cunt;
				*/
				i_conn_sh_match.incr = i_conn_sh_match.cunt - ptrPrevious->i_conn_sh_match.cunt;

				i_th_totalMatch.incr = i_th_totalMatch.cunt - ptrPrevious->i_th_totalMatch.cunt;
				i_th_totalProc.incr = i_th_totalProc.cunt - ptrPrevious->i_th_totalProc.cunt;
			}
			else
			{
				Sz_Recv.incr = iSzRecv;
				Sz_Confirm.incr = iSzCfm;
				Sz_MatchAll.incr = iSzMA;
				Sz_MatchPart.incr = iSzMP;
				Sz_Cancel.incr = iSzCC;
				Sz_Error.incr = iSzErr;

				i_conn_sz_match.incr = i_conn_sz_match.cunt;

				Sh_Recv.incr = iShRecv;
				Sh_Confirm.incr = iShCfm;
				Sh_MatchAll.incr = iShMA;
				Sh_MatchPart.incr = iShMP;
				Sh_Cancel.incr = iShCC;
				Sh_Error.incr = iShErr;

				i_conn_sh_match.incr = i_conn_sh_match.cunt;

				i_th_totalMatch.incr = i_th_totalMatch.cunt;
				i_th_totalProc.incr = i_th_totalProc.cunt;
			}

			ullTimeGap = ullGap;

			if (0 != ullGap)
			{
				double dTimeGap = ullGap / 1000.0;

				// gap �Ժ���Ϊ��λ
				i_conn_sz_match.rate = i_conn_sz_match.incr / dTimeGap;

				i_conn_sh_match.rate = i_conn_sh_match.incr / dTimeGap;

				i_th_totalMatch.rate = i_th_totalMatch.incr / dTimeGap;

				i_th_totalProc.rate = i_th_totalProc.incr / dTimeGap;
			}
		}
	};
}

class Proc_WorkLoadTrafficStat : public FlowWorkBase
{
	//
	// member
	//
private:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// ������
	boost::mutex m_mutexlock;

	// �����ʼʱ�� milliseconds
	uint64_t m_ullStart;

#ifdef _MSC_VER
#else
	timeval m_tvstart;
#endif	

	// �ϴ�����
	std::shared_ptr<struct simutgw::TrafficGroup> m_ptrLasttraffic;

	// ȫ��������ʷ
	std::list<std::shared_ptr<struct simutgw::TrafficGroup>> m_TrafficHis;

	// ������ʷ��������
	unsigned int m_maxHisLen;

	// loop print
	int m_LoopPrintCunt;

	//
	// function
	//
public:
	Proc_WorkLoadTrafficStat(void);
	virtual ~Proc_WorkLoadTrafficStat(void);

	virtual int TaskProc(void);

	int CuntTrafficRate(const uint64_t ullGap);

	int Print(void);
};

#endif
