#include "ETF_Conf_Reader.h"

#include "boost/filesystem.hpp"
#include <sstream>

#include "boost/foreach.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"
#include "boost/algorithm/string.hpp"

#include "etf/ValidateETF.h"

#include "tool_string/Tgw_StringUtil.h"

#include "util/EzLog.h"
#include "util/FileHandler.h"
#include "simutgw/stgw_config/g_values_biz.h"

ETF_Conf_Reader::ETF_Conf_Reader()
{
}


ETF_Conf_Reader::~ETF_Conf_Reader()
{
}


/*
读取深圳pcf文件
Return:
0 -- 成功
-1 -- 失败
*/
int ETF_Conf_Reader::Read_SZ_ETF_Pcf_Conf(const std::string& in_strFileName)
{
	static const std::string strTag("ETF_Conf_Reader::Read_SZ_ETF_Pcf_Conf() ");
	//std::string strFilePath("d://SimuTGW//simutgw//data//etf//sz//pcf_159901_20150115.xml");

	boost::filesystem::path filePath(in_strFileName);

	// 判断是否存在
	bool bExist = boost::filesystem::exists(filePath);
	if (!bExist)
	{
		std::string strError("file[");
		strError += in_strFileName;
		strError += "] not exitsts";
		EzLog::e(strTag, strError);

		return -1;
	}

	// 读取文件内容
	std::string strXMLContent;
	FileHandler xmlFile;
	int iRes = xmlFile.ReadFileContent(in_strFileName, strXMLContent, nullptr);
	if (0 != iRes)
	{
		std::string strError("file[");
		strError += in_strFileName;
		strError += "] xmlFile.ReadFileContent failed";
		EzLog::e(strTag, strError);

		return -1;
	}

	// 解析xml文件
	iRes = Parse_Pcf_XML_And_Store(strXMLContent);
	if (0 != iRes)
	{
		EzLog::e(strTag, in_strFileName + " 解析错误");
	}
	else
	{
		EzLog::i(strTag, in_strFileName + " 解析成功");
	}

	return iRes;
}

/*
读取上海pcf文件
Return:
0 -- 成功
-1 -- 失败
*/
int ETF_Conf_Reader::Read_SH_ETF_Conf(const std::string& in_strFileName)
{
	static const std::string strTag("ETF_Conf_Reader::Read_SH_ETF_Conf() ");
	//std::string strFilePath("d://SimuTGW//simutgw//data//etf//sz//pcf_159901_20150115.xml");

	//std::tr2::sys::path filePath("d:\Develop\VC\2013\etf_config\sh\5100301213.ETF");

	boost::filesystem::path filePath(in_strFileName);

	// 判断是否存在
	bool bExist = boost::filesystem::exists(filePath);
	if (!bExist)
	{
		std::string strError("file[");
		strError += in_strFileName;
		strError += "] not exitsts";
		EzLog::e(strTag, strError);

		return -1;
	}

	// 读取文件内容
	std::string strSHETFContent;
	FileHandler etfFile;
	int iRes = etfFile.ReadFileContent(in_strFileName, strSHETFContent, nullptr);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = Parse_SH_ETF_And_Store(strSHETFContent);
	if (0 != iRes)
	{
		EzLog::e(strTag, in_strFileName + " 解析错误");
	}

	return iRes;
}

/*
解析深圳pcf文件
Return:
0 -- 成功
-1 -- 失败
*/
int ETF_Conf_Reader::Parse_Pcf_XML_And_Store(const std::string& in_strXMLContent)
{
	static const std::string strTag("ETF_Conf_Reader::Parse_Pcf_XML_And_Store() ");

	try
	{
		boost::property_tree::ptree xmlPcf;
		std::stringstream ssXML(in_strXMLContent);
		boost::property_tree::read_xml(ssXML, xmlPcf);

		std::string key;
		std::string strData;

		std::shared_ptr<struct simutgw::SzETF> ptrEtf(new struct simutgw::SzETF);
		if (nullptr == ptrEtf)
		{
			EzLog::e(strTag, "No memory");
			return -1;
		}

		for (boost::property_tree::ptree::value_type node : xmlPcf.get_child("PCFFile"))
		{
			if (node.first == "<xmlcomment>") //去掉注释
				continue;

			key = node.first;

			if (0 == key.compare("Components"))
			{
				for (boost::property_tree::ptree::value_type nodeComps : node.second.get_child(""))
				{
					struct simutgw::SzETFComponent etfCpn;
					for (boost::property_tree::ptree::value_type nodeComp : nodeComps.second.get_child(""))
					{
						key = nodeComp.first;
						strData = nodeComp.second.data();

						Set_ETFComponent_Info(etfCpn, key, strData);
					}

					int iRes = ValidateETF::Validate_ETFComponent_Info(etfCpn);
					if (0 != iRes)
					{
						EzLog::e(strTag, ptrEtf->strSecurityID + " 成分股信息有误");
						return -1;
					}
					ptrEtf->vecComponents.push_back(etfCpn);
				}
			}
			else
			{
				strData = node.second.data();

				Set_ETF_Info(ptrEtf, key, strData);
			}
		}

		Add_ETF_Info(ptrEtf);
	}
	catch (exception& e)
	{
		EzLog::ex(strTag, e);
		return -1;
	}

	return 0;
}

/*
解析上海ETF文件
Return:
0 -- 成功
-1 -- 失败
*/
int ETF_Conf_Reader::Parse_SH_ETF_And_Store(const std::string& in_strETFContent)
{
	static const std::string strTag("ETF_Conf_Reader::Parse_SH_ETF_And_Store() ");

	const char cszReturn[] = { "\r\n" };

	try
	{
		std::vector<string> vecLine;
		// 按行切割
		boost::split(vecLine, in_strETFContent, boost::is_any_of(cszReturn));

		std::shared_ptr<struct simutgw::SzETF> ptrEtf(new struct simutgw::SzETF);
		if (nullptr == ptrEtf)
		{
			EzLog::e(strTag, "No memory");
			return -1;
		}

		bool bComponent = false;
		std::vector<string> vecMember;
		for (std::string strTmp : vecLine)
		{
			vecMember.clear();
			if (!bComponent)
			{
				boost::split(vecMember, strTmp, boost::is_any_of("="));

				if (1 == vecMember.size())
				{
					if (0 == vecMember[0].compare("TAGTAG"))
					{
						bComponent = true;
					}
					else
					{

					}
				}
				else if (2 == vecMember.size())
				{
					// 普通value
					Set_SH_ETF_Info(ptrEtf, vecMember[0], vecMember[1]);
				}
			}
			else
			{
				// 是成分股
				boost::split(vecMember, strTmp, boost::is_any_of("|"));
				if (7 == vecMember.size())
				{
					struct simutgw::SzETFComponent etfCpn;

					int iRes = Set_SHETFComponent_Info(etfCpn, vecMember);
					if (0 != iRes)
					{
						EzLog::e(strTag, ptrEtf->strSecurityID + " Set_SHETFComponent_Info failed");
						return -1;
					}

					iRes = ValidateETF::Validate_ETFComponent_Info(etfCpn);
					if (0 != iRes)
					{
						EzLog::e(strTag, ptrEtf->strSecurityID + " 成分股信息有误");
						return -1;
					}

					ptrEtf->vecComponents.push_back(etfCpn);
				}
				else
				{
					if (1 == vecMember.size() && 0 == vecMember[0].compare("ENDENDEND"))
					{
						bComponent = false;
					}
					else
					{
						EzLog::e(strTag, "Error ETF component[" + strTmp);
					}
				}
			}
		}

		Add_ETF_Info(ptrEtf);
	}
	catch (exception& e)
	{
		EzLog::ex(strTag, e);
		return -1;
	}

	return 0;
}

/*
将pcf内容中除成分股之外的内容存入结构体
Return:
0 -- 成功
-1 -- 失败
*/
int ETF_Conf_Reader::Set_ETF_Info(std::shared_ptr<struct simutgw::SzETF>& ptrEtf,
	const std::string& in_strKey, const std::string& in_strValue)
{
	static const std::string strTag("ETF_Conf_Reader::Set_ETF_Info() ");

	if (nullptr == ptrEtf)
	{
		EzLog::e(strTag, "ptrEtf is null");
		return -1;
	}

	uint64_t ui64Trans = 0;
	double dTrans = 0;

	if (0 == in_strKey.compare("SecurityID"))
	{
		ptrEtf->strSecurityID = in_strValue;
	}
	else if (0 == in_strKey.compare("SecurityIDSource"))
	{
		ptrEtf->strSecurityIDSource = in_strValue;
	}
	else if (0 == in_strKey.compare("UnderlyingSecurityID"))
	{
		ptrEtf->strUnderlyingSecurityID = in_strValue;
	}
	else if (0 == in_strKey.compare("UnderlyingSecurityIDSource"))
	{
		ptrEtf->strUnderlyingSecurityIDSource = in_strValue;
	}
	else if (0 == in_strKey.compare("CreationRedemptionUnit"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64CreationRedemptionUnit = ui64Trans;
	}
	else if (0 == in_strKey.compare("EstimateCashComponent"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64mEstimateCashComponent = ui64Trans;
	}
	else if (0 == in_strKey.compare("MaxCashRatio"))
	{
		dTrans = 0;
		Tgw_StringUtil::String2Double_atof(in_strValue, dTrans);
		ptrEtf->dMaxCashRatio = dTrans;
	}
	else if (0 == in_strKey.compare("Publish"))
	{
		if (0 == in_strValue.compare("Y"))
		{
			ptrEtf->bPublish = true;
		}
		else
		{
			ptrEtf->bPublish = false;
		}
	}
	else if (0 == in_strKey.compare("Creation"))
	{
		if (0 == in_strValue.compare("Y"))
		{
			ptrEtf->bCreation = true;
		}
		else
		{
			ptrEtf->bCreation = false;
		}
	}
	else if (0 == in_strKey.compare("Redemption"))
	{
		if (0 == in_strValue.compare("Y"))
		{
			ptrEtf->bRedemption = true;
		}
		else
		{
			ptrEtf->bRedemption = false;
		}
	}
	else if (0 == in_strKey.compare("RecordNum"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64RecordNum = ui64Trans;
	}
	else if (0 == in_strKey.compare("TotalRecordNum"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64TotalRecordNum = ui64Trans;
	}
	else if (0 == in_strKey.compare("TradingDay"))
	{
		ptrEtf->strTradingDay = in_strValue;
	}
	else if (0 == in_strKey.compare("PreTradingDay"))
	{
		ptrEtf->strPreTradingDay = in_strValue;
	}
	else if (0 == in_strKey.compare("CashComponent"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64mCashComponent = ui64Trans;
	}
	else if (0 == in_strKey.compare("NAVperCU"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64mNAVperCU = ui64Trans;
	}
	else if (0 == in_strKey.compare("NAV"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64mNAV = ui64Trans;
	}
	else if (0 == in_strKey.compare("DividendPerCU"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64mDividendPerCU = ui64Trans;
	}
	else if (0 == in_strKey.compare("CreationLimit"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64CreationLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare("RedemptionLimit"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64RedemptionLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare("CreationLimitPerUser"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64CreationLimitPerUser = ui64Trans;
	}
	else if (0 == in_strKey.compare("RedemptionLimitPerUser"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64RedemptionLimitPerUser = ui64Trans;
	}
	else if (0 == in_strKey.compare("NetCreationLimit"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64NetCreationLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare("NetRedemptionLimit"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64NetRedemptionLimit = ui64Trans;
	}
	else if (0 == in_strKey.compare("NetCreationLimitPerUser"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64NetCreationLimitPerUser = ui64Trans;
	}
	else if (0 == in_strKey.compare("NetRedemptionLimitPerUser"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64NetRedemptionLimitPerUser = ui64Trans;
	}
	else
	{

	}

	return 0;
}

/*
Set成分股信息
Return:
0 -- 成功
-1 -- 失败
*/
int ETF_Conf_Reader::Set_ETFComponent_Info(struct simutgw::SzETFComponent& cpn,
	const std::string& in_strKey, const std::string& in_strValue)
{
	static const std::string strTag("ETF_Conf_Reader::Set_ETFComponent_Info() ");

	double dTrans = 0;
	uint64_t ui64Trans = 0;
	if (0 == in_strKey.compare("UnderlyingSecurityID"))
	{
		cpn.strUnderlyingSecurityID = in_strValue;
	}
	else if (0 == in_strKey.compare("UnderlyingSecurityIDSource"))
	{
		cpn.strUnderlyingSecurityIDSource = in_strValue;
	}
	else if (0 == in_strKey.compare("SubstituteFlag"))
	{
		int iTrans;
		Tgw_StringUtil::String2Int_atoi(in_strValue, iTrans);
		if (0 == iTrans || 1 == iTrans || 2 == iTrans)
		{
			cpn.iSubstituteFlag = iTrans;
		}
		else
		{
			EzLog::e(strTag, "Error SubstituteFlag[" + in_strValue);
		}
	}
	else if (0 == in_strKey.compare("ComponentShare"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		cpn.ui64ComponentShare = ui64Trans;
	}
	else if (0 == in_strKey.compare("PremiumRatio"))
	{
		dTrans = 0;
		Tgw_StringUtil::String2Double_atof(in_strValue, dTrans);
		cpn.dPremiumRatio = dTrans;
	}
	else if (0 == in_strKey.compare("CreationCashSubstitute"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		cpn.ui64mCreationCashSubstitute = ui64Trans;
	}
	else if (0 == in_strKey.compare("RedemptionCashSubstitute"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		cpn.ui64mRedemptionCashSubstitute = ui64Trans;
	}
	else
	{

	}

	return 0;
}

/*
将SH ETF内容中除成分股之外的内容存入结构体
Return:
0 -- 成功
-1 -- 失败
*/
int ETF_Conf_Reader::Set_SH_ETF_Info(std::shared_ptr<struct simutgw::SzETF>& ptrEtf,
	const std::string& in_strKey, const std::string& in_strValue)
{
	static const std::string strTag("ETF_Conf_Reader::Set_SH_ETF_Info() ");

	if (nullptr == ptrEtf)
	{
		EzLog::e(strTag, "ptrEtf is null");
		return -1;
	}

	uint64_t ui64Trans = 0;
	double dTrans = 0;

	if (0 == in_strKey.compare("Fundid1"))
	{
		ptrEtf->strSecurityID = in_strValue;
	}
	else if (0 == in_strKey.compare("CreationRedemptionUnit"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64CreationRedemptionUnit = ui64Trans;
	}
	else if (0 == in_strKey.compare("EstimateCashComponent"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64mEstimateCashComponent = ui64Trans;
	}
	else if (0 == in_strKey.compare("MaxCashRatio"))
	{
		dTrans = 0;
		Tgw_StringUtil::String2Double_atof(in_strValue, dTrans);
		ptrEtf->dMaxCashRatio = dTrans;
	}
	else if (0 == in_strKey.compare("Publish"))
	{
		if (0 == in_strValue.compare("Y"))
		{
			ptrEtf->bPublish = true;
		}
		else
		{
			ptrEtf->bPublish = false;
		}
	}
	else if (0 == in_strKey.compare("CreationRedemption"))
	{
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		if (simutgw::NotCre_AND_NotRed == ui64Trans)
		{
			//0 - 不允许申购/赎回
			ptrEtf->bCreation = false;
			ptrEtf->bRedemption = false;
		}
		else if (simutgw::Cre_AND_Red == ui64Trans)
		{
			//1 - 申购和赎回皆允许
			ptrEtf->bCreation = true;
			ptrEtf->bRedemption = true;
		}
		else if (simutgw::Cre_AND_NotRed == ui64Trans)
		{
			//2 - 仅允许申购
			ptrEtf->bCreation = true;
			ptrEtf->bRedemption = false;
		}
		else if (simutgw::NotCre_AND_Red == ui64Trans)
		{
			//3 - 仅允许赎回
			ptrEtf->bCreation = false;
			ptrEtf->bRedemption = true;
		}
	}
	else if (0 == in_strKey.compare("Recordnum"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64TotalRecordNum = ui64Trans;
	}
	else if (0 == in_strKey.compare("TradingDay"))
	{
		ptrEtf->strTradingDay = in_strValue;
	}
	else if (0 == in_strKey.compare("PreTradingDay"))
	{
		ptrEtf->strPreTradingDay = in_strValue;
	}
	else if (0 == in_strKey.compare("CashComponent"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64mCashComponent = ui64Trans;
	}
	else if (0 == in_strKey.compare("NAVperCU"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64mNAVperCU = ui64Trans;
	}
	else if (0 == in_strKey.compare("NAV"))
	{
		ui64Trans = 0;
		Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(in_strValue, ui64Trans);
		ptrEtf->ui64mNAV = ui64Trans;
	}
	else
	{

	}

	return 0;
}

/*
Set SH成分股信息
Return:
0 -- 成功
-1 -- 失败
*/
int ETF_Conf_Reader::Set_SHETFComponent_Info(struct simutgw::SzETFComponent& cpn,
	const std::vector<std::string>& vecComponent)
{
	static const std::string strTag("ETF_Conf_Reader::Set_SHETFComponent_Info() ");

	if (7 != vecComponent.size())
	{
		EzLog::e(strTag, "Componet 格式错误");
		return -1;
	}

	double dTrans = 0;
	uint64_t ui64Trans = 0;
	cpn.strUnderlyingSecurityID = vecComponent[0];
	cpn.strUnderlyingSymbol = vecComponent[1];
	Tgw_StringUtil::String2UInt64_strtoui64(vecComponent[2], ui64Trans);
	cpn.ui64ComponentShare = ui64Trans;
	int iTrans = 0;
	Tgw_StringUtil::String2Int_atoi(vecComponent[3], iTrans);
	cpn.iSubstituteFlag = iTrans;

	Tgw_StringUtil::String2Double_atof(vecComponent[4], dTrans);
	cpn.dPremiumRatio = dTrans;

	ui64Trans = 0;
	Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(vecComponent[5], ui64Trans);

	cpn.ui64mCreationCashSubstitute = cpn.ui64mRedemptionCashSubstitute = ui64Trans;

	return 0;
}

/*
添加ETF信息
*/
void ETF_Conf_Reader::Add_ETF_Info(std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	static const std::string strTag("ETF_Conf_Reader::Add_ETF_Info() ");

	int iRes = ValidateETF::Validate_ETF_Info(ptrEtf);
	if (0 != iRes)
	{
		EzLog::e(strTag, ptrEtf->strSecurityID + " 信息有误");
		return;
	}

	simutgw::etfWriteLock Locker(simutgw::g_mtxMapSZEtf);
	simutgw::g_mapSZETF[ptrEtf->strSecurityID] = ptrEtf;
}
