#ifndef __PROC_SH_CONN_DB_DECLARE_H__
#define __PROC_SH_CONN_DB_DECLARE_H__

/*
�Ϻ� ���ݿⱨ�� ί����Ϣ
*/

#include "simutgw_flowwork/FlowWorkBase.h"

#include "util/EzLog.h"

class Proc_ShConn_Db_Declare : public FlowWorkBase
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
	Proc_ShConn_Db_Declare(void);
	virtual ~Proc_ShConn_Db_Declare(void);

	virtual int TaskProc( void );
	
	/*
	�����Ϻ�����Ϣ

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int ProcShMessage();
};

#endif