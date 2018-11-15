#include "RuleWordProc_ShDb.h"

#include "boost/algorithm/string.hpp"

#include "tool_string/sof_string.h"
#include "tool_string/TimeStringUtil.h"
#include "tool_string/Tgw_StringUtil.h"
#include "simutgw/stgw_config/g_values_biz.h"

RuleWordProc_ShDb::RuleWordProc_ShDb(void)
	:m_scl(keywords::channel = "RuleWordProc_ShDb")
{

}

RuleWordProc_ShDb::~RuleWordProc_ShDb(void)
{

}

/*
���ɽ��������� �����ַ���

@return 0 : �滻�ɹ�
@return 1 : ���滻����
@return -1 : �滻ʧ��
*/
int RuleWordProc_ShDb::ResolveRule(rapidjson::Value& in_docRule,
	const std::string& in_strRuleKey,
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::string& out_strTrans)
{
	static const string ftag("ResolveRule() ");

	int iRes = 0;

	if (!in_docRule.HasMember(in_strRuleKey.c_str())
		|| !in_docRule[in_strRuleKey.c_str()].IsString())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SH Order ordrec=" << in_ptrReport->strClordid
			<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << "in valid key[" << in_strRuleKey << "]";

		return -1;
	}

	string strWords = in_docRule[in_strRuleKey.c_str()].GetString();

	std::vector<string> vecWords;

	try
	{
		// ���ո��и�
		boost::split(vecWords, strWords, boost::is_any_of(simutgw::matchrule::cszBlank));
	}
	catch (exception& e)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SH Order ordrec=" << in_ptrReport->strClordid
			<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << "boost::split exception:" << e.what();
		return -1;
	}

	if (0 == vecWords.size())
	{
		return 1;
	}

	if (1 == vecWords.size())
	{
		iRes = ReplaceWord(vecWords[0], in_ptrReport, out_strTrans);
		return iRes;
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
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SH Order ordrec=" << in_ptrReport->strClordid
						<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId;
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
						BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SH Order ordrec=" << in_ptrReport->strClordid
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
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SH Order ordrec=" << in_ptrReport->strClordid
						<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId;
					return -1;
				}
			}
		}

		Tgw_StringUtil::DoubleToString(dWord_leftOfSign, out_strTrans, 2);

		return 0;
	}

	return -1;
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
int RuleWordProc_ShDb::ReplaceWord(const std::string& in_strRuleValue,
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::string& out_strWord)
{
	static const string ftag("ReplaceWord() ");

	if (in_strRuleValue.empty())
	{
		out_strWord = "";
		return 0;
	}

	uint64_t ui64Value = 0;
	string strItoa("");
	string strTmp("");

	if (in_strRuleValue[0] == simutgw::matchrule::cc_dollar)
	{
		if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_REC_NUM))
		{
			// ��¼��� rec_num		
			string strName = in_ptrReport->strSessionId;
			// ����session
			if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
			{
				ui64Value = simutgw::g_mapShConns[strName].IncRecNum();
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�Ϻ�connection[" << strName
					<< "]��ʧ";
				return -1;
			}

			out_strWord = sof_string::itostr(ui64Value, strItoa);
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_DATE))
		{
			// ���ڣ���ʽΪYYYYMMDD
			TimeStringUtil::GetTimeIn_YYYYMMDD(strTmp, 0);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_TIME_C8))
		{
			// ʱ�䣬��ʽΪHH:MM:SS
			TimeStringUtil::GetTimeIn_Time_C8(strTmp);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_TIME_C6))
		{
			// ʱ�䣬��ʽΪHHMMSS
			TimeStringUtil::GetTimeIn_Time_C6(strTmp);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_CJBH))
		{
			// �ɽ����
			string strName = in_ptrReport->strSessionId;
			// ����session
			if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
			{
				ui64Value = simutgw::g_mapShConns[strName].IncCjbh();
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "�Ϻ�connection[" << strName
					<< "]��ʧ";
				return -1;
			}

			out_strWord = sof_string::itostr(ui64Value, strItoa);
		}
		else
		{
			out_strWord = in_strRuleValue;
		}
	}
	else if (in_strRuleValue[0] == simutgw::matchrule::cc_at)
	{
		string strAtValue = in_strRuleValue.substr(1, in_strRuleValue.length() - 1);

		if (!in_ptrReport->jsOrder.HasMember(strAtValue.c_str()))
		{
			// �������޸��ֶ�
			return -1;
		}

		string strOrdValue = in_ptrReport->jsOrder[strAtValue.c_str()].GetString();
		out_strWord = strOrdValue;

		/*
		if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_reff))
		{
		// reff	��Ա�ڲ������ţ��������걨�����������У�����ɽ��ر��У����ḽ����������Ϊ��ʶ�ֶΣ���̨ϵͳ�������ô˱�Ž��ж�Ӧ����	C10
		out_strWord = in_ptrReport->strClordid;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_acc))
		{
		// acc	֤ȯ�˻�	C10
		out_strWord = in_ptrReport->strAccountId;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_stock))
		{
		// stock	֤ȯ����	C6
		out_strWord = in_ptrReport->strStockID;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_bs))
		{
		// bs	�������򣬡�B�����ߡ�b���������룬��S�����ߡ�s����������	C1
		out_strWord = in_ptrReport->strSide;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_price))
		{
		// price	�걨�۸�������ֶ�С��������ֳ���3λ��3λ֮�����Ϊ0	C8
		out_strWord = in_ptrReport->strOrderPrice;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_qty))
		{
		// qty	�걨����	C8
		out_strWord = in_ptrReport->strOrderqty_origin;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_owflag))
		{
		// owflag	�������ͱ�־�����ֶ�ȡֵ��Сд������	C3
		out_strWord = in_ptrReport->strOrdType;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_ordrec))
		{
		// ordrec	�������	C8
		out_strWord = in_ptrReport->strOrigClordid;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_firmid))
		{
		// firmid	B�ɽ����Ա���룬����A��Ͷ����ȡֵ������	C5
		out_strWord = in_ptrReport->strConfirmID;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_branchid))
		{
		// branchid	Ӫҵ������	C5
		out_strWord = in_ptrReport->strMarket_branchid;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_checkord))
		{
		// checkord	У���룬�Ͻ����ڲ�ʹ��	Binary
		//out_strWord = in_ptrReport->;
		}
		else
		{
		out_strWord = in_strRuleValue;
		}
		*/

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
int RuleWordProc_ShDb::ReplaceWordDouble(const std::string& in_strRuleValue,
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	double& out_dIntvalue)
{
	static const string ftag("ReplaceWordDouble() ");

	if (in_strRuleValue.empty())
	{
		out_dIntvalue = 0;
		return 0;
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
	iRes = Tgw_StringUtil::String2Int64_atoi64(in_strRuleValue, out_iIntvalue);
	if (0 != iRes)
	{
	BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "convert value=[" << in_strRuleValue
	<< "] to int64_t failed";
	return -1;
	}
	}
	*/
	string strSrcValue("");

	if (in_strRuleValue[0] == simutgw::matchrule::cc_at)
	{
		string strAtValue = in_strRuleValue.substr(1, in_strRuleValue.length() - 1);

		if (!in_ptrReport->jsOrder.HasMember(strAtValue.c_str()))
		{
			// �������޸��ֶ�
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "convert value=[" << in_strRuleValue
				<< "] to int64_t failed";
			return -1;
		}

		strSrcValue = in_ptrReport->jsOrder[strAtValue.c_str()].GetString();
	}
	else
	{
		strSrcValue = in_strRuleValue;
	}

	int iRes = Tgw_StringUtil::String2Double_atof(strSrcValue, out_dIntvalue);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "convert value=[" << in_strRuleValue
			<< "] to int64_t failed";
		return -1;
	}


	return 0;
}