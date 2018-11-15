#ifndef __MATCH_RULE_STRUCT_H__
#define __MATCH_RULE_STRUCT_H__

#include <map>
#include <string>
#include <mutex>
#include <regex>

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

struct MATCH_RULE_BASE
{
	uint64_t ui64RuleId;
	uint64_t ui64Timestamp;

	rapidjson::Document docRegexes;
	
	rapidjson::Document docRuleConfirm;
	rapidjson::Document docRuleMatch;

	MATCH_RULE_BASE()
		: ui64Timestamp(0),
		docRegexes(rapidjson::kObjectType),
		docRuleConfirm(rapidjson::kObjectType),
		docRuleMatch(rapidjson::kArrayType)

	{
	}

	void SetRuleConfirm(rapidjson::Value& docValue)
	{
		docRuleConfirm.CopyFrom(docValue, docRuleConfirm.GetAllocator());
	}

	void SetRuleMatch(rapidjson::Value& docValue)
	{
		docRuleMatch.CopyFrom(docValue, docRuleMatch.GetAllocator());
	}
};

#endif