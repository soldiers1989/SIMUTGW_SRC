#ifndef __PROC_SH_CONN_DB_REPORT_H__
#define __PROC_SH_CONN_DB_REPORT_H__

/*
�Ϻ� ���ݿⱨ�� �ر���Ϣ����
*/

#include <map>
#include <vector>
#include <string>


#include "simutgw_flowwork/FlowWorkBase.h"

#include "util/EzLog.h"

class Proc_ShConn_Db_Report : public FlowWorkBase
{
	//
	// member
	//
private:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;
	
	//
	// function
	//
public:
	Proc_ShConn_Db_Report(void);
	virtual ~Proc_ShConn_Db_Report(void);

	virtual int TaskProc( void );
	
	/*
	�����Ϻ�����Ϣ

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int ProcShReport();

protected:
	/*
	����һ���Ϻ��ر���ȷ��

	Return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	int Send_SH_ReportOrConfirm(map<string, vector<string>>& in_mapUpdate);
};

#endif