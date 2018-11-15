#include "Proc_ShConn_Db_Report.h"

#include "simutgw/msg_biz/ReportMsg_Sh.h"

#include "simutgw/stgw_config/g_values_biz.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/stgw_config/sys_function.h"
#include "simutgw_config/g_values_sys_run_config.h"

#include "util/SystemCounter.h"

#include "tool_odbc/OTLConn40240.h"

Proc_ShConn_Db_Report::Proc_ShConn_Db_Report(void)
	:m_scl(keywords::channel = "Proc_ShConn_Db_Report")
{
}

Proc_ShConn_Db_Report::~Proc_ShConn_Db_Report(void)
{
}

int Proc_ShConn_Db_Report::TaskProc(void)
{
	static const string fTag("Proc_ShConn_Db_Report::TaskProc() ");

	int iRes = ProcShReport();

	return iRes;
}

/*
处理上海的消息

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int Proc_ShConn_Db_Report::ProcShReport()
{
	static const string ftag("Proc_ShConn_Db_Report::ProcShReport() ");

	//---------------处理回报--------------------
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
		simutgw::Simutgw_Sleep();
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << " ProcSHReport() faild";

		return -1;
	}

	return 0;
}

/*
发送一条上海回报或确认

Return:
0 -- 成功
其他 -- 失败
*/
int Proc_ShConn_Db_Report::Send_SH_ReportOrConfirm(map<string, vector<string>>& in_mapUpdate)
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