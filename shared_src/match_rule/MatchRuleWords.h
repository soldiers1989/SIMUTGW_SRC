#ifndef __MATCH_RULE_WORDS_H__
#define __MATCH_RULE_WORDS_H__

#include <string>
#include <memory>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "util/EzLog.h"

#include "order/define_order_msg.h"

/*
成交规则参数配置 中具体单字的处理类
上海市场数据库报盘
*/
namespace simutgw
{
	namespace matchrule
	{
		//
		static const char cc_dollar = '$';
		static const char cc_at = '@';
		static const char cszBlank[] = { " " };
		// 四则运算符号
		static const string cszSign_Add("+");
		static const string cszSign_Minus("-");
		static const string cszSign_Times("*");
		static const string cszSign_Division("/");

		//
		// 需系统提供的特殊字段
		//
		// 上海
		// 记录编号
		static const string csz_shdb_REC_NUM("$REC_NUM");
		// 日期，格式为YYYYMMDD
		static const string csz_shdb_DATE("$DATE");
		// 时间，格式为HH:MM:SS
		static const string csz_shdb_TIME_C8("$TIME_C8");
		// 时间，格式为HHMMSS
		static const string csz_shdb_TIME_C6("$TIME_C6");
		// 成交编号
		static const string csz_shdb_CJBH("$SH_CJBH");

		//
		// 深圳
		// ExecID C16 交易所赋予的执行编号，单个交易日内不重复
		static const string csz_szstep_ExecID("$EXEC_ID");

		// OrderID C16 交易所赋予的订单编号，跨交易日不重复
		static const string csz_szstep_OrderID("$ORDER_ID");

		// LocalTimeStamp C21 本地时间戳 YYYYMMDD-HH:MM:SS.sss(毫秒)
		static const string csz_szstep_LocalTimeStamp("$LOCAL_TIME_STAMP");

		// 交易席位
		static const string csz_szstep_seat("$SEAT");
		// 交易账户
		static const string csz_szstep_account("$ACCOUNT");
		
		//
		// 原上海数据库报盘字段
		// reff	会员内部订单号，在整个申报的生命周期中，比如成交回报中，都会附带此数据作为标识字段，柜台系统可以利用此编号进行对应处理。	C10
		static const string csz_shdb_reff("@reff");
		// acc	证券账户	C10
		static const string csz_shdb_acc("@acc");
		// stock	证券代码	C6
		static const string csz_shdb_stock("@stock");
		// bs	买卖方向，‘B’或者‘b’代表买入，‘S’或者‘s’代表卖出	C1
		static const string csz_shdb_bs("@bs");
		// price	申报价格，如果该字段小数点后数字超过3位，3位之后必须为0	C8
		static const string csz_shdb_price("@price");
		// qty	申报数量	C8
		static const string csz_shdb_qty("@qty");
		// owflag	订单类型标志，该字段取值大小写不敏感	C3
		static const string csz_shdb_owflag("@owflag");
		// ordrec	撤单编号	C8
		static const string csz_shdb_ordrec("@ordrec");
		// firmid	B股结算会员代码，对于A股投资者取值无意义	C5
		static const string csz_shdb_firmid("@firmid");
		// branchid	营业部代码	C5
		static const string csz_shdb_branchid("@branchid");
        // checkord	校验码，上交所内部使用	Binary
		static const string csz_shdb_checkord("@checkord");

	}
};

#endif