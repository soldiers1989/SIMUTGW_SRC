#include "tool_mysql/MySqlCnnC602.h"

int MySqlCnnC602_Guard::MysqlLibraryInit(void)
{
	int iRes = mysql_library_init(0, NULL, NULL);

	if (0 != iRes)
	{
		EzLog::Out("MySqlCnnC602_Guard::MysqlLibraryInit()", trivial::fatal,
			"could not initialize MySQL library");
		return -1;
	}

	return iRes;

}

void MySqlCnnC602_Guard::MysqlLibraryEnd(void)
{
	mysql_library_end();
}


MySqlCnnC602::MySqlCnnC602(void)
	: m_scl(keywords::channel = "MySqlCnnC602"), m_pMysql(NULL), m_strHost(""), m_strUserName(""),
	m_strPassWd(""), m_strSchema(""), m_uiPort(0), m_timeLastUseTime(0)
{
}

MySqlCnnC602::~MySqlCnnC602(void)
{
	Close();
}

const char MySqlCnnC602::m_CHAR_SET_GBK[] = "gbk";

// 创建数据库连接
int MySqlCnnC602::Connect(const std::string& hostName, const std::string& userName, const std::string& password,
	unsigned int port)
{
	static const string ftag("MySqlCnnC602::Connect()");
	unsigned int uiErrno = 0;

	// isConnect() 
	if (!IsClosed())
	{
		// 连接已存在
		return -1;
	}

	/*
	Set up and bring down a thread; these function should be called
	for each thread in an application which opens at least one MySQL
	connection.  All uses of the connection(s) should be between these
	function calls.
	*/
	mysql_thread_init();

	m_strHost = hostName;
	m_strUserName = userName;
	m_strPassWd = password;
	m_uiPort = port;

	// 分配内存
	m_pMysql = (MYSQL *)malloc(sizeof(MYSQL));
	if (NULL == m_pMysql)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "(MYSQL *)malloc失败!";

		return -1;
	}

	mysql_init(m_pMysql);
	if (NULL == m_pMysql)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "MySQL sock init error!";

		return -1;
	}

	mysql_options(m_pMysql, MYSQL_INIT_COMMAND, "SET autocommit=0");

	// 连接到指定的数据库
	MYSQL* pSql = mysql_real_connect(m_pMysql, m_strHost.c_str(), m_strUserName.c_str(), m_strPassWd.c_str(), NULL, m_uiPort, NULL, 0);
	if (NULL == pSql)
	{
		// NULL if the connection was unsuccessful.

		// Get mysql_errno
		uiErrno = mysql_errno(m_pMysql);

		string strItoa;
		string strDebug("mysql_real_connect ERROR ");
		strDebug += sof_string::itostr(uiErrno, strItoa);
		strDebug += " ";
		strDebug += mysql_error(m_pMysql);

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		// 链接不成功，直接free
		free(m_pMysql);
		m_pMysql = NULL;
		mysql_thread_end();

		return -1;
	}

	/*自动重连*/
	my_bool reconnect = 1;
	mysql_options(m_pMysql, MYSQL_OPT_RECONNECT, &reconnect);

	/*
	// Sets autocommit mode on if mode is 1, off if mode is 0.
	// Zero for success. Nonzero if an error occurred.
	int iRes =  mysql_autocommit(m_pMysql, 0);
	if( 0 != iRes )
	{
	// Zero for success. Nonzero if an error occurred.

	// Get mysql_errno
	uiErrno = mysql_errno(m_pMysql);

	string strItoa;
	string strDebug("mysql_autocommit ERROR ");
	strDebug += sof_string::itostr(uiErrno, strItoa);
	strDebug += " ";
	strDebug += mysql_error(pSql);

	EzLog::Out( ftag, error, strDebug );

	return -2;
	}
	*/

	// 设置gbk字符集
	int iRes = mysql_set_character_set(m_pMysql, m_CHAR_SET_GBK);
	if (0 != iRes)
	{
		// Zero for success. Nonzero if an error occurred.

		// Get mysql_errno
		uiErrno = mysql_errno(m_pMysql);

		string strItoa;
		string strDebug("mysql_set_character_set ERROR ");
		strDebug += sof_string::itostr(uiErrno, strItoa);
		strDebug += " ";
		strDebug += mysql_error(pSql);

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -2;
	}

	/*
	// major_version*10000 + release_level*100 + sub_version;
	unsigned long ulSvrVersion = mysql_get_server_version(m_pMysql);
	//clear flags & buffer
	ssDebug.clear();
	ssDebug.str("");

	ssDebug << "connect mysql version=" << ulSvrVersion;

	EzLog::Out( ftag, trace, ssDebug );
	*/

	return 0;
}

// 创建数据库连接
int MySqlCnnC602::Connect(const std::string& hostName, const std::string& userName, const std::string& password,
	unsigned int port, const std::string& catalog)
{
	int iRes = Connect(hostName, userName, password, port);
	if (0 != iRes)
	{
		return iRes;
	}

	iRes = SetSchema(catalog);

	return iRes;
}

/*
Close
Return:
*/
void MySqlCnnC602::Close(void)
{
	if (NULL == m_pMysql)
	{
		return;
	}

	/*
	Closes a previously opened connection. mysql_close() also deallocates the connection handler pointed
	to by mysql if the handler was allocated automatically by mysql_init() or mysql_connect().
	*/
	mysql_close(m_pMysql);

	free(m_pMysql);

	m_pMysql = NULL;

	mysql_thread_end();
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
int  MySqlCnnC602::SetAutoCommit(int iAutoCommitMode)
{
	static const string ftag("MySqlCnnC602::SetAutoCommit()");

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	// Sets autocommit mode on if mode is 1, off if mode is 0.
	// Zero for success. Nonzero if an error occurred.
	int iRes = mysql_autocommit(m_pMysql, 0);
	if (0 != iRes)
	{
		// Zero for success. Nonzero if an error occurred.

		unsigned int uiErrno = 0;
		// Get mysql_errno
		uiErrno = mysql_errno(m_pMysql);

		string strItoa;
		string strDebug("mysql_autocommit ERROR ");
		strDebug += sof_string::itostr(uiErrno, strItoa);
		strDebug += " ";
		strDebug += mysql_error(m_pMysql);

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -2;
	}

	return 0;
}

// Schema
int MySqlCnnC602::SetSchema(const std::string& schema)
{
	static const string ftag("MySqlCnnC602::SetSchema()");

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	m_strSchema = schema;

	int iRes = mysql_select_db(m_pMysql, m_strSchema.c_str());

	if (0 != iRes)
	{
		// Zero for success. Nonzero if an error occurred.
		unsigned int uiErrno = 0;

		// Get mysql_errno
		uiErrno = mysql_errno(m_pMysql);

		string strItoa;
		string strDebug("select db[");
		strDebug += schema;
		strDebug += "] error ERROR ";
		strDebug += sof_string::itostr(uiErrno, strItoa);
		strDebug += " ";
		strDebug += mysql_error(m_pMysql);

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		m_strSchema.clear();

		return -2;
	}

	return 0;
}

// Schema
std::string MySqlCnnC602::GetSchema(void)
{
	return m_strSchema;
}

/*
is closed
Return:
true -- connection closed.
false -- connection not closed
*/
bool MySqlCnnC602::IsClosed(void)
{
	if (NULL == m_pMysql)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
ping
Return:
Zero if the connection to the server is active. Nonzero if an error occurred.
*/
int MySqlCnnC602::PingServer(void)
{
	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	// Zero if the connection to the server is active. Nonzero if an error occurred. 
	int iRes = mysql_ping(m_pMysql);
	if (0 != iRes)
	{
		// Close
	}

	return iRes;
}

/*
StartTransaction
Return:
int:
0 -- 执行成功
非0 -- 执行失败
*/
int MySqlCnnC602::StartTransaction(void)
{
	static const string ftag("MySqlCnnC602::StartTransaction()");

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	int iRes = mysql_query(m_pMysql, "START TRANSACTION");
	if (0 != iRes)
	{
		// error
		unsigned int uiErrno = 0;

		// Get mysql_errno
		uiErrno = mysql_errno(m_pMysql);

		string strItoa;
		string strDebug("mysql_query [");
		strDebug += "START TRANSACTION";
		strDebug += "] error ERROR ";
		strDebug += sof_string::itostr(uiErrno, strItoa);
		strDebug += " ";
		strDebug += mysql_error(m_pMysql);

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -2;
	}
	else // query succeeded, process any data returned by it
	{
		return 0;
	}
}

/*
执行语句
注：本函数暂未考虑 multiple-statement execution
Return:
int:
小于0 -- 执行失败
1 -- 有返回行数
2 -- 有受影响行数

OUT:
MYSQL_RES **ppResult : result_set指针.

unsigned int& out_uiAffectedRows :
如果是非SELECT，返回受影响的行数
*/
int MySqlCnnC602::Query(const string& strQueryString,
	MYSQL_RES **ppResult, unsigned long& out_ulAffectedRows)
{
	static const string ftag("MySqlCnnC602::Query()");
	unsigned int uiErrno = 0;

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	MYSQL_RES *result;

	int iRes = mysql_query(m_pMysql, strQueryString.c_str());
	if (0 != iRes)
	{
		// error

		// Get mysql_errno
		uiErrno = mysql_errno(m_pMysql);

		string strItoa;
		string strDebug("mysql_query [");
		strDebug += strQueryString;
		strDebug += "] error ERROR ";
		strDebug += sof_string::itostr(uiErrno, strItoa);
		strDebug += " ";
		strDebug += mysql_error(m_pMysql);

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -2;
	}
	else // query succeeded, process any data returned by it
	{
		result = mysql_store_result(m_pMysql);
		if (result)
		{
			// there are rows
			// retrieve rows, then call mysql_free_result(result)
			*ppResult = result;

			// 是执行SELECT，有ResultSet返回
			return 1;
		}
		else
		{
			// mysql_store_result() returned nothing; should it have?
			if (0 == mysql_field_count(m_pMysql))
			{
				// query does not return data
				// (it was not a SELECT)
				unsigned long ulNum_rows;

				ulNum_rows = static_cast<unsigned long>(mysql_affected_rows(m_pMysql));

				out_ulAffectedRows = ulNum_rows;

				// 是执行非SELECT
				return 2;
			}
			else // mysql_store_result() should have returned data
			{
				// Get mysql_errno
				uiErrno = mysql_errno(m_pMysql);

				string strItoa;
				string strDebug("mysql_store_result [");
				strDebug += strQueryString;
				strDebug += "] error ERROR ";
				strDebug += sof_string::itostr(uiErrno, strItoa);
				strDebug += " ";
				strDebug += mysql_error(m_pMysql);

				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

				return -2;
			}
		}
	}
}

/*
循环获取ResultSet的row
Return:
0 -- 没有数据
非0 -- 有数据
OUT:
map<string, struct MySqlCnnC602_DF::DataInRow>& out_mapRowData  --  以fieldName做Key的map
*/
int MySqlCnnC602::FetchNextRow(MYSQL_RES **ppResultSet, map<string, struct MySqlCnnC602_DF::DataInRow>& out_mapRowData)
{
	static const string ftag("MySqlCnnC602::FetchNextRow()");

	if (IsClosed())
	{
		// 连接已断开
		return 0;
	}

	if (NULL == *ppResultSet)
	{
		return 0;
	}

	// clean out data
	out_mapRowData.clear();

	MYSQL_ROW row;
	// A MYSQL_ROW structure for the next row. NULL if there are no more rows to retrieve or if an error occurred.	
	row = mysql_fetch_row(*ppResultSet);
	if (NULL == row)
	{
		return 0;
	}

	unsigned int uiNum_fields;
	unsigned int i;
	MYSQL_FIELD *fields;

	uiNum_fields = mysql_num_fields(*ppResultSet);
	fields = mysql_fetch_fields(*ppResultSet);
	for (i = 0; i < uiNum_fields; i++)
	{
		struct MySqlCnnC602_DF::DataInRow rowData;
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(*ppResultSet);

		/*
		The lengths of the field values in the row may be obtained by calling mysql_fetch_lengths().
		Empty fields and fields containing NULL both have length 0; you can distinguish these by checking
		the pointer for the field value. If the pointer is NULL, the field is NULL; otherwise, the field is empty.
		*/
		rowData.strFieldName = fields[i].name;
		rowData.eftypeType = fields[i].type;
		rowData.ulFieldValueLen = lengths[i];

		if (NULL == row[i])
		{
			// is NULL
			rowData.bIsNull = true;
		}
		else
		{
			// not NULL
			rowData.bIsNull = false;
			rowData.strValue = row[i];
		}

		out_mapRowData.insert(map<string,
		struct MySqlCnnC602_DF::DataInRow>::value_type(rowData.strFieldName, rowData));
	}

	return 1;
}

/*
Frees the memory allocated for a result set by mysql_store_result(), mysql_use_result(), mysql_list_dbs(), and so forth.
When you are done with a result set, you must free the memory it uses by calling mysql_free_result().

Do not attempt to access a result set after freeing it.
*/
void MySqlCnnC602::FreeResult(MYSQL_RES **ppResultSet)
{
	mysql_free_result(*ppResultSet);
}

/*
commit
Return:
Zero for success.
Nonzero if an error occurred.
*/
int MySqlCnnC602::Commit(void)
{
	static const string ftag("MySqlCnnC602::Commit()");

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	/*
	my_bool mysql_commit(MYSQL *mysql)

	Description

	Commits the current transaction.

	The action of this function is subject to the value of the completion_type system variable. In particular, if the value of completion_type is RELEASE (or 2), the server performs a release after terminating a transaction and closes the client connection. Call mysql_close() from the client program to close the connection from the client side.

	Return Values

	Zero for success. Nonzero if an error occurred.
	*/
	int iRes = mysql_commit(m_pMysql);
	if (0 != iRes)
	{
		// Zero for success. Nonzero if an error occurred.
		unsigned int uiErrno = 0;

		// Get mysql_errno
		uiErrno = mysql_errno(m_pMysql);

		string strItoa;
		string strDebug("mysql_commit() error ERROR ");
		strDebug += sof_string::itostr(uiErrno, strItoa);
		strDebug += " ";
		strDebug += mysql_error(m_pMysql);

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

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
int MySqlCnnC602::RollBack(void)
{
	static const string ftag("MySqlCnnC602::RollBack()");

	if (IsClosed())
	{
		// 连接已断开
		return -1;
	}

	/*
	my_bool mysql_rollback(MYSQL *mysql)

	Description

	Rolls back the current transaction.

	The action of this function is subject to the value of the completion_type system variable. In particular, if the value of completion_type is RELEASE (or 2), the server performs a release after terminating a transaction and closes the client connection. Call mysql_close() from the client program to close the connection from the client side.

	Return Values

	Zero for success. Nonzero if an error occurred.
	*/
	int iRes = mysql_rollback(m_pMysql);
	if (0 != iRes)
	{
		// Zero for success. Nonzero if an error occurred.
		unsigned int uiErrno = 0;

		// Get mysql_errno
		uiErrno = mysql_errno(m_pMysql);

		string strItoa;
		string strDebug("mysql_rollback() error ERROR ");
		strDebug += sof_string::itostr(uiErrno, strItoa);
		strDebug += " ";
		strDebug += mysql_error(m_pMysql);

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -1;
	}

	return 0;
}

// MySQL connector c 6.0.2 map-row
void MySqlCnnC602::Out(const string& strTag, trivial::severity_level eType, const string& strComment,
	const map<string, struct MySqlCnnC602_DF::DataInRow>& mapOutput)
{
	stringstream ss;

	ss << strComment << " mapAddr=[" << &mapOutput << "] " << "key      value" << endl;

	map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator iterMap;
	for (iterMap = mapOutput.begin(); iterMap != mapOutput.end(); ++iterMap)
	{
		ss << iterMap->first << "  struct{FieldName=[" << iterMap->second.strFieldName << "] IsNull=["
			<< iterMap->second.bIsNull << "] Type=[" << iterMap->second.eftypeType << "] FieldValueLen=["
			<< iterMap->second.ulFieldValueLen << "] Value=[" << iterMap->second.strValue << "]}" << endl;
	}

	BOOST_LOG_SEV(EzLog::m_lg, eType) << strTag << ss.str();
}