#include "TaskStockCacheSyn.h"
#include "simutgw/stgw_config/g_values_inner.h"

TaskStockCacheSyn::TaskStockCacheSyn(const unsigned int uiId) :TaskBase(uiId)
{
}


TaskStockCacheSyn::~TaskStockCacheSyn()
{
}

int TaskStockCacheSyn::TaskProc(void)
{
	ExcUpdateSql();

	return 0;
}

/*
执行更新sql语句
*/
int TaskStockCacheSyn::ExcUpdateSql()
{
	static const std::string strTag("TaskStockCacheSyn::ExcUpdateSql() ");

	try{
		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(strTag, "Get Connection is NULL");

			return -1;
		}

		mysqlConn->StartTransaction();

		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		std::string strItoa;
		int iRes = mysqlConn->Query(m_strUpdateSql, &pResultSet, ulAffectedRows);
		if (2 == iRes)
		{
			// 是更新
			if (1 != ulAffectedRows)
			{
				// 失败
				string strDebug("运行[");
				strDebug += m_strUpdateSql;
				strDebug += "]得到AffectedRows=";
				strDebug += sof_string::itostr((uint64_t)ulAffectedRows, strItoa);
				EzLog::e(strTag, strDebug);

				mysqlConn->RollBack();
				simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
				return -1;
			}
		}
		else
		{
			string strDebug("运行[");
			strDebug += m_strUpdateSql;
			strDebug += "]得到Res=";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(strTag, strDebug);

			mysqlConn->RollBack();
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
			return -1;
		}

		mysqlConn->Commit();

		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
	}
	catch (exception& e)
	{
		EzLog::ex(strTag, e);
	}
	catch (...)
	{
		EzLog::e(strTag, "未知异常");
	}

	return 0;
}