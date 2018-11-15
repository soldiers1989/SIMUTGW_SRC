#ifndef __CON_ETF_INFO_H__
#define __CON_ETF_INFO_H__

#include <string>
#include <vector>
#include <stdint.h>

#include "config/conf_msg.h"

namespace simutgw
{
	//申购和赎回允许状态
	//0 - 不允许申购 / 赎回
	//1 - 申购和赎回皆允许
	//2 - 仅允许申购
	//3 - 仅允许赎回
	enum emSHCreRedState
	{
		NotCre_AND_NotRed = 0,
		Cre_AND_Red = 1,
		Cre_AND_NotRed,
		NotCre_AND_Red
	};

	//0表示禁止现金替代（必须有证券）
	//1表示可以进行现金替代（先用证券，证券不足的话用现金替代）
	//2表示必须用现金替代。
	enum emSubstituteFlag
	{
		Only_Security = 0,
		Security_And_Cash = 1,
		Only_Cash = 2
	};

	// 深圳etf成分股信息
	struct SzETFComponent
	{
		//成份股列表 Components
		//→ 成份股信息 Component
		//→ → 证券代码 UnderlyingSecurityID C8
		std::string strUnderlyingSecurityID;

		//→ → 证券代码源 UnderlyingSecurityIDSource C4 101 = 上海证券交易所102 = 深圳证券交易所
		//103 = 香港交易所9999 = 其他
		std::string strUnderlyingSecurityIDSource;

		//→ → 证券简称 UnderlyingSymbol C40
		std::string strUnderlyingSymbol;
		
		//→ → 成份证券数 ComponentShare N15(2) 每个申购篮子中该成份证券的数量。
		//此字段只有现金替代标志为‘0’或‘1’时才有效
		uint64_t ui64ComponentShare;

		//→ → 现金替代标志SubstituteFlag C1 0 = 禁止现金替代（必须有证券）
		//1 = 可以进行现金替代（先用证券，证券不足时差额部分用现金替代）2 = 必须用现金替代
		int iSubstituteFlag;

		//→ → 溢价比例 PremiumRatio N7(5) 证券用现金进行替代的时候，计算价格时增加的比例。
		//例如：2.551％在文件中用 0.02551 表示；2.1%在文件中用 0.02100 表示。此字段只有现金替代标志为‘1’时才有效
		double dPremiumRatio;

		//→ → 申购替代金额CreationCashSubstitute N18(4) 当某只证券必须用现金替代的时候，
		//申购时该证券所需总金额此字段只有当现金替代标志为‘2’时才有效
		uint64_t_Money ui64mCreationCashSubstitute;

		//→ → 赎回替代金额RedemptionCashSubstitute N18(4) 当某只证券必须用现金替代的时候，
		//赎回时对应该证券返还的总金额。例如： 2000 在文件中用 2000.0000表示。
		//对于跨境 ETF、跨市场 ETF、黄金 ETF 和现金债券 ETF，该字段为 0.0000。此字段只有当现金替代标志为‘2’时才有效
		uint64_t_Money ui64mRedemptionCashSubstitute;

		void Copy(const struct SzETFComponent& orig)
		{
			strUnderlyingSecurityID = orig.strUnderlyingSecurityID;
			strUnderlyingSecurityIDSource = orig.strUnderlyingSecurityIDSource;
			strUnderlyingSymbol = orig.strUnderlyingSymbol;
			iSubstituteFlag = orig.iSubstituteFlag;
			ui64ComponentShare = orig.ui64ComponentShare;
			dPremiumRatio = orig.dPremiumRatio;
			ui64mCreationCashSubstitute = orig.ui64mCreationCashSubstitute;
			ui64mRedemptionCashSubstitute = orig.ui64mRedemptionCashSubstitute;
		}

		SzETFComponent()
		{
			iSubstituteFlag = 0;
			ui64ComponentShare = 0;
			dPremiumRatio = 0.0;
			ui64mCreationCashSubstitute = 0;
			ui64mRedemptionCashSubstitute = 0;
		}

		SzETFComponent(const struct SzETFComponent& orig)
		{
			Copy(orig);
		}

		SzETFComponent& operator =(const struct SzETFComponent& orig)
		{
			Copy(orig);
			return *this;
		}
	};

	// 深圳etf信息
	struct SzETF
	{
		// 版本号 Version C8 固定值1.0
		std::string strVersion;

		// 证券代码 SecurityID C8
		std::string strSecurityID;

		// 证券代码源 SecurityIDSource C4 101 = 上海证券交易所102 = 深圳证券交易所
		//103 = 香港交易所9999 = 其他
		std::string strSecurityIDSource;

		// 基金名称 Symbol C40
		std::string strSymbol;

		// 基金公司名称 FundManagementCompany C30
		std::string strFundManagementCompany;

		//拟合指数代码 UnderlyingSecurityID C8
		std::string strUnderlyingSecurityID;

		//拟合指数代码源 UnderlyingSecurityIDSource C4 
		//101 = 上海证券交易所102 = 深圳证券交易所103 = 香港交易所9999 = 其他
		std::string strUnderlyingSecurityIDSource;

		// 最小申购赎回单位 CreationRedemptionUnit N15(2) 每个篮子（最小申购赎回单位）对应的 ETF 份数，
		//目前只能为正整数
		uint64_t ui64CreationRedemptionUnit;

		// 预估现金差额 EstimateCashComponent N11(2) T 日每个篮子的预估现金差额
		uint64_t_Money ui64mEstimateCashComponent;

		// 最大现金替代比例 MaxCashRatio N6(5) 最大现金替代比例，例如：5.551％在文件中用 0.05551 表示
		double dMaxCashRatio;

		// 是否发布 IOPV Publish C1 Y = 是N = 否
		bool bPublish;

		//是否允许申购 Creation C1 Y = 是N = 否
		bool bCreation;

		//是否允许赎回 Redemption C1 Y = 是N = 否
		bool bRedemption;

		//深市成份证券数目 RecordNum N4 表示一个篮子中的深市成份证券数目（包含 159900 证券）
		uint64_t ui64RecordNum;

		//所有成份证券数目 TotalRecordNum N4 表示一个篮子中的所有成份证券数目（包含 159900 证券）
		uint64_t ui64TotalRecordNum;

		// 交易日 TradingDay N8 格式 YYYYMMDD
		std::string strTradingDay;

		//前交易日 PreTradingDay N8 T - X 日日期，格式 YYYYMMDD，X 由基金公司根据基金估值时间确定
		std::string strPreTradingDay;

		//现金余额 CashComponent N11(2) T - X 日申购赎回基准单位的现金余额
		uint64_t_Money ui64mCashComponent;

		//申购赎回基准单位净值 NAVperCU N12(2) T - X 日申购赎回基准单位净值
		uint64_t_Money ui64mNAVperCU;

		// 单位净值 NAV N8(4) T - X 日基金的单位净值
		uint64_t_Money ui64mNAV;

		//红利金额 DividendPerCU N12(2) T 日申购赎回基准单位的红利金额
		uint64_t_Money ui64mDividendPerCU;

		//累计申购总额限制 CreationLimit N18(2) 当天累计可申购的基金份额上限，为 0 表示没有限制，目前只能为整数
		uint64_t ui64CreationLimit;

		//累计赎回总额限制 RedemptionLimit N18(2) 当天累计可赎回的基金份额上限，为 0 表示没有限制， 目前只能为整数
		uint64_t ui64RedemptionLimit;

		//单个账户累计申购总额限制 CreationLimitPerUser N18(2) 单个证券账户当天累计可申购的基金份额上限，
		//为 0 表示没有限制，目前只能为整数单个账户累计赎回总额限制
		uint64_t ui64CreationLimitPerUser;

		//单个账户累计赎回总额限制  RedemptionLimitPerUser N18(2) 单个证券账户当天累计可赎回的基金份额上限，
		//为 0 表示没有限制，目前只能为整数
		uint64_t ui64RedemptionLimitPerUser;

		//净申购总额限制 NetCreationLimit N18(2) 当天净申购的基金份额上限，为 0表示没有限制，目前只能为整数
		uint64_t ui64NetCreationLimit;

		//净赎回总额限制 NetRedemptionLimit N18(2) 当天净赎回的基金份额上限，为 0表示没有限制，目前只能为整数
		uint64_t ui64NetRedemptionLimit;

		//单个账户净申购总额限制 NetCreationLimitPerUser N18(2) 单个证券账户当天净申购的基金份额上限，
		//为 0 表示没有限制，目前只能为整数
		uint64_t ui64NetCreationLimitPerUser;

		//单个账户净赎回总额限制 NetRedemptionLimitPerUser N18(2) 单个证券账户当天净赎回的基金份额上限，
		//为 0 表示没有限制，目前只能为整数
		uint64_t ui64NetRedemptionLimitPerUser;

		// 所有成分股信息
		std::vector<SzETFComponent> vecComponents;

		SzETF()
		{
			ui64CreationRedemptionUnit = 0;
			ui64mEstimateCashComponent = 0;
			dMaxCashRatio = 0.0;
			bCreation = false;
			bRedemption = false;
			ui64RecordNum = 0;
			ui64TotalRecordNum = 0;
			ui64mCashComponent = 0;
			ui64mNAVperCU = 0;
			ui64mNAV = 0;
			ui64mDividendPerCU = 0;
			ui64CreationLimit = 0;
			ui64RedemptionLimit = 0;
			ui64CreationLimitPerUser = 0;
			ui64RedemptionLimitPerUser = 0;
			ui64NetCreationLimit = 0;
			ui64NetRedemptionLimit = 0;
			ui64NetCreationLimitPerUser = 0;
			ui64NetRedemptionLimitPerUser = 0;
		}
	};

}

#endif