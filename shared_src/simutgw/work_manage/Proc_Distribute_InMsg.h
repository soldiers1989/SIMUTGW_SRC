#ifndef __PROC_DISTRIBUTE_IN_MSG_H__
#define __PROC_DISTRIBUTE_IN_MSG_H__

/*
����ί�С��ر���Ϣ����
���� STEP ��Ϣ
*/

#include "simutgw_flowwork/FlowWorkBase.h"

#include "util/EzLog.h"

class Proc_Distribute_InMsg : public FlowWorkBase
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
	Proc_Distribute_InMsg( void );
	virtual ~Proc_Distribute_InMsg( void );

	virtual int TaskProc( void );

	/*
	�����µ���Ϣ����

	Return :
	0 -- ����ɹ�
	-1 -- ����ʧ��
	*/
	int ProcInMessage();
};

#endif