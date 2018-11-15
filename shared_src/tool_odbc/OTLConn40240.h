#ifndef __OTL_CONN_4024_H__
#define __OTL_CONN_4024_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#define _ALLOW_RTCc_IN_STL 
#endif

#define OTL_STL
#define OTL_ODBC
#define OTL_ANSI_CPP

#include <string>
#include <map>


#include "otlv4.h"

/*
OTL ����ǰ���ػ��࣬һ�����κ�ʹ��OTL function֮ǰ����
*/
class OTLConn_Guard
{
	//
	// Members
	//


	//
	// Functions
	//
public:
	static int OTLConn_Init(void)
	{
		//function to initialize the OTL environment
		otl_connect::otl_initialize(1);

		return 0;
	}

private:
	/* Prevent use of these */
	OTLConn_Guard(void);
};

namespace OTLConn_DF
{
	struct DataInRow
	{
		std::string strFieldName;
		/*
		��ǰColumn��ֵ�Ƿ�ΪNULL
		true -- is NULL
		false -- not null
		*/
		bool bIsNull;
		/* Type of field. See otl_var_enum for types */
		int emOtlVarType;
		// The lengths of the field values in the row
		unsigned long ulFieldValueLen;
		std::string strValue;
	};
}

/*
odbc������
http://otl.sourceforge.net/
*/
class OTLConn40240
{
private:
	otl_connect *m_pDB; // connect object
	std::string m_strOdbc;

public:
	OTLConn40240();
	virtual ~OTLConn40240();

	/* �������ݿ� */
	int Connect(const std::string& m_strOdbc);
	
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
	StartTransaction
	Return:
	int:
	0 -- ִ�гɹ�
	��0 -- ִ��ʧ��
	*/
	int StartTransaction(void);

	/*
	ִ�в�ѯ���
	Return:
	int:
	С��0 -- ִ��ʧ��
	0 -- �ɹ�

	OUT:
	*/
	int Query(const std::string& strQueryString, otl_stream* streamDB);
	//, std::vector<std::map<std::string, struct OTLConn_DF::DataInRow>>

	/*
	ѭ����ȡotl_stream��row
	Return:
	0 -- û������
	��0 -- ������
	OUT:
	map<string, struct OTLConn_DF::DataInRow>& mapRowData  --  ��fieldName��Key��map
	*/
	int FetchNextRow(otl_stream* streamDB, std::map<std::string, struct OTLConn_DF::DataInRow>& mapRowData);

	/*
	ִ�����
	ע����������δ���� multiple-statement execution
	Return:
	int:
	-1 -- ִ��ʧ��
	���ڵ���0 -- �ɹ������Ѵ�������
	*/
	long Exec(const std::string& strQueryString, long& lAffectRows);

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

private:
	/* Prevent use of these */
	OTLConn40240(const OTLConn40240 &);
	void operator=(OTLConn40240 &);
};

#endif