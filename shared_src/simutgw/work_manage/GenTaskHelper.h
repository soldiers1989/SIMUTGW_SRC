#ifndef __GEN_TASK_HELPER_H__
#define __GEN_TASK_HELPER_H__

#include <memory>

/*
����task����
*/
#include "simutgw_flowwork/FlowWorkBase.h"

#include "order/define_order_msg.h"

class GenTaskHelper
{
	//
	// member
	//
private:

	//
	// function
	//
public:
	virtual ~GenTaskHelper( void );

	/*
	���ɴ���֤��Ϣ

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int GenTask_Valid( std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrderMsg );

	/*
	���ɴ��ɽ���Ϣ

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int GenTask_Match( std::shared_ptr<struct simutgw::OrderMessage>& in_ptrOrderMsg );

private:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	GenTaskHelper( void );
};

#endif