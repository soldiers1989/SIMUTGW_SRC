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
�����Ϻ�����Ϣ

Return :
0 -- �����ɹ�
-1 -- ����ʧ��
*/
int Proc_ShConn_Db_Report::ProcShReport()
{
	static const string ftag("Proc_ShConn_Db_Report::ProcShReport() ");

	//---------------����ر�--------------------
	int iRes = 0;

	map<string, vector<string>> mapUpdate;

	iRes = ReportMsg_Sh::Get_SHReport(mapUpdate);
	if (0 == iRes)
	{
		// ���ͻر�
		iRes = Send_SH_ReportOrConfirm(mapUpdate);

	}
	else if (1 == iRes)
	{
		// �޻ر�
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
����һ���Ϻ��ر���ȷ��

Return:
0 -- �ɹ�
���� -- ʧ��
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
			string strError("����ʧ�ܣ�str=");
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
				string strError("ִ��ʧ��[");
				strError += it->second[i];
				EzLog::e(strTag, strError);
			}

			{
				EzLog::Out(strTag, trivial::trace, it->second[i]);
			}

			// ��¼��·����
			simutgw::g_counter.IncSh_Link_SendBack(it->first);
		}

		otlCon.Commit();
	}

	return 0;
}