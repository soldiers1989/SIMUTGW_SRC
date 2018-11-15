#ifndef __ORDER_REPEAT_CHECKER_H__
#define __ORDER_REPEAT_CHECKER_H__

#include "OrderMemoryStoreFactory.h"

#include "simutgw_flowwork/FlowWorkBase.h"

/*
�����Ϣ�ص�
*/
class OrderRepeatChecker
{
	//
	// Members
	//
private:

	//
	// Functions
	//
public:
	virtual ~OrderRepeatChecker( void );

	/*
	��鵱ǰ���Ƿ����ص�

	Return :
	ture -- ���ص�
	false -- �����ص�
	*/
	static bool CheckIfRepeatOrder( std::shared_ptr<struct simutgw::OrderMessage>& in_order );

	/*
	��redis�м�¼��ǰ����
	Param :

	Return :
	0 -- д��ɹ�
	��0 -- д��ʧ��
	*/
	static int RecordOrderInRedis( std::shared_ptr<struct simutgw::OrderMessage>& orderMsg );

	/*
	����redis���ص�����

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	static int DayEndCleanUpInRedis(void);
protected:
	/*
	���redis�е�ǰ���Ƿ����ص�

	Return :
	ture -- ���ص�
	false -- �����ص�
	*/
	static bool CheckIfRepeatOrder_Redis( std::shared_ptr<struct simutgw::OrderMessage>& in_order );

	/*
	���mysql�е�ǰ���Ƿ����ص�

	Return :
	ture -- ���ص�
	false -- �����ص�
	*/
	static bool CheckIfRepeatOrder_MySql( std::shared_ptr<struct simutgw::OrderMessage>& in_order );

private:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	OrderRepeatChecker( void );
};

#endif