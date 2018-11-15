#ifndef __CONFIG_CLIENT_MSG_KEY_H__
#define __CONFIG_CLIENT_MSG_KEY_H__

#include <string>

namespace simutgw
{
	namespace client
	{
		static const char cstrMsgSeq[] = "msgseq";
		static const char cstrReportSeq[] = "reportseq";
		static const char cstrOrigMsgSeq[] = "origmsgseq";
		static const char cstrMsgKey[] = "key";
		static const char cstrMsgValue[] = "value";
		static const char cstrEngineIdKey[] = "engineId";
		static const char cstrVersionNumKey[] = "versionNo";

		// key
		static const char cstrRegisterKey[] = "saveEngine";
		static const char cstrGetParamKey[] = "getEngineLinkParam";
		static const char cstrEngineStateCheckKey[] = "engineStateCheck";
		static const char cstrSwitchModeKey[] = "changeClientModel";
		static const char cstrRestartKey[] = "restartEngine";
		static const char cstrKey_LinkStrategy[] = "changeLinkStrategy";
		static const char cstrKey_Etfinfo[] = "etfinfo";
		static const char cstrKey_MatchRule[] = "matchRule";
		static const char cstrKey_ChangeLinkRules[] = "changeLinkRules";
		static const char cstrKey_GetMatchRuleContent[] = "getMatchRuleContent";

		// 清算
		static const char cstrKey_SettleAccounts[] = "settleAccounts";
		// 清算文件上传结果消息
		static const char cstrKey_SettleFileUpload[] = "settleFileUpload";
		// 收盘
		static const char cstrKey_DayEnd[] = "dayEnd";

		// match rule
		static const char cstrKey_RuleId[] = "ruleid";
		static const char cstrKey_RuleName[] = "rule_name";
		static const char cstrKey_judge_cond[] = "judge_cond";

		static const char cstrKey_Sh_stock_regex[] = "stock_regex";
		static const char cstrKey_Sh_owflag_regex[] = "owflag_regex";

		static const char cstrKey_Sz_applid_regex[] = "applid_regex"; 
		static const char cstrKey_Sz_msgtype_regex[] = "msgtype_regex";
		static const char cstrKey_rule_timestamp[] = "timestamp";
		static const char cstrKey_rule_check[] = "check";
		static const char cstrKey_rule_confirm[] = "confirm";
		static const char cstrKey_rule_match[] = "match";
		static const char cstrKey_rule_wth[] = "wth";
		static const char cstrKey_rule_settlement[] = "settlement";

		// ETF
		static const char cstrKey_etf_info[] = "etf_info";
		static const char cstrKey_etf_component[] = "etf_component";

		static const std::string cstrKey_Version = "Version";
		static const std::string cstrKey_SecurityID = "SecurityID";
		static const std::string cstrKey_SecurityIDSource = "SecurityIDSource";
		static const std::string cstrKey_Symbol = "Symbol";
		static const std::string cstrKey_FundManagementCompany = "FundManagementCompany";
		static const std::string cstrKey_UnderlyingSecurityID = "UnderlyingSecurityID";
		static const std::string cstrKey_UnderlyingSecurityIDSource = "UnderlyingSecurityIDSource";
		static const std::string cstrKey_CreationRedemptionUnit = "CreationRedemptionUnit";
		static const std::string cstrKey_EstimateCashComponent = "EstimateCashComponent";
		static const std::string cstrKey_MaxCashRatio = "MaxCashRatio";
		static const std::string cstrKey_Publish = "Publish";
		static const std::string cstrKey_Creation = "Creation";
		static const std::string cstrKey_Redemption = "Redemption";
		static const std::string cstrKey_RecordNum = "RecordNum";
		static const std::string cstrKey_TotalRecordNum = "TotalRecordNum";
		static const std::string cstrKey_TradingDay = "TradingDay";
		static const std::string cstrKey_PreTradingDay = "PreTradingDay";
		static const std::string cstrKey_CashComponent = "CashComponent";
		static const std::string cstrKey_NAVperCU = "NAVperCU";
		static const std::string cstrKey_NAV = "NAV";
		static const std::string cstrKey_DividendPerCU = "DividendPerCU";
		static const std::string cstrKey_CreationLimit = "CreationLimit";
		static const std::string cstrKey_RedemptionLimit = "RedemptionLimit";
		static const std::string cstrKey_CreationLimitPerUser = "CreationLimitPerUser";
		static const std::string cstrKey_RedemptionLimitPerUser = "RedemptionLimitPerUser";
		static const std::string cstrKey_NetCreationLimit = "NetCreationLimit";
		static const std::string cstrKey_NetRedemptionLimit = "NetRedemptionLimit";
		static const std::string cstrKey_NetCreationLimitPerUser = "NetCreationLimitPerUser";
		static const std::string cstrKey_NetRedemptionLimitPerUser = "NetRedemptionLimitPerUser";

		//etf Components
		static const char cstrKey_UnderlyingSymbol[] = "UnderlyingSymbol";
		static const char cstrKey_SubstituteFlag[] = "SubstituteFlag";
		static const char cstrKey_ComponentShare[] = "ComponentShare";
		static const char cstrKey_PremiumRatio[] = "PremiumRatio";
		static const char cstrKey_CreationCashSubstitute[] = "CreationCashSubstitute";
		static const char cstrKey_RedemptionCashSubstitute[] = "RedemptionCashSubstitute";
	}
}

#endif