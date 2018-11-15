#ifndef __SH_DBF_ORDER_ORDER_HELPER_H__
#define __SH_DBF_ORDER_ORDER_HELPER_H__

#include "tool_json/RapidJsonHelper_tgw.h"

#include "order/define_order_msg.h"

#include "tool_odbc/OTLConn40240.h"

/*
�����Ϻ�sqlserver�ӿ���
��ashare_ordwthȡ�µ����ݣ���дstatus�ֶΣ���ص�����mysql���ݿ�
дashare_cjhb��ת������order_report���ֶε�ashare_cjhb���ֶ�
*/
class ShDbfOrderHelper
{
	/*
	member
	*/
private:

	/*
	function
	*/

public:
	virtual ~ShDbfOrderHelper( void );

	/*
	�Ϻ�ί������ת����FIXЭ���ʽ
	*/
	static int SHOrderToFix(OTLConn40240& otlConn, otl_stream& in_stream,
		std::vector<std::shared_ptr<struct simutgw::OrderMessage>> &io_vecOrder,
		const std::string &strSessionid, const struct TradePolicyCell& in_policy );

private:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	ShDbfOrderHelper( void );


	/*
	����Ϻ�ί���Ƿ���֧�ֵ�ҵ������

	Return:
	0 -- ֧��
	-1 -- ��֧��
	*/
	static int Validate_SH_Order( std::shared_ptr<struct simutgw::OrderMessage> &shOrder );

};

#endif