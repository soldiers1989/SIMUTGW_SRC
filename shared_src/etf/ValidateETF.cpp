#include "ValidateETF.h"
#include "util/EzLog.h"

ValidateETF::ValidateETF()
{
}


ValidateETF::~ValidateETF()
{
}


/*
У��ɷֹ���Ϣ
Return:
0 -- �Ϸ�
-1 -- �Ƿ�
*/
int ValidateETF::Validate_ETFComponent_Info(struct simutgw::SzETFComponent& cpn)
{
	static const std::string strTag("ValidateETF::Validate_ETFComponent_Info() ");

	if (cpn.strUnderlyingSecurityID.empty())
	{
		EzLog::e(strTag, "�ɷֹɴ���Ϊ��");
		return -1;
	}

	if (0 == cpn.strUnderlyingSecurityID.compare("159900"))
	{
		//����ɷ�֤ȯ 159900�������ֽ����������Ϣ��������ɷ�֤ȯ
		//�� �ֽ������־���ֶ�Ϊ�������ֽ���������� ֤ȯ����Դ���ֶ�Ϊ�� 102������ʾ����
		//��ɷ�֤ȯ������֤ȯ�����깺���ʱ���Ը�����ɷ�֤ȯ����ʹ���ֽ������
		if (simutgw::Only_Cash != cpn.iSubstituteFlag)
		{
			EzLog::e(strTag, "�ɷֹ�159900�ֽ������������");
			return -1;
		}

		if (0 == cpn.ui64mCreationCashSubstitute)
		{
			EzLog::e(strTag, "�ɷֹ�159900����������");
			return -1;
		}

		return 0;
	}

	// �����Ƿ�����ֽ���������ж�
	if (simutgw::Only_Security == cpn.iSubstituteFlag)
	{
		// �������ֽ���� 
		if (0 == cpn.ui64ComponentShare)
		{
			// �ɷֹ�����Ϊ0
			EzLog::e(strTag, "�ɷֹ�����Ϊ0");
			return -1;
		}
	}
	else if (simutgw::Security_And_Cash == cpn.iSubstituteFlag)
	{
		// �����ֽ����
		if (0 == cpn.ui64ComponentShare)
		{
			// �ɷֹ�����Ϊ0
			EzLog::e(strTag, "�ɷֹ�����Ϊ0");
			return -1;
		}

		if (0 == cpn.dPremiumRatio)
		{
			// �ֽ��������Ϊ0
			EzLog::e(strTag, "�ɷֹ��ֽ��������Ϊ0");
			return -1;
		}
	}
	else if (simutgw::Only_Cash == cpn.iSubstituteFlag)
	{
		// ֻ�����ֽ����
		if (0 == cpn.ui64mCreationCashSubstitute ||
			0 == cpn.ui64mRedemptionCashSubstitute)
		{
			// ����ܽ��Ϊ0
			EzLog::e(strTag, "�ɷֹ�������Ϊ0");
			return -1;
		}
	}

	return 0;
}

/*
У��ETF���ɷֹ�֮�����Ϣ
Return:
0 -- �Ϸ�
-1 -- �Ƿ�
*/
int ValidateETF::Validate_ETF_Info(const std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	static const std::string strTag("ValidateETF::Validate_ETF_Info() ");

	if (ptrEtf->strSecurityID.empty())
	{
		EzLog::e(strTag, "֤ȯ����Ϊ��");
		return -1;
	}

	if (0 == ptrEtf->ui64CreationRedemptionUnit)
	{
		EzLog::e(strTag, "��С�깺��ص�λΪ0");
		return -1;
	}

	if (0 == ptrEtf->ui64CreationRedemptionUnit)
	{
		EzLog::e(strTag, "��С�깺��ص�λΪ0");
		return -1;
	}

	if (ptrEtf->ui64TotalRecordNum != ptrEtf->vecComponents.size())
	{
		EzLog::e(strTag, "�ɷֹ�������ʵ�ʳɷֹ���������");
		return -1;
	}

	return 0;
}