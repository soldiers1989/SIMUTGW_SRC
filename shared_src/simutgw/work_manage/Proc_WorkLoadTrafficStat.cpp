#include "Proc_WorkLoadTrafficStat.h"

#include <memory>

#ifdef _MSC_VER
#include <Windows.h>
#else

#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "util/SystemCounter.h"
#include "tool_string/sof_string.h"

#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/stgw_config/g_values_net.h"

Proc_WorkLoadTrafficStat::Proc_WorkLoadTrafficStat(void)
	:m_scl(keywords::channel = "Proc_WorkLoadTrafficStat"),
	m_ptrLasttraffic(nullptr)
{
	m_maxHisLen = 10;

#ifdef _MSC_VER
	// milliseconds
	m_ullStart = GetTickCount64();
#else
	gettimeofday(&m_tvstart, 0);

	// milliseconds
	m_ullStart = m_tvstart.tv_sec * 1000 + m_tvstart.tv_usec / 1000;
#endif	

	m_LoopPrintCunt = 0;
}

Proc_WorkLoadTrafficStat::~Proc_WorkLoadTrafficStat(void)
{
}

int Proc_WorkLoadTrafficStat::TaskProc(void)
{
	static const string fTag("Proc_WorkLoadTrafficStat::TaskProc() ");

	int iRes = 0;
	
#ifdef _MSC_VER
	// sleep to provide a nearly 1second gap
	Sleep(980);
#else
	// sleep to provide a nearly 1second gap
	static const long clSeepTime = 980L * 1000L;
	usleep(clSeepTime);
#endif

	// end
#ifdef _MSC_VER
	// Ulong long WINAPI GetTickCount64(void); The number of milliseconds.
	uint64_t ullEnd = GetTickCount64();
#else
	gettimeofday(&m_tvstart, 0);

	// milliseconds
	uint64_t ullEnd = m_tvstart.tv_sec * 1000 + m_tvstart.tv_usec / 1000;
#endif

	// gap of milliseconds.
	uint64_t ullGap = ullEnd - m_ullStart;

	// switch
	m_ullStart = ullEnd;

	CuntTrafficRate(ullGap);

	return iRes;
}

int Proc_WorkLoadTrafficStat::CuntTrafficRate(const uint64_t ullGap)
{
	static const string fTag("Proc_WorkLoadTrafficStat::CuntTrafficRate() ");

	// lock
	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	// 深圳接收计数
	uint64_t iSzRecv = 0;
	// 深圳确认计数
	uint64_t iSzCfm = 0;
	// 深圳全部成交计数
	uint64_t iSzMA = 0;
	// 深圳部分成交计数
	uint64_t iSzMP = 0;
	// 深圳撤单计数
	uint64_t iSzCC = 0;
	// 深圳错误单计数
	uint64_t iSzErr = 0;

	simutgw::g_counter.GetSz_Inner(iSzRecv, iSzCfm, iSzMA, iSzMP, iSzCC, iSzErr);

	// 上海接收计数
	uint64_t iShRecv = 0;
	// 上海确认计数
	uint64_t iShCfm = 0;
	// 上海全部成交计数
	uint64_t iShMA = 0;
	// 上海部分成交计数
	uint64_t iShMP = 0;
	// 上海撤单计数
	uint64_t iShCC = 0;
	// 上海错误单计数
	uint64_t iShErr = 0;

	simutgw::g_counter.GetSh_Inner(iShRecv, iShCfm, iShMA, iShMP, iShCC, iShErr);

	//
	std::shared_ptr<struct simutgw::TrafficGroup> ptr_traffic(new struct simutgw::TrafficGroup);

	ptr_traffic->Calc(m_ptrLasttraffic, ullGap, iSzRecv, iSzCfm, iSzMA, iSzMP, iSzCC, iSzErr,
		iShRecv, iShCfm, iShMA, iShMP, iShCC, iShErr);

	// 记为最后一条
	m_ptrLasttraffic = ptr_traffic;

	// 将最新历史加入尾部
	m_TrafficHis.push_back(ptr_traffic);

	// 如果深度超过限制，弹出头部
	if (m_maxHisLen < m_TrafficHis.size())
	{
		m_TrafficHis.pop_front();
	}

	++m_LoopPrintCunt;
	if (10 <= m_LoopPrintCunt)
	{
		if (nullptr != simutgw::g_SocketClient)
		{
			rapidjson::Document doc(rapidjson::kObjectType);

			//获取分配器
			rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

			simutgw::g_counter.Print(doc, allocator);
			simutgw::g_SocketClient->SendMsgToServer("statistics", nullptr, doc, trivial::trace, false);
		}
		Print();
		m_LoopPrintCunt = 0;
	}

	return 0;
}

int Proc_WorkLoadTrafficStat::Print(void)
{
	static const string ftag("Proc_WorkLoadTrafficStat::Print() ");

	// lock
	//boost::unique_lock<boost::mutex> Locker( m_mutexlock );

	string sitoa;
	string sOut("list size=");
	sOut += sof_string::itostr(m_TrafficHis.size(), sitoa);
	sOut += " \n===============";
	string sRow;

	size_t i = 0;
	for (std::list<std::shared_ptr<struct simutgw::TrafficGroup>>::iterator it = m_TrafficHis.begin();
		m_TrafficHis.end() != it; ++it, ++i)
	{
		sRow = "\n row=";
		sRow += sof_string::itostr(i, sitoa);
		sRow += ", utc=";
		sRow += sof_string::itostr((*it)->tTime, sitoa);
		sRow += ", t=";
		sRow += (*it)->sTime;
		sRow += ", gap=";
		sRow += sof_string::itostr((*it)->ullTimeGap, sitoa);
		/*
		sRow += "ms, SZ Match ALL=";
		sRow += sof_string::itostr( ( *it )->Sz_MatchAll.cunt, sitoa );
		sRow += ", SZ Match Part=";
		sRow += sof_string::itostr( ( *it )->Sz_MatchPart.cunt, sitoa );
		sRow += ", SZ Cancel=";
		sRow += sof_string::itostr( ( *it )->Sz_Cancel.cunt, sitoa );
		sRow += ", SZ Error=";
		sRow += sof_string::itostr( ( *it )->Sz_Error.cunt, sitoa );
		sRow += ", SZ Confirm=";
		sRow += sof_string::itostr( ( *it )->Sz_Confirm.cunt, sitoa );
		*/

		sRow += "; SH Match ALL=";
		sRow += sof_string::itostr((*it)->Sh_MatchAll.cunt, sitoa);
		sRow += ", SH Match Part=";
		sRow += sof_string::itostr((*it)->Sh_MatchPart.cunt, sitoa);
		sRow += ", SH Cancel=";
		sRow += sof_string::itostr((*it)->Sh_Cancel.cunt, sitoa);
		sRow += ", SH Error=";
		sRow += sof_string::itostr((*it)->Sh_Error.cunt, sitoa);
		sRow += ", SH Confirm=";
		sRow += sof_string::itostr((*it)->Sh_Confirm.cunt, sitoa);

		/*
		sRow += ";  sz conn match=";
		sRow += sof_string::itostr( ( *it )->i_conn_sz_match.cunt, sitoa );
		sRow += ", incr=";
		sRow += sof_string::itostr( ( *it )->i_conn_sz_match.incr, sitoa );
		sRow += ", rate=";
		sRow += to_string( ( *it )->i_conn_sz_match.rate );
		*/

		sRow += "; sh conn match=";
		sRow += sof_string::itostr((*it)->i_conn_sh_match.cunt, sitoa);
		sRow += ", incr=";
		sRow += sof_string::itostr((*it)->i_conn_sh_match.incr, sitoa);
		sRow += ", rate=";
		sRow += to_string((*it)->i_conn_sh_match.rate);
		sRow += ";  total proc=";
		sRow += sof_string::itostr((*it)->i_th_totalProc.cunt, sitoa);
		sRow += ", incr=";
		sRow += sof_string::itostr((*it)->i_th_totalProc.incr, sitoa);
		sRow += ", rate=";
		sRow += to_string((*it)->i_th_totalProc.rate);

		sOut += sRow;
	}

	BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << sOut;

	string sTasks;
	simutgw::g_mtskPool_valid.GetTaskAssignInfo(sTasks);
	BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << sTasks;

	simutgw::g_mtskPool_match_cancel.GetTaskAssignInfo(sTasks);
	BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << sTasks;

	string bufferInfo;
	bufferInfo = "in buffer size=";
	bufferInfo += sof_string::itostr(simutgw::g_inMsg_buffer.GetSize(), sitoa);
	bufferInfo += ", out buffer sh size=";
	bufferInfo += sof_string::itostr(simutgw::g_outMsg_buffer.GetSize_sh(), sitoa);
	BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << bufferInfo;

	string strMysqlBackGroundTaskInfo;
	strMysqlBackGroundTaskInfo += "Mysql task size=";
	simutgw::g_asyncDbwriter.GetTaskAssignInfo(strMysqlBackGroundTaskInfo);
	BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strMysqlBackGroundTaskInfo;

	return 0;
}