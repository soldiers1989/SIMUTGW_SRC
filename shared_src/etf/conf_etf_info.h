#ifndef __CON_ETF_INFO_H__
#define __CON_ETF_INFO_H__

#include <string>
#include <vector>
#include <stdint.h>

#include "config/conf_msg.h"

namespace simutgw
{
	//�깺���������״̬
	//0 - �������깺 / ���
	//1 - �깺����ؽ�����
	//2 - �������깺
	//3 - ���������
	enum emSHCreRedState
	{
		NotCre_AND_NotRed = 0,
		Cre_AND_Red = 1,
		Cre_AND_NotRed,
		NotCre_AND_Red
	};

	//0��ʾ��ֹ�ֽ������������֤ȯ��
	//1��ʾ���Խ����ֽ����������֤ȯ��֤ȯ����Ļ����ֽ������
	//2��ʾ�������ֽ������
	enum emSubstituteFlag
	{
		Only_Security = 0,
		Security_And_Cash = 1,
		Only_Cash = 2
	};

	// ����etf�ɷֹ���Ϣ
	struct SzETFComponent
	{
		//�ɷݹ��б� Components
		//�� �ɷݹ���Ϣ Component
		//�� �� ֤ȯ���� UnderlyingSecurityID C8
		std::string strUnderlyingSecurityID;

		//�� �� ֤ȯ����Դ UnderlyingSecurityIDSource C4 101 = �Ϻ�֤ȯ������102 = ����֤ȯ������
		//103 = ��۽�����9999 = ����
		std::string strUnderlyingSecurityIDSource;

		//�� �� ֤ȯ��� UnderlyingSymbol C40
		std::string strUnderlyingSymbol;
		
		//�� �� �ɷ�֤ȯ�� ComponentShare N15(2) ÿ���깺�����иóɷ�֤ȯ��������
		//���ֶ�ֻ���ֽ������־Ϊ��0����1��ʱ����Ч
		uint64_t ui64ComponentShare;

		//�� �� �ֽ������־SubstituteFlag C1 0 = ��ֹ�ֽ������������֤ȯ��
		//1 = ���Խ����ֽ����������֤ȯ��֤ȯ����ʱ�������ֽ������2 = �������ֽ����
		int iSubstituteFlag;

		//�� �� ��۱��� PremiumRatio N7(5) ֤ȯ���ֽ���������ʱ�򣬼���۸�ʱ���ӵı�����
		//���磺2.551�����ļ����� 0.02551 ��ʾ��2.1%���ļ����� 0.02100 ��ʾ�����ֶ�ֻ���ֽ������־Ϊ��1��ʱ����Ч
		double dPremiumRatio;

		//�� �� �깺������CreationCashSubstitute N18(4) ��ĳֻ֤ȯ�������ֽ������ʱ��
		//�깺ʱ��֤ȯ�����ܽ����ֶ�ֻ�е��ֽ������־Ϊ��2��ʱ����Ч
		uint64_t_Money ui64mCreationCashSubstitute;

		//�� �� ���������RedemptionCashSubstitute N18(4) ��ĳֻ֤ȯ�������ֽ������ʱ��
		//���ʱ��Ӧ��֤ȯ�������ܽ����磺 2000 ���ļ����� 2000.0000��ʾ��
		//���ڿ羳 ETF�����г� ETF���ƽ� ETF ���ֽ�ծȯ ETF�����ֶ�Ϊ 0.0000�����ֶ�ֻ�е��ֽ������־Ϊ��2��ʱ����Ч
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

	// ����etf��Ϣ
	struct SzETF
	{
		// �汾�� Version C8 �̶�ֵ1.0
		std::string strVersion;

		// ֤ȯ���� SecurityID C8
		std::string strSecurityID;

		// ֤ȯ����Դ SecurityIDSource C4 101 = �Ϻ�֤ȯ������102 = ����֤ȯ������
		//103 = ��۽�����9999 = ����
		std::string strSecurityIDSource;

		// �������� Symbol C40
		std::string strSymbol;

		// ����˾���� FundManagementCompany C30
		std::string strFundManagementCompany;

		//���ָ������ UnderlyingSecurityID C8
		std::string strUnderlyingSecurityID;

		//���ָ������Դ UnderlyingSecurityIDSource C4 
		//101 = �Ϻ�֤ȯ������102 = ����֤ȯ������103 = ��۽�����9999 = ����
		std::string strUnderlyingSecurityIDSource;

		// ��С�깺��ص�λ CreationRedemptionUnit N15(2) ÿ�����ӣ���С�깺��ص�λ����Ӧ�� ETF ������
		//Ŀǰֻ��Ϊ������
		uint64_t ui64CreationRedemptionUnit;

		// Ԥ���ֽ��� EstimateCashComponent N11(2) T ��ÿ�����ӵ�Ԥ���ֽ���
		uint64_t_Money ui64mEstimateCashComponent;

		// ����ֽ�������� MaxCashRatio N6(5) ����ֽ�������������磺5.551�����ļ����� 0.05551 ��ʾ
		double dMaxCashRatio;

		// �Ƿ񷢲� IOPV Publish C1 Y = ��N = ��
		bool bPublish;

		//�Ƿ������깺 Creation C1 Y = ��N = ��
		bool bCreation;

		//�Ƿ�������� Redemption C1 Y = ��N = ��
		bool bRedemption;

		//���гɷ�֤ȯ��Ŀ RecordNum N4 ��ʾһ�������е����гɷ�֤ȯ��Ŀ������ 159900 ֤ȯ��
		uint64_t ui64RecordNum;

		//���гɷ�֤ȯ��Ŀ TotalRecordNum N4 ��ʾһ�������е����гɷ�֤ȯ��Ŀ������ 159900 ֤ȯ��
		uint64_t ui64TotalRecordNum;

		// ������ TradingDay N8 ��ʽ YYYYMMDD
		std::string strTradingDay;

		//ǰ������ PreTradingDay N8 T - X �����ڣ���ʽ YYYYMMDD��X �ɻ���˾���ݻ����ֵʱ��ȷ��
		std::string strPreTradingDay;

		//�ֽ���� CashComponent N11(2) T - X ���깺��ػ�׼��λ���ֽ����
		uint64_t_Money ui64mCashComponent;

		//�깺��ػ�׼��λ��ֵ NAVperCU N12(2) T - X ���깺��ػ�׼��λ��ֵ
		uint64_t_Money ui64mNAVperCU;

		// ��λ��ֵ NAV N8(4) T - X �ջ���ĵ�λ��ֵ
		uint64_t_Money ui64mNAV;

		//������� DividendPerCU N12(2) T ���깺��ػ�׼��λ�ĺ������
		uint64_t_Money ui64mDividendPerCU;

		//�ۼ��깺�ܶ����� CreationLimit N18(2) �����ۼƿ��깺�Ļ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
		uint64_t ui64CreationLimit;

		//�ۼ�����ܶ����� RedemptionLimit N18(2) �����ۼƿ���صĻ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ� Ŀǰֻ��Ϊ����
		uint64_t ui64RedemptionLimit;

		//�����˻��ۼ��깺�ܶ����� CreationLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ��깺�Ļ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ���������˻��ۼ�����ܶ�����
		uint64_t ui64CreationLimitPerUser;

		//�����˻��ۼ�����ܶ�����  RedemptionLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ���صĻ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
		uint64_t ui64RedemptionLimitPerUser;

		//���깺�ܶ����� NetCreationLimit N18(2) ���쾻�깺�Ļ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
		uint64_t ui64NetCreationLimit;

		//������ܶ����� NetRedemptionLimit N18(2) ���쾻��صĻ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
		uint64_t ui64NetRedemptionLimit;

		//�����˻����깺�ܶ����� NetCreationLimitPerUser N18(2) ����֤ȯ�˻����쾻�깺�Ļ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
		uint64_t ui64NetCreationLimitPerUser;

		//�����˻�������ܶ����� NetRedemptionLimitPerUser N18(2) ����֤ȯ�˻����쾻��صĻ���ݶ����ޣ�
		//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
		uint64_t ui64NetRedemptionLimitPerUser;

		// ���гɷֹ���Ϣ
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