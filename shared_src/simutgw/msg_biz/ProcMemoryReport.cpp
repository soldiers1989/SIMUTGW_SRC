#include "ProcMemoryReport.h"

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

#include "simutgw/msg_biz/ReportMsg_Sh.h"

#include "util/SystemCounter.h"

#include "simutgw/stgw_config/g_values_biz.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "tool_odbc/OTLConn40240.h"

ProcMemoryReport::ProcMemoryReport(void)
{
}

ProcMemoryReport::~ProcMemoryReport(void)
{
}

/*
发送一条上海回报或确认

Return:
0 -- 成功
其他 -- 失败
*/
int ProcMemoryReport::Send_SH_ReportOrConfirm(map<string, vector<string>>& in_mapUpdate)
{
	static const string strTag("ProcMemoryReport::Send_SH_ShReportOrConfirm() ");

	string strReturn, strSql;

	map<string, vector<string>>::iterator it = in_mapUpdate.begin();

	for (; it != in_mapUpdate.end(); ++it)
	{
		if (0 == it->second.size())
		{
			continue;
		}

		OTLConn40240 otlCon;
		int iRes = 0;
		iRes = otlCon.Connect(simutgw::g_mapShConns[it->first].GetConnection());
		if (0 != iRes)
		{
			string strError("连接失败，str=");
			strError += it->first;
			EzLog::e(strTag, strError);
			return -1;
		}

		long lAffectRows;
		for (size_t i = 0; i < it->second.size(); ++i)
		{
			iRes = otlCon.Exec(it->second[i], lAffectRows);
			if (-1 == iRes)
			{
				string strError("执行失败[");
				strError += it->second[i];
				EzLog::e(strTag, strError);
			}

			{
				EzLog::Out(strTag, trivial::trace, it->second[i]);
			}

			// 记录链路发送
			simutgw::g_counter.IncSh_Link_SendBack(it->first);
		}

		otlCon.Commit();
	}

	return 0;
}


/*
处理上海回报

Return:
0 -- 成功
-1 -- 失败
1 -- 无回报
*/
int ProcMemoryReport::ProcSHReport()
{
	static const string strTag("ProcMemoryReport::ProcSHReport() ");

	int iRes = 0;

	map<string, vector<string>> mapUpdate;

	iRes = ReportMsg_Sh::Get_SHReport(mapUpdate);
	if (0 == iRes)
	{
		// 发送回报
		iRes = Send_SH_ReportOrConfirm(mapUpdate);

	}
	else if (1 == iRes)
	{
		// 无回报
	}
	else
	{

	}

	return iRes;
}