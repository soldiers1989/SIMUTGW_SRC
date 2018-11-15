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
ִ�и���sql���
*/
int TaskStockCacheSyn::ExcUpdateSql()
{
	static const std::string strTag("TaskStockCacheSyn::ExcUpdateSql() ");

	try{
		//��mysql���ӳ�ȡ����
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//ȡ����mysql����ΪNULL

			//�黹����
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
			// �Ǹ���
			if (1 != ulAffectedRows)
			{
				// ʧ��
				string strDebug("����[");
				strDebug += m_strUpdateSql;
				strDebug += "]�õ�AffectedRows=";
				strDebug += sof_string::itostr((uint64_t)ulAffectedRows, strItoa);
				EzLog::e(strTag, strDebug);

				mysqlConn->RollBack();
				simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
				return -1;
			}
		}
		else
		{
			string strDebug("����[");
			strDebug += m_strUpdateSql;
			strDebug += "]�õ�Res=";
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
		EzLog::e(strTag, "δ֪�쳣");
	}

	return 0;
}