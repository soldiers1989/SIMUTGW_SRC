#include "RuleWordProc_ShDb.h"

#include "boost/algorithm/string.hpp"

#include "tool_string/sof_string.h"
#include "tool_string/TimeStringUtil.h"
#include "tool_string/Tgw_StringUtil.h"
#include "simutgw/stgw_config/g_values_biz.h"
#include "MatchRuleWords.h"

RuleWordProc_Base::RuleWordProc_Base(void)
	:m_scl(keywords::channel = "RuleWordProc_Base")
{

}

RuleWordProc_Base::~RuleWordProc_Base(void)
{

}

/*
判断是否为操作符

@return ture : 是操作符
@return false : 不是操作符
*/
bool RuleWordProc_Base::IsOperator(const std::string& in_strRuleValue)
{
	if (0 == in_strRuleValue.compare(simutgw::matchrule::cszSign_Add)
		|| 0 == in_strRuleValue.compare(simutgw::matchrule::cszSign_Minus)
		|| 0 == in_strRuleValue.compare(simutgw::matchrule::cszSign_Times)
		|| 0 == in_strRuleValue.compare(simutgw::matchrule::cszSign_Division))
	{
		return true;
	}
	else
	{
		return false;
	}
}
