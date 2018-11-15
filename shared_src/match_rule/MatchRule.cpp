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
����һ���Ϻ����׹������֮ǰ����ͬid�Ĺ���ͽ����滻

@param rapidjson::Value& docValue : ���׹��򣬿����ǵ�������Ҳ�����ǹ�������

@return 0 : ����ɹ�
@return -1 : ����ʧ��
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
����һ�����ڽ��׹������֮ǰ����ͬid�Ĺ���ͽ����滻

@param rapidjson::Value& docValue : ���׹��򣬿����ǵ�������Ҳ�����ǹ�������

@return 0 : ����ɹ�
@return -1 : ����ʧ��
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
��ȡ�Ϻ��ɽ���������

@param const std::map<uint64_t, uint64_t>& in_mapLinkRules : �ɽ����ú�ͨ���Ĺ�ϵ

@return 0 : ��ȡ�ɹ�
@return -1 : ��ȡʧ��
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
��ȡ���ڳɽ���������

@param const std::map<uint64_t, uint64_t>& in_mapLinkRules : �ɽ����ú�ͨ���Ĺ�ϵ

@return 0 : ��ȡ�ɹ�
@return -1 : ��ȡʧ��
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
�����ݿ��������� �ɽ���������

@return 0 : ��ȡ�ɹ�
@return -1 : ��ȡʧ��
*/
int MatchRule::ReloadFromDb(void)
{
	static const string ftag("ReLoadFromDb() ");

	// �Ϻ�
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
ͨ��ͨ��Rule�Ĺ�����ϵ����Web�Ĳ���

@param std::vector<uint64_t>& out_vctNeedFetchRule_Sh : ���Web��ȡ���Ϻ�����
@param std::vector<uint64_t>& out_vctNeedFetchRule_Sz : ���Web��ȡ�����ڲ���

*/
void MatchRule::CompareConnRuleRelation_ToLocalRule(std::vector<uint64_t>& out_vctNeedFetchRule_Sh,
	std::vector<uint64_t>& out_vctNeedFetchRule_Sz)
{
	static const string ftag("CompareConnRuleRelation_ToLocalRule() ");

	std::vector<uint64_t> vctNeedFetchRule_Sh;
	std::vector<uint64_t> vctNeedFetchRule_Sz;

	std::map<uint64_t, uint64_t> mapHaveRules;

	// �� ��¼�Ϻ��ӿں�Web���õĶ�Ӧ��ϵ ����
	std::map<std::string, struct Connection_webConfig>::const_iterator cit;
	for (cit = simutgw::g_mapShConn_webConfig.begin(); simutgw::g_mapShConn_webConfig.end() != cit; ++cit)
	{
		mapHaveRules.insert(cit->second.mapLinkRules.begin(), cit->second.mapLinkRules.end());
	}

	m_shRules.CompareRuleTime_ToLocalRule(mapHaveRules, vctNeedFetchRule_Sh);

	// �� ��¼���ڽӿں�Web���õĶ�Ӧ��ϵ ����
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