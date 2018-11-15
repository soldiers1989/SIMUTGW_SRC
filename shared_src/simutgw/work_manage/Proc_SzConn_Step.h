#ifndef __PROC_SZ_CONN_STEP_H__
#define __PROC_SZ_CONN_STEP_H__

/*
����ί�С��ر���Ϣ����
���� STEP ��Ϣ
*/

#include "simutgw_flowwork/FlowWorkBase.h"

#include "util/EzLog.h"

class Proc_SzConn_Step : public FlowWorkBase
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
	Proc_SzConn_Step( void );
	virtual ~Proc_SzConn_Step( void );

	virtual int TaskProc( void );
	
	/*
	�������ڵ���Ϣ

	Return :
	0 -- �����ɹ�
	-1 -- ����ʧ��
	*/
	int ProcSzMessage();
};

#endif