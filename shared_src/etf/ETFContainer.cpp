#include "ETFContainer.h"

#include "simutgw/tcp_simutgw_client/config_client_msgkey.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw_config/define_db_names.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/sof_string.h"
#include "etf/ValidateETF.h"

using namespace std;

ETFContainer::ETFContainer(void)
	:m_scl(keywords::channel = "ETFContainer")
{

}

ETFContainer::~ETFContainer(void)
{

}

/*
插入ETF信息

@return 0 : 成功
@return 1 : 失败
*/
int ETFContainer::InsertEtf_FromWebControl(rapidjson::Value& docValue, std::string& out_errmsg)
{
	static const string ftag("InsertEtf_FromWebControl() ");

	std::lock_guard<std::mutex> lock(m_mutex);

	//
	// insert etf info
	if (!docValue.HasMember(simutgw::client::cstrKey_etf_info) && !docValue[simutgw::client::cstrKey_etf_info].IsObject())
	{
		out_errmsg = "error etf_info";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error etf_info";
		return -1;
	}

	if (!docValue[simutgw::client::cstrKey_etf_info].HasMember(simutgw::client::cstrKey_SecurityID.c_str()))
	{
		out_errmsg = "error SecurityID";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error SecurityID";
		return -1;
	}

	string strSecurityID = docValue[simutgw::client::cstrKey_etf_info][simutgw::client::cstrKey_SecurityID.c_str()].GetString();

	if (strSecurityID.empty())
	{
		out_errmsg = "error SecurityID";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error SecurityID";
		return -1;
	}

	std::shared_ptr<struct simutgw::SzETF> ptr(new struct simutgw::SzETF());

	rapidjson::Value::ConstMemberIterator iter;
	string strName;
	string strValue;
	int iRes = 0;
	for (iter = docValue[simutgw::client::cstrKey_etf_info].MemberBegin(); docValue[simutgw::client::cstrKey_etf_info].MemberEnd() != iter; ++iter)
	{
		if (!iter->name.IsString())
		{
			// value不是string，格式有问题
			out_errmsg = "key not string";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "key not string";

			return -1;
		}

		strName = iter->name.GetString();
		strValue = iter->value.GetString();

		iRes = Set_ETF_Info_FromWebControl(strName, strValue, ptr, out_errmsg);
		if (-1 == iRes)
		{
			return -1;
		}
	}

	//
	// insert etf components info
	if (!docValue.HasMember(simutgw::client::cstrKey_etf_component) && !docValue[simutgw::client::cstrKey_etf_component].IsArray())
	{
		out_errmsg = "error SecurityID";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error SecurityID";
		return -1;
	}

	for (rapidjson::SizeType i = 0; i < docValue[simutgw::client::cstrKey_etf_component].Size(); ++i)
	{
		if (!docValue[simutgw::client::cstrKey_etf_component][i].IsObject())
		{
			// value不是string，格式有问题
			out_errmsg = "components not object";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "components not object";

			return -1;
		}

		struct simutgw::SzETFComponent etfCpn;

		iRes = Set_ETF_Component_FromWebControl(docValue[simutgw::client::cstrKey_etf_component][i], etfCpn, out_errmsg);
		if (-1 == iRes)
		{
			return -1;
		}

		iRes = ValidateETF::Validate_ETFComponent_Info(etfCpn);
		if (0 != iRes)
		{
			out_errmsg = strSecurityID;
			out_errmsg += " 成分股信息有误";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strSecurityID << " 成分股信息有误";

			return -1;
		}

		ptr->vecComponents.push_back(etfCpn);
	}

	iRes = ValidateETF::Validate_ETF_Info(ptr);
	if (0 != iRes)
	{
		out_errmsg = strSecurityID;
		out_errmsg += " 信息有误";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strSecurityID << " 信息有误";
		return -1;
	}

	iRes = UpdateOneEtfPackageToDb(strSecurityID, ptr, out_errmsg);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strSecurityID << " 更新数据库出错";
		return -1;
	}

	MAP_ETF_COMP::iterator it = m_mapEtfContents.find(strSecurityID);
	if (m_mapEtfContents.end() == it)
	{
		// 未找到
		m_mapEtfContents.insert(std::pair<string, std::shared_ptr<struct simutgw::SzETF>>(strSecurityID, ptr));
	}
	else
	{
		// 找到
		it->second = ptr;
	}

	return 0;
}

/*
将ETF内容中除成分股之外的内容存入结构体
内容来自Web管理端

Return:
0 -- 成功
-1 -- 失败
*/
int ETFContainer::Set_ETF_Info_FromWebControl(const std::string& in_strKey, const std::string& in_strValue,
	std::shared_ptr<struct simutgw::SzETF>& ptrEtf, std::string& out_errmsg)
{
	static const std::string ftag("ETFContainer::Set_ETF_Info_FromWebControl() ");

	if (nullptr == ptrEtf)
	{
		out_errmsg = "ptrEtf is null";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ptrEtf is null";
		return -1;
	}

	int iRes = 0;
	uint64_t ui64Trans = 0;
	double dTrans = 0;

	if (0 == in_strKey.compare(simutgw::client::cstrKey_Version))
	{
		// 版本号 Version C8 固定值1.0
		ptrEtf->strVersion = in_strValue;
	}
	if (0 == in_strKey.compare(simutgw::client::cstrKey_SecurityID))
	{
		// 证券代码 SecurityID C8
		ptrEtf->strSecurityID = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_SecurityIDSource))
	{
		// 证券代码源 SecurityIDSource C4 101 = 上海证券交易所 102 = 深圳证券交易所
		ptrEtf->strSecurityIDSource = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_Symbol))
	{
		// 基金名称 Symbol C40
		ptrEtf->strSymbol = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_FundManagementCompany))
	{
		// 基金公司名称 FundManagementCompany C30
		ptrEtf->strFundManagementCompany = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSecurityID))
	{
		//拟合指数代码 UnderlyingSecurityID C8
		ptrEtf->strUnderlyingSecurityID = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSecurityIDSource))
	{
		//拟合指数代码源 UnderlyingSecurityIDSource C4 
		//101 = 上海证券交易所102 = 深圳证券交易所103 = 香港交易所9999 = 其他
		ptrEtf->strUnderlyingSecurityIDSource = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CreationRedemptionUnit))
	{
		// 最小申购赎回单位 CreationRedemptionUnit N15(2) 每个篮子（最小申购赎回单位）对应的 ETF 份数，
		//目前只能为正整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "CreationRedemptionUni trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CreationRedemptionUni trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64CreationRedemptionUnit = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_EstimateCashComponent))
	{
		// 预估现金差额 EstimateCashComponent N11(2) T 日每个篮子的预估现金差额
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "EstimateCashComponent trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "EstimateCashComponent trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64mEstimateCashComponent = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_MaxCashRatio))
	{
		// 最大现金替代比例 MaxCashRatio N6(5) 最大现金替代比例，例如：5.551％在文件中用 0.05551 表示
		dTrans = 0;
		iRes = Tgw_StringUtil::String2Double_atof(in_strValue, dTrans);
		if (0 != iRes)
		{
			out_errmsg = "MaxCashRatio trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "MaxCashRatio trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->dMaxCashRatio = dTrans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_Publish))
	{
		// 是否发布 IOPV Publish C1 Y = 是N = 否		
		if (0 == in_strValue.compare("1"))
		{
			ptrEtf->bPublish = true;
		}
		else
		{
			ptrEtf->bPublish = false;
		}
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_Creation))
	{
		//是否允许申购 Creation C1 Y = 是N = 否
		if (0 == in_strValue.compare("1"))
		{
			ptrEtf->bCreation = true;
		}
		else
		{
			ptrEtf->bCreation = false;
		}
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_Redemption))
	{
		//是否允许赎回 Redemption C1 Y = 是N = 否
		if (0 == in_strValue.compare("1"))
		{
			ptrEtf->bRedemption = true;
		}
		else
		{
			ptrEtf->bRedemption = false;
		}
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_RecordNum))
	{
		//深市成份证券数目 RecordNum N4 表示一个篮子中的深市成份证券数目（包含 159900 证券）
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "RecordNum trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "RecordNum trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64RecordNum = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_TotalRecordNum))
	{
		//所有成份证券数目 TotalRecordNum N4 表示一个篮子中的所有成份证券数目（包含 159900 证券）
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "TotalRecordNum trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "TotalRecordNum trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64TotalRecordNum = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_TradingDay))
	{
		// 交易日 TradingDay N8 格式 YYYYMMDD
		ptrEtf->strTradingDay = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_PreTradingDay))
	{
		//前交易日 PreTradingDay N8 T - X 日日期，格式 YYYYMMDD，X 由基金公司根据基金估值时间确定
		ptrEtf->strPreTradingDay = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CashComponent))
	{
		//现金余额 CashComponent N11(2) T - X 日申购赎回基准单位的现金余额
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "CashComponent trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CashComponent trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64mCashComponent = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NAVperCU))
	{
		//申购赎回基准单位净值 NAVperCU N12(2) T - X 日申购赎回基准单位净值
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "NAVperCU trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NAVperCU trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64mNAVperCU = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NAV))
	{
		// 单位净值 NAV N8(4) T - X 日基金的单位净值
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "NAV trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NAV trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64mNAV = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_DividendPerCU))
	{
		//红利金额 DividendPerCU N12(2) T 日申购赎回基准单位的红利金额
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "DividendPerCU trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "DividendPerCU trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64mDividendPerCU = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CreationLimit))
	{
		//累计申购总额限制 CreationLimit N18(2) 当天累计可申购的基金份额上限，为 0 表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "CreationLimit trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CreationLimit trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64CreationLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_RedemptionLimit))
	{
		//累计赎回总额限制 RedemptionLimit N18(2) 当天累计可赎回的基金份额上限，为 0 表示没有限制， 目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "RedemptionLimit trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "RedemptionLimit trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64RedemptionLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CreationLimitPerUser))
	{
		//单个账户累计申购总额限制 CreationLimitPerUser N18(2) 单个证券账户当天累计可申购的基金份额上限，
		//为 0 表示没有限制，目前只能为整数单个账户累计赎回总额限制
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "CreationLimitPerUser trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CreationLimitPerUser trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64CreationLimitPerUser = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_RedemptionLimitPerUser))
	{
		//单个账户累计赎回总额限制  RedemptionLimitPerUser N18(2) 单个证券账户当天累计可赎回的基金份额上限，
		//为 0 表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "RedemptionLimitPerUser trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "RedemptionLimitPerUser trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64RedemptionLimitPerUser = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NetCreationLimit))
	{
		//净申购总额限制 NetCreationLimit N18(2) 当天净申购的基金份额上限，为 0表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "NetCreationLimit trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NetCreationLimit trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64NetCreationLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NetRedemptionLimit))
	{
		//净赎回总额限制 NetRedemptionLimit N18(2) 当天净赎回的基金份额上限，为 0表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "NetCreationLimit trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NetCreationLimit trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64NetRedemptionLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NetCreationLimitPerUser))
	{
		//单个账户净申购总额限制 NetCreationLimitPerUser N18(2) 单个证券账户当天净申购的基金份额上限，
		//为 0 表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "NetCreationLimitPerUser trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NetCreationLimitPerUser trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64NetCreationLimitPerUser = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NetRedemptionLimitPerUser))
	{
		//单个账户净赎回总额限制 NetRedemptionLimitPerUser N18(2) 单个证券账户当天净赎回的基金份额上限，
		//为 0 表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (0 != iRes)
		{
			out_errmsg = "NetRedemptionLimitPerUser trans failed, value=[";
			out_errmsg += in_strValue;
			out_errmsg += "]";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NetRedemptionLimitPerUser trans failed, value=[" << in_strValue << "]";

			return -1;
		}
		ptrEtf->ui64NetRedemptionLimitPerUser = ui64Trans;
	}
	else
	{
		//
	}

	return 0;
}

/*
Set 成分股信息
内容来自Web管理端

Return:
0 -- 成功
-1 -- 失败
*/
int ETFContainer::Set_ETF_Component_FromWebControl(rapidjson::Value& docValue,
struct simutgw::SzETFComponent& out_cpn, std::string& out_errmsg)
{
	static const std::string ftag("ETFContainer::Set_ETF_Component_FromWebControl() ");

	double dTrans = 0;
	uint64_t ui64Trans = 0;
	int iTrans = 0;
	int iRes = 0;

	// 证券代码 UnderlyingSecurityID C8
	if (!docValue.HasMember(simutgw::client::cstrKey_UnderlyingSecurityID.c_str()))
	{
		out_errmsg = "error UnderlyingSecurityID";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error UnderlyingSecurityID";
		return -1;
	}
	out_cpn.strUnderlyingSecurityID = docValue[simutgw::client::cstrKey_UnderlyingSecurityID.c_str()].GetString();

	// 证券代码源 UnderlyingSecurityIDSource C4 101 = 上海证券交易所102 = 深圳证券交易所
	//103 = 香港交易所9999 = 其他
	if (!docValue.HasMember(simutgw::client::cstrKey_UnderlyingSecurityIDSource.c_str()))
	{
		out_errmsg = "error UnderlyingSecurityIDSource";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error UnderlyingSecurityIDSource";
		return -1;
	}
	out_cpn.strUnderlyingSecurityIDSource = docValue[simutgw::client::cstrKey_UnderlyingSecurityIDSource.c_str()].GetString();

	// 证券简称 UnderlyingSymbol C40
	if (!docValue.HasMember(simutgw::client::cstrKey_UnderlyingSymbol))
	{
		out_errmsg = "error UnderlyingSymbol";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error UnderlyingSymbol";
		return -1;
	}
	out_cpn.strUnderlyingSymbol = docValue[simutgw::client::cstrKey_UnderlyingSymbol].GetString();

	// 成份证券数 ComponentShare N15(2) 每个申购篮子中该成份证券的数量。
	//此字段只有现金替代标志为‘0’或‘1’时才有效
	if (!docValue.HasMember(simutgw::client::cstrKey_ComponentShare))
	{
		out_errmsg = "error ComponentShare";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error ComponentShare";
		return -1;
	}

	ui64Trans = 0;
	iRes = Tgw_StringUtil::String2UInt64_strtoui64(docValue[simutgw::client::cstrKey_ComponentShare].GetString(), ui64Trans);
	if (0 != iRes)
	{
		out_errmsg = "ComponentShare trans failed, value=[";
		out_errmsg += docValue[simutgw::client::cstrKey_ComponentShare].GetString();
		out_errmsg += "]";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ComponentShare trans failed, value=[" << docValue[simutgw::client::cstrKey_ComponentShare].GetString() << "]";

		return -1;
	}
	out_cpn.ui64ComponentShare = ui64Trans;

	// 现金替代标志SubstituteFlag C1 0 = 禁止现金替代（必须有证券）
	//1 = 可以进行现金替代（先用证券，证券不足时差额部分用现金替代）2 = 必须用现金替代
	if (!docValue.HasMember(simutgw::client::cstrKey_SubstituteFlag))
	{
		out_errmsg = "error SubstituteFlag";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error SubstituteFlag";
		return -1;
	}

	iTrans = 0;
	iRes = Tgw_StringUtil::String2Int_atoi(docValue[simutgw::client::cstrKey_SubstituteFlag].GetString(), iTrans);
	if (0 != iRes)
	{
		out_errmsg = "SubstituteFlag trans failed, value=[";
		out_errmsg += docValue[simutgw::client::cstrKey_SubstituteFlag].GetString();
		out_errmsg += "]";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SubstituteFlag trans failed, value=[" << docValue[simutgw::client::cstrKey_SubstituteFlag].GetString() << "]";

		return -1;
	}
	out_cpn.iSubstituteFlag = iTrans;

	// 溢价比例 PremiumRatio N7(5) 证券用现金进行替代的时候，计算价格时增加的比例。
	//例如：2.551％在文件中用 0.02551 表示；2.1%在文件中用 0.02100 表示。此字段只有现金替代标志为‘1’时才有效
	if (!docValue.HasMember(simutgw::client::cstrKey_PremiumRatio))
	{
		out_errmsg = "error PremiumRatio";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error PremiumRatio";
		return -1;
	}

	dTrans = 0;
	iRes = Tgw_StringUtil::String2Double_atof(docValue[simutgw::client::cstrKey_PremiumRatio].GetString(), dTrans);
	if (0 != iRes)
	{
		out_errmsg = "PremiumRatio trans failed, value=[";
		out_errmsg += docValue[simutgw::client::cstrKey_PremiumRatio].GetString();
		out_errmsg += "]";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "PremiumRatio trans failed, value=[" << docValue[simutgw::client::cstrKey_PremiumRatio].GetString() << "]";

		return -1;
	}
	out_cpn.dPremiumRatio = dTrans;

	// 申购替代金额CreationCashSubstitute N18(4) 当某只证券必须用现金替代的时候，
	//申购时该证券所需总金额此字段只有当现金替代标志为‘2’时才有效
	if (!docValue.HasMember(simutgw::client::cstrKey_CreationCashSubstitute))
	{
		out_errmsg = "error CreationCashSubstitute";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error CreationCashSubstitute";
		return -1;
	}

	ui64Trans = 0;
	iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(docValue[simutgw::client::cstrKey_CreationCashSubstitute].GetString(), ui64Trans);
	if (0 != iRes)
	{
		out_errmsg = "CreationCashSubstitute trans failed, value=[";
		out_errmsg += docValue[simutgw::client::cstrKey_CreationCashSubstitute].GetString();
		out_errmsg += "]";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CreationCashSubstitute trans failed, value=[" << docValue[simutgw::client::cstrKey_CreationCashSubstitute].GetString() << "]";

		return -1;
	}
	out_cpn.ui64mCreationCashSubstitute = ui64Trans;


	// 赎回替代金额RedemptionCashSubstitute N18(4) 当某只证券必须用现金替代的时候，
	//赎回时对应该证券返还的总金额。例如： 2000 在文件中用 2000.0000表示。
	//对于跨境 ETF、跨市场 ETF、黄金 ETF 和现金债券 ETF，该字段为 0.0000。此字段只有当现金替代标志为‘2’时才有效
	if (!docValue.HasMember(simutgw::client::cstrKey_RedemptionCashSubstitute))
	{
		out_errmsg = "error RedemptionCashSubstitute";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error RedemptionCashSubstitute";
		return -1;
	}
	ui64Trans = 0;
	iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(docValue[simutgw::client::cstrKey_RedemptionCashSubstitute].GetString(), ui64Trans);
	if (0 != iRes)
	{
		out_errmsg = "RedemptionCashSubstitute trans failed, value=[";
		out_errmsg += docValue[simutgw::client::cstrKey_RedemptionCashSubstitute].GetString();
		out_errmsg += "]";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "RedemptionCashSubstitute trans failed, value=[" << docValue[simutgw::client::cstrKey_RedemptionCashSubstitute].GetString() << "]";

		return -1;
	}
	out_cpn.ui64mRedemptionCashSubstitute = ui64Trans;

	return 0;
}

/*
执行更新sql语句

@return 0 : 更新成功
@return -1 : 更新失败
*/
int ETFContainer::UpdateOneEtfPackageToDb(const std::string& in_strSecurityID,
	std::shared_ptr<struct simutgw::SzETF>& ptrEtf, std::string& out_errmsg)
{
	static const std::string ftag("UpdateOneEtfPackageToDb() ");

	try	{
		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			out_errmsg = "Get Connection is NULL";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Get Connection is NULL";

			return -1;
		}

		mysqlConn->StartTransaction();

		int iRes = Update_EtfInfo(mysqlConn, ptrEtf);
		if (0 != iRes)
		{
			// 失败
			out_errmsg = "db etf_info failed, strSecurityID=";
			out_errmsg += in_strSecurityID;

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "db etf_info failed, strSecurityID=" << in_strSecurityID;

			mysqlConn->RollBack();
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
			return -1;
		}

		iRes = Update_EtfComponent(mysqlConn, ptrEtf);
		if (0 != iRes)
		{
			// 失败
			out_errmsg = "db etf_component failed, strSecurityID=";
			out_errmsg += in_strSecurityID;

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "db etf_component failed, strSecurityID=" << in_strSecurityID;

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
		EzLog::e(ftag, "未知异常");
		return -1;
	}

	return 0;
}

/*
执行更新sql语句

@return 0 : 更新成功
@return -1 : 更新失败
*/
int ETFContainer::Update_EtfInfo(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
	std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	static const std::string ftag("Update_EtfInfo() ");

	string strItoa;
	string strUpdateSql = "REPLACE INTO ";
	strUpdateSql += simutgw::g_DBTable_etf_info;
	strUpdateSql += " (`Version`,`SecurityID`,`SecurityIDSource`,`Symbol`,`FundManagementCompany`,"
		"`UnderlyingSecurityID`,`UnderlyingSecurityIDSource`,`CreationRedemptionUnit`,`EstimateCashComponent`,`MaxCashRatio`,"
		"`Publish`,`Creation`,`Redemption`,`RecordNum`,`TotalRecordNum`,"
		"`TradingDay`,`PreTradingDay`,`CashComponent`,`NAVperCU`,`NAV`,"
		"`DividendPerCU`,`CreationLimit`,`RedemptionLimit`,`CreationLimitPerUser`,`RedemptionLimitPerUser`,"
		"`NetCreationLimit`,`NetRedemptionLimit`,`NetCreationLimitPerUser`,`NetRedemptionLimitPerUser`) VALUES('";

	// `Version`,`SecurityID`,`SecurityIDSource`,`Symbol`,`FundManagementCompany`,"
	strUpdateSql += ptrEtf->strVersion;
	strUpdateSql += "','";

	strUpdateSql += ptrEtf->strSecurityID;
	strUpdateSql += "','";

	strUpdateSql += ptrEtf->strSecurityIDSource;
	strUpdateSql += "',";

	if (ptrEtf->strSymbol.empty())
	{
		strUpdateSql += "NULL,";
	}
	else
	{
		strUpdateSql += "'";
		strUpdateSql += ptrEtf->strSymbol;
		strUpdateSql += "',";
	}

	if (ptrEtf->strFundManagementCompany.empty())
	{
		strUpdateSql += "NULL,";
	}
	else
	{
		strUpdateSql += "'";
		strUpdateSql += ptrEtf->strFundManagementCompany;
		strUpdateSql += "',";
	}

	// "`UnderlyingSecurityID`,`UnderlyingSecurityIDSource`,`CreationRedemptionUnit`,`EstimateCashComponent`,`MaxCashRatio`,"
	if (ptrEtf->strUnderlyingSecurityID.empty())
	{
		strUpdateSql += "NULL,";
	}
	else
	{
		strUpdateSql += "'";
		strUpdateSql += ptrEtf->strUnderlyingSecurityID;
		strUpdateSql += "',";
	}

	if (ptrEtf->strUnderlyingSecurityIDSource.empty())
	{
		strUpdateSql += "NULL,";
	}
	else
	{
		strUpdateSql += "'";
		strUpdateSql += ptrEtf->strUnderlyingSecurityIDSource;
		strUpdateSql += "',";
	}

	strUpdateSql += sof_string::itostr(ptrEtf->ui64CreationRedemptionUnit, strItoa);
	strUpdateSql += ",'";

	strItoa = std::to_string(ptrEtf->ui64mEstimateCashComponent);
	strUpdateSql += strItoa;
	strUpdateSql += "','";

	strItoa = std::to_string(ptrEtf->dMaxCashRatio);
	strUpdateSql += strItoa;
	strUpdateSql += "',";

	// "`Publish`,`Creation`,`Redemption`,`RecordNum`,`TotalRecordNum`,"
	if (ptrEtf->bPublish)
	{
		strUpdateSql += "1,";
	}
	else
	{
		strUpdateSql += "0,";
	}

	if (ptrEtf->bCreation)
	{
		strUpdateSql += "1,";
	}
	else
	{
		strUpdateSql += "0,";
	}

	if (ptrEtf->bRedemption)
	{
		strUpdateSql += "1,";
	}
	else
	{
		strUpdateSql += "0,";
	}

	strUpdateSql += sof_string::itostr(ptrEtf->ui64RecordNum, strItoa);
	strUpdateSql += ",";

	strUpdateSql += sof_string::itostr(ptrEtf->ui64TotalRecordNum, strItoa);
	strUpdateSql += ",'";

	// "`TradingDay`,`PreTradingDay`,`CashComponent`,`NAVperCU`,`NAV`,"
	strUpdateSql += ptrEtf->strTradingDay;
	strUpdateSql += "','";

	strUpdateSql += ptrEtf->strPreTradingDay;
	strUpdateSql += "',";

	strUpdateSql += sof_string::itostr(ptrEtf->ui64mCashComponent, strItoa);
	strUpdateSql += ",";

	strUpdateSql += sof_string::itostr(ptrEtf->ui64mNAVperCU, strItoa);
	strUpdateSql += ",";

	strUpdateSql += sof_string::itostr(ptrEtf->ui64mNAV, strItoa);
	strUpdateSql += ",";

	// "`DividendPerCU`,`CreationLimit`,`RedemptionLimit`,`CreationLimitPerUser`,`RedemptionLimitPerUser`,"
	strUpdateSql += sof_string::itostr(ptrEtf->ui64mDividendPerCU, strItoa);
	strUpdateSql += ",";

	strUpdateSql += sof_string::itostr(ptrEtf->ui64CreationLimit, strItoa);
	strUpdateSql += ",";

	strUpdateSql += sof_string::itostr(ptrEtf->ui64RedemptionLimit, strItoa);
	strUpdateSql += ",";

	strUpdateSql += sof_string::itostr(ptrEtf->ui64CreationLimitPerUser, strItoa);
	strUpdateSql += ",";

	strUpdateSql += sof_string::itostr(ptrEtf->ui64RedemptionLimitPerUser, strItoa);
	strUpdateSql += ",";

	// "`NetCreationLimit`,`NetRedemptionLimit`,`NetCreationLimitPerUser`,`NetRedemptionLimitPerUser`
	strUpdateSql += sof_string::itostr(ptrEtf->ui64NetCreationLimit, strItoa);
	strUpdateSql += ",";

	strUpdateSql += sof_string::itostr(ptrEtf->ui64NetRedemptionLimit, strItoa);
	strUpdateSql += ",";

	strUpdateSql += sof_string::itostr(ptrEtf->ui64NetCreationLimitPerUser, strItoa);
	strUpdateSql += ",";

	strUpdateSql += sof_string::itostr(ptrEtf->ui64NetRedemptionLimitPerUser, strItoa);
	strUpdateSql += ");";

	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	int iRes = in_mysqlConn->Query(strUpdateSql, &pResultSet, ulAffectedRows);
	if (2 == iRes)
	{
		// 是更新
		if (2 < ulAffectedRows)
		{
			// 失败
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "运行[" << strUpdateSql
				<< "]得到AffectedRows=" << ulAffectedRows;

			return -1;
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "运行[" << strUpdateSql
			<< "]得到Res=" << iRes;
		return -1;
	}

	return 0;
}

/*
执行更新sql语句

@return 0 : 更新成功
@return -1 : 更新失败
*/
int ETFContainer::Update_EtfComponent(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
	std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	static const std::string ftag("Update_EtfComponent() ");

	string strDeleteSql = "DELETE FROM ";
	strDeleteSql += simutgw::g_DBTable_etf_component;
	strDeleteSql += " WHERE `SecurityID`='";
	strDeleteSql += ptrEtf->strSecurityID;
	strDeleteSql += "';";


	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;
	std::string strItoa;
	int iRes = in_mysqlConn->Query(strDeleteSql, &pResultSet, ulAffectedRows);
	if (2 == iRes)
	{
		// 是更新
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "运行[" << strDeleteSql
			<< "]得到Res=" << iRes;
		return -1;
	}

	string strUpdateSql_Head = "INSERT INTO ";
	strUpdateSql_Head += simutgw::g_DBTable_etf_component;
	strUpdateSql_Head += " (`SecurityID` ,`UnderlyingSecurityID`,`UnderlyingSecurityIDSource`,`UnderlyingSymbol`,`ComponentShare`,"
		"`SubstituteFlag`,`PremiumRatio`,`CreationCashSubstitute`,`RedemptionCashSubstitute`) VALUES ";

	// 一次批量插入的数量
	const size_t iBatchInsertSize = 50;
	// 当前批次插入数量
	size_t iInsertNumCount = 0;

	string strUpdateSql = strUpdateSql_Head;
	strUpdateSql += "('";

	for (size_t i = 0, iInsertNumCount = 0; i < ptrEtf->vecComponents.size(); ++i, ++iInsertNumCount)
	{
		// `SecurityID` ,`UnderlyingSecurityID`,`UnderlyingSecurityIDSource`,`UnderlyingSymbol`,`ComponentShare`,"
		strUpdateSql += ptrEtf->strSecurityID;
		strUpdateSql += "','";

		strUpdateSql += ptrEtf->vecComponents[i].strUnderlyingSecurityID;
		strUpdateSql += "','";

		strUpdateSql += ptrEtf->vecComponents[i].strUnderlyingSecurityIDSource;
		strUpdateSql += "','";

		strUpdateSql += ptrEtf->vecComponents[i].strUnderlyingSymbol;
		strUpdateSql += "',";

		strUpdateSql += sof_string::itostr(ptrEtf->vecComponents[i].ui64ComponentShare, strItoa);
		strUpdateSql += ",";

		// "`SubstituteFlag`,`PremiumRatio`,`CreationCashSubstitute`,`RedemptionCashSubstitute`
		strUpdateSql += sof_string::itostr(ptrEtf->vecComponents[i].iSubstituteFlag, strItoa);
		strUpdateSql += ",";

		strItoa = std::to_string(ptrEtf->vecComponents[i].dPremiumRatio);
		strUpdateSql += strItoa;
		strUpdateSql += ",";

		strUpdateSql += sof_string::itostr(ptrEtf->vecComponents[i].ui64mCreationCashSubstitute, strItoa);
		strUpdateSql += ",";

		strUpdateSql += sof_string::itostr(ptrEtf->vecComponents[i].ui64mRedemptionCashSubstitute, strItoa);
		strUpdateSql += ")";

		if ((i == ptrEtf->vecComponents.size() - 1) || iInsertNumCount >= iBatchInsertSize)
		{
			strUpdateSql += ";";

			// 到达批次数量，进行批量插入
			pResultSet = NULL;
			ulAffectedRows = 0;
			iRes = in_mysqlConn->Query(strUpdateSql, &pResultSet, ulAffectedRows);
			if (2 == iRes)
			{
				// 是更新
				if (0 == ulAffectedRows)
				{
					// 失败
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "运行[" << strUpdateSql
						<< "]得到AffectedRows=" << ulAffectedRows;

					return -1;
				}
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "运行[" << strUpdateSql
					<< "]得到Res=" << iRes;
				return -1;
			}

			// 重置批量插入条件
			iInsertNumCount = 0;
			strUpdateSql = strUpdateSql_Head;
			strUpdateSql += " ('";
		}
		else
		{
			strUpdateSql += ",('";
		}
	}

	return 0;
}

/*
从数据库重新载入 ETF信息

@return 0 : 成功
@return -1 : 失败
*/
int ETFContainer::ReloadFromDb(void)
{
	static const string ftag("InsertEtf_FromWebControl() ");

	std::lock_guard<std::mutex> lock(m_mutex);

	m_mapEtfContents.clear();

	string strQueryString;
	int iReturn = 0;

	try
	{
		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Get Mysql Connection is NULL";

			return -1;
		}

		std::vector<string> vctEtfStockIds;
		int iRes = GetFromDB_AllEtfStockIds(mysqlConn, vctEtfStockIds);
		if (0 != iRes)
		{
			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetAllEtfStockIds failed";
			return -1;
		}

		string strSecurityID;
		for (size_t i = 0; i < vctEtfStockIds.size(); ++i)
		{
			std::shared_ptr<struct simutgw::SzETF> ptr(new struct simutgw::SzETF());

			strSecurityID = vctEtfStockIds[i];
			if (vctEtfStockIds.empty())
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetFromDB_AllEtfStockIds find empty SecurityId";
				continue;
			}

			// 每个ETF代码循环查找
			iRes = GetFromDB_OneEtfPackage(mysqlConn, strSecurityID, ptr);
			if (0 != iRes)
			{
				//归还连接
				simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetFromDB_OneEtfPackage failed";
				return -1;
			}

			MAP_ETF_COMP::iterator it = m_mapEtfContents.find(strSecurityID);
			if (m_mapEtfContents.end() == it)
			{
				// 未找到
				m_mapEtfContents.insert(std::pair<string, std::shared_ptr<struct simutgw::SzETF>>(strSecurityID, ptr));
			}
			else
			{
				// 找到
				it->second = ptr;
			}
		}

		//归还连接
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		if (0 != iReturn)
		{
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
获取所有的 ETF SecurityId

Return :
0 -- 成功
非0 -- 失败
*/
int ETFContainer::GetFromDB_AllEtfStockIds(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
	std::vector<string>& out_vctEtfStockIds)
{
	static const string ftag("GetFromDB_AllEtfStockIds() ");

	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		// 查询
		string strQueryString = "SELECT `SecurityID` FROM ";
		strQueryString += simutgw::g_DBTable_etf_info;
		strQueryString += " GROUP BY `SecurityID`;";

		int iRes = in_mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
			while (0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				if (mapRowData["SecurityID"].bIsNull)
				{
					// IS NULL
					out_vctEtfStockIds.push_back("");
				}
				else
				{
					out_vctEtfStockIds.push_back(mapRowData["SecurityID"].strValue);
				}
			}

			// 释放
			in_mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;
		}
	}
	catch (std::exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

/*
从数据库读取一个ETF代码的全部数据

Return :
0 -- 成功
非0 -- 失败
*/
int ETFContainer::GetFromDB_OneEtfPackage(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::string& in_strSecurityID,
	std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	static const string ftag("GetFromDB_OneEtfPackage() ");

	int iRes = GetFromDB_OneEtfInfo(in_mysqlConn, in_strSecurityID, ptrEtf);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << in_strSecurityID << " 信息有误";
		return -1;
	}

	iRes = GetFromDB_OneEtfComponents(in_mysqlConn, in_strSecurityID, ptrEtf);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << in_strSecurityID << " 信息有误";
		return -1;
	}

	iRes = ValidateETF::Validate_ETF_Info(ptrEtf);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << in_strSecurityID << " 信息有误";
		return -1;
	}

	return 0;
}

/*
从数据库读取一个ETF代码的info

Return :
0 -- 成功
非0 -- 失败
*/
int ETFContainer::GetFromDB_OneEtfInfo(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::string& in_strSecurityID,
	std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	static const string ftag("GetFromDB_OneEtfInfo() ");

	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		// 查询
		string strQueryString = "SELECT `Version`,`SecurityID`,`SecurityIDSource`,`Symbol`,`FundManagementCompany`,"
			"`UnderlyingSecurityID`,`UnderlyingSecurityIDSource`,`CreationRedemptionUnit`,`EstimateCashComponent`,`MaxCashRatio`,"
			"`Publish`,`Creation`,`Redemption`,`RecordNum`,`TotalRecordNum`,"
			"`TradingDay`,`PreTradingDay`,`CashComponent`,`NAVperCU`,`NAV`,"
			"`DividendPerCU`,`CreationLimit`,`RedemptionLimit`,`CreationLimitPerUser`,`RedemptionLimitPerUser`,"
			"`NetCreationLimit`,`NetRedemptionLimit`,`NetCreationLimitPerUser`,`NetRedemptionLimitPerUser` FROM ";
		strQueryString += simutgw::g_DBTable_etf_info;
		strQueryString += " WHERE `SecurityID`=";
		strQueryString += in_strSecurityID;
		strQueryString += ";";

		int iRes = in_mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;

			string strName("");
			while (0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				// `Version`,`SecurityID`,`SecurityIDSource`,`Symbol`,`FundManagementCompany`,"				 
				strName = "Version";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `Version` failed";
					return -1;
				}
				strName = "SecurityID";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `SecurityID` failed";
					return -1;
				}

				strName = "SecurityIDSource";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `SecurityIDSource` failed";
					return -1;
				}

				strName = "Symbol";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `Symbol` failed";
					return -1;
				}

				strName = "FundManagementCompany";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `FundManagementCompany` failed";
					return -1;
				}

				// "`UnderlyingSecurityID`,`UnderlyingSecurityIDSource`,`CreationRedemptionUnit`,`EstimateCashComponent`,`MaxCashRatio`,"
				strName = "UnderlyingSecurityID";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `UnderlyingSecurityID` failed";
					return -1;
				}

				strName = "UnderlyingSecurityIDSource";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `UnderlyingSecurityIDSource` failed";
					return -1;
				}

				strName = "CreationRedemptionUnit";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `CreationRedemptionUnit` failed";
					return -1;
				}

				strName = "EstimateCashComponent";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `EstimateCashComponent` failed";
					return -1;
				}

				strName = "MaxCashRatio";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `MaxCashRatio` failed";
					return -1;
				}

				// "`Publish`,`Creation`,`Redemption`,`RecordNum`,`TotalRecordNum`,"
				strName = "Publish";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `Publish` failed";
					return -1;
				}

				strName = "Creation";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `Creation` failed";
					return -1;
				}

				strName = "Redemption";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `Redemption` failed";
					return -1;
				}

				strName = "RecordNum";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `RecordNum` failed";
					return -1;
				}

				strName = "TotalRecordNum";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `TotalRecordNum` failed";
					return -1;
				}

				// "`TradingDay`,`PreTradingDay`,`CashComponent`,`NAVperCU`,`NAV`,"
				strName = "TradingDay";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `TradingDay` failed";
					return -1;
				}

				strName = "PreTradingDay";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `PreTradingDay` failed";
					return -1;
				}

				strName = "CashComponent";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `CashComponent` failed";
					return -1;
				}

				strName = "NAVperCU";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `NAVperCU` failed";
					return -1;
				}

				strName = "NAV";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `NAV` failed";
					return -1;
				}

				// "`DividendPerCU`,`CreationLimit`,`RedemptionLimit`,`CreationLimitPerUser`,`RedemptionLimitPerUser`,"
				strName = "DividendPerCU";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `DividendPerCU` failed";
					return -1;
				}

				strName = "CreationLimit";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `CreationLimit` failed";
					return -1;
				}

				strName = "RedemptionLimit";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `RedemptionLimit` failed";
					return -1;
				}

				strName = "CreationLimitPerUser";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `CreationLimitPerUser` failed";
					return -1;
				}

				strName = "RedemptionLimitPerUser";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `RedemptionLimitPerUser` failed";
					return -1;
				}

				// "`NetCreationLimit`,`NetRedemptionLimit`,`NetCreationLimitPerUser`,`NetRedemptionLimitPerUser`
				strName = "NetCreationLimit";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `NetCreationLimit` failed";
					return -1;
				}

				strName = "NetRedemptionLimit";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `NetRedemptionLimit` failed";
					return -1;
				}

				strName = "NetCreationLimitPerUser";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `NetCreationLimitPerUser` failed";
					return -1;
				}

				strName = "NetRedemptionLimitPerUser";
				iRes = Set_ETF_Info_FromDb(strName, mapRowData, ptrEtf);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Info_FromDb `NetRedemptionLimitPerUser` failed";
					return -1;
				}

			}

			// 释放
			in_mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

			return 0;
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "sql=" << strQueryString << " Get unexpected Res=" << iRes;
			return -1;
		}
	}
	catch (std::exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
}

/*
从数据库读取一个ETF代码的所有的Components

Return :
0 -- 成功
非0 -- 失败
*/
int ETFContainer::GetFromDB_OneEtfComponents(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::string& in_strSecurityID,
	std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	static const string ftag("GetFromDB_OneEtfComponents() ");

	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		// 查询
		string strQueryString = "SELECT `UnderlyingSecurityID`,`UnderlyingSecurityIDSource`,`UnderlyingSymbol`,`ComponentShare`,`SubstituteFlag`,"
			"`PremiumRatio`,`CreationCashSubstitute`,`RedemptionCashSubstitute` FROM ";
		strQueryString += simutgw::g_DBTable_etf_component;
		strQueryString += " WHERE `SecurityID`=";
		strQueryString += in_strSecurityID;
		strQueryString += ";";

		int iRes = in_mysqlConn->Query(strQueryString, &pResultSet, ulAffectedRows);
		if (1 == iRes)
		{
			// select
			map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;

			string strName("");
			while (0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData))
			{
				struct simutgw::SzETFComponent etfCpn;

				// `UnderlyingSecurityID`,`UnderlyingSecurityIDSource`,`UnderlyingSymbol`,`ComponentShare`,`SubstituteFlag`,"					
				strName = "UnderlyingSecurityID";
				iRes = Set_ETF_Component_FromDb(strName, mapRowData, etfCpn);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Component_FromDb `UnderlyingSecurityID` failed, SecurityID=" << in_strSecurityID;
					return -1;
				}

				strName = "UnderlyingSecurityIDSource";
				iRes = Set_ETF_Component_FromDb(strName, mapRowData, etfCpn);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Component_FromDb `UnderlyingSecurityIDSource` failed, SecurityID=" << in_strSecurityID;
					return -1;
				}

				strName = "UnderlyingSymbol";
				iRes = Set_ETF_Component_FromDb(strName, mapRowData, etfCpn);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Component_FromDb `UnderlyingSymbol` failed, SecurityID=" << in_strSecurityID;
					return -1;
				}

				strName = "ComponentShare";
				iRes = Set_ETF_Component_FromDb(strName, mapRowData, etfCpn);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Component_FromDb `ComponentShare` failed, SecurityID=" << in_strSecurityID;
					return -1;
				}

				strName = "SubstituteFlag";
				iRes = Set_ETF_Component_FromDb(strName, mapRowData, etfCpn);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Component_FromDb `SubstituteFlag` failed, SecurityID=" << in_strSecurityID;
					return -1;
				}

				// "`PremiumRatio`,`CreationCashSubstitute`,`RedemptionCashSubstitute`		
				strName = "PremiumRatio";
				iRes = Set_ETF_Component_FromDb(strName, mapRowData, etfCpn);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Component_FromDb `PremiumRatio` failed, SecurityID=" << in_strSecurityID;
					return -1;
				}

				strName = "CreationCashSubstitute";
				iRes = Set_ETF_Component_FromDb(strName, mapRowData, etfCpn);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Component_FromDb `CreationCashSubstitute` failed, SecurityID=" << in_strSecurityID;
					return -1;
				}

				strName = "RedemptionCashSubstitute";
				iRes = Set_ETF_Component_FromDb(strName, mapRowData, etfCpn);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Set_ETF_Component_FromDb `RedemptionCashSubstitute` failed, SecurityID=" << in_strSecurityID;
					return -1;
				}

				iRes = ValidateETF::Validate_ETFComponent_Info(etfCpn);
				if (0 != iRes)
				{
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << in_strSecurityID << " 成分股信息有误";
					return -1;
				}

				ptrEtf->vecComponents.push_back(etfCpn);
			}

			// 释放
			in_mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

			return 0;
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "sql=" << strQueryString << " Get unexpected Res=" << iRes;
			return -1;
		}
	}
	catch (std::exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
}

/*
将ETF内容中除成分股之外的内容存入结构体
内容来自数据库

Return:
0 -- 成功
-1 -- 失败
*/
int ETFContainer::Set_ETF_Info_FromDb(const std::string& in_strKey,
	const map<string, struct MySqlCnnC602_DF::DataInRow>& in_mapRowData,
	std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	static const std::string ftag("ETFContainer::Set_ETF_Info_FromDb() ");

	if (nullptr == ptrEtf)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ptrEtf is null";
		return -1;
	}

	const map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator cit = in_mapRowData.find(in_strKey);

	if (in_mapRowData.end() == cit || cit->second.bIsNull)
	{
		return 0;
	}

	string strValue = cit->second.strValue;

	int iRes = 0;
	uint64_t ui64Trans = 0;
	double dTrans = 0;

	if (0 == in_strKey.compare(simutgw::client::cstrKey_Version))
	{
		// 版本号 Version C8 固定值1.0
		ptrEtf->strVersion = strValue;
	}
	if (0 == in_strKey.compare(simutgw::client::cstrKey_SecurityID))
	{
		// 证券代码 SecurityID C8
		ptrEtf->strSecurityID = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_SecurityIDSource))
	{
		// 证券代码源 SecurityIDSource C4 101 = 上海证券交易所 102 = 深圳证券交易所
		ptrEtf->strSecurityIDSource = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_Symbol))
	{
		// 基金名称 Symbol C40
		ptrEtf->strSymbol = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_FundManagementCompany))
	{
		// 基金公司名称 FundManagementCompany C30
		ptrEtf->strFundManagementCompany = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSecurityID))
	{
		//拟合指数代码 UnderlyingSecurityID C8
		ptrEtf->strUnderlyingSecurityID = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSecurityIDSource))
	{
		//拟合指数代码源 UnderlyingSecurityIDSource C4 
		//101 = 上海证券交易所102 = 深圳证券交易所103 = 香港交易所9999 = 其他
		ptrEtf->strUnderlyingSecurityIDSource = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CreationRedemptionUnit))
	{
		// 最小申购赎回单位 CreationRedemptionUnit N15(2) 每个篮子（最小申购赎回单位）对应的 ETF 份数，
		//目前只能为正整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CreationRedemptionUni trans failed, value=[" << strValue << "]";
			return -1;
		}
		ptrEtf->ui64CreationRedemptionUnit = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_EstimateCashComponent))
	{
		// 预估现金差额 EstimateCashComponent N11(2) T 日每个篮子的预估现金差额
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "EstimateCashComponent trans failed, value=[" << strValue << "]";
			return -1;
		}
		ptrEtf->ui64mEstimateCashComponent = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_MaxCashRatio))
	{
		// 最大现金替代比例 MaxCashRatio N6(5) 最大现金替代比例，例如：5.551％在文件中用 0.05551 表示
		dTrans = 0;
		iRes = Tgw_StringUtil::String2Double_atof(strValue, dTrans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "MaxCashRatio trans failed, value=[" << strValue << "]";
			return -1;
		}
		ptrEtf->dMaxCashRatio = dTrans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_Publish))
	{
		// 是否发布 IOPV Publish C1 Y = 是N = 否		
		if (0 == strValue.compare("1"))
		{
			ptrEtf->bPublish = true;
		}
		else
		{
			ptrEtf->bPublish = false;
		}
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_Creation))
	{
		//是否允许申购 Creation C1 Y = 是N = 否
		if (0 == strValue.compare("1"))
		{
			ptrEtf->bCreation = true;
		}
		else
		{
			ptrEtf->bCreation = false;
		}
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_Redemption))
	{
		//是否允许赎回 Redemption C1 Y = 是N = 否
		if (0 == strValue.compare("1"))
		{
			ptrEtf->bRedemption = true;
		}
		else
		{
			ptrEtf->bRedemption = false;
		}
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_RecordNum))
	{
		//深市成份证券数目 RecordNum N4 表示一个篮子中的深市成份证券数目（包含 159900 证券）
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "RecordNum trans failed, value=[" << strValue << "]";
			return -1;
		}
		ptrEtf->ui64RecordNum = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_TotalRecordNum))
	{
		//所有成份证券数目 TotalRecordNum N4 表示一个篮子中的所有成份证券数目（包含 159900 证券）
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "TotalRecordNum trans failed, value=[" << strValue << "]";
			return -1;
		}
		ptrEtf->ui64TotalRecordNum = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_TradingDay))
	{
		// 交易日 TradingDay N8 格式 YYYYMMDD
		ptrEtf->strTradingDay = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_PreTradingDay))
	{
		//前交易日 PreTradingDay N8 T - X 日日期，格式 YYYYMMDD，X 由基金公司根据基金估值时间确定
		ptrEtf->strPreTradingDay = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CashComponent))
	{
		//现金余额 CashComponent N11(2) T - X 日申购赎回基准单位的现金余额
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CashComponent trans failed, value=[" << strValue << "]";
			return -1;
		}
		ptrEtf->ui64mCashComponent = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NAVperCU))
	{
		//申购赎回基准单位净值 NAVperCU N12(2) T - X 日申购赎回基准单位净值
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NAVperCU trans failed, value=[" << strValue << "]";
			return -1;
		}
		ptrEtf->ui64mNAVperCU = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NAV))
	{
		// 单位净值 NAV N8(4) T - X 日基金的单位净值
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NAV trans failed, value=[" << strValue << "]";
			return -1;
		}
		ptrEtf->ui64mNAV = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_DividendPerCU))
	{
		//红利金额 DividendPerCU N12(2) T 日申购赎回基准单位的红利金额
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "DividendPerCU trans failed, value=[" << strValue << "]";
			return -1;
		}
		ptrEtf->ui64mDividendPerCU = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CreationLimit))
	{
		//累计申购总额限制 CreationLimit N18(2) 当天累计可申购的基金份额上限，为 0 表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CreationLimit trans failed, value=[" << strValue << "]";
			return -1;
		}
		ptrEtf->ui64CreationLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_RedemptionLimit))
	{
		//累计赎回总额限制 RedemptionLimit N18(2) 当天累计可赎回的基金份额上限，为 0 表示没有限制， 目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "RedemptionLimit trans failed, value=[" << strValue << "]";
			return -1;
		}
		ptrEtf->ui64RedemptionLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CreationLimitPerUser))
	{
		//单个账户累计申购总额限制 CreationLimitPerUser N18(2) 单个证券账户当天累计可申购的基金份额上限，
		//为 0 表示没有限制，目前只能为整数单个账户累计赎回总额限制
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CreationLimitPerUser trans failed, value=[" << strValue << "]";

			return -1;
		}
		ptrEtf->ui64CreationLimitPerUser = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_RedemptionLimitPerUser))
	{
		//单个账户累计赎回总额限制  RedemptionLimitPerUser N18(2) 单个证券账户当天累计可赎回的基金份额上限，
		//为 0 表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "RedemptionLimitPerUser trans failed, value=[" << strValue << "]";

			return -1;
		}
		ptrEtf->ui64RedemptionLimitPerUser = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NetCreationLimit))
	{
		//净申购总额限制 NetCreationLimit N18(2) 当天净申购的基金份额上限，为 0表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NetCreationLimit trans failed, value=[" << strValue << "]";

			return -1;
		}
		ptrEtf->ui64NetCreationLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NetRedemptionLimit))
	{
		//净赎回总额限制 NetRedemptionLimit N18(2) 当天净赎回的基金份额上限，为 0表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NetCreationLimit trans failed, value=[" << strValue << "]";

			return -1;
		}
		ptrEtf->ui64NetRedemptionLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NetCreationLimitPerUser))
	{
		//单个账户净申购总额限制 NetCreationLimitPerUser N18(2) 单个证券账户当天净申购的基金份额上限，
		//为 0 表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NetCreationLimitPerUser trans failed, value=[" << strValue << "]";

			return -1;
		}
		ptrEtf->ui64NetCreationLimitPerUser = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_NetRedemptionLimitPerUser))
	{
		//单个账户净赎回总额限制 NetRedemptionLimitPerUser N18(2) 单个证券账户当天净赎回的基金份额上限，
		//为 0 表示没有限制，目前只能为整数
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "NetRedemptionLimitPerUser trans failed, value=[" << strValue << "]";

			return -1;
		}
		ptrEtf->ui64NetRedemptionLimitPerUser = ui64Trans;
	}
	else
	{
		//
	}

	return 0;
}

/*
Set 成分股信息
内容来自数据库

Return:
0 -- 成功
-1 -- 失败
*/
int ETFContainer::Set_ETF_Component_FromDb(const std::string& in_strKey,
	const map<string, struct MySqlCnnC602_DF::DataInRow>& in_mapRowData,
struct simutgw::SzETFComponent& out_cpn)
{
	static const std::string ftag("Set_ETF_Component_FromDb() ");

	const map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator cit = in_mapRowData.find(in_strKey);

	if (in_mapRowData.end() == cit || cit->second.bIsNull)
	{
		return 0;
	}

	string strValue = cit->second.strValue;

	double dTrans = 0;
	uint64_t ui64Trans = 0;
	int iTrans = 0;
	int iRes = 0;

	if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSecurityID))
	{
		// 证券代码 UnderlyingSecurityID C8
		out_cpn.strUnderlyingSecurityID = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSecurityIDSource))
	{
		// 证券代码源 UnderlyingSecurityIDSource C4 101 = 上海证券交易所102 = 深圳证券交易所
		//103 = 香港交易所9999 = 其他

		out_cpn.strUnderlyingSecurityIDSource = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSymbol))
	{
		// 证券简称 UnderlyingSymbol C40
		out_cpn.strUnderlyingSymbol = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_ComponentShare))
	{
		// 成份证券数 ComponentShare N15(2) 每个申购篮子中该成份证券的数量。
		//此字段只有现金替代标志为‘0’或‘1’时才有效
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ComponentShare trans failed, value=[" << strValue << "]";
			return -1;
		}
		out_cpn.ui64ComponentShare = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_SubstituteFlag))
	{
		// 现金替代标志SubstituteFlag C1 0 = 禁止现金替代（必须有证券）
		//1 = 可以进行现金替代（先用证券，证券不足时差额部分用现金替代）2 = 必须用现金替代
		iTrans = 0;
		iRes = Tgw_StringUtil::String2Int_atoi(strValue, iTrans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "SubstituteFlag trans failed, value=[" << strValue << "]";
			return -1;
		}
		out_cpn.iSubstituteFlag = iTrans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_PremiumRatio))
	{
		// 溢价比例 PremiumRatio N7(5) 证券用现金进行替代的时候，计算价格时增加的比例。
		//例如：2.551％在文件中用 0.02551 表示；2.1%在文件中用 0.02100 表示。此字段只有现金替代标志为‘1’时才有效
		dTrans = 0;
		iRes = Tgw_StringUtil::String2Double_atof(strValue, dTrans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "PremiumRatio trans failed, value=[" << strValue << "]";
			return -1;
		}
		out_cpn.dPremiumRatio = dTrans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CreationCashSubstitute))
	{
		// 申购替代金额CreationCashSubstitute N18(4) 当某只证券必须用现金替代的时候，
		//申购时该证券所需总金额此字段只有当现金替代标志为‘2’时才有效	
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "CreationCashSubstitute trans failed, value=[" << strValue << "]";
			return -1;
		}
		out_cpn.ui64mCreationCashSubstitute = ui64Trans;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_RedemptionCashSubstitute))
	{
		// 赎回替代金额RedemptionCashSubstitute N18(4) 当某只证券必须用现金替代的时候，
		//赎回时对应该证券返还的总金额。例如： 2000 在文件中用 2000.0000表示。
		//对于跨境 ETF、跨市场 ETF、黄金 ETF 和现金债券 ETF，该字段为 0.0000。此字段只有当现金替代标志为‘2’时才有效
		ui64Trans = 0;
		iRes = Tgw_StringUtil::String2UInt64_strtoui64(strValue, ui64Trans);
		if (0 != iRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "RedemptionCashSubstitute trans failed, value=[" << strValue << "]";
			return -1;
		}
		out_cpn.ui64mRedemptionCashSubstitute = ui64Trans;
	}
	else
	{
		// do nothing.
	}

	return 0;
}