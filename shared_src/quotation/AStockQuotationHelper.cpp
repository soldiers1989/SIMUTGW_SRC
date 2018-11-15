#include "AStockQuotationHelper.h"

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"

#include "tool_json/RapidJsonHelper_tgw.h"

#include "util/EzLog.h"

using namespace std;

AStockQuotationHelper::AStockQuotationHelper(void)
{
}

AStockQuotationHelper::~AStockQuotationHelper(void)
{
}

// ��սṹ������
int AStockQuotationHelper::EmptyContent( struct AStockQuot& inout_astockQuot)
{
	// ֤ȯ����
	// \"ZQDM\":\"300571\",	
	inout_astockQuot.zqdm.clear();

	// ֤ȯ����
	// \"ZQMC\":\"\",
	inout_astockQuot.zqmc.clear();

	// ����ʱ��
	// \"OrigTime\":\"2017-04-13 14:01:06.000\"
	inout_astockQuot.hqsj.clear();

	// ��������
	// \"hqktype\":\"SZSTEP.W.010\",
	inout_astockQuot.hqktype.clear();

	// ͣ�̱�־ --�䶯
	// \"TPBZ\":\"F\",
	inout_astockQuot.TPBZ.clear();

	// �������̼�
	// \"ZRSP\":\"135.1000\",
	inout_astockQuot.zrsp = 0;

	// ��������Ƿ�
	inout_astockQuot.maxgain = 0;

	// ������͵���
	inout_astockQuot.minfall = 0;

	// ���տ��̼�
	// \"JRKP\":\"134.500000\",
	inout_astockQuot.jrkp = 0;

	// ��߳ɽ��� --�䶯
	// \"ZGJG\":\"139.800000\",
	inout_astockQuot.zgjg = 0;

	// ��ͳɽ��� --�䶯
	// \"ZDJG\":\"134.500000\",
	inout_astockQuot.zdjg = 0;

	// ����ɽ��� --�ּ� �䶯
	// \"ZJJG\":\"136.400000\",
	inout_astockQuot.zjjg = 0;

	// �ɽ����� --�䶯
	// \"CJSL\":\"536524.00\",
	inout_astockQuot.cjsl = 0;

	// �ɽ���� --�䶯
	// \"CJJE\":\"73703408.0700\",
	inout_astockQuot.cjje = 0;

	// ��ӯ��1 --�䶯
	// \"SYL1\":\"211.800000\",
	inout_astockQuot.SYL1 = 0.0;

	// ��ӯ��2 --�䶯
	// \"SYL2\":\"0\",
	inout_astockQuot.SYL2 = 0.0;

	// �ɽ����� --�䶯
	// \"CJBS\":\"2065\",
	inout_astockQuot.cjbs = 0;

	// �����۸�1 --�䶯
	// \"SJW1\":\"136.000000\",
	inout_astockQuot.SJW1 = 0;

	// �����۸�2
	// \"SJW2\":\"136.500000\",
	inout_astockQuot.SJW2 = 0;

	// �����۸�3
	// \"SJW3\":\"136.600000\",
	inout_astockQuot.SJW3 = 0;

	// �����۸�4
	// \"SJW4\":\"136.630000\",
	inout_astockQuot.SJW4 = 0;

	// �����۸�5
	// \"SJW5\":\"136.640000\",
	inout_astockQuot.SJW5 = 0;

	// ��������1 --�䶯
	// \"SSL1\":\"100.00\",
	inout_astockQuot.SSL1 = 0;

	// ��������2 --�䶯
	// \"SSL2\":\"600.00\",
	inout_astockQuot.SSL2 = 0;

	// ��������3 --�䶯
	// \"SSL3\":\"300.00\",
	inout_astockQuot.SSL3 = 0;

	// ��������4 --�䶯
	// \"SSL4\":\"100.00\",
	inout_astockQuot.SSL4 = 0;

	// ��������5 --�䶯
	// \"SSL5\":\"100.00\",
	inout_astockQuot.SSL5 = 0;

	// ����۸�1 --�䶯
	// \"BJW1\":\"135.760000\",
	inout_astockQuot.BJW1 = 0;

	// ����۸�2 --�䶯
	// \"BJW2\":\"135.610000\",
	inout_astockQuot.BJW2 = 0;

	// ����۸�3 --�䶯
	// \"BJW3\":\"135.600000\",
	inout_astockQuot.BJW3 = 0;

	// ����۸�4 --�䶯
	// \"BJW4\":\"135.580000\",
	inout_astockQuot.BJW4 = 0;

	// ����۸�5 --�䶯
	// \"BJW5\":\"135.560000\",
	inout_astockQuot.BJW5 = 0;

	// ��������1 --�䶯
	// \"BSL1\":\"100.00\",
	inout_astockQuot.BSL1 = 0;

	// ��������2 --�䶯
	// \"BSL2\":\"300.00\",
	inout_astockQuot.BSL2 = 0;

	// ��������3 --�䶯
	// \"BSL3\":\"800.00\",
	inout_astockQuot.BSL3 = 0;

	// ��������4 --�䶯
	// \"BSL4\":\"1000.00\",
	inout_astockQuot.BSL4 = 0;

	// ��������5 --�䶯
	// \"BSL5\":\"100.00\",
	inout_astockQuot.BSL5 = 0;

	// ������г�����
	// 1 -- ��������
	// 2 -- �Ϻ�����
	inout_astockQuot.hqmarket = 0;

	// PriceUpperLimit(��ͣ��)
	inout_astockQuot.PriceUpperLimit.clear();

	// PriceLowerLimit����ͣ�ۣ�
	inout_astockQuot.PriceLowerLimit.clear();

	// ����ʱ�� 
	memset(&inout_astockQuot.timehqsj, 0, sizeof(inout_astockQuot.timehqsj));

	return 0;
}

// ����Redis��ȡ��RedisString
int AStockQuotationHelper::DecodeJsonStr(const string& in_strMsg, struct AStockQuot& out_astockQuot )
{
	static const string ftag("AStockQuotationHelper::DecodeJsonStr()");

	string strDebug;

	if( 0 == in_strMsg.length() )
	{
		// ����ǿ��ַ���ֱ�ӷ���
		return 1;
	}

	/*{"ZQDM":"002821","ZQMC":"","CJSL":"0.00","CJJE":"0.0000","ZJJG":"72.5100",
	"ZRSP":"72.5100","JRKP":"0","ZGJG":"0","ZDJG":"0",
	"CJBS":"0","SJW1":"72.490000","SJW2":"0","SJW3":"0",
	"SJW4":"0","SJW5":"0","SSL1":"100.00","SSL2":"0","SSL3":"0",
	"SSL4":"0","SSL5":"0","BJW1":"72.490000","BJW2":"0","BJW3":"0",
	"BJW4":"0","BJW5":"0","BSL1":"100.00","BSL2":"0","BSL3":"0","BSL4":"0",
	"BSL5":"0","SYL1":"64.760000","SYL2":"0","TPBZ":"F","PriceUpperLimit":"79.760000",
	"PriceLowerLimit":"65.260000","OrigTime":"20170629091921000","hqrq":"20170629",
	"hqsj":"091921","OrgTradingPhaseCode":"O0","hqktype":"SZSTEP.W.010"}*/


	try
	{				
		rapidjson::Document jsonMessage;

		if( (jsonMessage.Parse<0>(in_strMsg.c_str())).HasParseError() || jsonMessage.IsNull() )
		{		
			string strItoa;
			string strDebug("�������� rapidjson::HasParseError[");
			strDebug += sof_string::itostr( jsonMessage.HasParseError(), strItoa );
			strDebug += "] rapidjson::IsNull[";
			strDebug += sof_string::itostr( jsonMessage.IsNull(), strItoa );
			strDebug += ";Դ-";
			strDebug += in_strMsg;
			EzLog::e(ftag, strDebug);

			return -2;
		}
		
		int iRes = 0;

		//
		// ��ʼ���֤ȯ������Ϣ

		// ֤ȯ����
		// \"ZQDM\":\"300571\",
		iRes = RapidJsonHelper_tgw::GetStringFromJsonValue(jsonMessage, AStockQuotName::zqdm, out_astockQuot.zqdm);
		if( 0 != iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::zqdm;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;

			return -2;
		}

		/*
		// ֤ȯ����
		// \"ZQMC\":\"\",
		iRes = JsonTools_tgw::GetStringFromJsonValue(jsonMessage, AStockQuotName::zqmc, out_astockQuot.zqmc);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::zqmc + "]��Դ-" + strJsonMessage);
		return -2;
		}
		*/

		// ����ʱ��
		// \"OrigTime\":\"2017-04-13 14:01:06.000\"
		// 2017 06 29 09 19 21 000
		// "hqrq" : "20170629",
		// "hqsj" : "091921",
		iRes = RapidJsonHelper_tgw::GetStringFromJsonValue(jsonMessage, AStockQuotName::hqsj, out_astockQuot.hqsj);
		if( 0 != iRes )
		{			
			strDebug = "��������[";
			strDebug += AStockQuotName::hqsj;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );
			return -2;
		}

		/*
		// ������ʱ��תΪptime����������Ƚ�
		iRes = Tgw_StringUtil::ConvertStringToPTime(out_astockQuot.hqsj, out_astockQuot.timehqsj);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "ʱ��ת������[" + AStockQuotName::hqsj + "]��Դ-" + strJsonMessage);
		return -2;
		}
		*/

		// ������������ ��ʶ��ĸMD�����ͱ�� �Ͻ�������ר��
		// MDStreamID
		RapidJsonHelper_tgw::GetStringFromJsonValue(jsonMessage, AStockQuotName::MDStreamID, out_astockQuot.MDStreamID);
		/*
		if( 0 > iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::MDStreamID;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}
		*/


		// ��������
		// \"hqktype\":\"SZSTEP.W.010\",
		iRes = RapidJsonHelper_tgw::GetStringFromJsonValue(jsonMessage, AStockQuotName::hqktype, out_astockQuot.hqktype);
		if( 0 != iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::hqktype;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}


		// ͣ�̱�־ --�䶯
		// \"TPBZ\":\"F\",
		iRes = RapidJsonHelper_tgw::GetStringFromJsonValue(jsonMessage, AStockQuotName::TPBZ, out_astockQuot.TPBZ);
		if( 0 != iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::TPBZ;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}


		// �������̼�
		// \"ZRSP\":\"135.1000\",
		//iRes = RapidJsonHelper_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::zrsp, out_astockQuot.zrsp);
		//if( 0 != iRes )
		//{
		//	strDebug = "��������[";
		//	strDebug += AStockQuotName::zrsp;
		//	strDebug += "]��Դ-";
		//	strDebug += in_strMsg;
		//	EzLog::e( ftag, strDebug );
		//	return -2;
		//}

		// PriceUpperLimit(��ͣ��)
		iRes = RapidJsonHelper_tgw::GetStringFromJsonValue(jsonMessage, AStockQuotName::PriceUpperLimit, out_astockQuot.PriceUpperLimit);
		if( 0 > iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::PriceUpperLimit;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}

		if( 0 == out_astockQuot.PriceUpperLimit.compare("999999999.999900"))
		{
			// PriceUpperLimitΪ999999999.9999��ʾ����ͣ�۸�����
			// ��������Ƿ�
			out_astockQuot.maxgain = 0;
		}
		else
		{
			iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(out_astockQuot.PriceUpperLimit, out_astockQuot.maxgain );
			if( 0 != iRes )
			{
				strDebug = "��������[";
				strDebug += AStockQuotName::PriceUpperLimit;
				strDebug += "]��Դ-";
				strDebug += in_strMsg;
				EzLog::e( ftag, strDebug );
				return -2;
			}
		}

		// PriceLowerLimit����ͣ�ۣ�
		iRes = RapidJsonHelper_tgw::GetStringFromJsonValue(jsonMessage, AStockQuotName::PriceLowerLimit, out_astockQuot.PriceLowerLimit);
		if( 0 > iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::PriceLowerLimit;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}

		if(  0 == out_astockQuot.PriceLowerLimit.compare("-999999999.999900"))
		{
			// PriceLowerLimitΪ-999999999.9999��ʾ�޵�ͣ�۸�����
			// ������͵���
			out_astockQuot.minfall = 0;
		}
		else
		{
			iRes = Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(out_astockQuot.PriceLowerLimit, out_astockQuot.minfall );
			if( 0 != iRes )
			{
				strDebug = "��������[";
				strDebug += AStockQuotName::PriceLowerLimit;
				strDebug += "]��Դ-";
				strDebug += in_strMsg;
				EzLog::e( ftag, strDebug );
				return -2;
			}
		}

		
		//// ���տ��̼�
		//// \"JRKP\":\"134.500000\",
		//iRes = JsonTools_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::jrkp, out_astockQuot.jrkp);
		//if( 0 != iRes )
		//{
		//EzLog::e( ftag, "��������[" + AStockQuotName::jrkp + "]��Դ-" + strJsonMessage);
		//return -2;
		//}

		//// ��߳ɽ��� --�䶯
		//// \"ZGJG\":\"139.800000\",
		//iRes = JsonTools_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::zgjg, out_astockQuot.zgjg);
		//if( 0 != iRes )
		//{
		//EzLog::e( ftag, "��������[" + AStockQuotName::zgjg + "]��Դ-" + strJsonMessage);
		//return -2;
		//}

		//// ��ͳɽ��� --�䶯
		//// \"ZDJG\":\"134.500000\",
		//iRes = JsonTools_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::zdjg, out_astockQuot.zdjg);
		//if( 0 != iRes )
		//{
		//EzLog::e( ftag, "��������[" + AStockQuotName::zdjg + "]��Դ-" + strJsonMessage);
		//return -2;
		//}

		// ����ɽ��� --�ּ� �䶯
		// \"ZJJG\":\"136.400000\",
		iRes = RapidJsonHelper_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::zjjg, out_astockQuot.zjjg);
		if( 0 != iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::zjjg;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}
		
		// �ɽ����� --�䶯
		// \"CJSL\":\"536524.00\",
		iRes = RapidJsonHelper_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::cjsl, out_astockQuot.cjsl);
		if( 0 != iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::cjsl;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}


		// �ɽ���� --�䶯
		// \"CJJE\":\"73703408.0700\",
		iRes = RapidJsonHelper_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::cjje, out_astockQuot.cjje);
		if( 0 != iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::cjje;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}

		/*
		// ��ӯ��1 --�䶯
		// \"SYL1\":\"211.800000\",
		iRes = JsonTools_tgw::GetDoubleFromJsonValue(jsonMessage, AStockQuotName::SYL1, out_astockQuot.SYL1);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::SYL1 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// ��ӯ��2 --�䶯
		// \"SYL2\":\"0\",
		iRes = JsonTools_tgw::GetDoubleFromJsonValue(jsonMessage, AStockQuotName::SYL2, out_astockQuot.SYL2);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::SYL2 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// �ɽ����� --�䶯
		// \"CJBS\":\"2065\",
		iRes = JsonTools_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::cjbs, out_astockQuot.cjbs);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::cjbs + "]��Դ-" + strJsonMessage);
		return -2;
		}
		*/

		// �����۸�1 --�䶯
		// \"SJW1\":\"136.000000\",
		iRes = RapidJsonHelper_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::SJW1, out_astockQuot.SJW1);
		if( 0 != iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::SJW1;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}
		
		/*// �����۸�2
		// \"SJW2\":\"136.500000\",
		iRes = JsonTools_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::SJW2, out_astockQuot.SJW2);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::SJW2 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// �����۸�3
		// \"SJW3\":\"136.600000\",
		iRes = JsonTools_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::SJW3, out_astockQuot.SJW3);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::SJW3 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// �����۸�4
		// \"SJW4\":\"136.630000\",
		iRes = JsonTools_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::SJW4, out_astockQuot.SJW4);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::SJW4 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// �����۸�5
		// \"SJW5\":\"136.640000\",
		iRes = JsonTools_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::SJW5, out_astockQuot.SJW5);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::SJW5 + "]��Դ-" + strJsonMessage);
		return -2;
		}
		*/

		// ��������1 --�䶯
		// \"SSL1\":\"100.00\",
		iRes = RapidJsonHelper_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::SSL1, out_astockQuot.SSL1);
		if( 0 != iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::SSL1;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}

		/*// ��������2 --�䶯
		// \"SSL2\":\"600.00\",
		iRes = JsonTools_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::SSL2, out_astockQuot.SSL2);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::SSL2 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// ��������3 --�䶯
		// \"SSL3\":\"300.00\",
		iRes = JsonTools_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::SSL3, out_astockQuot.SSL3);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::SSL3 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// ��������4 --�䶯
		// \"SSL4\":\"100.00\",
		iRes = JsonTools_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::SSL4, out_astockQuot.SSL4);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::SSL4 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// ��������5 --�䶯
		// \"SSL5\":\"100.00\",
		iRes = JsonTools_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::SSL5, out_astockQuot.SSL5);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::SSL5 + "]��Դ-" + strJsonMessage);
		return -2;
		}
		*/

		// ����۸�1 --�䶯
		// \"BJW1\":\"135.760000\",
		iRes = RapidJsonHelper_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::BJW1, out_astockQuot.BJW1);
		if( 0 != iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::BJW1;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}

		/*// ����۸�2 --�䶯
		// \"BJW2\":\"135.610000\",
		iRes = JsonTools_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::BJW2, out_astockQuot.BJW2);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::BJW2 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// ����۸�3 --�䶯
		// \"BJW3\":\"135.600000\",
		iRes = JsonTools_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::BJW3, out_astockQuot.BJW3);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::BJW3 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// ����۸�4 --�䶯
		// \"BJW4\":\"135.580000\",
		iRes = JsonTools_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::BJW4, out_astockQuot.BJW4);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::BJW4 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// ����۸�5 --�䶯
		// \"BJW5\":\"135.560000\",
		iRes = JsonTools_tgw::GetUINT64MoneyFromJsonValue(jsonMessage, AStockQuotName::BJW5, out_astockQuot.BJW5);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::BJW5 + "]��Դ-" + strJsonMessage);
		return -2;
		}
		*/

		// ��������1 --�䶯
		// \"BSL1\":\"100.00\",
		iRes = RapidJsonHelper_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::BSL1, out_astockQuot.BSL1);
		if( 0 != iRes )
		{
			strDebug = "��������[";
			strDebug += AStockQuotName::BSL1;
			strDebug += "]��Դ-";
			strDebug += in_strMsg;
			EzLog::e( ftag, strDebug );

			return -2;
		}

		/*// ��������2 --�䶯
		// \"BSL2\":\"300.00\",
		iRes = JsonTools_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::BSL2, out_astockQuot.BSL2);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::BSL2 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// ��������3 --�䶯
		// \"BSL3\":\"800.00\",
		iRes = JsonTools_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::BSL3, out_astockQuot.BSL3);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::BSL3 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// ��������4 --�䶯
		// \"BSL4\":\"1000.00\",
		iRes = JsonTools_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::BSL4, out_astockQuot.BSL4);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::BSL4 + "]��Դ-" + strJsonMessage);
		return -2;
		}

		// ��������5 --�䶯
		// \"BSL5\":\"100.00\",
		iRes = JsonTools_tgw::GetUINT64FromJsonValue(jsonMessage, AStockQuotName::BSL5, out_astockQuot.BSL5);
		if( 0 != iRes )
		{
		EzLog::e( ftag, "��������[" + AStockQuotName::BSL5 + "]��Դ-" + strJsonMessage);
		return -2;
		}
		*/

		// ԭʼ�ַ���
		out_astockQuot.OriginStr = in_strMsg;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

/*
����StockQuotation�ĺϷ���
Return :
0 -- �����ɹ�
С��0 -- ����ʧ��
����0 -- �ǹ�Ʊ����
*/
int AStockQuotationHelper::Validate( struct AStockQuot& io_astockQuot )
{
	static const string ftag( "AStockQuotationHelper::Validate()" );

	if (io_astockQuot.zqdm.empty())
	{
		return -1;
	}

	if( 0 == io_astockQuot.hqktype.compare("SZSTEP.W.010") )
	{
		// ��������
		io_astockQuot.hqmarket = 0;
	}
	else if ( 0 == io_astockQuot.hqktype.compare("SHHQ") )
	{
		if( 0 == io_astockQuot.MDStreamID.compare("MD002") // AB��
			|| 0 == io_astockQuot.MDStreamID.compare("MD003")// ծȯ
			|| 0 == io_astockQuot.MDStreamID.compare("MD004"))// ����
		{
			// �Ϻ�����
			io_astockQuot.hqmarket = 1;
		}
		else
		{
			string strDebug("unkown hqktype=[");
			strDebug += io_astockQuot.hqktype;
			strDebug += "] MDStreamID=[";
			strDebug += io_astockQuot.MDStreamID;
			EzLog::Out(ftag, trivial::trace, strDebug);
			return -1;
		}

	} 
	else
	{
		string strDebug("unkown hqktype=[");
		strDebug += io_astockQuot.hqktype;
		strDebug += "] MDStreamID=[";
		strDebug += io_astockQuot.MDStreamID;
		EzLog::Out(ftag, trivial::debug, strDebug);
		return -1;
	}

	if (1 == io_astockQuot.hqmarket &&
		ValidateIndexNumber(io_astockQuot.zqdm))
	{
		return 1;
	}
	

	return 0;
}

string& AStockQuotationHelper::ToRedisString( struct AStockQuot& astockQuot, string& out_strRedisValue )
{
	// static const string ftag( "AStockQuotationHelper::ToRedisString()" );

	/*
	lpush  hqk_change "{\"ZQDM\":\"300573\",\"ZQMC\":\"\",\"CJSL\":\"539524.00\",\"CJJE\":\"73706408.0700\",
	\"ZJJG\":\"136.400000\",\"ZRSP\":\"135.1000\",\"JRKP\":\"134.500000\",
	\"ZGJG\":\"139.800000\",\"ZDJG\":\"134.500000\",\"CJBS\":\"2065\",\"SJW1\":\"136.000000\",
	\"SJW2\":\"136.500000\",\"SJW3\":\"136.600000\",\"SJW4\":\"136.630000\",\"SJW5\":\"136.640000\",
	\"SSL1\":\"100.00\",\"SSL2\":\"600.00\",\"SSL3\":\"300.00\",\"SSL4\":\"100.00\",\"SSL5\":\"100.00\",\"BJW1\":\"135.760000\"
	,\"BJW2\":\"135.610000\",\"BJW3\":\"135.600000\",\"BJW4\":\"135.580000\",\"BJW5\":\"135.560000\",\"BSL1\":\"100.00\",
	\"BSL2\":\"300.00\",\"BSL3\":\"800.00\",\"BSL4\":\"1000.00\",\"BSL5\":\"100.00\",\"SYL1\":\"211.800000\",\"SYL2\":\"0\",
	\"TPBZ\":\"F\",\"hqktype\":\"SZSTEP.W.010\",\"OrigTime\":\"2017-04-13 14:01:07.000\"}"
	*/

	string strTran;
	out_strRedisValue = "{\"";
	out_strRedisValue += AStockQuotName::zqdm;
	out_strRedisValue += "\":\"";
	out_strRedisValue += astockQuot.zqdm;

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::zqmc;
	out_strRedisValue += "\":\"";
	out_strRedisValue += astockQuot.zqmc;

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::cjsl;
	out_strRedisValue += "\":\"";
	sof_string::itostr( astockQuot.cjsl, strTran );
	strTran += ".00";
	out_strRedisValue += strTran;

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::cjje;	
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.cjje, strTran, 4 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::zjjg;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.zjjg, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::zrsp;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.zrsp, strTran, 4 );

	// PriceUpperLimit(��ͣ��)
	// PriceUpperLimitΪ999999999.9999��ʾ����ͣ�۸�����	
	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::PriceUpperLimit;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.maxgain, strTran, 4 );

	// PriceLowerLimit����ͣ�ۣ�
	// PriceLowerLimitΪ-999999999.9999��ʾ�޵�ͣ�۸�����
	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::PriceLowerLimit;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.minfall, strTran, 4 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::jrkp;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.jrkp, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::zgjg;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.zgjg, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::zdjg;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.zdjg, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::cjbs;
	out_strRedisValue += "\":\"";
	out_strRedisValue += sof_string::itostr( astockQuot.cjbs, strTran );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SJW1;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.SJW1, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SJW2;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.SJW2, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SJW3;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.SJW3, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SJW4;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.SJW4, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SJW5;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.SJW5, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SSL1;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.SSL1, strTran, 2 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SSL2;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.SSL2, strTran, 2 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SSL3;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.SSL3, strTran, 2 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SSL4;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.SSL4, strTran, 2 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SSL5;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.SSL5, strTran, 2 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::BJW1;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.BJW1, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::BJW2;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.BJW2, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::BJW3;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.BJW3, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::BJW4;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.BJW4, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::BJW5;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.BJW5, strTran, 6 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::BSL1;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.BSL1, strTran, 2 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::BSL2;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.BSL2, strTran, 2 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::BSL3;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.BSL3, strTran, 2 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::BSL4;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.BSL4, strTran, 2 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::BSL5;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( astockQuot.BSL5, strTran, 2 );

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SYL1;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr( (int64_t)astockQuot.SYL1, strTran, 6 );


	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::SYL2;
	out_strRedisValue += "\":\"";
	out_strRedisValue += Tgw_StringUtil::iLiToStr((int64_t)astockQuot.SYL2, strTran, 6);

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::TPBZ;
	out_strRedisValue += "\":\"";
	out_strRedisValue += astockQuot.TPBZ;

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::hqktype;
	out_strRedisValue += "\":\"";
	out_strRedisValue += astockQuot.hqktype;

	out_strRedisValue += "\",\"";
	out_strRedisValue += AStockQuotName::hqsj;
	out_strRedisValue += "\":\"";
	out_strRedisValue += astockQuot.hqsj;

	out_strRedisValue += "\"}";

	return out_strRedisValue;
}

/*
���ݹ�Ʊ������г����ж��Ƿ�Ϊ��ָ֤��

�Ϻ��г�����000��ͷ�Ĺ�Ʊ���붼��ָ��
Return :
true -- ָ��
false -- ��ָ��

*/
bool AStockQuotationHelper::ValidateIndexNumber(const string& in_strIndexNumber)
{
	static const string strTag("AStockQuotationHelper::ValidateIndexNumber");

	if (in_strIndexNumber.length() != 6)
	{
		// ָ������Ϊ6λ�������Ĳ��Ϸ�
		string strError("IndexNumber[");
		strError += in_strIndexNumber;
		strError += "] illeagal";

		EzLog::e(strTag, strError);
		return true;
	}

	string strHead = in_strIndexNumber.substr(0, 3);

	if (0 == strHead.compare("000"))
	{
		// ��ָ��
		return true;
	}
	else
	{
		return false;
	}
}