#ifndef __APPID_010_ORDER_VALIDE_H__
#define __APPID_010_ORDER_VALIDE_H__

#include "order/define_order_msg.h"

/*
ҵ����Ϣ���
	����ApplIDΪ010��ҵ��
	�ֻ�����Ʊ������ծȯ�ȣ����о��۽����걨
*/
class AppId_010_OrderValide 
{
	//
	// member
	//
protected:

	//
	// function
	//
public:	
	virtual ~AppId_010_OrderValide();

	/*
	����ǰ��麯��
	Return:
	0 -- �Ϸ�
	-1 -- ���Ϸ�
	*/
	static int ValidateOrder( std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg );
	
private:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	AppId_010_OrderValide( void );

	/*
	����ǰ��麯��
	��һЩ�������
	Return:
	0 -- �Ϸ�
	-1 -- ���Ϸ�
	*/
	static int BasicValidate( std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg );

	/*
	ȡ���걨ί�е�����

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int GetOrder_DEL_TYPE( std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg );
};

#endif