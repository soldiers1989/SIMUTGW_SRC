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
����ETF��Ϣ

@return 0 : �ɹ�
@return 1 : ʧ��
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
			// value����string����ʽ������
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
			// value����string����ʽ������
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
			out_errmsg += " �ɷֹ���Ϣ����";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strSecurityID << " �ɷֹ���Ϣ����";

			return -1;
		}

		ptr->vecComponents.push_back(etfCpn);
	}

	iRes = ValidateETF::Validate_ETF_Info(ptr);
	if (0 != iRes)
	{
		out_errmsg = strSecurityID;
		out_errmsg += " ��Ϣ����";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strSecurityID << " ��Ϣ����";
		return -1;
	}

	iRes = UpdateOneEtfPackageToDb(strSecurityID, ptr, out_errmsg);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strSecurityID << " �������ݿ����";
		return -1;
	}

	MAP_ETF_COMP::iterator it = m_mapEtfContents.find(strSecurityID);
	if (m_mapEtfContents.end() == it)
	{
		// δ�ҵ�
		m_mapEtfContents.insert(std::pair<string, std::shared_ptr<struct simutgw::SzETF>>(strSecurityID, ptr));
	}
	else
	{
		// �ҵ�
		it->second = ptr;
	}

	return 0;
}

/*
��ETF�����г��ɷֹ�֮������ݴ���ṹ��
��������Web�����

Return:
0 -- �ɹ�
-1 -- ʧ��
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
		// �汾�� Version C8 �̶�ֵ1.0
		ptrEtf->strVersion = in_strValue;
	}
	if (0 == in_strKey.compare(simutgw::client::cstrKey_SecurityID))
	{
		// ֤ȯ���� SecurityID C8
		ptrEtf->strSecurityID = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_SecurityIDSource))
	{
		// ֤ȯ����Դ SecurityIDSource C4 101 = �Ϻ�֤ȯ������ 102 = ����֤ȯ������
		ptrEtf->strSecurityIDSource = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_Symbol))
	{
		// �������� Symbol C40
		ptrEtf->strSymbol = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_FundManagementCompany))
	{
		// ����˾���� FundManagementCompany C30
		ptrEtf->strFundManagementCompany = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSecurityID))
	{
		//���ָ������ UnderlyingSecurityID C8
		ptrEtf->strUnderlyingSecurityID = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSecurityIDSource))
	{
		//���ָ������Դ UnderlyingSecurityIDSource C4 
		//101 = �Ϻ�֤ȯ������102 = ����֤ȯ������103 = ��۽�����9999 = ����
		ptrEtf->strUnderlyingSecurityIDSource = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CreationRedemptionUnit))
	{
		// ��С�깺��ص�λ CreationRedemptionUnit N15(2) ÿ�����ӣ���С�깺��ص�λ����Ӧ�� ETF ������
		//Ŀǰֻ��Ϊ������
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
		// Ԥ���ֽ��� EstimateCashComponent N11(2) T ��ÿ�����ӵ�Ԥ���ֽ���
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
		// ����ֽ�������� MaxCashRatio N6(5) ����ֽ�������������磺5.551�����ļ����� 0.05551 ��ʾ
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
		// �Ƿ񷢲� IOPV Publish C1 Y = ��N = ��		
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
		//�Ƿ������깺 Creation C1 Y = ��N = ��
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
		//�Ƿ�������� Redemption C1 Y = ��N = ��
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
		//���гɷ�֤ȯ��Ŀ RecordNum N4 ��ʾһ�������е����гɷ�֤ȯ��Ŀ������ 159900 ֤ȯ��
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
		//���гɷ�֤ȯ��Ŀ TotalRecordNum N4 ��ʾһ�������е����гɷ�֤ȯ��Ŀ������ 159900 ֤ȯ��
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
		// ������ TradingDay N8 ��ʽ YYYYMMDD
		ptrEtf->strTradingDay = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_PreTradingDay))
	{
		//ǰ������ PreTradingDay N8 T - X �����ڣ���ʽ YYYYMMDD��X �ɻ���˾���ݻ����ֵʱ��ȷ��
		ptrEtf->strPreTradingDay = in_strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CashComponent))
	{
		//�ֽ���� CashComponent N11(2) T - X ���깺��ػ�׼��λ���ֽ����
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
		//�깺��ػ�׼��λ��ֵ NAVperCU N12(2) T - X ���깺��ػ�׼��λ��ֵ
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
		// ��λ��ֵ NAV N8(4) T - X �ջ���ĵ�λ��ֵ
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
		//������� DividendPerCU N12(2) T ���깺��ػ�׼��λ�ĺ������
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
		//�ۼ��깺�ܶ����� CreationLimit N18(2) �����ۼƿ��깺�Ļ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
		//�ۼ�����ܶ����� RedemptionLimit N18(2) �����ۼƿ���صĻ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ� Ŀǰֻ��Ϊ����
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
		//�����˻��ۼ��깺�ܶ����� CreationLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ��깺�Ļ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ���������˻��ۼ�����ܶ�����
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
		//�����˻��ۼ�����ܶ�����  RedemptionLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ���صĻ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
		//���깺�ܶ����� NetCreationLimit N18(2) ���쾻�깺�Ļ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
		//������ܶ����� NetRedemptionLimit N18(2) ���쾻��صĻ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
		//�����˻����깺�ܶ����� NetCreationLimitPerUser N18(2) ����֤ȯ�˻����쾻�깺�Ļ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
		//�����˻�������ܶ����� NetRedemptionLimitPerUser N18(2) ����֤ȯ�˻����쾻��صĻ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
Set �ɷֹ���Ϣ
��������Web�����

Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ETFContainer::Set_ETF_Component_FromWebControl(rapidjson::Value& docValue,
struct simutgw::SzETFComponent& out_cpn, std::string& out_errmsg)
{
	static const std::string ftag("ETFContainer::Set_ETF_Component_FromWebControl() ");

	double dTrans = 0;
	uint64_t ui64Trans = 0;
	int iTrans = 0;
	int iRes = 0;

	// ֤ȯ���� UnderlyingSecurityID C8
	if (!docValue.HasMember(simutgw::client::cstrKey_UnderlyingSecurityID.c_str()))
	{
		out_errmsg = "error UnderlyingSecurityID";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error UnderlyingSecurityID";
		return -1;
	}
	out_cpn.strUnderlyingSecurityID = docValue[simutgw::client::cstrKey_UnderlyingSecurityID.c_str()].GetString();

	// ֤ȯ����Դ UnderlyingSecurityIDSource C4 101 = �Ϻ�֤ȯ������102 = ����֤ȯ������
	//103 = ��۽�����9999 = ����
	if (!docValue.HasMember(simutgw::client::cstrKey_UnderlyingSecurityIDSource.c_str()))
	{
		out_errmsg = "error UnderlyingSecurityIDSource";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error UnderlyingSecurityIDSource";
		return -1;
	}
	out_cpn.strUnderlyingSecurityIDSource = docValue[simutgw::client::cstrKey_UnderlyingSecurityIDSource.c_str()].GetString();

	// ֤ȯ��� UnderlyingSymbol C40
	if (!docValue.HasMember(simutgw::client::cstrKey_UnderlyingSymbol))
	{
		out_errmsg = "error UnderlyingSymbol";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error UnderlyingSymbol";
		return -1;
	}
	out_cpn.strUnderlyingSymbol = docValue[simutgw::client::cstrKey_UnderlyingSymbol].GetString();

	// �ɷ�֤ȯ�� ComponentShare N15(2) ÿ���깺�����иóɷ�֤ȯ��������
	//���ֶ�ֻ���ֽ������־Ϊ��0����1��ʱ����Ч
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

	// �ֽ������־SubstituteFlag C1 0 = ��ֹ�ֽ������������֤ȯ��
	//1 = ���Խ����ֽ����������֤ȯ��֤ȯ����ʱ�������ֽ������2 = �������ֽ����
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

	// ��۱��� PremiumRatio N7(5) ֤ȯ���ֽ���������ʱ�򣬼���۸�ʱ���ӵı�����
	//���磺2.551�����ļ����� 0.02551 ��ʾ��2.1%���ļ����� 0.02100 ��ʾ�����ֶ�ֻ���ֽ������־Ϊ��1��ʱ����Ч
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

	// �깺������CreationCashSubstitute N18(4) ��ĳֻ֤ȯ�������ֽ������ʱ��
	//�깺ʱ��֤ȯ�����ܽ����ֶ�ֻ�е��ֽ������־Ϊ��2��ʱ����Ч
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


	// ���������RedemptionCashSubstitute N18(4) ��ĳֻ֤ȯ�������ֽ������ʱ��
	//���ʱ��Ӧ��֤ȯ�������ܽ����磺 2000 ���ļ����� 2000.0000��ʾ��
	//���ڿ羳 ETF�����г� ETF���ƽ� ETF ���ֽ�ծȯ ETF�����ֶ�Ϊ 0.0000�����ֶ�ֻ�е��ֽ������־Ϊ��2��ʱ����Ч
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
ִ�и���sql���

@return 0 : ���³ɹ�
@return -1 : ����ʧ��
*/
int ETFContainer::UpdateOneEtfPackageToDb(const std::string& in_strSecurityID,
	std::shared_ptr<struct simutgw::SzETF>& ptrEtf, std::string& out_errmsg)
{
	static const std::string ftag("UpdateOneEtfPackageToDb() ");

	try	{
		//��mysql���ӳ�ȡ����
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//ȡ����mysql����ΪNULL

			//�黹����
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			out_errmsg = "Get Connection is NULL";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Get Connection is NULL";

			return -1;
		}

		mysqlConn->StartTransaction();

		int iRes = Update_EtfInfo(mysqlConn, ptrEtf);
		if (0 != iRes)
		{
			// ʧ��
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
			// ʧ��
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
		EzLog::e(ftag, "δ֪�쳣");
		return -1;
	}

	return 0;
}

/*
ִ�и���sql���

@return 0 : ���³ɹ�
@return -1 : ����ʧ��
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
		// �Ǹ���
		if (2 < ulAffectedRows)
		{
			// ʧ��
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "����[" << strUpdateSql
				<< "]�õ�AffectedRows=" << ulAffectedRows;

			return -1;
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "����[" << strUpdateSql
			<< "]�õ�Res=" << iRes;
		return -1;
	}

	return 0;
}

/*
ִ�и���sql���

@return 0 : ���³ɹ�
@return -1 : ����ʧ��
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
		// �Ǹ���
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "����[" << strDeleteSql
			<< "]�õ�Res=" << iRes;
		return -1;
	}

	string strUpdateSql_Head = "INSERT INTO ";
	strUpdateSql_Head += simutgw::g_DBTable_etf_component;
	strUpdateSql_Head += " (`SecurityID` ,`UnderlyingSecurityID`,`UnderlyingSecurityIDSource`,`UnderlyingSymbol`,`ComponentShare`,"
		"`SubstituteFlag`,`PremiumRatio`,`CreationCashSubstitute`,`RedemptionCashSubstitute`) VALUES ";

	// һ���������������
	const size_t iBatchInsertSize = 50;
	// ��ǰ���β�������
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

			// ��������������������������
			pResultSet = NULL;
			ulAffectedRows = 0;
			iRes = in_mysqlConn->Query(strUpdateSql, &pResultSet, ulAffectedRows);
			if (2 == iRes)
			{
				// �Ǹ���
				if (0 == ulAffectedRows)
				{
					// ʧ��
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "����[" << strUpdateSql
						<< "]�õ�AffectedRows=" << ulAffectedRows;

					return -1;
				}
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "����[" << strUpdateSql
					<< "]�õ�Res=" << iRes;
				return -1;
			}

			// ����������������
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
�����ݿ��������� ETF��Ϣ

@return 0 : �ɹ�
@return -1 : ʧ��
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

		std::vector<string> vctEtfStockIds;
		int iRes = GetFromDB_AllEtfStockIds(mysqlConn, vctEtfStockIds);
		if (0 != iRes)
		{
			//�黹����
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

			// ÿ��ETF����ѭ������
			iRes = GetFromDB_OneEtfPackage(mysqlConn, strSecurityID, ptr);
			if (0 != iRes)
			{
				//�黹����
				simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "GetFromDB_OneEtfPackage failed";
				return -1;
			}

			MAP_ETF_COMP::iterator it = m_mapEtfContents.find(strSecurityID);
			if (m_mapEtfContents.end() == it)
			{
				// δ�ҵ�
				m_mapEtfContents.insert(std::pair<string, std::shared_ptr<struct simutgw::SzETF>>(strSecurityID, ptr));
			}
			else
			{
				// �ҵ�
				it->second = ptr;
			}
		}

		//�黹����
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
��ȡ���е� ETF SecurityId

Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int ETFContainer::GetFromDB_AllEtfStockIds(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
	std::vector<string>& out_vctEtfStockIds)
{
	static const string ftag("GetFromDB_AllEtfStockIds() ");

	try
	{
		MYSQL_RES *pResultSet = NULL;
		unsigned long ulAffectedRows = 0;

		// ��ѯ
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

			// �ͷ�
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
�����ݿ��ȡһ��ETF�����ȫ������

Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int ETFContainer::GetFromDB_OneEtfPackage(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::string& in_strSecurityID,
	std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	static const string ftag("GetFromDB_OneEtfPackage() ");

	int iRes = GetFromDB_OneEtfInfo(in_mysqlConn, in_strSecurityID, ptrEtf);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << in_strSecurityID << " ��Ϣ����";
		return -1;
	}

	iRes = GetFromDB_OneEtfComponents(in_mysqlConn, in_strSecurityID, ptrEtf);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << in_strSecurityID << " ��Ϣ����";
		return -1;
	}

	iRes = ValidateETF::Validate_ETF_Info(ptrEtf);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << in_strSecurityID << " ��Ϣ����";
		return -1;
	}

	return 0;
}

/*
�����ݿ��ȡһ��ETF�����info

Return :
0 -- �ɹ�
��0 -- ʧ��
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

		// ��ѯ
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

			// �ͷ�
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
�����ݿ��ȡһ��ETF��������е�Components

Return :
0 -- �ɹ�
��0 -- ʧ��
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

		// ��ѯ
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
					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << in_strSecurityID << " �ɷֹ���Ϣ����";
					return -1;
				}

				ptrEtf->vecComponents.push_back(etfCpn);
			}

			// �ͷ�
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
��ETF�����г��ɷֹ�֮������ݴ���ṹ��
�����������ݿ�

Return:
0 -- �ɹ�
-1 -- ʧ��
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
		// �汾�� Version C8 �̶�ֵ1.0
		ptrEtf->strVersion = strValue;
	}
	if (0 == in_strKey.compare(simutgw::client::cstrKey_SecurityID))
	{
		// ֤ȯ���� SecurityID C8
		ptrEtf->strSecurityID = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_SecurityIDSource))
	{
		// ֤ȯ����Դ SecurityIDSource C4 101 = �Ϻ�֤ȯ������ 102 = ����֤ȯ������
		ptrEtf->strSecurityIDSource = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_Symbol))
	{
		// �������� Symbol C40
		ptrEtf->strSymbol = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_FundManagementCompany))
	{
		// ����˾���� FundManagementCompany C30
		ptrEtf->strFundManagementCompany = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSecurityID))
	{
		//���ָ������ UnderlyingSecurityID C8
		ptrEtf->strUnderlyingSecurityID = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSecurityIDSource))
	{
		//���ָ������Դ UnderlyingSecurityIDSource C4 
		//101 = �Ϻ�֤ȯ������102 = ����֤ȯ������103 = ��۽�����9999 = ����
		ptrEtf->strUnderlyingSecurityIDSource = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CreationRedemptionUnit))
	{
		// ��С�깺��ص�λ CreationRedemptionUnit N15(2) ÿ�����ӣ���С�깺��ص�λ����Ӧ�� ETF ������
		//Ŀǰֻ��Ϊ������
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
		// Ԥ���ֽ��� EstimateCashComponent N11(2) T ��ÿ�����ӵ�Ԥ���ֽ���
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
		// ����ֽ�������� MaxCashRatio N6(5) ����ֽ�������������磺5.551�����ļ����� 0.05551 ��ʾ
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
		// �Ƿ񷢲� IOPV Publish C1 Y = ��N = ��		
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
		//�Ƿ������깺 Creation C1 Y = ��N = ��
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
		//�Ƿ�������� Redemption C1 Y = ��N = ��
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
		//���гɷ�֤ȯ��Ŀ RecordNum N4 ��ʾһ�������е����гɷ�֤ȯ��Ŀ������ 159900 ֤ȯ��
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
		//���гɷ�֤ȯ��Ŀ TotalRecordNum N4 ��ʾһ�������е����гɷ�֤ȯ��Ŀ������ 159900 ֤ȯ��
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
		// ������ TradingDay N8 ��ʽ YYYYMMDD
		ptrEtf->strTradingDay = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_PreTradingDay))
	{
		//ǰ������ PreTradingDay N8 T - X �����ڣ���ʽ YYYYMMDD��X �ɻ���˾���ݻ����ֵʱ��ȷ��
		ptrEtf->strPreTradingDay = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_CashComponent))
	{
		//�ֽ���� CashComponent N11(2) T - X ���깺��ػ�׼��λ���ֽ����
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
		//�깺��ػ�׼��λ��ֵ NAVperCU N12(2) T - X ���깺��ػ�׼��λ��ֵ
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
		// ��λ��ֵ NAV N8(4) T - X �ջ���ĵ�λ��ֵ
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
		//������� DividendPerCU N12(2) T ���깺��ػ�׼��λ�ĺ������
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
		//�ۼ��깺�ܶ����� CreationLimit N18(2) �����ۼƿ��깺�Ļ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
		//�ۼ�����ܶ����� RedemptionLimit N18(2) �����ۼƿ���صĻ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ� Ŀǰֻ��Ϊ����
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
		//�����˻��ۼ��깺�ܶ����� CreationLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ��깺�Ļ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ���������˻��ۼ�����ܶ�����
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
		//�����˻��ۼ�����ܶ�����  RedemptionLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ���صĻ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
		//���깺�ܶ����� NetCreationLimit N18(2) ���쾻�깺�Ļ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
		//������ܶ����� NetRedemptionLimit N18(2) ���쾻��صĻ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
		//�����˻����깺�ܶ����� NetCreationLimitPerUser N18(2) ����֤ȯ�˻����쾻�깺�Ļ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
		//�����˻�������ܶ����� NetRedemptionLimitPerUser N18(2) ����֤ȯ�˻����쾻��صĻ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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
Set �ɷֹ���Ϣ
�����������ݿ�

Return:
0 -- �ɹ�
-1 -- ʧ��
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
		// ֤ȯ���� UnderlyingSecurityID C8
		out_cpn.strUnderlyingSecurityID = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSecurityIDSource))
	{
		// ֤ȯ����Դ UnderlyingSecurityIDSource C4 101 = �Ϻ�֤ȯ������102 = ����֤ȯ������
		//103 = ��۽�����9999 = ����

		out_cpn.strUnderlyingSecurityIDSource = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_UnderlyingSymbol))
	{
		// ֤ȯ��� UnderlyingSymbol C40
		out_cpn.strUnderlyingSymbol = strValue;
	}
	else if (0 == in_strKey.compare(simutgw::client::cstrKey_ComponentShare))
	{
		// �ɷ�֤ȯ�� ComponentShare N15(2) ÿ���깺�����иóɷ�֤ȯ��������
		//���ֶ�ֻ���ֽ������־Ϊ��0����1��ʱ����Ч
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
		// �ֽ������־SubstituteFlag C1 0 = ��ֹ�ֽ������������֤ȯ��
		//1 = ���Խ����ֽ����������֤ȯ��֤ȯ����ʱ�������ֽ������2 = �������ֽ����
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
		// ��۱��� PremiumRatio N7(5) ֤ȯ���ֽ���������ʱ�򣬼���۸�ʱ���ӵı�����
		//���磺2.551�����ļ����� 0.02551 ��ʾ��2.1%���ļ����� 0.02100 ��ʾ�����ֶ�ֻ���ֽ������־Ϊ��1��ʱ����Ч
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
		// �깺������CreationCashSubstitute N18(4) ��ĳֻ֤ȯ�������ֽ������ʱ��
		//�깺ʱ��֤ȯ�����ܽ����ֶ�ֻ�е��ֽ������־Ϊ��2��ʱ����Ч	
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
		// ���������RedemptionCashSubstitute N18(4) ��ĳֻ֤ȯ�������ֽ������ʱ��
		//���ʱ��Ӧ��֤ȯ�������ܽ����磺 2000 ���ļ����� 2000.0000��ʾ��
		//���ڿ羳 ETF�����г� ETF���ƽ� ETF ���ֽ�ծȯ ETF�����ֶ�Ϊ 0.0000�����ֶ�ֻ�е��ֽ������־Ϊ��2��ʱ����Ч
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