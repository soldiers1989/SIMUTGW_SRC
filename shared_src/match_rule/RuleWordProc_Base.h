#ifndef __RULE_WORD_PROC_BASE_H__
#define __RULE_WORD_PROC_BASE_H__

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

#include "MatchRuleWords.h"

/*
成交规则参数配置 中具体单字的处理类
上海市场数据库报盘
*/
class RuleWordProc_Base
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	//
	// Functions
	//
public:
	RuleWordProc_Base(void);

	virtual ~RuleWordProc_Base(void);

protected:

	/*
	判断是否为操作符

	@return ture : 是操作符
	@return false : 不是操作符
	*/
	bool IsOperator(const std::string& in_strRuleValue);

};

#endif