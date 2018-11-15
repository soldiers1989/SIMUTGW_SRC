#include "MatchRule_Sz.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/sof_string.h"
#include "tool_json/RapidJsonHelper_tgw.h"

#include "simutgw_config/define_db_names.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/tcp_simutgw_client/config_client_msgkey.h"
#include "simutgw/stgw_fix_acceptor/StgwFixUtil.h"

using namespace std;

MatchRule_Sz::MatchRule_Sz(void)
	:m_scl(keywords::channel = "MatchRule_Sz")
{
}

MatchRule_Sz::~MatchRule_Sz(void)
{
}

/*
����һ�����׹������֮ǰ����ͬid�Ĺ���ͽ����滻

@param bool bUpdateToDb : �Ƿ����ݸ�����DB
true -- �����ݸ�����DB
false -- �������ݸ�����DB

@return 0 : ����ɹ�
@return -1 : ����ʧ��
*/
int MatchRule_Sz::InsertSingleRule_Withoutlock(rapidjson::Value& docValue, bool bUpdateToDb)
{
	static const string ftag("InsertSingleRule_Withoutlock() ");

	if (!docValue.HasMember(simutgw::client::cstrKey_RuleId)
		|| !docValue[simutgw::client::cstrKey_RuleId].IsString())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error ruleid";
		return -1;
	}

	string strRuleId = docValue[simutgw::client::cstrKey_RuleId].GetString();

	uint64_t ui64RuleId = 0;

	int iRes = Tgw_StringUtil::String2UInt64_strtoui64(strRuleId, ui64RuleId);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ruleid not legal";
		return -1;
	}

	// regex string objects
	if (docValue.HasMember(simutgw::client::cstrKey_judge_cond)
		&& docValue[simutgw::client::cstrKey_judge_cond].IsObject())
	{
		for (rapidjson::Value::ConstMemberIterator citer = docValue[simutgw::client::cstrKey_judge_cond].MemberBegin();
			docValue[simutgw::client::cstrKey_judge_cond].MemberEnd() != citer; ++citer)
		{
			const rapidjson::Value& name_json = citer->name;
			const rapidjson::Value& value_json = citer->value;
			if (!value_json.IsString())
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "regex for " << name_json.GetString() << " all empty";
				return -1;
			}
		}
	}

	// timestamp
	uint64_t ui64Timestamp = 0;
	if (docValue.HasMember(simutgw::client::cstrKey_rule_timestamp)
		&& docValue[simutgw::client::cstrKey_rule_timestamp].IsInt())
	{
		ui64Timestamp = docValue[simutgw::client::cstrKey_rule_timestamp].GetInt();
	}

	if (0 == ui64Timestamp)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "timestamp empty";
		return -1;
	}

	MAP_SZ_RULES::iterator itRule = m_mapSzRules.find(ui64RuleId);

	if (m_mapSzRules.end() == itRule)
	{
		// δ�ҵ�
		std::shared_ptr<struct MATCH_RULE_SZ> ptr(new struct MATCH_RULE_SZ());

		iRes = ptr->SetRegex(ui64RuleId, docValue[simutgw::client::cstrKey_judge_cond], ui64Timestamp);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ruleid=" << ui64RuleId << " set failed";
			return -1;
		}

		if (docValue.HasMember(simutgw::client::cstrKey_rule_confirm)
			&& docValue[simutgw::client::cstrKey_rule_confirm].IsObject())
		{
			ptr->SetRuleConfirm(docValue[simutgw::client::cstrKey_rule_confirm]);
		}

		if (docValue.HasMember(simutgw::client::cstrKey_rule_match)
			&& docValue[simutgw::client::cstrKey_rule_match].IsObject())
		{
			ptr->SetRuleMatch(docValue[simutgw::client::cstrKey_rule_confirm]);
		}

		m_mapSzRules.insert(std::pair<uint64_t, std::shared_ptr<struct MATCH_RULE_SZ>>(ui64RuleId, ptr));
	}
	else
	{
		// �ҵ�
		iRes = itRule->second->SetRegex(ui64RuleId, docValue[simutgw::client::cstrKey_judge_cond], ui64Timestamp);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ruleid=" << ui64RuleId << " set failed";
			return -1;
		}

		if (docValue.HasMember(simutgw::client::cstrKey_rule_confirm)
			&& docValue[simutgw::client::cstrKey_rule_confirm].IsObject())
		{
			itRule->second->SetRuleConfirm(docValue[simutgw::client::cstrKey_rule_confirm]);
		}

		if (docValue.HasMember(simutgw::client::cstrKey_rule_match)
			&& docValue[simutgw::client::cstrKey_rule_match].IsObject())
		{
			itRule->second->SetRuleMatch(docValue[simutgw::client::cstrKey_rule_match]);
		}
	}

	if (bUpdateToDb)
	{
		iRes = UpdateSingleRuleToDb(strRuleId, docValue[simutgw::client::cstrKey_judge_cond], docValue);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "UpdateSingleRuleToDb failed";
			return -1;
		}
	}

	return 0;
}

/*
ִ�и���sql���

@return 0 : ���³ɹ�
@return -1 : ����ʧ��
*/
int MatchRule_Sz::UpdateSingleRuleToDb(const std::string& strRuleId,
	const rapidjson::Value& in_docJudgeCond, rapidjson::Value& docValue)
{
	static const std::string ftag("UpdateSingleRuleToDb() ");

	try{
		//��mysql���ӳ�ȡ����
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//ȡ����mysql����ΪNULL

			//�黹����
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Get Connection is NULL";

			return -1;
		}

		mysqlConn->StartTransaction();

		string strUpdateSql = "REPLACE INTO ";
		strUpdateSql += simutgw::g_DBTable_match_rule_sz;
		strUpdateSql += "(`idmatch_rule_sz`,`rule_name`,`judge_cond`,`rule_check`,"
			"`rule_confirm`,`rule_match`,`rule_wth`,`rule_settlement`,`is_delete`,"
			"`changer`,`change_time`) VALUES ('";

		// `idmatch_rule_sh`,`rule_name`,`regex_applid`,`rule_check`,
		strUpdateSql += strRuleId;

		string strValueTmp = "";
		if (!docValue.HasMember(simutgw::client::cstrKey_RuleName)
			|| !docValue[simutgw::client::cstrKey_RuleName].IsString())
		{
			strUpdateSql += "',NULL";
		}
		else
		{
			strValueTmp = docValue[simutgw::client::cstrKey_RuleName].GetString();
			if (strValueTmp.empty())
			{
				strUpdateSql += "',NULL";
			}
			else
			{
				strUpdateSql += "','";
				strUpdateSql += strValueTmp;
				strUpdateSql += "'";
			}
		}

		rapidjson::StringBuffer sbBuf;
		rapidjson::Writer<rapidjson::StringBuffer> jWriter(sbBuf);
		in_docJudgeCond.Accept(jWriter);

		string strJudgeCondValue = sbBuf.GetString();

		if (strJudgeCondValue.empty())
		{
			strUpdateSql += ",NULL";
		}
		else
		{
			strUpdateSql += ",'";
			strUpdateSql += strJudgeCondValue;
			strUpdateSql += "'";
		}

		if (!docValue.HasMember(simutgw::client::cstrKey_rule_check) || docValue[simutgw::client::cstrKey_rule_check].IsNull())
		{
			strUpdateSql += ",NULL";
		}
		else
		{
			strValueTmp = "";
			Tgw_StringUtil::GetJsonString(docValue[simutgw::client::cstrKey_rule_check], strValueTmp);
			if (strValueTmp.empty() || 0 == strValueTmp.compare("\"\""))
			{
				strUpdateSql += ",NULL";
			}
			else
			{
				strUpdateSql += ",'";
				strUpdateSql += strValueTmp;
				strUpdateSql += "'";
			}
		}

		// `rule_confirm`,`rule_match`,`rule_wth`,`rule_settlement`,`is_delete`,
		if (!docValue.HasMember(simutgw::client::cstrKey_rule_confirm) || docValue[simutgw::client::cstrKey_rule_confirm].IsNull())
		{
			strUpdateSql += ",NULL";
		}
		else
		{
			strValueTmp = "";
			Tgw_StringUtil::GetJsonString(docValue[simutgw::client::cstrKey_rule_confirm], strValueTmp);
			if (strValueTmp.empty() || 0 == strValueTmp.compare("\"\""))
			{
				strUpdateSql += ",NULL";
			}
			else
			{
				strUpdateSql += ",'";
				strUpdateSql += strValueTmp;
				strUpdateSql += "'";
			}
		}

		if (!docValue.HasMember(simutgw::client::cstrKey_rule_match) || docValue[simutgw::client::cstrKey_rule_match].IsNull())
		{
			strUpdateSql += ",NULL";
		}
		else
		{
			strValueTmp = "";
			Tgw_StringUtil::GetJsonString(docValue[simutgw::client::cstrKey_rule_match], strValueTmp);
			if (strValueTmp.empty() || 0 == strValueTmp.compare("\"\""))
			{
				strUpdateSql += ",NULL";
			}
			else
			{
				strUpdateSql += ",'";
				strUpdateSql += strValueTmp;
				strUpdateSql += "'";
			}
		}

		if (!docValue.HasMember(simutgw::client::cstrKey_rule_wth) || docValue[simutgw::client::cstrKey_rule_wth].IsNull())
		{
			strUpdateSql += ",NULL";
		}
		else
		{
			strValueTmp = "";
			Tgw_StringUtil::GetJsonString(docValue[simutgw::client::cstrKey_rule_wth], strValueTmp);
			if (strValueTmp.empty() || 0 == strValueTmp.compare("\"\""))
			{
				strUpdateSql += ",NULL";
			}
			else
			{
				strUpdateSql += ",'";
				strUpdateSql += strValueTmp;
				strUpdateSql += "'";
			}
		}

		if (!docValue.HasMember(simutgw::client::cstrKey_rule_settlement) || docValue[simutgw::client::cstrKey_rule_settlement].IsNull())
		{
			strUpdateSql += ",NULL";
		}
		else
		{
			strValueTmp = "";
			Tgw_StringUtil::GetJsonString(docValue[simutgw::client::cstrKey_rule_settlement], strValueTmp);
			if (strValueTmp.empty() || 0 == strValueTmp.compare("\"\""))
			{
				strUpdateSql += ",NULL";
			}
			else
			{
				strUpdateSql += ",'";
				strUpdateSql += strValueTmp;
				strUpdateSql += "'";
			}
		}

		// `changer`,`change_time`
		strUpdateSql += ",0,'simutgw',from_unixtime(";
		string strItoa;
		sof_string::itostr(docValue[simutgw::client::cstrKey_rule_timestamp].GetInt(), strItoa);
		strUpdateSql += strItoa;
		strUpdateSql += "));";

		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		int iRes = mysqlConn->Query(strUpdateSql, &pResultSet, ulAffectedRows);
		if (2 == iRes)
		{
			// �Ǹ���
			if (2 < ulAffectedRows)
			{
				// ʧ��
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "����[" << strUpdateSql
					<< "]�õ�AffectedRows=" << ulAffectedRows;

				mysqlConn->RollBack();
				simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
				return -1;
			}
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "����[" << strUpdateSql
				<< "]�õ�Res=" << iRes;
			mysqlConn->RollBack();
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
			return -1;
		}

		mysqlConn->Commit();

		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}
	catch (...)
	{
		EzLog::e(ftag, "δ֪�쳣");
		return -1;
	}

	return 0;
}

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
int MatchRule_Sz::InsertRule_Withoutlock(rapidjson::Value& docValue, bool bUpdateToDb)
{
	static const string ftag("InsertRule_Withoutlock() ");

	if (!docValue.IsObject() && !docValue.IsArray())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error input value";
		return -1;
	}

	int iRes = 0;
	if (docValue.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < docValue.Size(); ++i)
		{
			iRes = InsertSingleRule_Withoutlock(docValue[i], bUpdateToDb);
			if (0 != iRes)
			{
				return -1;
			}
		}
	}
	else
	{
		iRes = InsertSingleRule_Withoutlock(docValue, bUpdateToDb);
		if (0 != iRes)
		{
			return -1;
		}
	}

	return 0;
}

/*
����һ�����׹������֮ǰ����ͬid�Ĺ���ͽ����滻

@param rapidjson::Value& docValue : ���׹��򣬿����ǵ�������Ҳ�����ǹ�������
@param bool bUpdateToDb : �Ƿ����ݸ�����DB
true -- �����ݸ�����DB
false -- �������ݸ�����DB

@return 0 : ����ɹ�
@return -1 : ����ʧ��
*/
int MatchRule_Sz::InsertRule(rapidjson::Value& docValue, bool bUpdateToDb)
{
	static const string ftag("InsertRule() ");

	std::lock_guard<std::mutex> lock(m_mutexMapRules);

	int iRes = InsertRule_Withoutlock(docValue, bUpdateToDb);
	if (0 != iRes)
	{
		return -1;
	}

	return 0;
}


/*
�����ݿ��������� �ɽ��������� �Ϻ�

@return 0 : ��ȡ�ɹ�
@return -1 : ��ȡʧ��
*/
int MatchRule_Sz::ReloadFromDb(void)
{
	static const string ftag("ReloadFromDb() ");

	std::lock_guard<std::mutex> lock(m_mutexMapRules);

	m_mapSzRules.clear();

	string strQueryString;
	int iReturn = 0;

	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		// ��ѯ�û���Ϣ�����
		strQueryString = "SELECT `idmatch_rule_sz`,`judge_cond`,`rule_check`,`rule_confirm`,"
			"`rule_match`,`rule_wth`,`rule_settlement`,unix_timestamp(`change_time`) AS RULE_CHANGE_TIME FROM ";
		strQueryString += simutgw::g_DBTable_match_rule_sz;
		strQueryString += " WHERE `is_delete`=0;";

		//��mysql���ӳ�ȡ����
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//ȡ����mysql����ΪNULL

			//�黹����
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Get Mysql Connection is NULL";

			return -1;
		}

		rapidjson::Document docRuleArray(rapidjson::Type::kArrayType);
		rapidjson::Document::AllocatorType &allocator = docRuleArray.GetAllocator();
		string strQueryValue("");

		int iRes = mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
			while (0 != mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				rapidjson::Value jsonRuleElem(rapidjson::kObjectType);

				// `idmatch_rule_sz`,
				strQueryValue = mapRowData["idmatch_rule_sz"].strValue;
				if (mapRowData["idmatch_rule_sz"].bIsNull || strQueryValue.empty())
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "invalid idmatch_rule_sz";
					iReturn = -1;
					break;
				}
				jsonRuleElem.AddMember(simutgw::client::cstrKey_RuleId, rapidjson::Value(strQueryValue.c_str(), allocator), allocator);

				// `judge_cond`,
				strQueryValue = mapRowData["judge_cond"].strValue;
				if (!strQueryValue.empty())
				{
					rapidjson::Document docData_judge_cond;
					if (docData_judge_cond.Parse<0>(strQueryValue.c_str()).HasParseError() || docData_judge_cond.IsNull())
					{
						//������Ϣʧ��
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Parse raw string to Json failed,string=[" << strQueryValue
							<< "] idmatch_rule_sz=" << mapRowData["idmatch_rule_sz"].strValue;
						return -1;
					}
					jsonRuleElem.AddMember(simutgw::client::cstrKey_judge_cond, rapidjson::Value(docData_judge_cond, allocator), allocator);
				}

				// `rule_check`,				

				// `rule_confirm`,"
				strQueryValue = mapRowData["rule_confirm"].strValue;

				if (!strQueryValue.empty())
				{
					rapidjson::Document docData_rule_confirm;
					if (docData_rule_confirm.Parse<0>(strQueryValue.c_str()).HasParseError() || docData_rule_confirm.IsNull())
					{
						//������Ϣʧ��
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Parse raw string to Json failed,string=[" << strQueryValue
							<< "] idmatch_rule_sz=" << mapRowData["idmatch_rule_sz"].strValue;
						return -1;
					}
					jsonRuleElem.AddMember(simutgw::client::cstrKey_rule_confirm, rapidjson::Value(docData_rule_confirm, allocator), allocator);
				}

				// "`rule_match`,
				strQueryValue = mapRowData["rule_match"].strValue;

				if (!strQueryValue.empty())
				{
					rapidjson::Document docData_rule_match;
					if (docData_rule_match.Parse<0>(strQueryValue.c_str()).HasParseError() || docData_rule_match.IsNull())
					{
						//������Ϣʧ��
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Parse raw string to Json failed,string=[" << strQueryValue
							<< "] idmatch_rule_sz=" << mapRowData["idmatch_rule_sz"].strValue;
						return -1;
					}
					jsonRuleElem.AddMember(simutgw::client::cstrKey_rule_match, rapidjson::Value(docData_rule_match, allocator), allocator);
				}

				// `rule_wth`,
				// `rule_settlement`

				// `change_time`
				strQueryValue = mapRowData["RULE_CHANGE_TIME"].strValue;
				uint64_t ui64Timestamp = 0;
				int iResTran = Tgw_StringUtil::String2UInt64_strtoui64(strQueryValue, ui64Timestamp);
				if (0 != iResTran)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Parse raw string to Json failed,string=[" << strQueryValue
						<< "] idmatch_rule_sz=" << mapRowData["idmatch_rule_sz"].strValue;
					return -1;
				}
				jsonRuleElem.AddMember(simutgw::client::cstrKey_rule_timestamp, ui64Timestamp, allocator);

				// add to array
				docRuleArray.PushBack(jsonRuleElem, allocator);
			}

			// �ͷ�
			mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "����[" << strQueryString << "]�õ�" << iRes;
			iReturn = -1;
		}

		// �ͷ�
		mysqlConn->FreeResult(&pResultSet);
		pResultSet = NULL;

		//�黹����
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		if (0 != iReturn)
		{
			return -1;
		}

		iRes = InsertRule_Withoutlock(docRuleArray, false);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error when InsertRule sz";
			return -1;
		}

		return 0;

	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
}

/*
��ȡ�ɽ���������

@param const std::map<uint64_t, uint64_t>& in_mapLinkRules : �ɽ����ú�ͨ���Ĺ�ϵ

@return 0 : ��ȡ�ɹ�
@return -1 : ��ȡʧ��
*/
int MatchRule_Sz::GetRule(const FIX::Message& in_fixmsg,
	const std::map<uint64_t, uint64_t>& in_mapLinkRules,
	uint64_t& out_ruleId, std::shared_ptr<struct MATCH_RULE_SZ>& out_rule)
{
	static const string ftag("GetRule() ");

	std::lock_guard<std::mutex> lock(m_mutexMapRules);

	bool bIsMapLinkRulesEmpty = in_mapLinkRules.empty();

	MAP_SZ_RULES::iterator itRule = m_mapSzRules.begin();
	for (; m_mapSzRules.end() != itRule; ++itRule)
	{
		if (nullptr == itRule->second)
		{
			BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << "find empty content, id=" << itRule->first;
			continue;
		}

		if (!bIsMapLinkRulesEmpty)
		{
			// �ɽ����ú�ͨ���Ĺ�ϵ ��Ϊ�գ���Ҫ���б�Ѱ��
			std::map<uint64_t, uint64_t>::const_iterator itLinkRule = in_mapLinkRules.find(itRule->first);
			if (in_mapLinkRules.end() == itLinkRule)
			{
				// �Ǹ�ͨ��ָ����rule
				continue;
			}
		}

		string strOrdValue("");
		// ���б��ʽ
		bool bRegexHit = false;
		// iterator
		std::map<int, std::regex>::const_iterator citRegex;
		for (citRegex = itRule->second->mapRegexs.cbegin();
			itRule->second->mapRegexs.cend() != citRegex;
			++citRegex)
		{
			try
			{
				int iRes = StgwFixUtil::GetStringField(in_fixmsg, citRegex->first, strOrdValue);
				if (0 != iRes)
				{
					return -1;
				}
			}
			catch (...)
			{
				return -1;
			}

			bRegexHit = std::regex_match(strOrdValue, citRegex->second);
			if (!bRegexHit)
			{
				// �����е�rule
				break;
			}
		}

		if (bRegexHit)
		{
			out_ruleId = itRule->first;
			out_rule = itRule->second;
			return 0;
		}
	}

	return -1;
}

/*
ͨ��Rule��Timestamp�ж�rule�Ƿ�Ҫˢ��

@param const std::map<uint64_t, std::string>& in_mapRuleTime : ����Web�·��õ���Rule
@param std::vector<uint64_t>& out_vctNeedFetchRule : ���Web��ȡ�Ĳ��Լ�

*/
void MatchRule_Sz::CompareRuleTime_ToLocalRule(const std::map<uint64_t, uint64_t>& in_mapRuleTime, std::vector<uint64_t>& out_vctNeedFetchRule)
{
	static const string ftag("CompareRuleTime_ToLocalRule() ");

	std::lock_guard<std::mutex> lock(m_mutexMapRules);

	std::map<uint64_t, uint64_t>::const_iterator citWebRuleTime;
	MAP_SZ_RULES::iterator itLocalRule;
	for (citWebRuleTime = in_mapRuleTime.begin(); in_mapRuleTime.end() != citWebRuleTime; ++citWebRuleTime)
	{
		itLocalRule = m_mapSzRules.find(citWebRuleTime->first);
		if (m_mapSzRules.end() == itLocalRule)
		{
			// δ�ڱ����ҵ������Web����
			out_vctNeedFetchRule.push_back(citWebRuleTime->first);
		}
		else
		{
			// �ҵ���ͬRuleId���Ƚ�ʱ���
			if (citWebRuleTime->second != itLocalRule->second->ui64Timestamp)
			{
				// ʱ�����ͬ�����Web����
				out_vctNeedFetchRule.push_back(citWebRuleTime->first);
			}
			else
			{
				// ʱ�����ͬ��do nothing
			}
		}
	}

	return;
}