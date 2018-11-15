#include "TgwMarketInfoProc.h"

#include "json/json.h"

#include "quotation/MarketInfoHelper.h"
#include "quotation/AStockQuotationHelper.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "tool_redis/Tgw_RedisHelper.h"

#include "util/TimeDuration.h"

#include "config/sys_function_base.h"

// �г�����������Key�Ƿ��Ѵ���
// true -- �Ѵ���
// false -- δ����
bool TgwMarketInfoProc::m_bIsMarketTradeVolumnCreated = false;

TgwMarketInfoProc::TgwMarketInfoProc(void)
	:m_scl(keywords::channel = "TgwMarketInfoProc")
{

}


TgwMarketInfoProc::~TgwMarketInfoProc(void)
{

}

//�Ӹ�����������д���Redis��ȡ����
int TgwMarketInfoProc::ReadRealMarketInfo()
{
	static const string ftag("TgwMarketInfoProc::ReadRealMarketInfo() ");

	// read market-info from Redis.
	// ���̶���Ʊǰ׺��������������Ϣ
	// �ֽ�֧��A��

	// read redis 
	// ��ȡstatic����	

	string strRedisCmd;
	bool bHaveData = true;

	struct AStockQuot struAstockQuot;

	while (bHaveData)
	{
		// ��ȡ�䶯����
		strRedisCmd = "LPOP ";
		strRedisCmd += simutgw::g_Key_AStockQuotationChange;

		long long llRedisRes = 0;
		string strRedisRes;

		RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmd(strRedisCmd, &llRedisRes, &strRedisRes,
			nullptr, nullptr, nullptr);
		if (RedisReply_nil == emPcbCallRes)
		{
			// redis��ί��
			simutgw::Simutgw_Sleep();

			bHaveData = false;
		}
		else if (RedisReply_string != emPcbCallRes)
		{
			string strTran;
			sof_string::itostr(emPcbCallRes, strTran);

			string strDebug("Redisִ�� LPOP AStockQuotationChange����cmd=[");
			strDebug += strRedisCmd;
			strDebug += "],res=[";
			strDebug += strTran;
			strDebug += "]. str=[";
			strDebug += strRedisRes;
			strDebug += "]";

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			return -1;
		}

		// �������ַ������װ��ṹ��		
		AStockQuotationHelper astockQuotHelper;

		int iRes = astockQuotHelper.DecodeJsonStr(strRedisRes, struAstockQuot);
		if (0 != iRes)
		{
			if (1 != iRes)
			{
				string strDebug("hq decode error ");
				strDebug += struAstockQuot.OriginStr;
				BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << strDebug;

			}
			else
			{
				// ������
				simutgw::Simutgw_Sleep();

				bHaveData = false;
			}

			break;
		}

		iRes = astockQuotHelper.Validate(struAstockQuot);
		if (0 > iRes)
		{
			string strDebug("hq not valide ");
			strDebug += struAstockQuot.OriginStr;
			BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << strDebug;

			break;
		}

		// ����һ�ε�������бȽϣ���������ʱ���ڿ��Բ������г���������
		// �ɽ����� -- ����
		uint64_t ui64GapCjsl = 0;
		// �ɽ���� -- ����
		uint64_t ui64GapCjje = 0;

		// �������Ƿ��Ѿ�������Redis��
		bool bIsInRedisBefore = false;
		int iRes_CalcGap = CalcQuotationGap(struAstockQuot, ui64GapCjsl, ui64GapCjje, bIsInRedisBefore);
		if (0 > iRes_CalcGap)
		{
			string strTran;
			string strDebug("hq calc not valide res=");
			strDebug += sof_string::itostr(iRes_CalcGap, strTran);
			strDebug += "  ";
			strDebug += struAstockQuot.OriginStr;
			BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << strDebug;

			continue;
		}
		else if (1 == iRes_CalcGap)
		{
			// CJSL CJJE �ޱ仯
			continue;
		}

		// process after read market-info from Redis. ��Ҫ����ʷģ�⽻�׽������
		//???

		// write processed market-info into Redis.
		// ��д���õ�����
		iRes = StoreSelfUseQuotation(struAstockQuot, ui64GapCjsl, ui64GapCjje, bIsInRedisBefore);
		if (0 != iRes)
		{

			continue;
		}
	}

	return 0;
}

/*
������ϴ�����֮��Ľ��ײ�
Param :
bool& out_bIsRecordBefore :
true -- ������֮ǰ��Redis�д���
false -- ������֮ǰ��Redis�в�����

Return:
0 -- ����
1 -- �����������
��0 -- ����ʧ��
*/
int TgwMarketInfoProc::CalcQuotationGap(const struct AStockQuot& struAstockQuot,
	uint64_t& out_ui64GapCjsl, simutgw::uint64_t_Money& out_ui64mGapCjje,
	bool& out_bIsRecordBefore)
{
	static const string ftag("TgwMarketInfoProc::CalcQuotationGap()");

	// read redis 
	string strRedisRes;

	Json::Value jsonRedisCmd;
	string strRedisCmd;

	out_bIsRecordBefore = true;

	// �ɽ����� --�䶯	// \"CJSL\":\"536524.00\",
	uint64_t ui64PreviousCjsl = 0;
	// �ɽ���� --�䶯	// \"CJJE\":\"73703408.0700\",
	simutgw::uint64_t_Money ui64mPreviousCjje = 0;

	string strPreviousHqsj;

	// ���Լ�����������ж�ȡ
	int iRes = MarketInfoHelper::GetCurrentQuotationByStockId(struAstockQuot.zqdm,
		ui64PreviousCjsl, ui64mPreviousCjje, strPreviousHqsj);
	if (0 != iRes)
	{
		// ��ȡʧ��
		ui64PreviousCjsl = 0;
		ui64mPreviousCjje = 0;

		/*
		string strDebug("δ��ȡ");
		strDebug += struAstockQuot.zqdm;
		strDebug += "��ʷ���飬��ֵ�ع�Ĭ��";
		EzLog::Out(ftag, debug, strDebug);
		*/

		out_bIsRecordBefore = false;
	}

	if (out_bIsRecordBefore)
	{
		// ֮ǰ����ʷ����
		int64_t i64TimeDiff = 0;

		iRes = TimeStringUtil::CompareTime(strPreviousHqsj, struAstockQuot.hqsj, i64TimeDiff);
		if (0 > iRes)
		{
			string strTran;
			string strDebug("unkown error when call CompareTime(");
			strDebug += strPreviousHqsj;
			strDebug += ",";
			strDebug += struAstockQuot.hqsj;
			strDebug += ") res=";
			strDebug += sof_string::itostr(iRes, strTran);
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			return -1;
		}

		// �Ƚ��������ʱ��
		if (0 == i64TimeDiff)
		{
			// ʱ��һ�£��������
			return 1;
		}
		else if (0 > i64TimeDiff)
		{
			// ��Ϣ����
			{
				string strTran;

				// ����ʱ����ҵ�����
				string strDebug("�ҵ�ʱ��������� [");
				strDebug += struAstockQuot.OriginStr;
				strDebug += "] against��diff[";
				strDebug += sof_string::itostr(i64TimeDiff, strTran);
				strDebug += "s] Previous CJSL=[";
				strDebug += sof_string::itostr(ui64PreviousCjsl, strTran);
				strDebug += "],CJJE=[";
				strDebug += sof_string::itostr(ui64mPreviousCjje, strTran);
				strDebug += "],hqsj=[";
				strDebug += strPreviousHqsj;
				strDebug += "].";
				BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << strDebug;
			}

			return -2;
		}

		// ���ܴ��ڳɽ�û�б䶯�����������䶯�����
		// �Ƚϳɽ��������ɽ����ޱ䶯�򲻸���
		if (ui64PreviousCjsl == struAstockQuot.cjsl || ui64mPreviousCjje == struAstockQuot.cjje)
		{
			return 1;
		}
	}

	// �鿴�Ƿ��Ѿ�ͣ��
	if (0 != struAstockQuot.TPBZ.compare("F"))
	{
		// ����Ѿ�ͣ��
		out_ui64GapCjsl = 0;
		out_ui64mGapCjje = 0;

		return 0;
	}

	// �ɽ����� -- ����
	simutgw::uint64_t_Money ui64GapCjsl = struAstockQuot.cjsl - ui64PreviousCjsl;
	// �ɽ���� -- ����
	simutgw::uint64_t_Money ui64mGapCjje = struAstockQuot.cjje - ui64mPreviousCjje;

	if ((ui64PreviousCjsl > struAstockQuot.cjsl) || (ui64mPreviousCjje > struAstockQuot.cjje))
	{
		// ��Ϣ����
		{
			string strTran;

			string strDebug("zqdm[");
			strDebug += struAstockQuot.zqdm;
			strDebug += "]�۸񵹹ң�Previous CJSL=[";
			strDebug += sof_string::itostr(ui64PreviousCjsl, strTran);
			strDebug += "],CJJE=[";
			strDebug += sof_string::itostr(ui64mPreviousCjje, strTran);
			strDebug += "]. now CJSL=[";
			strDebug += sof_string::itostr(struAstockQuot.cjsl, strTran);
			strDebug += "],CJJE=[";
			strDebug += sof_string::itostr(struAstockQuot.cjje, strTran);
			strDebug += "]";
			BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << strDebug;
		}

		return -3;
	}

	out_ui64GapCjsl = ui64GapCjsl;
	out_ui64mGapCjje = ui64mGapCjje;

	if (0 == out_ui64GapCjsl || 0 == out_ui64mGapCjje)
	{
		//BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << "zqdm[" << struAstockQuot.zqdm << "] GapCjsl=" << out_ui64GapCjsl << ", GapCjje=" << out_ui64mGapCjje;
		return 1;
	}

	// ��Ϣ����
	{
		//string strTran;
		//string strDebug( "�ó� gap CJSL=[" );
		//strDebug += sof_string::itostr( out_ui64GapCjsl, strTran );
		//strDebug += "],CJJE=[";
		//strDebug += sof_string::itostr( out_ui64mGapCjje, strTran );
		//strDebug += "].";
		//BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
	}

	return 0;
}

// ������������
int TgwMarketInfoProc::StoreSelfUseQuotation(const struct AStockQuot& in_struAstockQuot,
	const simutgw::uint64_t_Money& in_ui64mGapCjsl,
	const simutgw::uint64_t_Money& in_ui64mGapCjje,
	bool in_bIsRecordBefore)
{
	static const string ftag("TgwMarketInfoProc::StoreSelfUseQuotation()");

	// HMSET KEY_NAME FIELD1 VALUE1 ...FIELDN VALUEN
	// ������д��Redis	

	// ��������Ƿ�
	string strMaxgain;
	// ������͵���
	string strMinfall;

	string strCmdTmp;
	string strItoATran;

	//
	// Redis CMD
	std::vector<string> vctCmd;
	vctCmd.push_back("HMSET");

	// key
	strCmdTmp = simutgw::g_Key_AStockQuotTGW_Prefix;
	strCmdTmp += in_struAstockQuot.zqdm;

	vctCmd.push_back(strCmdTmp);

	// ��������Ƿ�
	vctCmd.push_back(AStockQuotName::maxgain);

	sof_string::itostr(in_struAstockQuot.maxgain, strMaxgain);
	vctCmd.push_back(strMaxgain);

	// ������͵���
	vctCmd.push_back(AStockQuotName::minfall);

	sof_string::itostr(in_struAstockQuot.minfall, strMinfall);
	vctCmd.push_back(strMinfall);

	/*
	// ���տ��̼�(���ͣ���λ��)	// \"JRKP\":\"134.500000\",
	jsonRedisCmd.append(AStockQuotName::jrkp);
	jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.jrkp));

	// ��߳ɽ���(���ͣ���λ��) --�䶯	// \"ZGJG\":\"139.800000\",
	jsonRedisCmd.append(AStockQuotName::zgjg);
	jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.zgjg));

	// ��ͳɽ���(���ͣ���λ��) --�䶯	// \"ZDJG\":\"134.500000\",
	jsonRedisCmd.append(AStockQuotName::zdjg);
	jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.zdjg));
	*/
	// ����ɽ���(���ͣ���λ��) --�ּ� �䶯	// \"ZJJG\":\"136.400000\",
	vctCmd.push_back(AStockQuotName::zjjg);

	sof_string::itostr(in_struAstockQuot.zjjg, strItoATran);
	vctCmd.push_back(strItoATran);

	// �ɽ����� --�䶯	// \"CJSL\":\"536524.00\",
	vctCmd.push_back(AStockQuotName::cjsl);

	sof_string::itostr(in_struAstockQuot.cjsl, strItoATran);
	vctCmd.push_back(strItoATran);

	// �ɽ����(���ͣ���λ��) --�䶯	// \"CJJE\":\"73703408.0700\",
	vctCmd.push_back(AStockQuotName::cjje);

	sof_string::itostr(in_struAstockQuot.cjje, strItoATran);
	vctCmd.push_back(strItoATran);

	//// ��ӯ��1 --�䶯	// \"SYL1\":\"211.800000\",
	//jsonRedisCmd.append(AStockQuotName::SYL1);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SYL1));

	//// ��ӯ��2 --�䶯	// \"SYL2\":\"0\",
	//jsonRedisCmd.append(AStockQuotName::SYL2);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SYL2));

	//// �ɽ����� --�䶯
	//// \"CJBS\":\"2065\",
	//jsonRedisCmd.append(AStockQuotName::cjbs);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.cjbs));

	// �����۸�1(���ͣ���λ��) --�䶯	// \"SJW1\":\"136.000000\",
	vctCmd.push_back(AStockQuotName::SJW1);

	sof_string::itostr(in_struAstockQuot.SJW1, strItoATran);
	vctCmd.push_back(strItoATran);

	//// �����۸�2(���ͣ���λ��)	// \"SJW2\":\"136.500000\",
	//jsonRedisCmd.append(AStockQuotName::SJW2);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SJW2));

	//// �����۸�3(���ͣ���λ��)	// \"SJW3\":\"136.600000\",
	//jsonRedisCmd.append(AStockQuotName::SJW3);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SJW3));

	//// �����۸�4(���ͣ���λ��)	// \"SJW4\":\"136.630000\",
	//jsonRedisCmd.append(AStockQuotName::SJW4);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SJW4));

	//// �����۸�5(���ͣ���λ��)	// \"SJW5\":\"136.640000\",
	//jsonRedisCmd.append(AStockQuotName::SJW5);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SJW5));

	// ��������1 --�䶯	// \"SSL1\":\"100.00\",
	vctCmd.push_back(AStockQuotName::SSL1);

	sof_string::itostr(in_struAstockQuot.SSL1, strItoATran);
	vctCmd.push_back(strItoATran);

	//// ��������2 --�䶯	// \"SSL2\":\"600.00\",
	//jsonRedisCmd.append(AStockQuotName::SSL2);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SSL2));

	//// ��������3 --�䶯	// \"SSL3\":\"300.00\",
	//jsonRedisCmd.append(AStockQuotName::SSL3);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SSL3));

	//// ��������4 --�䶯	// \"SSL4\":\"100.00\",
	//jsonRedisCmd.append(AStockQuotName::SSL4);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SSL4));

	//// ��������5 --�䶯	// \"SSL5\":\"100.00\",
	//jsonRedisCmd.append(AStockQuotName::SSL5);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SSL5));

	// ����۸�1(���ͣ���λ��) --�䶯	// \"BJW1\":\"135.760000\",
	vctCmd.push_back(AStockQuotName::BJW1);

	sof_string::itostr(in_struAstockQuot.BJW1, strItoATran);
	vctCmd.push_back(strItoATran);

	//// ����۸�2(���ͣ���λ��) --�䶯	// \"BJW2\":\"135.610000\",
	//jsonRedisCmd.append(AStockQuotName::BJW2);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BJW2));

	//// ����۸�3(���ͣ���λ��) --�䶯	// \"BJW3\":\"135.600000\",
	//jsonRedisCmd.append(AStockQuotName::BJW3);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BJW3));

	//// ����۸�4(���ͣ���λ��) --�䶯	// \"BJW4\":\"135.580000\",
	//jsonRedisCmd.append(AStockQuotName::BJW4);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BJW4));

	//// ����۸�5(���ͣ���λ��) --�䶯
	//// \"BJW5\":\"135.560000\",
	//jsonRedisCmd.append(AStockQuotName::BJW5);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BJW5));

	// ��������1 --�䶯	// \"BSL1\":\"100.00\",
	vctCmd.push_back(AStockQuotName::BSL1);

	sof_string::itostr(in_struAstockQuot.BSL1, strItoATran);
	vctCmd.push_back(strItoATran);

	//// ��������2 --�䶯	// \"BSL2\":\"300.00\",
	//jsonRedisCmd.append(AStockQuotName::BSL2);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BSL2));

	//// ��������3 --�䶯	// \"BSL3\":\"800.00\",
	//jsonRedisCmd.append(AStockQuotName::BSL3);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BSL3));

	//// ��������4 --�䶯	// \"BSL4\":\"1000.00\",
	//jsonRedisCmd.append(AStockQuotName::BSL4);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BSL4));

	//// ��������5 --�䶯	// \"BSL5\":\"100.00\",
	//jsonRedisCmd.append(AStockQuotName::BSL5);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BSL5));

	// ����ʱ�� 
	//jsonRedisCmd.append(AStockQuotName::hqsj + "_");
	//jsonRedisCmd.append(in_struAstockQuot.timehqsj);

	// ֤ȯ����	// \"ZQDM\":\"300571\",	
	vctCmd.push_back(AStockQuotName::zqdm);

	vctCmd.push_back(in_struAstockQuot.zqdm);

	/*
	// ֤ȯ����	// \"ZQMC\":\"\",
	jsonRedisCmd.append(AStockQuotName::zqmc);
	jsonRedisCmd.append(in_struAstockQuot.zqmc);
	*/
	// ����ʱ��	// \"OrigTime\":\"2017-04-13 14:01:06.000\"
	vctCmd.push_back(AStockQuotName::hqsj);

	vctCmd.push_back(in_struAstockQuot.hqsj);

	/*
	// ��������	// \"hqktype\":\"SZSTEP.W.010\",
	jsonRedisCmd.append(AStockQuotName::hqktype);
	jsonRedisCmd.append(in_struAstockQuot.hqktype);
	*/
	// ͣ�̱�־ --�䶯	// \"TPBZ\":\"F\",
	vctCmd.push_back(AStockQuotName::TPBZ);

	vctCmd.push_back(in_struAstockQuot.TPBZ);

	// ԭʼ�ַ���
	vctCmd.push_back(AStockQuotName::OriginStr);

	vctCmd.push_back(in_struAstockQuot.OriginStr);

	// read redis 
	string strRedisRes;
	long long llRedisRes_ll = 0;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv(vctCmd, &llRedisRes_ll, &strRedisRes,
		nullptr, nullptr, nullptr);

	bool bIsSuccees = Tgw_RedisHelper::IsRedisCmdSuccess(emPcbCallRes, llRedisRes_ll, strRedisRes);
	if (!bIsSuccees)
	{
		string strDebug("Redisִ�� HMSET AStockQuotTGW_Prefix ����,res = [");
		strDebug += strRedisRes;
		strDebug += "].";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	if (!in_bIsRecordBefore)
	{
		// ���֮ǰû�д����
		// ����Key��Ч��
		vctCmd.clear();

		string strKey(simutgw::g_Key_AStockQuotTGW_Prefix);
		strKey += in_struAstockQuot.zqdm;

		vctCmd.push_back("EXPIRE");
		vctCmd.push_back(strKey);
		vctCmd.push_back(Tgw_RedisHelper::g_Expire_8Hour);

		// redis res
		strRedisRes = "";
		llRedisRes_ll = 0;

		emPcbCallRes = Tgw_RedisHelper::RunCmdArgv(vctCmd, &llRedisRes_ll, &strRedisRes,
			nullptr, nullptr, nullptr);

		bIsSuccees = Tgw_RedisHelper::IsRedisCmdSuccess(emPcbCallRes, llRedisRes_ll, strRedisRes);
		if (!bIsSuccees)
		{
			string strDebug("Redisִ�� EXPIRE AStockQuotTGW_Prefix����res=[");
			strDebug += strRedisRes;
			strDebug += "].";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
			return -1;
		}
	}

	//
	// ����������֮��Ľ�������д��Redis
	vctCmd.clear();
	vctCmd.push_back("HMSET");
	vctCmd.push_back(simutgw::g_Key_AStockQuotTGW_TradeVolume);

	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::maxgain;
	vctCmd.push_back(strCmdTmp);

	vctCmd.push_back(strMaxgain);

	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::minfall;
	vctCmd.push_back(strCmdTmp);

	vctCmd.push_back(strMinfall);

	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::TPBZ;
	vctCmd.push_back(strCmdTmp);

	vctCmd.push_back(in_struAstockQuot.TPBZ);

	// ---------------����--------------
	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::cjsl;
	vctCmd.push_back(strCmdTmp);

	sof_string::itostr(in_ui64mGapCjsl, strItoATran);
	vctCmd.push_back(strItoATran);

	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::cjje;
	vctCmd.push_back(strCmdTmp);

	sof_string::itostr(in_ui64mGapCjje, strItoATran);
	vctCmd.push_back(strItoATran);
	// ---------------����--------------

	//--------------��һ��һ----------------
	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::SJW1;
	vctCmd.push_back(strCmdTmp);

	sof_string::itostr(in_struAstockQuot.SJW1, strItoATran);
	vctCmd.push_back(strItoATran);

	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::SSL1;
	vctCmd.push_back(strCmdTmp);

	sof_string::itostr(in_struAstockQuot.SSL1, strItoATran);
	vctCmd.push_back(strItoATran);

	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::BJW1;
	vctCmd.push_back(strCmdTmp);

	sof_string::itostr(in_struAstockQuot.BJW1, strItoATran);
	vctCmd.push_back(strItoATran);

	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::BSL1;
	vctCmd.push_back(strCmdTmp);

	sof_string::itostr(in_struAstockQuot.BSL1, strItoATran);
	vctCmd.push_back(strItoATran);
	//--------------��һ��һ----------------

	//--------------����ɽ���----------------
	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::zjjg;
	vctCmd.push_back(strCmdTmp);

	sof_string::itostr(in_struAstockQuot.zjjg, strItoATran);
	vctCmd.push_back(strItoATran);
	//--------------����ɽ���----------------
	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::hqsj;
	vctCmd.push_back(strCmdTmp);

	vctCmd.push_back(in_struAstockQuot.hqsj);

	strRedisRes = "";
	llRedisRes_ll = 0;
	emPcbCallRes = Tgw_RedisHelper::RunCmdArgv(vctCmd, &llRedisRes_ll, &strRedisRes,
		nullptr, nullptr, nullptr);

	bIsSuccees = Tgw_RedisHelper::IsRedisCmdSuccess(emPcbCallRes, llRedisRes_ll, strRedisRes);
	if (!bIsSuccees)
	{
		string strDebug("Redisִ�� HMSET AStockQuotTGW_TradeVolume����res=[");
		strDebug += strRedisRes;
		strDebug += "].";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	if (!TgwMarketInfoProc::m_bIsMarketTradeVolumnCreated)
	{
		// ���֮ǰû�д����
		// ����Key��Ч��
		// 8Сʱ����ʱ��
		vctCmd.clear();
		vctCmd.push_back("EXPIRE");
		vctCmd.push_back(simutgw::g_Key_AStockQuotTGW_TradeVolume);
		vctCmd.push_back(Tgw_RedisHelper::g_Expire_8Hour);

		strRedisRes = "";
		llRedisRes_ll = 0;
		emPcbCallRes = Tgw_RedisHelper::RunCmdArgv(vctCmd, &llRedisRes_ll, &strRedisRes,
			nullptr, nullptr, nullptr);

		bIsSuccees = Tgw_RedisHelper::IsRedisCmdSuccess(emPcbCallRes, llRedisRes_ll, strRedisRes);
		if (!bIsSuccees)
		{
			string strDebug("Redisִ�� EXPIRE AStockQuotTGW_TradeVolume����res=[");
			strDebug += strRedisRes;
			strDebug += "].";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
			return -1;
		}

		TgwMarketInfoProc::m_bIsMarketTradeVolumnCreated = true;
	}

	return 0;
}


int TgwMarketInfoProc::TaskProc(void)
{
	int iRes = ReadRealMarketInfo();

	return iRes;
}