#ifndef __MATCH_RULE_SZ_H__
#define __MATCH_RULE_SZ_H__

#include <stdint.h>
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

#include "quickfix/Message.h"

#include "tool_string/Tgw_StringUtil.h"
#include "util/EzLog.h"

#include "simutgw/tcp_simutgw_client/config_client_msgkey.h"

#include "MatchRule_Base.h"

struct MATCH_RULE_SZ : public MATCH_RULE_BASE
{	
	std::map<int, std::regex> mapRegexs;

	int SetRegex(uint64_t iRuleId, const rapidjson::Value& in_docJudgeCond, const uint64_t& timestamp)
	{
		ui64RuleId = iRuleId;
		docRegexes.CopyFrom(in_docJudgeCond, docRegexes.GetAllocator());

		int iRes = 0;
		std::string strName;
		int iTag = 0;
		std::string strValue;
		for (rapidjson::Value::ConstMemberIterator citer = in_docJudgeCond.MemberBegin();
			in_docJudgeCond.MemberEnd() != citer; ++citer)
		{
			const rapidjson::Value& name_json = citer->name;
			const rapidjson::Value& value_json = citer->value;
			strName = name_json.GetString();

			iRes = Tgw_StringUtil::String2Int_atoi(strName, iTag);
			if (0 != iRes)
			{
				return -1;
			}

			strValue = value_json.GetString();

			mapRegexs[iTag].assign(strValue);
		}

		ui64Timestamp = timestamp;

		return 0;
	}
};

/*
单个 成交规则参数配置 对象 深圳
*/
class MatchRule_Sz
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// Mutex for MapRules
	std::mutex m_mutexMapRules;

	typedef std::map<uint64_t, std::shared_ptr<struct MATCH_RULE_SZ>> MAP_SZ_RULES;

	MAP_SZ_RULES m_mapSzRules;

	//
	// Functions
	//
public:
	MatchRule_Sz(void);

	virtual ~MatchRule_Sz(void);

	/*
	插入一条交易规则，如果之前有相同id的规则就进行替换

	@param rapidjson::Value& docValue : 交易规则，可以是单个规则，也可以是规则数组
	@param bool bUpdateToDb : 是否将数据更新至DB
	true -- 将数据更新至DB
	false -- 不将数据更新至DB

	@return 0 : 插入成功
	@return -1 : 插入失败
	*/
	int InsertRule(rapidjson::Value& docValue, bool bUpdateToDb);

	/*
	从数据库重新载入 成交规则配置 上海

	@return 0 : 获取成功
	@return -1 : 获取失败
	*/
	int ReloadFromDb(void);

	/*
	获取成交规则配置

	@param const std::map<uint64_t, uint64_t>& in_mapLinkRules : 成交配置和通道的关系

	@return 0 : 获取成功
	@return -1 : 获取失败
	*/
	int GetRule(const FIX::Message& in_fixmsg,
		const std::map<uint64_t, uint64_t>& in_mapLinkRules,
		uint64_t& out_ruleId, std::shared_ptr<struct MATCH_RULE_SZ>& out_rule);

	/*
	通过Rule的Timestamp判断rule是否要刷新

	@param const std::map<uint64_t, std::string>& in_mapRuleTime : 根据Web下发得到的Rule
	@param std::vector<uint64_t>& out_vctNeedFetchRule : 需从Web获取的策略集

	*/
	void CompareRuleTime_ToLocalRule(const std::map<uint64_t, uint64_t>& in_mapRuleTime, std::vector<uint64_t>& out_vctNeedFetchRule);

protected:
	/*
	插入一条交易规则，如果之前有相同id的规则就进行替换

	@param bool bUpdateToDb : 是否将数据更新至DB
	true -- 将数据更新至DB
	false -- 不将数据更新至DB

	@return 0 : 插入成功
	@return -1 : 插入失败
	*/
	int InsertSingleRule_Withoutlock(rapidjson::Value& docValue, bool bUpdateToDb);

	/*
	执行更新sql语句

	@return 0 : 更新成功
	@return -1 : 更新失败
	*/
	int UpdateSingleRuleToDb(const std::string& strRuleId,
		const rapidjson::Value& in_docJudgeCond, rapidjson::Value& docValue);

	/*
	插入交易规则，如果之前有相同id的规则就进行替换
	内部使用的无锁

	@param rapidjson::Value& docValue : 交易规则，可以是单个规则，也可以是规则数组
	@param bool bUpdateToDb : 是否将数据更新至DB
	true -- 将数据更新至DB
	false -- 不将数据更新至DB

	@return 0 : 插入成功
	@return -1 : 插入失败
	*/
	int InsertRule_Withoutlock(rapidjson::Value& docValue, bool bUpdateToDb);

};

#endif