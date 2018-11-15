#ifndef __PROC_WORKLOAD_TRAFFIC_STAT_H__
#define __PROC_WORKLOAD_TRAFFIC_STAT_H__

/*
进行工作流量统计
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
		// 流量总值
		uint64_t cunt;
		// 与上次流量相比的差值
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
		// 深圳接收计数
		struct TrafficUnit Sz_Recv;
		// 深圳确认计数
		struct TrafficUnit Sz_Confirm;
		// 深圳全部成交计数
		struct TrafficUnit Sz_MatchAll;
		// 深圳部分成交计数
		struct TrafficUnit Sz_MatchPart;
		// 深圳撤单计数
		struct TrafficUnit Sz_Cancel;
		// 深圳错误单计数
		struct TrafficUnit Sz_Error;

		// 上海接收计数
		struct TrafficUnit Sh_Recv;
		// 上海确认计数
		struct TrafficUnit Sh_Confirm;
		// 上海全部成交计数
		struct TrafficUnit Sh_MatchAll;
		// 上海部分成交计数
		struct TrafficUnit Sh_MatchPart;
		// 上海撤单计数
		struct TrafficUnit Sh_Cancel;
		// 上海错误单计数
		struct TrafficUnit Sh_Error;

		// 连接流量
		struct TrafficUnit i_conn_sz_match;
		struct TrafficUnit i_conn_sh_match;

		// 线程总成交流量
		struct TrafficUnit i_th_totalMatch;
		// 线程总处理流量
		struct TrafficUnit i_th_totalProc;

		// 当前流量记录时间
		time_t tTime;
		// Time in YYYY-MM-DD HH:mm:ss
		std::string sTime;

		// 计数时间间隔
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

			// 计算差值
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

				// gap 以毫秒为单位
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

	// 锁对象
	boost::mutex m_mutexlock;

	// 间隔开始时间 milliseconds
	uint64_t m_ullStart;

#ifdef _MSC_VER
#else
	timeval m_tvstart;
#endif	

	// 上次流量
	std::shared_ptr<struct simutgw::TrafficGroup> m_ptrLasttraffic;

	// 全部流量历史
	std::list<std::shared_ptr<struct simutgw::TrafficGroup>> m_TrafficHis;

	// 流量历史保存的深度
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
