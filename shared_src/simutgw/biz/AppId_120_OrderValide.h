#ifndef __APPID_120_ORDER_VALIDE_H__
#define __APPID_120_ORDER_VALIDE_H__

#include "order/define_order_msg.h"

/*
ҵ����Ϣ���
����ApplIDΪ120��ҵ��
Etf����
*/
class AppId_120_OrderValide
{
	//
	// member
	//
private:

	//
	// function
	//
public:
	virtual ~AppId_120_OrderValide( void );

	/*
	����ǰ��麯��
	Return:
	0 -- �Ϸ�
	-1 -- ���Ϸ�
	*/
	static int ValidateOrder( std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg );

protected:


private:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	AppId_120_OrderValide( void );
};

#endif