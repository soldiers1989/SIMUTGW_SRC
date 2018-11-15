#ifndef __MATCH_RULE_SH_H__
#define __MATCH_RULE_SH_H__

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

#include "util/EzLog.h"

#include "simutgw/tcp_simutgw_client/config_client_msgkey.h"

#include "MatchRule_Base.h"

struct MATCH_RULE_SH : public MATCH_RULE_BASE
{	
	std::map<std::string, std::regex> mapRegexs;

	int SetRegex(uint64_t iRuleId, const rapidjson::Value& in_docJudgeCond, const uint64_t& timestamp)
	{
		ui64RuleId = iRuleId;
		docRegexes.CopyFrom(in_docJudgeCond, docRegexes.GetAllocator());

		std::string strName;
		std::string strValue;
		for (rapidjson::Value::ConstMemberIterator citer = in_docJudgeCond.MemberBegin();
			in_docJudgeCond.MemberEnd() != citer; ++citer)
		{
			const rapidjson::Value& name_json = citer->name;
			const rapidjson::Value& value_json = citer->value;
			strName = name_json.GetString();
			strValue = value_json.GetString();

			mapRegexs[strName].assign(strValue);
		}

		ui64Timestamp = timestamp;

		return 0;
	}
};

/*
���� �ɽ������������ ���� �Ϻ�
*/
class MatchRule_Sh
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// Mutex for MapRules
	std::mutex m_mutexMapRules;

	typedef std::map<uint64_t, std::shared_ptr<struct MATCH_RULE_SH>> MAP_SH_RULES;

	MAP_SH_RULES m_mapShRules;

	//
	// Functions
	//
public:
	MatchRule_Sh(void);

	virtual ~MatchRule_Sh(void);

	/*
	����һ�����׹������֮ǰ����ͬid�Ĺ���ͽ����滻

	@param rapidjson::Value& docValue : ���׹��򣬿����ǵ�������Ҳ�����ǹ�������
	@param bool bUpdateToDb : �Ƿ����ݸ�����DB
	true -- �����ݸ�����DB
	false -- �������ݸ�����DB

	@return 0 : ����ɹ�
	@return -1 : ����ʧ��
	*/
	int InsertRule(rapidjson::Value& docValue, bool bUpdateToDb);

	/*
	�����ݿ��������� �ɽ��������� �Ϻ�

	@return 0 : ��ȡ�ɹ�
	@return -1 : ��ȡʧ��
	*/
	int ReloadFromDb(void);

	/*
	��ȡ�ɽ���������

	@param const std::map<uint64_t, uint64_t>& in_mapLinkRules : �ɽ����ú�ͨ���Ĺ�ϵ

	@return 0 : ��ȡ�ɹ�
	@return -1 : ��ȡʧ��
	*/
	int GetRule(const rapidjson::Value& in_docDeclareOrder,
		const std::map<uint64_t, uint64_t>& in_mapLinkRules,
		uint64_t& out_ruleId, std::shared_ptr<struct MATCH_RULE_SH>& out_rule);

	/*
	ͨ��Rule��Timestamp�ж�rule�Ƿ�Ҫˢ��

	@param const std::map<uint64_t, std::string>& in_mapRuleTime : ����Web�·��õ���Rule
	@param std::vector<uint64_t>& out_vctNeedFetchRule : ���Web��ȡ�Ĳ��Լ�

	*/
	void CompareRuleTime_ToLocalRule(const std::map<uint64_t, uint64_t>& in_mapRuleTime, std::vector<uint64_t>& out_vctNeedFetchRule);

protected:
	/*
	����һ�����׹������֮ǰ����ͬid�Ĺ���ͽ����滻

	@param bool bUpdateToDb : �Ƿ����ݸ�����DB
	true -- �����ݸ�����DB
	false -- �������ݸ�����DB

	@return 0 : ����ɹ�
	@return -1 : ����ʧ��
	*/
	int InsertSingleRule_Withoutlock(rapidjson::Value& docValue, bool bUpdateToDb);

	/*
	ִ�и���sql���

	@return 0 : ���³ɹ�
	@return -1 : ����ʧ��
	*/
	int UpdateSingleRuleToDb(const std::string& strRuleId, 
		const rapidjson::Value& in_docJudgeCond, rapidjson::Value& docValue);

	/*
	���뽻�׹������֮ǰ����ͬid�Ĺ���ͽ����滻
	�ڲ�ʹ�õ�����

	@param rapidjson::Value& docValue : ���׹��򣬿����ǵ�������Ҳ�����ǹ�������
	@param bool bUpdateToDb : �Ƿ����ݸ�����DB
	true -- �����ݸ�����DB
	false -- �������ݸ�����DB

	@return 0 : ����ɹ�
	@return -1 : ����ʧ��
	*/
	int InsertRule_Withoutlock(rapidjson::Value& docValue, bool bUpdateToDb);

};

#endif