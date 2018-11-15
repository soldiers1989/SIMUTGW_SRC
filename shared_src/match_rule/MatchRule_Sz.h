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
���� �ɽ������������ ���� ����
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
	int GetRule(const FIX::Message& in_fixmsg,
		const std::map<uint64_t, uint64_t>& in_mapLinkRules,
		uint64_t& out_ruleId, std::shared_ptr<struct MATCH_RULE_SZ>& out_rule);

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