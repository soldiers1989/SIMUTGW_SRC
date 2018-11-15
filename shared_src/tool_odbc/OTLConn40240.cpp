#include "OTLConn40240.h"

#include <stdio.h>
#include <stdlib.h>
#include <sqlext.h>

#include "util/EzLog.h"

OTLConn40240::OTLConn40240() : m_pDB(NULL)
{
}


OTLConn40240::~OTLConn40240()
{
	Close();
}

/* 
连接数据库，并且设置

*/
int OTLConn40240::Connect(const std::string& strOdbcStr)
{
	static const std::string strTag("OTLConn40240::Connect() ");

	if (!IsClosed())
	{
		// 已经连接上
		return -1;
	}

	m_strOdbc = strOdbcStr;

	try
	{
		// connect, autocomit = 0
		m_pDB = new otl_connect(m_strOdbc.c_str(), 0);

		if (NULL == m_pDB)
		{
			std::string strError("connect DB Error, odbc str=");
			strError += m_strOdbc;
			EzLog::e(strTag, strError);

			return -1;
		}

		// set autocommit off
		m_pDB->auto_commit_off();

		//EzLog::t(strTag, "connect DB success!");
	}
	catch (otl_exception& e)
	{
		std::string strError("connect DB Error, EXCEPTION=");

		const int ciSize = 4096;
		char szTrans[ciSize] = { 0 };
		memset(szTrans, 0, ciSize);

#ifdef _MSC_VER
		sprintf_s(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#else
		snprintf(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#endif

		strError += szTrans;

		EzLog::e(strTag, strError);
		return -1;
	}
	catch (std::exception& e)
	{
		EzLog::ex(strTag, e);
		return -1;
	}

	return 0;
}

/*
Close
Return:
*/
void OTLConn40240::Close(void)
{
	if (m_pDB)
	{
		// 先登出
		m_pDB->logoff();
		delete m_pDB;
	}

	m_pDB = NULL;
}

// Schema
int OTLConn40240::SetSchema(const std::string& catalog)
{
	return 0;
}

// Schema
std::string GetSchema(void);

/*
is closed
Return:
true -- connection closed.
false -- connection not closed
*/
bool OTLConn40240::IsClosed(void)
{
	if (m_pDB && m_pDB->connected)
	{
		return false;
	}

	return true;
}

/*
Sets autocommit mode on or off.
IN:
int iAutoCommitMode :
1 -- Sets autocommit mode on
0 -- Sets autocommit mode off
Return:
Zero for success.
Nonzero if an error occurred.
*/
int OTLConn40240::SetAutoCommit(int iAutoCommitMode)
{
	static const std::string strTag("OTLConn40240::SetAutoCommit()");

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	try
	{
		if (0 == iAutoCommitMode)
		{
			m_pDB->auto_commit_off();
		}
		else if (1 == iAutoCommitMode)
		{
			m_pDB->auto_commit_on();
		}
		else
		{
			//Error
		}
	}
	catch (otl_exception& e)
	{
		std::string strError("EXCEPTION=");

		const int ciSize = 4096;
		char szTrans[ciSize] = { 0 };
		memset(szTrans, 0, ciSize);

#ifdef _MSC_VER
		sprintf_s(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#else
		snprintf(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#endif

		strError += szTrans;

		EzLog::e(strTag, strError);
		return -1;
	}
	catch (std::exception& e)
	{
		EzLog::ex(strTag, e);
		return -1;
	}

	return 0;
}

/*
StartTransaction
Return:
int:
0 -- 执行成功
非0 -- 执行失败
*/
int OTLConn40240::StartTransaction(void)
{
	static const string strTag("OTLConn40240::StartTransaction()");

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	std::string strBeginTransaction("BEGIN TRANSACTION");

	otl_stream streamDB;
	streamDB.open(1, strBeginTransaction.c_str(), *m_pDB);

	otl_var_desc *var;
	int var_len;
	var = streamDB.describe_out_vars(var_len);

	return 0;
}

/*
执行查询语句
Return:
int:
小于0 -- 执行失败
0 -- 成功

OUT:
*/
int OTLConn40240::Query(const std::string& strQueryString, otl_stream* streamDB)
{
	static const string strTag("OTLConn40240::Query()");

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	if (NULL == streamDB)
	{
		EzLog::e(strTag, "Param streamDB is NULL");
		return -1;
	}

	try
	{
		//long rpc = 0;

		m_pDB->direct_exec(strQueryString.c_str());

		(*streamDB).open(1, strQueryString.c_str(), *m_pDB);
	}
	catch (otl_exception& e)
	{
		std::string strError("EXCEPTION=");

		const int ciSize = 4096;
		char szTrans[ciSize] = { 0 };
		memset(szTrans, 0, ciSize);

#ifdef _MSC_VER
		sprintf_s(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#else
		snprintf(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#endif

		strError += szTrans;

		EzLog::e(strTag, strError);
		return -1;
	}
	catch (std::exception& e)
	{
		EzLog::ex(strTag, e);
		return -1;
	}

	return 0;
}

/*
循环获取otl_stream的row
Return:
0 -- 没有数据
非0 -- 有数据
OUT:
map<string, struct OTLConn_DF::DataInRow>& mapRowData  --  以fieldName做Key的map
*/
int OTLConn40240::FetchNextRow(otl_stream* streamDB, std::map<std::string, struct OTLConn_DF::DataInRow>& mapRowData)
{
	static const string strTag("OTLConn40240::FetchNextRow()");

	try{
		otl_var_desc *var;
		int var_next_len;

		//otl_var_desc *var;
		//int var_len;
		//var = streamDB.describe_out_vars(var_len);

		while ((!(*streamDB).eof()))
		{
			mapRowData.clear();
			var = (*streamDB).describe_out_vars(var_next_len);
			//std::string strValueField;
			std::stringstream s;
			//std::string strValue;
			long lValue;
			double dbValue;
			bool bIsString = false;
			struct OTLConn_DF::DataInRow data;
			for (int i = 0; i < var_next_len; i++)
			{
				//strValueField.clear();
				s.clear();
				//strValue.clear();
				lValue = 0;
				dbValue = 0;
				bIsString = false;

				//OutputDebugString(var[i].name);                

				switch (var[i].ftype){
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
				case otl_var_nchar:
				case otl_var_nclob:
				{
					streamDB >> strValue;
					s << strValue;
					break;
			}
#endif
				case otl_var_double:
				case otl_var_float:
				case otl_var_bfloat:
				case otl_var_bdouble:
					(*streamDB) >> dbValue;
					s << dbValue;
					break;
				case otl_var_int:
				case otl_var_unsigned_int:
				case otl_var_short:
				case otl_var_long_int:
				case otl_var_bigint:
				case otl_var_ubigint:
					(*streamDB) >> lValue;
					s << lValue;
					break;
				case otl_var_timestamp:
				case otl_var_db2date:
				case otl_var_db2time:
				case otl_var_tz_timestamp:
				case otl_var_ltz_timestamp:
				case otl_var_raw_long:
				case otl_var_clob:
				case otl_var_blob:
				case otl_var_raw:
				case otl_var_long_string:
				case otl_var_lob_stream:
				case otl_var_char:
				case otl_var_varchar_long:
				default:
					bIsString = true;
					try
					{
						(*streamDB) >> data.strValue;
					}
					catch (...){}
					break;
		}

				if (bIsString)
				{
					//strValueField = strValue;
				}
				else
				{
					s >> data.strValue;
				}

				data.emOtlVarType = var[i].ftype;
				data.strFieldName = var[i].name;
				//data.strValue = strValueField;
				data.bIsNull = false;
				data.ulFieldValueLen = (unsigned long)data.strValue.length();

				mapRowData.insert(make_pair(data.strFieldName, data));
	}

			return 1;
}
	}
	catch (otl_exception& e)
	{
		std::string strError("EXCEPTION=");

		const int ciSize = 4096;
		char szTrans[ciSize] = { 0 };
		memset(szTrans, 0, ciSize);

#ifdef _MSC_VER
		sprintf_s(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#else
		snprintf(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#endif

		strError += szTrans;

		EzLog::e(strTag, strError);
		return -1;
	}
	catch (std::exception& e)
	{
		EzLog::ex(strTag, e);
		return -1;
	}

	return 0;
}

/*
执行语句
注：本函数暂未考虑 multiple-statement execution
Return:
int:
-1 -- 执行失败
大于等于0 -- 成功，是已处理行数
*/
long OTLConn40240::Exec(const std::string& strQueryString, long& lAffectRows)
{
	static const string strTag("OTLConn40240::Exec()");

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	try{
		lAffectRows = m_pDB->direct_exec(strQueryString.c_str());
	}
	catch (otl_exception& e)
	{
		std::string strError("EXCEPTION=");

		const int ciSize = 4096;
		char szTrans[ciSize] = { 0 };
		memset(szTrans, 0, ciSize);

#ifdef _MSC_VER
		sprintf_s(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#else
		snprintf(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#endif

		strError += szTrans;

		EzLog::e(strTag, strError);
		return -1;
	}
	catch (std::exception& e)
	{
		EzLog::ex(strTag, e);
		return -1;
	}

	return lAffectRows;
}

/*
commit
Return:
Zero for success.
Nonzero if an error occurred.
*/
int OTLConn40240::Commit(void)
{
	static const std::string strTag("OTLConn40240::Commit()");

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	try
	{
		m_pDB->commit();
	}
	catch (otl_exception& e)
	{
		std::string strError("EXCEPTION=");

		const int ciSize = 4096;
		char szTrans[ciSize] = { 0 };
		memset(szTrans, 0, ciSize);

#ifdef _MSC_VER
		sprintf_s(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#else
		snprintf(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#endif

		strError += szTrans;

		EzLog::e(strTag, strError);
		return -1;
	}
	catch (std::exception& e)
	{
		EzLog::ex(strTag, e);
		return -1;
	}

	return 0;
}

/*
rollback
Return:
Zero for success.
Nonzero if an error occurred.
*/
int OTLConn40240::RollBack(void)
{
	static const std::string strTag("OTLConn40240::RollBack()");

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	try
	{
		m_pDB->rollback();
	}
	catch (otl_exception& e)
	{
		std::string strError("EXCEPTION=");

		const int ciSize = 4096;
		char szTrans[ciSize] = { 0 };
		memset(szTrans, 0, ciSize);

#ifdef _MSC_VER
		sprintf_s(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#else
		snprintf(szTrans, sizeof(szTrans), "%s,%s,%s,%s", e.msg, e.stm_text, e.sqlstate, e.var_info);
#endif

		strError += szTrans;

		EzLog::e(strTag, strError);
		return -1;
	}
	catch (std::exception& e)
	{
		EzLog::ex(strTag, e);
		return -1;
	}

	return 0;
}