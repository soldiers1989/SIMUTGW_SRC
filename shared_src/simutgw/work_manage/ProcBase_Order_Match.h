#ifndef __PROC_BASE_ORDER_MATCH_H__
#define __PROC_BASE_ORDER_MATCH_H__

#include <memory>

#include "thread_pool_base/TaskBase.h"

#include "order/define_order_msg.h"
#include "util/WorkCounter.h"

/*
�����׳ɽ��Ļ���
*/
class ProcBase_Order_Match : public TaskBase
{
	//
	// member
	//
protected:
	std::shared_ptr<struct simutgw::OrderMessage> m_orderMsg;

	std::shared_ptr<WorkCounter> m_WorkCounter;

	//
	// function
	//
public:
	ProcBase_Order_Match( const unsigned int uiId ) : TaskBase( uiId )
	{
	}

	virtual ~ProcBase_Order_Match()
	{
	}

	void SetOrderMsg( std::shared_ptr<struct simutgw::OrderMessage>& in_msg )
	{
		m_orderMsg = in_msg;
	}
	
	/*
	����ص�
	*/
	virtual int TaskProc( void )
	{
		int iRes = MatchOrder();
		return iRes;
	}

protected:
	/*
	���׳ɽ�����
	Return:
	0 -- �ɽ�
	-1 -- ʧ��
	*/
	virtual int MatchOrder() = 0;
};

#endif