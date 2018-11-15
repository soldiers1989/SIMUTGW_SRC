#ifndef __SH_CONN_DBF_ORDER_H__
#define __SH_CONN_DBF_ORDER_H__

#include "tool_json/RapidJsonHelper_tgw.h"

#include "order/define_order_msg.h"

/*
�����Ϻ�sqlserver�ӿ���
��ashare_ordwthȡ�µ����ݣ���дstatus�ֶΣ���ص�����mysql���ݿ�
дashare_cjhb��ת������order_report���ֶε�ashare_cjhb���ֶ�
*/
class ShConn_DbfOrder
{
	/*
	member
	*/
private:

	/*
	function
	*/
public:
	virtual ~ShConn_DbfOrder( void );

	/*
	��ashare_ordwthȡί��
	*/
	static int BatchGetSHOrder(std::vector<std::shared_ptr<struct simutgw::OrderMessage>> &io_vecOrder);


	// У���µ���Ϣ����¼�����ݿ�
	static int Valide_Record_Order( std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg );

	/*
	�޸�ordwth���status״̬ΪP
	*/
	static int SetOrdwthStatus( const std::string& in_strName,
		const std::string &in_strRec_num, const std::string &in_strStatus );

	/*
	�޸�ordwth���һ����¼��status״̬ΪP
	*/
	static int Set_MutilOrd_Status( const std::string& in_strShConnName,
		const vector<std::string> &in_VecRec_num );

private:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	ShConn_DbfOrder( void );

	/*
	ȡ�Ϻ��ɽ����
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int GetSHCjbh( const std::string& in_strShConnName );

	/*
	ȡ�Ϻ�ȷ�ϱ��
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int GetSHRec_Num( const std::string& in_strShConnName );

	/*
	ȡһ���Ϻ��ӿڵ�ί��
	*/
	static int GetOneConnSHOrder( const std::string& in_strShConnName,
		std::vector<std::shared_ptr<struct simutgw::OrderMessage>> &io_vecOrder );
};

#endif