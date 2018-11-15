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
按成交规则配置 解析字符串

@return 0 : 替换成功
@return 1 : 无替换对象
@return -1 : 替换失败
*/
int RuleWordProc_SzEzSTEP::ResolveRule(rapidjson::Value& in_docRule,
	std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	FIX::Message& out_fixReport)
{
	static const string ftag("ResolveRule() ");

	if (in_ptrReport->strSenderCompID.empty())
	{
		//	该字段不能为空
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Sz Order ordrec=" << in_ptrReport->strClordid
			<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " 字段[sendercompid]为空";
		return -1;
	}

	if (in_ptrReport->strTargetCompID.empty())
	{
		//	该字段不能为空
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Sz Order ordrec=" << in_ptrReport->strClordid
			<< " ruleid=" << in_ptrReport->tradePolicy.ui64RuleId << " 字段[targetcompid]为空";
		return -1;
	}

	// Begin string
	StgwFixUtil::SetField(out_fixReport, FIX::FIELD::BeginString, in_ptrReport->szfixMsg.getHeader().getField(FIX::FIELD::BeginString));

	StgwFixUtil::SetField(out_fixReport, FIX::FIELD::MsgType, "8");

	// 先交换senderID和targetID
	StgwFixUtil::SetField(out_fixReport, FIX::FIELD::TargetCompID, in_ptrReport->szfixMsg.getHeader().getField(FIX::FIELD::SenderCompID));
	StgwFixUtil::SetField(out_fixReport, FIX::FIELD::SenderCompID, in_ptrReport->szfixMsg.getHeader().getField(FIX::FIELD::TargetCompID));

	// 再添加reportindex
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
			// 深圳STEP回报是Ver1.10
			// 取PartitionNo
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

	// 席位 账号
	std::string strSeat, strAccount;
	GetSeat_Account(in_ptrReport->szfixMsg, strSeat, strAccount);

	// 交易席位
	m_strSeat = strSeat;
	// 交易账户
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

		// 转换为数字
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
				// 按空格切割
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
				// 运算符左边的数字
				double dWord_leftOfSign = 0;
				// 运算符右边的数字
				double dWord_rightOfSign = 0;
				string strVectElem("");
				for (size_t i = 0; i < vecWords.size(); ++i)
				{
					strVectElem = vecWords[i];
					bool bIsOper = IsOperator(strVectElem);
					if (bIsOper)
					{
						// 遇到操作符，取出下一个数字，并计算，计算结果存入左数
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
						// 非操作符，取出数字存入左数
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
				// 遍历Array的Value
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
取的席位、账号、营业部代码
@param msessage 收到的委托消息
@param strSeat 席位
@param strAccount 账号
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
按指定规则 替换字符串
@param const std::string& in_strRuleValue : 规则字符串
@param std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport : 订单数据
@param std::string& out_strWord : 转换后的字符

@return 0 : 替换成功
@return 1 : 无替换对象
@return -1 : 替换失败
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
			// ExecID C16 交易所赋予的执行编号，单个交易日内不重复

			TimeStringUtil::ExRandom15(strTmp);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_szstep_OrderID))
		{
			// OrderID C16 交易所赋予的订单编号，跨交易日不重复
			TimeStringUtil::ExRandom15(strTmp);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_szstep_LocalTimeStamp))
		{
			// 时间，格式为HH:MM:SS
			TimeStringUtil::GetCurrTimeInFixmsgType(strTmp);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_szstep_seat))
		{
			// 交易席位
			out_strWord = m_strSeat;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_szstep_account))
		{
			// 交易账户
			out_strWord = m_strAccount;
		}
		else
		{
			out_strWord = in_strRuleValue;
		}
	}
	else if (in_strRuleValue[0] == simutgw::matchrule::cc_at)
	{
		// 用委托消息的字段 如$ID11，表示11字段
		int iOrderField = 0;
		int iRes = Tgw_StringUtil::String2Int_atoi(in_strRuleValue.substr(1, in_strRuleValue.length() - 1), iOrderField);
		if (0 == iRes)
		{
			// 可以转换为整数
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
按指定规则 替换字符串，转换为四则运算所需的整型数值
@param const std::string& in_strRuleValue : 规则字符串
@param std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport : 订单数据
@param double& out_dIntvalue : 转换后的doubel数值

@return 0 : 替换成功
@return 1 : 无替换对象
@return -1 : 替换失败
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

		// 用委托消息的字段 如$ID11，表示11字段
		int iOrderField = 0;
		int iRes = Tgw_StringUtil::String2Int_atoi(strAtValue, iOrderField);
		if (0 == iRes)
		{
			// 可以转换为整数
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
			// 订单中无改字段
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "convert value=[" << in_strRuleValue
				<< "] to int64_t failed";
			return -1;
		}
	}
	else
	{
		// 订单中无改字段
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
	// price	申报价格，如果该字段小数点后数字超过3位，3位之后必须为0	C8
	out_iIntvalue = in_ptrReport->ui64mOrderPrice;
	}
	else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_qty))
	{
	// qty	申报数量	C8
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