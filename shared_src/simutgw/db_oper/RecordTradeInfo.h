#ifndef __RECORD_TRADE_INFO_H__
#define __RECORD_TRADE_INFO_H__

#include "order/define_order_msg.h"
#include "tool_mysql/MySqlCnnC602.h"

class RecordTradeInfo
{
public:
	RecordTradeInfo();
	virtual ~RecordTradeInfo();

	/*
		function
	*/
public:
	/*
	�����ݿ���д��ɽ�������
	�ɽ�
	Param :

	Return :
	0 -- д��ɹ�
	��0 -- д��ʧ��
	*/
	static int WriteTransInfoInDb(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport);

	/*
	�����ݿ���д��ɽ�������
	�����ɹ�
	Param :

	Return :
	0 -- д��ɹ�
	��0 -- д��ʧ��
	*/
	static int WriteTransInfoInDb_CancelSuccess(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport);

	/*
	�����ݿ���д��ɽ�������
	����ʧ��
	Param :

	Return :
	0 -- д��ɹ�
	��0 -- д��ʧ��
	*/
	static int WriteTransInfoInDb_CancelFail(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport);

	/*
	�����ݿ���д��ɽ�������
	�ϵ�
	Param :

	Return :
	0 -- д��ɹ�
	��0 -- д��ʧ��
	*/
	static int WriteTransInfoInDb_Error(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport);

	/*
	����order_record���ֶ�
	Param :

	Return :
	0 -- д��ɹ�
	��0 -- д��ʧ��
	*/
	static int UpdateRecordTable(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);
};

#endif