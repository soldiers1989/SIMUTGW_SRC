#ifndef __A_STOCK_QUOTATION_HELPER_H__
#define __A_STOCK_QUOTATION_HELPER_H__

#include "boost/date_time.hpp"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif

using namespace std;

#include "config/conf_msg.h"

/*
A�����鴦������
*/
struct AStockQuot
{
	/*
	{"value":"{\"ZQDM\":\"300571\",\"ZQMC\":\"\",\"CJSL\":\"536524.00\",\"CJJE\":\"73703408.0700\",
	\"ZJJG\":\"136.400000\",\"ZRSP\":\"135.1000\",\"JRKP\":\"134.500000\",
	\"ZGJG\":\"139.800000\",\"ZDJG\":\"134.500000\",\"CJBS\":\"2065\",\"SJW1\":\"136.000000\",
	\"SJW2\":\"136.500000\",\"SJW3\":\"136.600000\",\"SJW4\":\"136.630000\",\"SJW5\":\"136.640000\",
	\"SSL1\":\"100.00\",\"SSL2\":\"600.00\",\"SSL3\":\"300.00\",\"SSL4\":\"100.00\",\"SSL5\":\"100.00\",
	\"BJW1\":\"135.760000\",\"BJW2\":\"135.610000\",\"BJW3\":\"135.600000\",\"BJW4\":\"135.580000\",\"BJW5\":\"135.560000\",
	\"BSL1\":\"100.00\",\"BSL2\":\"300.00\",\"BSL3\":\"800.00\",\"BSL4\":\"1000.00\",\"BSL5\":\"100.00\",
	\"SYL1\":\"211.800000\",\"SYL2\":\"0\",\"TPBZ\":\"F\",\"hqktype\":\"SZSTEP.W.010\",\"OrigTime\":\"2017-04-13 14:01:06.000\"}"};
	*/

	// �������̼�(���ͣ���λ��)
	// \"ZRSP\":\"135.1000\",
	simutgw::uint64_t_Money zrsp;

	// ��������Ƿ�
	simutgw::uint64_t_Money maxgain;

	// ������͵���
	simutgw::uint64_t_Money minfall;

	// ���տ��̼�(���ͣ���λ��)
	// \"JRKP\":\"134.500000\",
	simutgw::uint64_t_Money jrkp;

	// ��߳ɽ���(���ͣ���λ��) --�䶯
	// \"ZGJG\":\"139.800000\",
	simutgw::uint64_t_Money zgjg;

	// ��ͳɽ���(���ͣ���λ��) --�䶯
	// \"ZDJG\":\"134.500000\",
	simutgw::uint64_t_Money zdjg;

	// ����ɽ���(���ͣ���λ��) --�ּ� �䶯
	// \"ZJJG\":\"136.400000\",
	simutgw::uint64_t_Money zjjg;

	// �ɽ����� --�䶯
	// \"CJSL\":\"536524.00\",
	uint64_t cjsl;

	// �ɽ����(���ͣ���λ��) --�䶯
	// \"CJJE\":\"73703408.0700\",
	simutgw::uint64_t_Money cjje;

	// ��ӯ��1 --�䶯
	// \"SYL1\":\"211.800000\",
	double SYL1;

	// ��ӯ��2 --�䶯
	// \"SYL2\":\"0\",
	double SYL2;

	// �ɽ����� --�䶯
	// \"CJBS\":\"2065\",
	uint64_t cjbs;

	// �����۸�1(���ͣ���λ��) --�䶯
	// \"SJW1\":\"136.000000\",
	simutgw::uint64_t_Money SJW1;

	// �����۸�2(���ͣ���λ��)
	// \"SJW2\":\"136.500000\",
	simutgw::uint64_t_Money SJW2;

	// �����۸�3(���ͣ���λ��)
	// \"SJW3\":\"136.600000\",
	simutgw::uint64_t_Money SJW3;

	// �����۸�4(���ͣ���λ��)
	// \"SJW4\":\"136.630000\",
	simutgw::uint64_t_Money SJW4;

	// �����۸�5(���ͣ���λ��)
	// \"SJW5\":\"136.640000\",
	simutgw::uint64_t_Money SJW5;

	// ��������1 --�䶯
	// \"SSL1\":\"100.00\",
	uint64_t SSL1;

	// ��������2 --�䶯
	// \"SSL2\":\"600.00\",
	uint64_t SSL2;

	// ��������3 --�䶯
	// \"SSL3\":\"300.00\",
	uint64_t SSL3;

	// ��������4 --�䶯
	// \"SSL4\":\"100.00\",
	uint64_t SSL4;

	// ��������5 --�䶯
	// \"SSL5\":\"100.00\",
	uint64_t SSL5;

	// ����۸�1(���ͣ���λ��) --�䶯
	// \"BJW1\":\"135.760000\",
	simutgw::uint64_t_Money BJW1;

	// ����۸�2(���ͣ���λ��) --�䶯
	// \"BJW2\":\"135.610000\",
	simutgw::uint64_t_Money BJW2;

	// ����۸�3(���ͣ���λ��) --�䶯
	// \"BJW3\":\"135.600000\",
	simutgw::uint64_t_Money BJW3;

	// ����۸�4(���ͣ���λ��) --�䶯
	// \"BJW4\":\"135.580000\",
	simutgw::uint64_t_Money BJW4;

	// ����۸�5(���ͣ���λ��) --�䶯
	// \"BJW5\":\"135.560000\",
	simutgw::uint64_t_Money BJW5;

	// ��������1 --�䶯
	// \"BSL1\":\"100.00\",
	uint64_t BSL1;

	// ��������2 --�䶯
	// \"BSL2\":\"300.00\",
	uint64_t BSL2;

	// ��������3 --�䶯
	// \"BSL3\":\"800.00\",
	uint64_t BSL3;

	// ��������4 --�䶯
	// \"BSL4\":\"1000.00\",
	uint64_t BSL4;

	// ��������5 --�䶯
	// \"BSL5\":\"100.00\",
	uint64_t BSL5;

	// ������г�����
	// 0 -- ��������
	// 1 -- �Ϻ�����
	int hqmarket;

	// ����ʱ�� 
	boost::posix_time::ptime timehqsj;

	// ֤ȯ����
	// \"ZQDM\":\"300571\",	
	string zqdm;

	// ֤ȯ����
	// \"ZQMC\":\"\",
	string zqmc;

	// ����ʱ��
	// \"OrigTime\":\"2017-04-13 14:01:06.000\"
	string hqsj;

	// ������������ ��ʶ��ĸMD�����ͱ�� �Ͻ�������ר��
	// MDStreamID
	// MD002 ��ʾ��Ʊ��A��B�ɣ��������ݸ�ʽ���ͣ�
	// MD003 ��ʾծȯ�������ݸ�ʽ���ͣ�
	// MD004 ��ʾ�����������ݸ�ʽ���ͣ�
	string MDStreamID;

	// ��������
	// \"hqktype\":\"SZSTEP.W.010\",
	string hqktype;

	// ͣ�̱�־ --�䶯
	// \"TPBZ\":\"F\",
	string TPBZ;

	// PriceUpperLimit(��ͣ��)
	// PriceUpperLimitΪ999999999.9999��ʾ����ͣ�۸�����		
	string PriceUpperLimit;

	// PriceLowerLimit����ͣ�ۣ�
	// PriceLowerLimitΪ-999999999.9999��ʾ�޵�ͣ�۸����� 
	string PriceLowerLimit;

	// ԭʼ�ַ���
	string OriginStr;
};

namespace AStockQuotName
{
	/*
	{"value":"{\"ZQDM\":\"300571\",\"ZQMC\":\"\",\"CJSL\":\"536524.00\",\"CJJE\":\"73703408.0700\",
	\"ZJJG\":\"136.400000\",\"ZRSP\":\"135.1000\",\"JRKP\":\"134.500000\",
	\"ZGJG\":\"139.800000\",\"ZDJG\":\"134.500000\",\"CJBS\":\"2065\",\"SJW1\":\"136.000000\",
	\"SJW2\":\"136.500000\",\"SJW3\":\"136.600000\",\"SJW4\":\"136.630000\",\"SJW5\":\"136.640000\",
	\"SSL1\":\"100.00\",\"SSL2\":\"600.00\",\"SSL3\":\"300.00\",\"SSL4\":\"100.00\",\"SSL5\":\"100.00\",
	\"BJW1\":\"135.760000\",\"BJW2\":\"135.610000\",\"BJW3\":\"135.600000\",\"BJW4\":\"135.580000\",\"BJW5\":\"135.560000\",
	\"BSL1\":\"100.00\",\"BSL2\":\"300.00\",\"BSL3\":\"800.00\",\"BSL4\":\"1000.00\",\"BSL5\":\"100.00\",
	\"SYL1\":\"211.800000\",\"SYL2\":\"0\",\"TPBZ\":\"F\",\"hqktype\":\"SZSTEP.W.010\",\"OrigTime\":\"2017-04-13 14:01:06.000\"}"};
	*/

	// �������̼�
	// \"ZRSP\":\"135.1000\",
	const string zrsp("ZRSP");

	// ��������Ƿ�
	const string maxgain("maxgain");

	// ������͵���
	const string minfall("minfall");

	// ���տ��̼�
	// \"JRKP\":\"134.500000\",
	const string jrkp("JRKP");

	// ��߳ɽ��� --�䶯
	// \"ZGJG\":\"139.800000\",
	const string zgjg("ZGJG");

	// ��ͳɽ��� --�䶯
	// \"ZDJG\":\"134.500000\",
	const string zdjg("ZDJG");

	// ����ɽ��� --�ּ� �䶯
	// \"ZJJG\":\"136.400000\",
	const string zjjg("ZJJG");

	// �ɽ����� --�䶯
	// \"CJSL\":\"536524.00\",
	const string cjsl("CJSL");

	// �ɽ���� --�䶯
	// \"CJJE\":\"73703408.0700\",
	const string cjje("CJJE");

	// ��ӯ��1 --�䶯
	// \"SYL1\":\"211.800000\",
	const string SYL1("SYL1");

	// ��ӯ��2 --�䶯
	// \"SYL2\":\"0\",
	const string SYL2("SYL2");

	// �ɽ����� --�䶯
	// \"CJBS\":\"2065\",
	const string cjbs("CJBS");

	// �����۸�1 --�䶯
	// \"SJW1\":\"136.000000\",
	const string SJW1("SJW1");

	// �����۸�2
	// \"SJW2\":\"136.500000\",
	const string SJW2("SJW2");

	// �����۸�3
	// \"SJW3\":\"136.600000\",
	const string SJW3("SJW3");

	// �����۸�4
	// \"SJW4\":\"136.630000\",
	const string SJW4("SJW4");

	// �����۸�5
	// \"SJW5\":\"136.640000\",
	const string SJW5("SJW5");

	// ��������1 --�䶯
	// \"SSL1\":\"100.00\",
	const string SSL1("SSL1");

	// ��������2 --�䶯
	// \"SSL2\":\"600.00\",
	const string SSL2("SSL2");

	// ��������3 --�䶯
	// \"SSL3\":\"300.00\",
	const string SSL3("SSL3");

	// ��������4 --�䶯
	// \"SSL4\":\"100.00\",
	const string SSL4("SSL4");

	// ��������5 --�䶯
	// \"SSL5\":\"100.00\",
	const string SSL5("SSL5");

	// ����۸�1 --�䶯
	// \"BJW1\":\"135.760000\",
	const string BJW1("BJW1");

	// ����۸�2 --�䶯
	// \"BJW2\":\"135.610000\",
	const string BJW2("BJW2");

	// ����۸�3 --�䶯
	// \"BJW3\":\"135.600000\",
	const string BJW3("BJW3");

	// ����۸�4 --�䶯
	// \"BJW4\":\"135.580000\",
	const string BJW4("BJW4");

	// ����۸�5 --�䶯
	// \"BJW5\":\"135.560000\",
	const string BJW5("BJW5");

	// ��������1 --�䶯
	// \"BSL1\":\"100.00\",
	const string BSL1("BSL1");

	// ��������2 --�䶯
	// \"BSL2\":\"300.00\",
	const string BSL2("BSL2");

	// ��������3 --�䶯
	// \"BSL3\":\"800.00\",
	const string BSL3("BSL3");

	// ��������4 --�䶯
	// \"BSL4\":\"1000.00\",
	const string BSL4("BSL4");

	// ��������5 --�䶯
	// \"BSL5\":\"100.00\",
	const string BSL5("BSL5");

	// ������г�����
	// 1 -- ��������
	// 2 -- �Ϻ�����
	const string hqmarket("hqmarket");

	// ֤ȯ����
	// \"ZQDM\":\"300571\",	
	const string zqdm("ZQDM");

	// ֤ȯ����
	// \"ZQMC\":\"\",
	const string zqmc("ZQMC");

	// ����ʱ��
	// \"OrigTime\":\"2017-04-13 14:01:06.000\"
	const string hqsj("OrigTime");

	// ������������ ��ʶ��ĸMD�����ͱ�� �Ͻ�������ר��
	// MDStreamID
	const string MDStreamID("MDStreamID");

	// ��������
	// \"hqktype\":\"SZSTEP.W.010\",
	const string hqktype("hqktype");

	// ͣ�̱�־ --�䶯
	// \"TPBZ\":\"F\",
	const string TPBZ("TPBZ");

	// PriceUpperLimit(��ͣ��)
	const string PriceUpperLimit("PriceUpperLimit");

	// PriceLowerLimit����ͣ�ۣ�
	const string PriceLowerLimit("PriceLowerLimit");

	// ԭʼ�ַ���
	const string OriginStr("OriginStr");
}

class AStockQuotationHelper
{
	//
	// Members
	//
public:

	//
	// Functions
	//
public:
	AStockQuotationHelper(void);
	virtual ~AStockQuotationHelper(void);

	// ��սṹ������
	static int EmptyContent(struct AStockQuot& inout_astockQuot);

	// ����Redis��ȡ��RedisString
	static int DecodeJsonStr(const string& strMsg, struct AStockQuot& out_astockQuot);

	/*
	����StockQuotation�ĺϷ���
	Return :
	0 -- �����ɹ�
	С��0 -- ����ʧ��
	����0 -- �ǹ�Ʊ����
	*/
	static int Validate(struct AStockQuot& io_astockQuot);

	static string& ToRedisString(struct AStockQuot& astockQuot, string& out_strRedisValue);

	/*
	���ݹ�Ʊ������г����ж��Ƿ�Ϊ��ָ֤��

	�Ϻ��г�����000��ͷ�Ĺ�Ʊ���붼��ָ��
	Return :
	true -- ָ��
	false -- ��ָ��

	*/
	static bool ValidateIndexNumber(const string& in_strIndexNumber);
};

#endif