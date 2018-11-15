#ifndef __MATCH_RULE_H__
#define __MATCH_RULE_H__

#include <string>
#include <memory>

#include "MatchRule_Sh.h"
#include "MatchRule_Sz.h"

/*
ȫҵ��� �ɽ������������ ������
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
	����һ���Ϻ����׹������֮ǰ����ͬid�Ĺ���ͽ����滻

	@param rapidjson::Value& docValue : ���׹��򣬿����ǵ�������Ҳ�����ǹ�������

	@return 0 : ����ɹ�
	@return -1 : ����ʧ��
	*/
	int InsertShRule(rapidjson::Value& docValue);

	/*
	����һ�����ڽ��׹������֮ǰ����ͬid�Ĺ���ͽ����滻

	@param rapidjson::Value& docValue : ���׹��򣬿����ǵ�������Ҳ�����ǹ�������

	@return 0 : ����ɹ�
	@return -1 : ����ʧ��
	*/
	int InsertSzRule(rapidjson::Value& docValue);

	/*
	��ȡ�Ϻ��ɽ���������

	@param const std::map<uint64_t, uint64_t>& in_mapLinkRules : �ɽ����ú�ͨ���Ĺ�ϵ

	@return 0 : ��ȡ�ɹ�
	@return -1 : ��ȡʧ��
	*/
	int GetShRule(const rapidjson::Value& in_docDeclareOrder,
		const std::map<uint64_t, uint64_t>& in_mapLinkRules,
		uint64_t& out_ruleId, std::shared_ptr<struct MATCH_RULE_SH>& out_rule);

	/*
	��ȡ���ڳɽ���������

	@param const std::map<uint64_t, uint64_t>& in_mapLinkRules : �ɽ����ú�ͨ���Ĺ�ϵ

	@return 0 : ��ȡ�ɹ�
	@return -1 : ��ȡʧ��
	*/
	int GetSzRule(const FIX::Message& in_fixmsg,
		const std::map<uint64_t, uint64_t>& in_mapLinkRules,
		uint64_t& out_ruleId, std::shared_ptr<struct MATCH_RULE_SZ>& out_rule);

	/*
	�����ݿ��������� �ɽ���������

	@return 0 : ��ȡ�ɹ�
	@return -1 : ��ȡʧ��
	*/
	int ReloadFromDb(void);

	/*
	ͨ��ͨ��Rule�Ĺ�����ϵ����Web�Ĳ���

	@param std::vector<uint64_t>& out_vctNeedFetchRule_Sh : ���Web��ȡ���Ϻ�����
	@param std::vector<uint64_t>& out_vctNeedFetchRule_Sz : ���Web��ȡ�����ڲ���

	*/
	void CompareConnRuleRelation_ToLocalRule(std::vector<uint64_t>& out_vctNeedFetchRule_Sh,
		std::vector<uint64_t>& out_vctNeedFetchRule_Sz);

protected:
};

#endif