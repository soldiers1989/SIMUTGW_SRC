#include "MatchRule.h"

#include "simutgw/stgw_config/g_values_biz.h"

MatchRule::MatchRule(void)
	:m_scl(keywords::channel = "MatchRule")
{

}

MatchRule::~MatchRule(void)
{

}

/*
插入一条上海交易规则，如果之前有相同id的规则就进行替换

@param rapidjson::Value& docValue : 交易规则，可以是单个规则，也可以是规则数组

@return 0 : 插入成功
@return -1 : 插入失败
*/
int MatchRule::InsertShRule(rapidjson::Value& docValue)
{
	static const string ftag("InsertShRule() ");

	int iRes = m_shRules.InsertRule(docValue, true);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error";
		return -1;
	}

	return iRes;
}

/*
插入一条深圳交易规则，如果之前有相同id的规则就进行替换

@param rapidjson::Value& docValue : 交易规则，可以是单个规则，也可以是规则数组

@return 0 : 插入成功
@return -1 : 插入失败
*/
int MatchRule::InsertSzRule(rapidjson::Value& docValue)
{
	static const string ftag("InsertSzRule() ");

	int iRes = m_szRules.InsertRule(docValue, true);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error";
		return -1;
	}

	return iRes;
}

/*
获取上海成交规则配置

@param const std::map<uint64_t, uint64_t>& in_mapLinkRules : 成交配置和通道的关系

@return 0 : 获取成功
@return -1 : 获取失败
*/
int MatchRule::GetShRule(const rapidjson::Value& in_docDeclareOrder,
	const std::map<uint64_t, uint64_t>& in_mapLinkRules,
	uint64_t& out_ruleId, std::shared_ptr<struct MATCH_RULE_SH>& out_rule)
{
	static const string ftag("GetShRule() ");

	int iRes = m_shRules.GetRule(in_docDeclareOrder,
		in_mapLinkRules,
		out_ruleId, out_rule);
	if (0 != iRes)
	{
		//BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error when GetShRule";
		return -1;
	}

	return 0;
}

/*
获取深圳成交规则配置

@param const std::map<uint64_t, uint64_t>& in_mapLinkRules : 成交配置和通道的关系

@return 0 : 获取成功
@return -1 : 获取失败
*/
int MatchRule::GetSzRule(const FIX::Message& in_fixmsg,
	const std::map<uint64_t, uint64_t>& in_mapLinkRules,
	uint64_t& out_ruleId, std::shared_ptr<struct MATCH_RULE_SZ>& out_rule)
{
	static const string ftag("GetSzRule() ");

	int iRes = m_szRules.GetRule(in_fixmsg,
		in_mapLinkRules,
		out_ruleId, out_rule);
	if (0 != iRes)
	{
		//BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error when GetSzRule";
		return -1;
	}

	return 0;
}

/*
从数据库重新载入 成交规则配置

@return 0 : 获取成功
@return -1 : 获取失败
*/
int MatchRule::ReloadFromDb(void)
{
	static const string ftag("ReLoadFromDb() ");

	// 上海
	int iRes = m_shRules.ReloadFromDb();
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error when InsertRule sh";
		return -1;
	}

	iRes = m_szRules.ReloadFromDb();
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error when InsertRule sz";
		return -1;
	}

	return 0;
}

/*
通过通道Rule的关联关系检查和Web的差异

@param std::vector<uint64_t>& out_vctNeedFetchRule_Sh : 需从Web获取的上海策略
@param std::vector<uint64_t>& out_vctNeedFetchRule_Sz : 需从Web获取的深圳策略

*/
void MatchRule::CompareConnRuleRelation_ToLocalRule(std::vector<uint64_t>& out_vctNeedFetchRule_Sh,
	std::vector<uint64_t>& out_vctNeedFetchRule_Sz)
{
	static const string ftag("CompareConnRuleRelation_ToLocalRule() ");

	std::vector<uint64_t> vctNeedFetchRule_Sh;
	std::vector<uint64_t> vctNeedFetchRule_Sz;

	std::map<uint64_t, uint64_t> mapHaveRules;

	// 从 记录上海接口和Web配置的对应关系 查找
	std::map<std::string, struct Connection_webConfig>::const_iterator cit;
	for (cit = simutgw::g_mapShConn_webConfig.begin(); simutgw::g_mapShConn_webConfig.end() != cit; ++cit)
	{
		mapHaveRules.insert(cit->second.mapLinkRules.begin(), cit->second.mapLinkRules.end());
	}

	m_shRules.CompareRuleTime_ToLocalRule(mapHaveRules, vctNeedFetchRule_Sh);

	// 从 记录深圳接口和Web配置的对应关系 查找
	mapHaveRules.clear();
	for (cit = simutgw::g_mapSzConn_webConfig.begin(); simutgw::g_mapSzConn_webConfig.end() != cit; ++cit)
	{
		mapHaveRules.insert(cit->second.mapLinkRules.begin(), cit->second.mapLinkRules.end());
	}
	
	m_szRules.CompareRuleTime_ToLocalRule(mapHaveRules, vctNeedFetchRule_Sz);

	out_vctNeedFetchRule_Sh.swap(vctNeedFetchRule_Sh);
	out_vctNeedFetchRule_Sz.swap(vctNeedFetchRule_Sz);

	return;
}