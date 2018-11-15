#ifndef __RECORD_MATCH_TABLE_HELPER_H__
#define __RECORD_MATCH_TABLE_HELPER_H__

#include "order/define_order_msg.h"

#include "tool_mysql/MySqlCnnC602.h"

/*
	记录成交流水match表
	发生在成交完后
*/
class RecordMatchTableHelper
{
public:
	RecordMatchTableHelper();
	virtual ~RecordMatchTableHelper();

	/*
		普通委托(包括etf申赎)记录到流水表
	*/
	static int RecordMatchInfo(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::shared_ptr<MySqlCnnC602> &in_mysqlConn);

	/*
		etf申赎成分股记录到流水表
	*/
	static int RecordCompnentMatchInfo(std::shared_ptr<struct simutgw::OrderMessage> in_ptrReport,
		const std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent>& in_ptrFrozeComponent);
};

#endif