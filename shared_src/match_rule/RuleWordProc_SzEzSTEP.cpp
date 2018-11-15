#include "RuleWordProc_SzEzSTEP.h"

#include "boost/algorithm/string.hpp"

#include "tool_string/sof_string.h"
#include "tool_string/TimeStringUtil.h"
#include "tool_string/Tgw_StringUtil.h"
#include "simutgw/stgw_config/g_values_biz.h"
#include "simutgw_config/g_values_sys_run_config.h"

#include "simutgw/stgw_fix_acceptor/StgwFixUtil.h"

RuleWordProc_SzEzSTEP::RuleWordProc_SzEzSTEP(void)
	:m_scl(keywords::channel = "RuleWordProc_SzEzSTEP")
{

}

RuleWordProc_SzEzSTEP::~RuleWordProc_SzEzSTEP(void)
{

}

/*
���ɽ��������� �����ַ���

@return 0 : �滻�ɹ�
@return 1 : ���滻����
@return -1 : �滻ʧ��
*/
int RuleWordProc_SzEzSTEP::ResolveRule(rapidjson::Value& in_docRule,
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	FIX::Message& out_fixReport)
{
	static const string ftag("ResolveRule() ");

	if (in_ptrReport->strSenderCompID.empty())
	{
		//	���ֶβ���Ϊ��
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Sz Order ordrec=" << in_ptrReport->strClordid
			<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " �ֶ�[sendercompid]Ϊ��";
		return -1;
	}

	if (in_ptrReport->strTargetCompID.empty())
	{
		//	���ֶβ���Ϊ��
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Sz Order ordrec=" << in_ptrReport->strClordid
			<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " �ֶ�[targetcompid]Ϊ��";
		return -1;
	}

	// Begin string
	StgwFixUtil::SetField(out_fixReport, FIX::FIELD::BeginString, in_ptrReport->szfixMsg.getHeader().getField(FIX::FIELD::BeginString));

	StgwFixUtil::SetField(out_fixReport, FIX::FIELD::MsgType, "8");

	// �Ƚ���senderID��targetID
	StgwFixUtil::SetField(out_fixReport, FIX::FIELD::TargetCompID, in_ptrReport->szfixMsg.getHeader().getField(FIX::FIELD::SenderCompID));
	StgwFixUtil::SetField(out_fixReport, FIX::FIELD::SenderCompID, in_ptrReport->szfixMsg.getHeader().getField(FIX::FIELD::TargetCompID));

	// �����reportindex
	string strValue("");
	int iRes = StgwFixUtil::GetStringField(in_ptrReport->szfixMsg, FIX::FIELD::SenderCompID, strValue);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Sz Order ordrec=" << in_ptrReport->strClordid
			<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " couldnt find FIX::FIELD::SenderCompID";

		return -1;
	}

	uint64_t ui64Num = 0;
	string strItoa;
	int iPartitionNo = 0;

	if (simutgw::g_mapSzConns.end() != simutgw::g_mapSzConns.find(strValue))
	{
		ui64Num = simutgw::g_mapSzConns[strValue].GetRptIdex();

		if (simutgw::g_bSZ_Step_ver110)
		{
			// ����STEP�ر���Ver1.10
			// ȡPartitionNo
			std::shared_ptr<std::map<int, uint64_t>> prtMapPati = simutgw::g_mapSzConns[strValue].GetPartitionsMap();
			if (nullptr != prtMapPati)
			{
				std::map<int, uint64_t>::iterator it = prtMapPati->begin();
				if (prtMapPati->end() != it)
				{
					iPartitionNo = it->first;
				}
			}
		}
	}
	sof_string::itostr(ui64Num, strValue);
	StgwFixUtil::SetField(out_fixReport, FIX::FIELD::ReportIndex, strValue);

	if (simutgw::g_bSZ_Step_ver110)
	{
		sof_string::itostr(iPartitionNo, strItoa);
		StgwFixUtil::SetField(out_fixReport, FIX::FIELD::PartitionNo, strItoa);
	}

	string strApplID("");
	StgwFixUtil::GetStringField(in_ptrReport->szfixMsg, FIX::FIELD::ApplID, strApplID);
	if (strApplID.empty())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Sz Order ordrec=" << in_ptrReport->strClordid
			<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " couldnt find FIX::FIELD::ApplID";

		return -1;
	}
	StgwFixUtil::SetField(out_fixReport, FIX::FIELD::ApplID, strApplID);

	// ϯλ �˺�
	std::string strSeat, strAccount;
	GetSeat_Account(in_ptrReport->szfixMsg, strSeat, strAccount);

	// ����ϯλ
	m_strSeat = strSeat;
	// �����˻�
	m_strAccount = strAccount;

	iRes = ResolveElement(in_docRule, in_ptrReport, out_fixReport);
	if (0 != iRes)
	{
		return -1;
	}

	return 0;
}

int RuleWordProc_SzEzSTEP::ResolveElement(rapidjson::Value& in_docRule,
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	FIX::FieldMap& out_fixReport)
{
	static const string ftag("ResolveElement() ");

	int iRes = 0;

	rapidjson::Document::MemberIterator iter = in_docRule.MemberBegin();
	int iTranRes = 0;
	int iField = 0;
	// field name
	std::string strField;
	// field value
	std::string strValue;
	// field value split
	std::vector<string> vecWords;
	// word after trans
	std::string strAfterTrans;

	for (; iter != in_docRule.MemberEnd(); ++iter)
	{
		strField.clear();
		strValue.clear();

		// ת��Ϊ����
		strField = iter->name.GetString();
		iTranRes = Tgw_StringUtil::String2Int_atoi(strField, iField);
		if (0 != iTranRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Sz Order ordrec=" << in_ptrReport->strClordid
				<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " Field=" << strField << " not int";
			continue;
		}

		if (iter->value.IsString())
		{
			strValue = iter->value.GetString();
			size_t iLength = strValue.length();

			if (iLength == 0)
			{
				continue;
			}

			try
			{
				vecWords.clear();
				// ���ո��и�
				boost::split(vecWords, strValue, boost::is_any_of(simutgw::matchrule::cszBlank));
			}
			catch (exception& e)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Sz Order ordrec=" << in_ptrReport->strClordid
					<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << "boost::split exception:" << e.what();
				return -1;
			}

			if (0 == vecWords.size())
			{
				StgwFixUtil::SetField(out_fixReport, iField, "");
				continue;
			}

			if (1 == vecWords.size())
			{
				iRes = ReplaceWord(vecWords[0], in_ptrReport, strAfterTrans);
				if (0 == iRes)
				{
					StgwFixUtil::SetField(out_fixReport, iField, strAfterTrans);
				}
				continue;
			}
			else
			{
				// �������ߵ�����
				double dWord_leftOfSign = 0;
				// ������ұߵ�����
				double dWord_rightOfSign = 0;
				string strVectElem("");
				for (size_t i = 0; i < vecWords.size(); ++i)
				{
					strVectElem = vecWords[i];
					bool bIsOper = IsOperator(strVectElem);
					if (bIsOper)
					{
						// ������������ȡ����һ�����֣������㣬��������������
						++i;
						if (i >= vecWords.size())
						{
							break;
						}

						iRes = ReplaceWordDouble(vecWords[i], in_ptrReport, dWord_rightOfSign);
						if (0 != iRes)
						{
							BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SZ Order ordrec=" << in_ptrReport->strClordid
								<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << "ReplaceWordDouble " << vecWords[i];
							return -1;
						}

						if (0 == strVectElem.compare(simutgw::matchrule::cszSign_Add))
						{
							dWord_leftOfSign += dWord_rightOfSign;
						}
						else if (0 == strVectElem.compare(simutgw::matchrule::cszSign_Minus))
						{
							dWord_leftOfSign -= dWord_rightOfSign;
						}
						else if (0 == strVectElem.compare(simutgw::matchrule::cszSign_Times))
						{
							dWord_leftOfSign *= dWord_rightOfSign;
						}
						else if (0 == strVectElem.compare(simutgw::matchrule::cszSign_Division))
						{
							if (0 == dWord_rightOfSign)
							{
								BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SZ Order ordrec=" << in_ptrReport->strClordid
									<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " try to divid zero!";
								return -1;
							}

							dWord_leftOfSign /= dWord_rightOfSign;
						}
					}
					else
					{
						// �ǲ�������ȡ�����ִ�������
						iRes = ReplaceWordDouble(vecWords[i], in_ptrReport, dWord_leftOfSign);
						if (0 != iRes)
						{
							BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SZ Order ordrec=" << in_ptrReport->strClordid
								<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId;
							return -1;
						}
					}
				}

				Tgw_StringUtil::DoubleToString(dWord_leftOfSign, strAfterTrans, 2);

				StgwFixUtil::SetField(out_fixReport, iField, strAfterTrans);
			}
		}
		else if (iter->value.IsArray())
		{
			rapidjson::Document::ValueIterator iterValue = iter->value.Begin();
			for (; iterValue != iter->value.End(); ++iterValue)
			{
				// ����Array��Value
				rapidjson::Document::MemberIterator iterMember = iterValue->MemberBegin();
				std::string strFieldSub = iterMember->name.GetString();
				int iFieldSub = 0;
				Tgw_StringUtil::String2Int_atoi(strFieldSub, iFieldSub);

				FIX::Group group(iField, iFieldSub);
				iRes = ResolveElement(*iterValue, in_ptrReport, group);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SZ Order ordrec=" << in_ptrReport->strClordid
						<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId;
					return -1;
				}

				out_fixReport.addGroup(iField, group);
			}
		}
		else
		{

		}
	}

	return 0;
}

/*
ȡ��ϯλ���˺š�Ӫҵ������
@param msessage �յ���ί����Ϣ
@param strSeat ϯλ
@param strAccount �˺�
*/
int RuleWordProc_SzEzSTEP::GetSeat_Account(const FIX::Message& message,
	std::string& strSeat,
	std::string& strAccount)
{
	static const std::string ftag("GetSeat_Account() ");

	try
	{
		FIX::FieldMap& msg = (FIX::FieldMap&)message;

		std::string strMarket_branchId;
		int iRes = StgwFixUtil::GetNopartyIds(msg, strSeat, strAccount, strMarket_branchId);
		if (0 != iRes)
		{
			return -1;
		}
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

/*
��ָ������ �滻�ַ���
@param const std::string& in_strRuleValue : �����ַ���
@param std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport : ��������
@param std::string& out_strWord : ת������ַ�

@return 0 : �滻�ɹ�
@return 1 : ���滻����
@return -1 : �滻ʧ��
*/
int RuleWordProc_SzEzSTEP::ReplaceWord(const std::string& in_strRuleValue,
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::string& out_strWord)
{
	static const string ftag("ReplaceWord() ");

	if (in_strRuleValue.empty())
	{
		return 1;
	}

	uint64_t ui64Value = 0;
	string strItoa("");
	string strTmp("");

	if (in_strRuleValue[0] == simutgw::matchrule::cc_dollar)
	{
		if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_szstep_ExecID))
		{
			// ExecID C16 �����������ִ�б�ţ������������ڲ��ظ�

			TimeStringUtil::ExRandom15(strTmp);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_szstep_OrderID))
		{
			// OrderID C16 ����������Ķ�����ţ��罻���ղ��ظ�
			TimeStringUtil::ExRandom15(strTmp);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_szstep_LocalTimeStamp))
		{
			// ʱ�䣬��ʽΪHH:MM:SS
			TimeStringUtil::GetCurrTimeInFixmsgType(strTmp);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_szstep_seat))
		{
			// ����ϯλ
			out_strWord = m_strSeat;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_szstep_account))
		{
			// �����˻�
			out_strWord = m_strAccount;
		}
		else
		{
			out_strWord = in_strRuleValue;
		}
	}
	else if (in_strRuleValue[0] == simutgw::matchrule::cc_at)
	{
		// ��ί����Ϣ���ֶ� ��$ID11����ʾ11�ֶ�
		int iOrderField = 0;
		int iRes = Tgw_StringUtil::String2Int_atoi(in_strRuleValue.substr(1, in_strRuleValue.length() - 1), iOrderField);
		if (0 == iRes)
		{
			// ����ת��Ϊ����
			std::string strOrderValue("");
			int iRes = StgwFixUtil::GetStringField(in_ptrReport->szfixMsg, iOrderField, strOrderValue);
			if (0 != iRes)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SZ Order ordrec=" << in_ptrReport->strClordid
					<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " couldn't getFiled=" << in_strRuleValue;
				return -1;
			}

			out_strWord = strOrderValue;
		}
		else
		{
			out_strWord = in_strRuleValue;
		}
	}
	else
	{
		out_strWord = in_strRuleValue;
	}

	return 0;
}

/*
��ָ������ �滻�ַ�����ת��Ϊ�������������������ֵ
@param const std::string& in_strRuleValue : �����ַ���
@param std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport : ��������
@param double& out_dIntvalue : ת�����doubel��ֵ

@return 0 : �滻�ɹ�
@return 1 : ���滻����
@return -1 : �滻ʧ��
*/
int RuleWordProc_SzEzSTEP::ReplaceWordDouble(const std::string& in_strRuleValue,
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	double& out_dIntvalue)
{
	static const string ftag("ReplaceWordDouble() ");

	if (in_strRuleValue.empty())
	{
		out_dIntvalue = 0;
		return 0;
	}

	string strSrcValue("");

	if (in_strRuleValue[0] == simutgw::matchrule::cc_at)
	{
		string strAtValue = in_strRuleValue.substr(1, in_strRuleValue.length() - 1);

		// ��ί����Ϣ���ֶ� ��$ID11����ʾ11�ֶ�
		int iOrderField = 0;
		int iRes = Tgw_StringUtil::String2Int_atoi(strAtValue, iOrderField);
		if (0 == iRes)
		{
			// ����ת��Ϊ����
			int iResGetField = StgwFixUtil::GetStringField(in_ptrReport->szfixMsg, iOrderField, strSrcValue);
			if (0 != iResGetField)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SZ Order ordrec=" << in_ptrReport->strClordid
					<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " couldn't getFiled=" << in_strRuleValue;
				return -1;
			}
		}
		else
		{
			// �������޸��ֶ�
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "convert value=[" << in_strRuleValue
				<< "] to int64_t failed";
			return -1;
		}
	}
	else
	{
		// �������޸��ֶ�
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "convert value=[" << in_strRuleValue
			<< "] to int64_t failed";
		return -1;
	}

	int iRes = Tgw_StringUtil::String2Double_atof(strSrcValue, out_dIntvalue);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "convert value=[" << in_strRuleValue
			<< "] to int64_t failed";
		return -1;
	}

	/*
	uint64_t ui64Value = 0;
	string strItoa("");
	string strTmp("");
	if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_price))
	{
	// price	�걨�۸�������ֶ�С��������ֳ���3λ��3λ֮�����Ϊ0	C8
	out_iIntvalue = in_ptrReport->ui64mOrderPrice;
	}
	else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_qty))
	{
	// qty	�걨����	C8
	out_iIntvalue = in_ptrReport->ui64Orderqty_origin;
	}
	else
	{
	int iRes = Tgw_StringUtil::String2Int64_atoi64(in_strRuleValue, out_iIntvalue);
	if (0 != iRes)
	{
	BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "convert value=[" << in_strRuleValue
	<< "] to int64_t failed";
	return -1;
	}
	}
	*/

	return 0;
}