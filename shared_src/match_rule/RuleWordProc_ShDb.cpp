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
按成交规则配置 解析字符串

@return 0 : 替换成功
@return 1 : 无替换对象
@return -1 : 替换失败
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
		// 按空格切割
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
				// 非操作符，取出数字存入左数
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
按指定规则 替换字符串
@param const std::string& in_strRuleValue : 规则字符串
@param std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport : 订单数据
@param std::string& out_strWord : 转换后的字符

@return 0 : 替换成功
@return 1 : 无替换对象
@return -1 : 替换失败
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
			// 记录编号 rec_num		
			string strName = in_ptrReport->strSessionId;
			// 先找session
			if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
			{
				ui64Value = simutgw::g_mapShConns[strName].IncRecNum();
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "上海connection[" << strName
					<< "]丢失";
				return -1;
			}

			out_strWord = sof_string::itostr(ui64Value, strItoa);
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_DATE))
		{
			// 日期，格式为YYYYMMDD
			TimeStringUtil::GetTimeIn_YYYYMMDD(strTmp, 0);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_TIME_C8))
		{
			// 时间，格式为HH:MM:SS
			TimeStringUtil::GetTimeIn_Time_C8(strTmp);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_TIME_C6))
		{
			// 时间，格式为HHMMSS
			TimeStringUtil::GetTimeIn_Time_C6(strTmp);
			out_strWord = strTmp;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_CJBH))
		{
			// 成交编号
			string strName = in_ptrReport->strSessionId;
			// 先找session
			if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
			{
				ui64Value = simutgw::g_mapShConns[strName].IncCjbh();
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "上海connection[" << strName
					<< "]丢失";
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
			// 订单中无改字段
			return -1;
		}

		string strOrdValue = in_ptrReport->jsOrder[strAtValue.c_str()].GetString();
		out_strWord = strOrdValue;

		/*
		if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_reff))
		{
		// reff	会员内部订单号，在整个申报的生命周期中，比如成交回报中，都会附带此数据作为标识字段，柜台系统可以利用此编号进行对应处理。	C10
		out_strWord = in_ptrReport->strClordid;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_acc))
		{
		// acc	证券账户	C10
		out_strWord = in_ptrReport->strAccountId;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_stock))
		{
		// stock	证券代码	C6
		out_strWord = in_ptrReport->strStockID;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_bs))
		{
		// bs	买卖方向，‘B’或者‘b’代表买入，‘S’或者‘s’代表卖出	C1
		out_strWord = in_ptrReport->strSide;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_price))
		{
		// price	申报价格，如果该字段小数点后数字超过3位，3位之后必须为0	C8
		out_strWord = in_ptrReport->strOrderPrice;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_qty))
		{
		// qty	申报数量	C8
		out_strWord = in_ptrReport->strOrderqty_origin;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_owflag))
		{
		// owflag	订单类型标志，该字段取值大小写不敏感	C3
		out_strWord = in_ptrReport->strOrdType;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_ordrec))
		{
		// ordrec	撤单编号	C8
		out_strWord = in_ptrReport->strOrigClordid;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_firmid))
		{
		// firmid	B股结算会员代码，对于A股投资者取值无意义	C5
		out_strWord = in_ptrReport->strConfirmID;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_branchid))
		{
		// branchid	营业部代码	C5
		out_strWord = in_ptrReport->strMarket_branchid;
		}
		else if (0 == in_strRuleValue.compare(simutgw::matchrule::csz_shdb_checkord))
		{
		// checkord	校验码，上交所内部使用	Binary
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
按指定规则 替换字符串，转换为四则运算所需的整型数值
@param const std::string& in_strRuleValue : 规则字符串
@param std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport : 订单数据
@param double& out_dIntvalue : 转换后的doubel数值

@return 0 : 替换成功
@return 1 : 无替换对象
@return -1 : 替换失败
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
			// 订单中无改字段
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