#ifndef __MYSQL_CNN_C602_H__
#define __MYSQL_CNN_C602_H__

#ifdef _MSC_VER
#include <winsock2.h>
// Need to link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
#else

#endif


#include "mysql.h"

#include "util/EzLog.h"

using namespace std;

/*
Call this function to initialize the MySQL library before you call any other MySQL function.

MySql Connector C ����ǰ���ػ��࣬һ�����κ�ʹ��MySql Connector C֮ǰ����
*/
class MySqlCnnC602_Guard
{
	//
	// Members
	//


	//
	// Functions
	//
public:
	static int MysqlLibraryInit(void);


	static void MysqlLibraryEnd(void);

private:
	/* Prevent use of these */
	MySqlCnnC602_Guard(void);
};

namespace MySqlCnnC602_DF
{
	struct DataInRow
	{
		string strFieldName;
		/*
		��ǰColumn��ֵ�Ƿ�ΪNULL
		true -- is NULL
		false -- not null
		*/
		bool bIsNull;
		/* Type of field. See mysql_com.h for types */
		enum enum_field_types eftypeType;
		// The lengths of the field values in the row
		unsigned long ulFieldValueLen;
		string strValue;
	};
}

/*
��װmysql-connector-c-6.0.2-win32�Ĺ�����
https://dev.mysql.com/doc/refman/5.6/en/mysql-real-connect.html
*/
class MySqlCnnC602
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	static const char m_CHAR_SET_GBK[];

	MYSQL* m_pMysql;
	string m_strHost;
	string m_strUserName;
	string m_strPassWd;
	string m_strSchema;
	unsigned int m_uiPort;

	// �ϴ�ʹ�õ�ʱ��� ��λ����
	time_t m_timeLastUseTime;

	//
	// Functions
	//
public:
	MySqlCnnC602(void);
	virtual ~MySqlCnnC602(void);

	inline void UpdateUseTime(void)
	{
		time(&m_timeLastUseTime);
	}

	inline time_t GetLastUseTime(void)
	{
		return m_timeLastUseTime;
	}

	// �������ݿ�����
	int Connect(const std::string& hostName, const std::string& userName, const std::string& password,
		unsigned int port);

	// �������ݿ�����
	int Connect(const std::string& hostName, const std::string& userName, const std::string& password,
		unsigned int port, const std::string& catalog);

	/*
	Close
	Return:
	*/
	void Close(void);

	// Schema
	int SetSchema(const std::string& catalog);

	// Schema
	std::string GetSchema(void);

	/*
	is closed
	Return:
	true -- connection closed.
	false -- connection not closed
	*/
	bool IsClosed(void);

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
	int SetAutoCommit(int iAutoCommitMode);

	/*
	ping
	Return:
	Zero if the connection to the server is active. Nonzero if an error occurred.
	*/
	int PingServer(void);

	/*
	StartTransaction
	Return:
	int:
	0 -- ִ�гɹ�
	��0 -- ִ��ʧ��
	*/
	int StartTransaction(void);

	/*
	ִ�����
	ע����������δ���� multiple-statement execution
	Return:
	int:
	С��0 -- ִ��ʧ��
	1 -- �з�������
	2 -- ����Ӱ������

	OUT:
	MYSQL_RES **ppResult : result_setָ��.

	unsigned int& out_uiAffectedRows :
	����Ƿ�SELECT��������Ӱ�������
	*/
	int Query(const string& strQueryString,
		MYSQL_RES **ppResult, unsigned long& out_ulAffectedRows);

	/*
	ѭ����ȡResultSet��row
	Return:
	0 -- û������
	��0 -- ������
	OUT:
	map<string, struct MySqlCnnC602_DF::DataInRow>& mapRowData  --  ��fieldName��Key��map
	*/
	int FetchNextRow(MYSQL_RES **ppResultSet, map<string, struct MySqlCnnC602_DF::DataInRow>& mapRowData);

	/*
	Frees the memory allocated for a result set by mysql_store_result(), mysql_use_result(), mysql_list_dbs(), and so forth.
	When you are done with a result set, you must free the memory it uses by calling mysql_free_result().

	Do not attempt to access a result set after freeing it.
	*/
	void FreeResult(MYSQL_RES **ppResultSet);

	/*
	commit
	Return:
	Zero for success.
	Nonzero if an error occurred.
	*/
	int Commit(void);

	/*
	rollback
	Return:
	Zero for success.
	Nonzero if an error occurred.
	*/
	int RollBack(void);

	// MySQL connector c 6.0.2 map-row
	static void Out(const string& strTag, trivial::severity_level eType, const string& strComment,
		const map<string, struct MySqlCnnC602_DF::DataInRow>& mapOutput);

private:
	/* Prevent use of these */
	MySqlCnnC602(const MySqlCnnC602 &);
	void operator=(MySqlCnnC602 &);
};

#endif