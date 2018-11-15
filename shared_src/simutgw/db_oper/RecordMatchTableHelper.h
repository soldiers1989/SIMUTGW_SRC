#ifndef __RECORD_MATCH_TABLE_HELPER_H__
#define __RECORD_MATCH_TABLE_HELPER_H__

#include "order/define_order_msg.h"

#include "tool_mysql/MySqlCnnC602.h"

/*
	��¼�ɽ���ˮmatch��
	�����ڳɽ����
*/
class RecordMatchTableHelper
{
public:
	RecordMatchTableHelper();
	virtual ~RecordMatchTableHelper();

	/*
		��ͨί��(����etf����)��¼����ˮ��
	*/
	static int RecordMatchInfo(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);

	/*
		etf����ɷֹɼ�¼����ˮ��
	*/
	static int RecordCompnentMatchInfo(std::shared_ptr<struct simutgw::OrderMessage> in_ptrReport,
		const std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent>& in_ptrFrozeComponent);
};

#endif