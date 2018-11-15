#include "TgwMarketInfoProc.h"

#include "json/json.h"

#include "quotation/MarketInfoHelper.h"
#include "quotation/AStockQuotationHelper.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "tool_redis/Tgw_RedisHelper.h"

#include "util/TimeDuration.h"

#include "config/sys_function_base.h"

// 市场交易容量的Key是否已创立
// true -- 已创立
// false -- 未创立
bool TgwMarketInfoProc::m_bIsMarketTradeVolumnCreated = false;

TgwMarketInfoProc::TgwMarketInfoProc(void)
	:m_scl(keywords::channel = "TgwMarketInfoProc")
{

}


TgwMarketInfoProc::~TgwMarketInfoProc(void)
{

}

//从高速行情网关写入的Redis读取行情
int TgwMarketInfoProc::ReadRealMarketInfo()
{
	static const string ftag("TgwMarketInfoProc::ReadRealMarketInfo() ");

	// read market-info from Redis.
	// 按固定股票前缀读出所有行情信息
	// 现仅支持A股

	// read redis 
	// 读取static行情	

	string strRedisCmd;
	bool bHaveData = true;

	struct AStockQuot struAstockQuot;

	while (bHaveData)
	{
		// 读取变动行情
		strRedisCmd = "LPOP ";
		strRedisCmd += simutgw::g_Key_AStockQuotationChange;

		long long llRedisRes = 0;
		string strRedisRes;

		RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmd(strRedisCmd, &llRedisRes, &strRedisRes,
			nullptr, nullptr, nullptr);
		if (RedisReply_nil == emPcbCallRes)
		{
			// redis无委托
			simutgw::Simutgw_Sleep();

			bHaveData = false;
		}
		else if (RedisReply_string != emPcbCallRes)
		{
			string strTran;
			sof_string::itostr(emPcbCallRes, strTran);

			string strDebug("Redis执行 LPOP AStockQuotationChange错误，cmd=[");
			strDebug += strRedisCmd;
			strDebug += "],res=[";
			strDebug += strTran;
			strDebug += "]. str=[";
			strDebug += strRedisRes;
			strDebug += "]";

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

			return -1;
		}

		// 将行情字符串拆解装入结构体		
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
				// 无行情
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

		// 和上一次的行情进行比较，计算出这段时间内可以产生的市场交易容量
		// 成交数量 -- 容量
		uint64_t ui64GapCjsl = 0;
		// 成交金额 -- 容量
		uint64_t ui64GapCjje = 0;

		// 该行情是否已经保存在Redis过
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
			// CJSL CJJE 无变化
			continue;
		}

		// process after read market-info from Redis. 需要和历史模拟交易进行拟合
		//???

		// write processed market-info into Redis.
		// 回写自用的行情
		iRes = StoreSelfUseQuotation(struAstockQuot, ui64GapCjsl, ui64GapCjje, bIsInRedisBefore);
		if (0 != iRes)
		{

			continue;
		}
	}

	return 0;
}

/*
计算和上次行情之间的交易差
Param :
bool& out_bIsRecordBefore :
true -- 该行情之前在Redis中存在
false -- 该行情之前在Redis中不存在

Return:
0 -- 计算
1 -- 无需更新行情
非0 -- 计算失败
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

	// 成交数量 --变动	// \"CJSL\":\"536524.00\",
	uint64_t ui64PreviousCjsl = 0;
	// 成交金额 --变动	// \"CJJE\":\"73703408.0700\",
	simutgw::uint64_t_Money ui64mPreviousCjje = 0;

	string strPreviousHqsj;

	// 从自己缓存的行情中读取
	int iRes = MarketInfoHelper::GetCurrentQuotationByStockId(struAstockQuot.zqdm,
		ui64PreviousCjsl, ui64mPreviousCjje, strPreviousHqsj);
	if (0 != iRes)
	{
		// 获取失败
		ui64PreviousCjsl = 0;
		ui64mPreviousCjje = 0;

		/*
		string strDebug("未获取");
		strDebug += struAstockQuot.zqdm;
		strDebug += "历史行情，数值回归默认";
		EzLog::Out(ftag, debug, strDebug);
		*/

		out_bIsRecordBefore = false;
	}

	if (out_bIsRecordBefore)
	{
		// 之前有历史行情
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

		// 比较新行情的时间
		if (0 == i64TimeDiff)
		{
			// 时间一致，无需更新
			return 1;
		}
		else if (0 > i64TimeDiff)
		{
			// 消息过滤
			{
				string strTran;

				// 发现时序错乱的行情
				string strDebug("找到时序错乱行情 [");
				strDebug += struAstockQuot.OriginStr;
				strDebug += "] against，diff[";
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

		// 可能存在成交没有变动，但是其他变动的情况
		// 比较成交数量、成交金额，无变动则不更新
		if (ui64PreviousCjsl == struAstockQuot.cjsl || ui64mPreviousCjje == struAstockQuot.cjje)
		{
			return 1;
		}
	}

	// 查看是否已经停牌
	if (0 != struAstockQuot.TPBZ.compare("F"))
	{
		// 如果已经停牌
		out_ui64GapCjsl = 0;
		out_ui64mGapCjje = 0;

		return 0;
	}

	// 成交数量 -- 容量
	simutgw::uint64_t_Money ui64GapCjsl = struAstockQuot.cjsl - ui64PreviousCjsl;
	// 成交金额 -- 容量
	simutgw::uint64_t_Money ui64mGapCjje = struAstockQuot.cjje - ui64mPreviousCjje;

	if ((ui64PreviousCjsl > struAstockQuot.cjsl) || (ui64mPreviousCjje > struAstockQuot.cjje))
	{
		// 消息过滤
		{
			string strTran;

			string strDebug("zqdm[");
			strDebug += struAstockQuot.zqdm;
			strDebug += "]价格倒挂，Previous CJSL=[";
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

	// 消息过滤
	{
		//string strTran;
		//string strDebug( "得出 gap CJSL=[" );
		//strDebug += sof_string::itostr( out_ui64GapCjsl, strTran );
		//strDebug += "],CJJE=[";
		//strDebug += sof_string::itostr( out_ui64mGapCjje, strTran );
		//strDebug += "].";
		//BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << strDebug;
	}

	return 0;
}

// 储存自用行情
int TgwMarketInfoProc::StoreSelfUseQuotation(const struct AStockQuot& in_struAstockQuot,
	const simutgw::uint64_t_Money& in_ui64mGapCjsl,
	const simutgw::uint64_t_Money& in_ui64mGapCjje,
	bool in_bIsRecordBefore)
{
	static const string ftag("TgwMarketInfoProc::StoreSelfUseQuotation()");

	// HMSET KEY_NAME FIELD1 VALUE1 ...FIELDN VALUEN
	// 将行情写入Redis	

	// 今日最高涨幅
	string strMaxgain;
	// 今日最低跌幅
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

	// 今日最高涨幅
	vctCmd.push_back(AStockQuotName::maxgain);

	sof_string::itostr(in_struAstockQuot.maxgain, strMaxgain);
	vctCmd.push_back(strMaxgain);

	// 今日最低跌幅
	vctCmd.push_back(AStockQuotName::minfall);

	sof_string::itostr(in_struAstockQuot.minfall, strMinfall);
	vctCmd.push_back(strMinfall);

	/*
	// 今日开盘价(整型，单位分)	// \"JRKP\":\"134.500000\",
	jsonRedisCmd.append(AStockQuotName::jrkp);
	jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.jrkp));

	// 最高成交价(整型，单位分) --变动	// \"ZGJG\":\"139.800000\",
	jsonRedisCmd.append(AStockQuotName::zgjg);
	jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.zgjg));

	// 最低成交价(整型，单位分) --变动	// \"ZDJG\":\"134.500000\",
	jsonRedisCmd.append(AStockQuotName::zdjg);
	jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.zdjg));
	*/
	// 最近成交价(整型，单位分) --现价 变动	// \"ZJJG\":\"136.400000\",
	vctCmd.push_back(AStockQuotName::zjjg);

	sof_string::itostr(in_struAstockQuot.zjjg, strItoATran);
	vctCmd.push_back(strItoATran);

	// 成交数量 --变动	// \"CJSL\":\"536524.00\",
	vctCmd.push_back(AStockQuotName::cjsl);

	sof_string::itostr(in_struAstockQuot.cjsl, strItoATran);
	vctCmd.push_back(strItoATran);

	// 成交金额(整型，单位分) --变动	// \"CJJE\":\"73703408.0700\",
	vctCmd.push_back(AStockQuotName::cjje);

	sof_string::itostr(in_struAstockQuot.cjje, strItoATran);
	vctCmd.push_back(strItoATran);

	//// 市盈率1 --变动	// \"SYL1\":\"211.800000\",
	//jsonRedisCmd.append(AStockQuotName::SYL1);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SYL1));

	//// 市盈率2 --变动	// \"SYL2\":\"0\",
	//jsonRedisCmd.append(AStockQuotName::SYL2);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SYL2));

	//// 成交笔数 --变动
	//// \"CJBS\":\"2065\",
	//jsonRedisCmd.append(AStockQuotName::cjbs);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.cjbs));

	// 卖出价格1(整型，单位分) --变动	// \"SJW1\":\"136.000000\",
	vctCmd.push_back(AStockQuotName::SJW1);

	sof_string::itostr(in_struAstockQuot.SJW1, strItoATran);
	vctCmd.push_back(strItoATran);

	//// 卖出价格2(整型，单位分)	// \"SJW2\":\"136.500000\",
	//jsonRedisCmd.append(AStockQuotName::SJW2);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SJW2));

	//// 卖出价格3(整型，单位分)	// \"SJW3\":\"136.600000\",
	//jsonRedisCmd.append(AStockQuotName::SJW3);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SJW3));

	//// 卖出价格4(整型，单位分)	// \"SJW4\":\"136.630000\",
	//jsonRedisCmd.append(AStockQuotName::SJW4);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SJW4));

	//// 卖出价格5(整型，单位分)	// \"SJW5\":\"136.640000\",
	//jsonRedisCmd.append(AStockQuotName::SJW5);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SJW5));

	// 卖出数量1 --变动	// \"SSL1\":\"100.00\",
	vctCmd.push_back(AStockQuotName::SSL1);

	sof_string::itostr(in_struAstockQuot.SSL1, strItoATran);
	vctCmd.push_back(strItoATran);

	//// 卖出数量2 --变动	// \"SSL2\":\"600.00\",
	//jsonRedisCmd.append(AStockQuotName::SSL2);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SSL2));

	//// 卖出数量3 --变动	// \"SSL3\":\"300.00\",
	//jsonRedisCmd.append(AStockQuotName::SSL3);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SSL3));

	//// 卖出数量4 --变动	// \"SSL4\":\"100.00\",
	//jsonRedisCmd.append(AStockQuotName::SSL4);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SSL4));

	//// 卖出数量5 --变动	// \"SSL5\":\"100.00\",
	//jsonRedisCmd.append(AStockQuotName::SSL5);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.SSL5));

	// 买入价格1(整型，单位分) --变动	// \"BJW1\":\"135.760000\",
	vctCmd.push_back(AStockQuotName::BJW1);

	sof_string::itostr(in_struAstockQuot.BJW1, strItoATran);
	vctCmd.push_back(strItoATran);

	//// 买入价格2(整型，单位分) --变动	// \"BJW2\":\"135.610000\",
	//jsonRedisCmd.append(AStockQuotName::BJW2);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BJW2));

	//// 买入价格3(整型，单位分) --变动	// \"BJW3\":\"135.600000\",
	//jsonRedisCmd.append(AStockQuotName::BJW3);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BJW3));

	//// 买入价格4(整型，单位分) --变动	// \"BJW4\":\"135.580000\",
	//jsonRedisCmd.append(AStockQuotName::BJW4);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BJW4));

	//// 买入价格5(整型，单位分) --变动
	//// \"BJW5\":\"135.560000\",
	//jsonRedisCmd.append(AStockQuotName::BJW5);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BJW5));

	// 买入数量1 --变动	// \"BSL1\":\"100.00\",
	vctCmd.push_back(AStockQuotName::BSL1);

	sof_string::itostr(in_struAstockQuot.BSL1, strItoATran);
	vctCmd.push_back(strItoATran);

	//// 买入数量2 --变动	// \"BSL2\":\"300.00\",
	//jsonRedisCmd.append(AStockQuotName::BSL2);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BSL2));

	//// 买入数量3 --变动	// \"BSL3\":\"800.00\",
	//jsonRedisCmd.append(AStockQuotName::BSL3);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BSL3));

	//// 买入数量4 --变动	// \"BSL4\":\"1000.00\",
	//jsonRedisCmd.append(AStockQuotName::BSL4);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BSL4));

	//// 买入数量5 --变动	// \"BSL5\":\"100.00\",
	//jsonRedisCmd.append(AStockQuotName::BSL5);
	//jsonRedisCmd.append(boost::lexical_cast<string>(in_struAstockQuot.BSL5));

	// 行情时间 
	//jsonRedisCmd.append(AStockQuotName::hqsj + "_");
	//jsonRedisCmd.append(in_struAstockQuot.timehqsj);

	// 证券代码	// \"ZQDM\":\"300571\",	
	vctCmd.push_back(AStockQuotName::zqdm);

	vctCmd.push_back(in_struAstockQuot.zqdm);

	/*
	// 证券名称	// \"ZQMC\":\"\",
	jsonRedisCmd.append(AStockQuotName::zqmc);
	jsonRedisCmd.append(in_struAstockQuot.zqmc);
	*/
	// 行情时间	// \"OrigTime\":\"2017-04-13 14:01:06.000\"
	vctCmd.push_back(AStockQuotName::hqsj);

	vctCmd.push_back(in_struAstockQuot.hqsj);

	/*
	// 行情类型	// \"hqktype\":\"SZSTEP.W.010\",
	jsonRedisCmd.append(AStockQuotName::hqktype);
	jsonRedisCmd.append(in_struAstockQuot.hqktype);
	*/
	// 停盘标志 --变动	// \"TPBZ\":\"F\",
	vctCmd.push_back(AStockQuotName::TPBZ);

	vctCmd.push_back(in_struAstockQuot.TPBZ);

	// 原始字符串
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
		string strDebug("Redis执行 HMSET AStockQuotTGW_Prefix 错误,res = [");
		strDebug += strRedisRes;
		strDebug += "].";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	if (!in_bIsRecordBefore)
	{
		// 如果之前没有储存过
		// 设置Key有效期
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
			string strDebug("Redis执行 EXPIRE AStockQuotTGW_Prefix错误，res=[");
			strDebug += strRedisRes;
			strDebug += "].";
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
			return -1;
		}
	}

	//
	// 将两次行情之间的交易容量写入Redis
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

	// ---------------均价--------------
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
	// ---------------均价--------------

	//--------------买一卖一----------------
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
	//--------------买一卖一----------------

	//--------------最近成交价----------------
	strCmdTmp = in_struAstockQuot.zqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::zjjg;
	vctCmd.push_back(strCmdTmp);

	sof_string::itostr(in_struAstockQuot.zjjg, strItoATran);
	vctCmd.push_back(strItoATran);
	//--------------最近成交价----------------
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
		string strDebug("Redis执行 HMSET AStockQuotTGW_TradeVolume错误，res=[");
		strDebug += strRedisRes;
		strDebug += "].";
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		return -1;
	}

	if (!TgwMarketInfoProc::m_bIsMarketTradeVolumnCreated)
	{
		// 如果之前没有储存过
		// 设置Key有效期
		// 8小时过期时间
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
			string strDebug("Redis执行 EXPIRE AStockQuotTGW_TradeVolume错误，res=[");
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