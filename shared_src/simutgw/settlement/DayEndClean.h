#ifndef __DAYEND_CLEAN_H__
#define __DAYEND_CLEAN_H__

#include <memory>

#include "tool_mysql/MySqlCnnC602.h"

/*
����������
*/
class DayEndClean
{
	//
	// Members
	//
private:

	//
	// Functions
	//

public:
	DayEndClean(void);
	virtual ~DayEndClean(void);

	/*
	��������
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int RunDayEndClean(std::shared_ptr<MySqlCnnC602> &in_mysqlConn);

protected:
	/*
	����database
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int BackupDb(std::shared_ptr<MySqlCnnC602> &in_mysqlConn);

	/*
	�����ݿ�����ݸ�������һ����
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int MoveDbdata(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const string& in_strMoveCmd);

	/*
	truncate��
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int TruncateTable(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const string& in_strTableName);

	/*
	����bLogOn = false;
	g_iReportIndex = 0;
	g_iRec_Num = 0;
	g_iRec_Num2 = 0;
	//��¼�Ϻ�ȷ�ϵ�����������
	g_iTeordernum = 1;
	//��¼�Ϻ��ر��ĳɽ����
	g_iCjbh = 1;
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Reset(void);
};

#endif


