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
�ɽ������������ �о��嵥�ֵĴ�����
�Ϻ��г����ݿⱨ��
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
	�ж��Ƿ�Ϊ������

	@return ture : �ǲ�����
	@return false : ���ǲ�����
	*/
	bool IsOperator(const std::string& in_strRuleValue);

};

#endif