#ifndef __MATCH_RULE_H__
#define __MATCH_RULE_H__

#include <string>
#include <memory>

#include "MatchRule_Sh.h"
#include "MatchRule_Sz.h"

/*
全业务的 成交规则参数配置 对象类
*/
class MatchRule
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	MatchRule_Sh m_shRules;
	MatchRule_Sz m_szRules;

	//
	// Functions
	//
public:
	MatchRule(void);

	virtual ~MatchRule(void);

	/*
	插入一条上海交易规则，如果之前有相同id的规则就进行替换

	@param rapidjson::Value& docValue : 交易规则，可以是单个规则，也可以是规则数组

	@return 0 : 插入成功
	@return -1 : 插入失败
	*/
	int InsertShRule(rapidjson::Value& docValue);

	/*
	插入一条深圳交易规则，如果之前有相同id的规则就进行替换

	@param rapidjson::Value& docValue : 交易规则，可以是单个规则，也可以是规则数组

	@return 0 : 插入成功
	@return -1 : 插入失败
	*/
	int InsertSzRule(rapidjson::Value& docValue);

	/*
	获取上海成交规则配置

	@param const std::map<uint64_t, uint64_t>& in_mapLinkRules : 成交配置和通道的关系

	@return 0 : 获取成功
	@return -1 : 获取失败
	*/
	int GetShRule(const rapidjson::Value& in_docDeclareOrder,
		const std::map<uint64_t, uint64_t>& in_mapLinkRules,
		uint64_t& out_ruleId, std::shared_ptr<struct MATCH_RULE_SH>& out_rule);

	/*
	获取深圳成交规则配置

	@param const std::map<uint64_t, uint64_t>& in_mapLinkRules : 成交配置和通道的关系

	@return 0 : 获取成功
	@return -1 : 获取失败
	*/
	int GetSzRule(const FIX::Message& in_fixmsg,
		const std::map<uint64_t, uint64_t>& in_mapLinkRules,
		uint64_t& out_ruleId, std::shared_ptr<struct MATCH_RULE_SZ>& out_rule);

	/*
	从数据库重新载入 成交规则配置

	@return 0 : 获取成功
	@return -1 : 获取失败
	*/
	int ReloadFromDb(void);

	/*
	通过通道Rule的关联关系检查和Web的差异

	@param std::vector<uint64_t>& out_vctNeedFetchRule_Sh : 需从Web获取的上海策略
	@param std::vector<uint64_t>& out_vctNeedFetchRule_Sz : 需从Web获取的深圳策略

	*/
	void CompareConnRuleRelation_ToLocalRule(std::vector<uint64_t>& out_vctNeedFetchRule_Sh,
		std::vector<uint64_t>& out_vctNeedFetchRule_Sz);

protected:
};

#endif