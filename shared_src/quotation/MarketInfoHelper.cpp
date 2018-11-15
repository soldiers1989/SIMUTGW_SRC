#include "MarketInfoHelper.h"

#include "simutgw_config/g_values_sys_run_config.h"

#include "quotation/AStockQuotationHelper.h"

#include "tool_redis/Tgw_RedisHelper.h"
#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"

#include "order/StockHelper.h"


MarketInfoHelper::MarketInfoHelper( void )
{
}

MarketInfoHelper::~MarketInfoHelper( void )
{
}


/*
获取当前储存的实时行情
Return:
0 -- 获取成功
非0 -- 获取失败
*/
int MarketInfoHelper::GetCurrentQuotationByStockId( const string& in_cstrZqdm,
	uint64_t& out_ui64Cjsl, uint64_t& out_ui64Cjje, string& out_strHqsj )
{
	static const string ftag( "MarketInfoHelper::GetCurrentQuotationByStockId() " );

	std::string strCmdTmp;
	string straItoa;
	std::vector<string> vctCmd;

	// 从自己缓存的行情中读取	
	vctCmd.push_back( "HMGET" );

	strCmdTmp = simutgw::g_Key_AStockQuotTGW_Prefix;
	strCmdTmp += in_cstrZqdm;
	vctCmd.push_back( strCmdTmp );

	vctCmd.push_back( AStockQuotName::cjsl );

	vctCmd.push_back( AStockQuotName::cjje );

	vctCmd.push_back( AStockQuotName::hqsj );

	//
	long long llRedisRes = 0;
	std::string strRedisRes;
	std::vector<StgwRedisReply> vectArray;
	size_t stElemSize = 0;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv( vctCmd, &llRedisRes, &strRedisRes,
		nullptr, &vectArray, &stElemSize );
	if ( RedisReply_array != emPcbCallRes )
	{
		EzLog::e( ftag, "redis call error! reply not array" );

		return -1;

	}

	// 已知会返回数组
	size_t iSize = vectArray.size();

	if ( 3 > iSize )
	{
		// 值不存在
		return -1;
	}

	int i = 0;
	string strCjsl = vectArray[i].str;
	i = 1;
	string strCjje = vectArray[i].str;
	i = 2;
	out_strHqsj = vectArray[i].str;

	if ( 0 == strCjsl.length() || 0 == strCjje.length() || 0 == out_strHqsj.length() )
	{
		// 值不存在
		return -1;
	}

	int iRes = Tgw_StringUtil::String2UInt64_strtoui64( strCjsl, out_ui64Cjsl );
	if ( 0 != iRes )
	{
		return -1;
	}

	iRes = Tgw_StringUtil::String2UInt64_strtoui64( strCjje, out_ui64Cjje );
	if ( 0 != iRes )
	{
		return -1;
	}

	return 0;
}

/*
获取当前储存的交易圈实时行情容量
如果当前交易圈容量没有，则取总容量的，如果总容量也没有，则返回失败
Return:
0 -- 获取成功
非0 -- 获取失败
*/
int MarketInfoHelper::GetCurrQuotGapByCircle( const string& in_cstrZqdm,
	const string& in_strSide, const string& in_cstrTradeCircle,
	simutgw::QuotationType& in_quotType,
	simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
	uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje, string& out_strHqsj,
	string& out_strTpbz )
{
	static const string ftag( "MarketInfoHelper::GetCurrQuotGapByCircle() " );

	if ( 0 == in_cstrZqdm.length() || 0 == in_cstrTradeCircle.length() )
	{
		return 1;
	}

	int iRes = 0;
	if (simutgw::QuotationType::AveragePrice == in_quotType)
	{
		iRes = GetCurrQuotGapByCircle_AveragePrice( in_cstrZqdm, in_cstrTradeCircle,
			out_ui64mMaxGain, out_ui64mMinFall,
			out_ui64Cjsl, out_ui64Cjje, out_strHqsj,
			out_strTpbz );
	}
	else if (simutgw::QuotationType::SellBuyPrice == in_quotType)
	{
		iRes = GetCurrQuotGapByCircle_SellBuyPrice( in_cstrZqdm,
			in_strSide, in_cstrTradeCircle,
			out_ui64mMaxGain, out_ui64mMinFall,
			out_ui64Cjsl, out_ui64Cjje, out_strHqsj,
			out_strTpbz );
	}
	else if (simutgw::QuotationType::RecentMatchPrice == in_quotType)
	{
		iRes = GetCurrQuotGapByCircle_RecentPrice( in_cstrZqdm, in_cstrTradeCircle,
			out_ui64mMaxGain, out_ui64mMinFall,
			out_ui64Cjsl, out_ui64Cjje, out_strHqsj,
			out_strTpbz );
	}
	else
	{
		//
		return -1;
	}

	return iRes;
}

/*
获取当前储存的交易圈实时行情容量
如果当前交易圈容量没有，则取总容量的，如果总容量也没有，则返回失败

区间段均价

Return:
0 -- 获取成功
非0 -- 获取失败
*/
int MarketInfoHelper::GetCurrQuotGapByCircle_AveragePrice( const string& in_cstrZqdm, const string& in_cstrTradeCircle,
	simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
	uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje, string& out_strHqsj,
	string& out_strTpbz )
{
	static const string ftag( "MarketInfoHelper::GetCurrQuotGapByCircle() " );

	if ( 0 == in_cstrZqdm.length() || 0 == in_cstrTradeCircle.length() )
	{
		return 1;
	}

	// read redis 
	std::string strCmdTmp;
	string straItoa;
	std::vector<string> vctCmd;

	// 从自己缓存的行情中读取
	vctCmd.push_back( "HMGET" );
	vctCmd.push_back( simutgw::g_Key_AStockQuotTGW_TradeVolume );

	//
	// 实时的行情
	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_maxgain";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_minfall";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_CJSL";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_CJJE";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_OrigTime";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_TPBZ";
	vctCmd.push_back( strCmdTmp );

	//
	// 缓存历史交易圈行情
	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_CJSL_";
	strCmdTmp += in_cstrTradeCircle;
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_CJJE_";
	strCmdTmp += in_cstrTradeCircle;
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_OrigTime_";
	strCmdTmp += in_cstrTradeCircle;
	vctCmd.push_back( strCmdTmp );

	//
	long long llRedisRes = 0;
	std::string strRedisRes;
	std::vector<StgwRedisReply> vectArray;
	size_t stElemSize = 0;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv( vctCmd, &llRedisRes, &strRedisRes,
		nullptr, &vectArray, &stElemSize );
	if ( RedisReply_array != emPcbCallRes )
	{
		EzLog::e( ftag, "redis call error! HMGET AStockQuotTGW_TradeVolume reply not array" );

		return -1;
	}

	// 已知会返回数组
	size_t iSize = vectArray.size();

	// 实时行情	
	string strMaxGain;
	string strMinFall;
	string strLive_Cjsl;
	string strLive_Cjje;
	string strLive_Hqsj;
	string strTpbz;
	bool bIsLiveDataExist = true;

	// 交易圈值
	string strCircle_Cjsl;
	string strCircle_Cjje;
	string strCircle_Hqsj;
	bool bIsCircleDataExist = true;

	// 采用的行情值
	string strOut_Cjsl;
	string strOut_Cjje;


	if ( 8 <= iSize )
	{
		// 值存在
		int i = 0;
		strMaxGain = vectArray[i].str;
		i = 1;
		strMinFall = vectArray[i].str;
		i = 2;
		strLive_Cjsl = vectArray[i].str;
		i = 3;
		strLive_Cjje = vectArray[i].str;
		i = 4;
		strLive_Hqsj = vectArray[i].str;
		i = 5;
		strTpbz = vectArray[i].str;

		i = 6;
		strCircle_Cjsl = vectArray[i].str;
		i = 7;
		strCircle_Cjje = vectArray[i].str;
		i = 8;
		strCircle_Hqsj = vectArray[i].str;
	}
	else
	{
		return -1;
	}

	if ( 0 == strLive_Cjsl.length() || 0 == strLive_Cjje.length() || 0 == strLive_Hqsj.length() || 0 == strTpbz.length() )
	{
		// 值不存在
		bIsLiveDataExist = false;
	}

	if ( 0 == strCircle_Cjsl.length() || 0 == strCircle_Cjje.length() || 0 == strCircle_Hqsj.length() )
	{
		// 值不存在
		bIsCircleDataExist = false;
	}

	// 判断返回的数据
	if ( bIsLiveDataExist )
	{
		// 实时行情数据存在
		out_strTpbz = strTpbz;

		if ( bIsCircleDataExist )
		{
			// 交易圈行情存在
			// 比较两者的时间，取最新的那个
			int64_t i64TimeDiff = 0;

			int iRes = TimeStringUtil::CompareTime(strCircle_Hqsj, strLive_Hqsj, i64TimeDiff);
			if ( 0 > iRes )
			{
				string straItoa;
				string strDebug;
				strDebug += "unkown error when call CompareTime(";
				strDebug += strCircle_Hqsj;
				strDebug += ",";
				strDebug += strLive_Hqsj;
				strDebug += ") res=";
				strDebug += sof_string::itostr( iRes, straItoa );
				EzLog::e( ftag, strDebug );

				return -1;
			}

			if ( 0 == i64TimeDiff )
			{
				// 交易圈行情已和实时行情同步
				strOut_Cjsl = strCircle_Cjsl;
				strOut_Cjje = strCircle_Cjje;

				out_strHqsj = strCircle_Hqsj;
			}
			else if ( 0 < i64TimeDiff )
			{
				// 实时行情比交易圈行情更新
				strOut_Cjsl = strLive_Cjsl;
				strOut_Cjje = strLive_Cjje;

				out_strHqsj = strLive_Hqsj;
			}
			else
			{
				string straItoa;
				string strDebug;

				strDebug += "unkown error when call CompareTime(";
				strDebug += strCircle_Hqsj;
				strDebug += ",";
				strDebug += strLive_Hqsj;
				strDebug += ") TimeDiff=";
				strDebug += sof_string::itostr( i64TimeDiff, straItoa );
				EzLog::e( ftag, strDebug );

				// 采用实时行情
				strOut_Cjsl = strLive_Cjsl;
				strOut_Cjje = strLive_Cjje;

				out_strHqsj = strLive_Hqsj;
			}
		}
		else // if(bIsCircleDataExist)
		{
			// 交易圈行情不存在
			// 采用实时行情
			strOut_Cjsl = strLive_Cjsl;
			strOut_Cjje = strLive_Cjje;

			out_strHqsj = strLive_Hqsj;
		}
	}
	else // if( bIsLiveDataExist )
	{
		// 实时行情数据不存在
		return -1;
	}

	// 转换字符串为整型

	int iRes = Tgw_StringUtil::String2UInt64_strtoui64( strMaxGain, out_ui64mMaxGain );
	if ( 0 != iRes )
	{
		return -1;
	}

	iRes = Tgw_StringUtil::String2UInt64_strtoui64( strMinFall, out_ui64mMinFall );
	if ( 0 != iRes )
	{
		return -1;
	}

	iRes = Tgw_StringUtil::String2UInt64_strtoui64( strOut_Cjsl, out_ui64Cjsl );
	if ( 0 != iRes )
	{
		return -1;
	}

	iRes = Tgw_StringUtil::String2UInt64_strtoui64( strOut_Cjje, out_ui64Cjje );
	if ( 0 != iRes )
	{
		return -1;
	}

	return 0;
}

/*
获取当前储存的交易圈实时行情容量
如果当前交易圈容量没有，则取总容量的，如果总容量也没有，则返回失败

买一卖一

Return:
0 -- 获取成功
非0 -- 获取失败
*/
int MarketInfoHelper::GetCurrQuotGapByCircle_SellBuyPrice( const string& in_cstrZqdm,
	const string& in_strSide, const string& in_cstrTradeCircle,
	simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
	uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje, string& out_strHqsj,
	string& out_strTpbz )
{
	static const string ftag( "MarketInfoHelper::GetCurrQuotGapByCircle_SellBuyPrice() " );

	if ( 0 == in_cstrZqdm.length() || 0 == in_cstrTradeCircle.length() )
	{
		return 1;
	}

	// read redis 
	std::string strCmdTmp;
	string straItoa;
	std::vector<string> vctCmd;

	// 从自己缓存的行情中读取
	vctCmd.push_back( "HMGET" );
	vctCmd.push_back( simutgw::g_Key_AStockQuotTGW_TradeVolume );

	//
	// 实时的行情
	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_maxgain";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_minfall";
	vctCmd.push_back( strCmdTmp );

	if (0 == in_strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == in_strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
	{
		// 买取卖行情
		strCmdTmp = in_cstrZqdm;
		strCmdTmp += "_SSL1";
		vctCmd.push_back( strCmdTmp );

		strCmdTmp = in_cstrZqdm;
		strCmdTmp += "_SJW1";
		vctCmd.push_back( strCmdTmp );
	}
	else if (0 == in_strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == in_strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
	{
		// 卖取买行情
		strCmdTmp = in_cstrZqdm;
		strCmdTmp += "_BSL1";
		vctCmd.push_back( strCmdTmp );

		strCmdTmp = in_cstrZqdm;
		strCmdTmp += "_BJW1";
		vctCmd.push_back( strCmdTmp );
	}

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_OrigTime";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_TPBZ";
	vctCmd.push_back( strCmdTmp );

	//
	// 缓存历史交易圈行情
	if (0 == in_strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == in_strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
	{
		// 买取卖行情
		strCmdTmp = in_cstrZqdm;
		strCmdTmp += "_SSL1_";
		strCmdTmp += in_cstrTradeCircle;
		vctCmd.push_back( strCmdTmp );

		strCmdTmp = in_cstrZqdm;
		strCmdTmp += "_SJW1_";
		strCmdTmp += in_cstrTradeCircle;
		vctCmd.push_back( strCmdTmp );
	}
	else if (0 == in_strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == in_strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
	{
		// 卖取买行情
		strCmdTmp = in_cstrZqdm;
		strCmdTmp += "_BSL1_";
		strCmdTmp += in_cstrTradeCircle;
		vctCmd.push_back( strCmdTmp );

		strCmdTmp = in_cstrZqdm;
		strCmdTmp += "_BJW1_";
		strCmdTmp += in_cstrTradeCircle;
		vctCmd.push_back( strCmdTmp );
	}
	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_OrigTime_";
	strCmdTmp += in_cstrTradeCircle;
	vctCmd.push_back( strCmdTmp );

	//
	long long llRedisRes = 0;
	std::string strRedisRes;
	std::vector<StgwRedisReply> vectArray;
	size_t stElemSize = 0;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv( vctCmd, &llRedisRes, &strRedisRes,
		nullptr, &vectArray, &stElemSize );
	if ( RedisReply_array != emPcbCallRes )
	{
		EzLog::e( ftag, "redis call error! HMGET AStockQuotTGW_TradeVolume reply not array" );

		return -1;
	}

	// 已知会返回数组
	size_t iSize = vectArray.size();

	// 实时行情	
	string strMaxGain;
	string strMinFall;
	string strLive_Cjsl; // 买一卖一数量
	string strLive_Cjjg; // 买一卖一价格
	string strLive_Hqsj;
	string strTpbz;
	bool bIsLiveDataExist = true;

	// 交易圈值
	string strCircle_Cjsl; // 买一卖一数量
	string strCircle_Cjjg; // 买一卖一价格
	string strCircle_Hqsj;
	bool bIsCircleDataExist = true;

	// 采用的行情值
	string strOut_Cjsl;
	string strOut_Cjjg;

	if ( 8 <= iSize )
	{
		// 值存在
		int i = 0;
		strMaxGain = vectArray[i].str;
		i = 1;
		strMinFall = vectArray[i].str;
		i = 2;
		strLive_Cjsl = vectArray[i].str;
		i = 3;
		strLive_Cjjg = vectArray[i].str;
		i = 4;
		strLive_Hqsj = vectArray[i].str;
		i = 5;
		strTpbz = vectArray[i].str;

		i = 6;
		strCircle_Cjsl = vectArray[i].str;
		i = 7;
		strCircle_Cjjg = vectArray[i].str;
		i = 8;
		strCircle_Hqsj = vectArray[i].str;
	}
	else
	{
		return -1;
	}

	if ( 0 == strLive_Cjsl.length() || 0 == strLive_Cjjg.length() || 0 == strLive_Hqsj.length() || 0 == strTpbz.length() )
	{
		// 值不存在
		bIsLiveDataExist = false;
	}

	if ( 0 == strCircle_Cjsl.length() || 0 == strCircle_Cjjg.length() || 0 == strCircle_Hqsj.length() )
	{
		// 值不存在
		bIsCircleDataExist = false;
	}

	// 判断返回的数据
	if ( bIsLiveDataExist )
	{
		// 实时行情数据存在
		out_strTpbz = strTpbz;

		if ( bIsCircleDataExist )
		{
			// 交易圈行情存在
			// 比较两者的时间，取最新的那个
			int64_t i64TimeDiff = 0;

			int iRes = TimeStringUtil::CompareTime(strCircle_Hqsj, strLive_Hqsj, i64TimeDiff);
			if ( 0 > iRes )
			{
				string straItoa;
				string strDebug( "unkown error when call CompareTime(" );
				strDebug += strCircle_Hqsj;
				strDebug += ",";
				strDebug += strLive_Hqsj;
				strDebug += ") res=";
				strDebug += sof_string::itostr( iRes, straItoa );
				EzLog::e( ftag, strDebug );

				return -1;
			}

			if ( 0 == i64TimeDiff )
			{
				// 交易圈行情已和实时行情同步
				strOut_Cjsl = strCircle_Cjsl;
				strOut_Cjjg = strCircle_Cjjg;

				out_strHqsj = strCircle_Hqsj;
			}
			else if ( 0 < i64TimeDiff )
			{
				// 实时行情比交易圈行情更新
				strOut_Cjsl = strLive_Cjsl;
				strOut_Cjjg = strLive_Cjjg;

				out_strHqsj = strLive_Hqsj;
			}
			else
			{
				string straItoa;
				string strDebug( "unkown error when call CompareTime(" );
				strDebug += strCircle_Hqsj;
				strDebug += ",";
				strDebug += strLive_Hqsj;
				strDebug += ") TimeDiff=";
				strDebug += sof_string::itostr( i64TimeDiff, straItoa );
				EzLog::e( ftag, strDebug );

				// 采用实时行情
				strOut_Cjsl = strLive_Cjsl;
				strOut_Cjjg = strLive_Cjjg;

				out_strHqsj = strLive_Hqsj;
			}
		}
		else // if(bIsCircleDataExist)
		{
			// 交易圈行情不存在
			// 采用实时行情
			strOut_Cjsl = strLive_Cjsl;
			strOut_Cjjg = strLive_Cjjg;

			out_strHqsj = strLive_Hqsj;
		}
	}
	else // if( bIsLiveDataExist )
	{
		// 实时行情数据不存在
		return -1;
	}

	// 转换字符串为整型

	int iRes = Tgw_StringUtil::String2UInt64_strtoui64( strMaxGain, out_ui64mMaxGain );
	if ( 0 != iRes )
	{
		return -1;
	}

	iRes = Tgw_StringUtil::String2UInt64_strtoui64( strMinFall, out_ui64mMinFall );
	if ( 0 != iRes )
	{
		return -1;
	}

	iRes = Tgw_StringUtil::String2UInt64_strtoui64( strOut_Cjsl, out_ui64Cjsl );
	if ( 0 != iRes )
	{
		return -1;
	}

	simutgw::uint64_t_Money ui64mCjjg = 0;
	iRes = Tgw_StringUtil::String2UInt64_strtoui64( strOut_Cjjg, ui64mCjjg );
	if ( 0 != iRes )
	{
		return -1;
	}

	out_ui64Cjje = ui64mCjjg * out_ui64Cjsl;

	return 0;
}

/*
获取当前储存的交易圈实时行情容量
如果当前交易圈容量没有，则取总容量的，如果总容量也没有，则返回失败

最近成交价

Return:
0 -- 获取成功
非0 -- 获取失败
*/
int MarketInfoHelper::GetCurrQuotGapByCircle_RecentPrice( const string& in_cstrZqdm, const string& in_cstrTradeCircle,
	simutgw::uint64_t_Money& out_ui64mMaxGain, simutgw::uint64_t_Money& out_ui64mMinFall,
	uint64_t& out_ui64Cjsl, simutgw::uint64_t_Money& out_ui64Cjje, string& out_strHqsj,
	string& out_strTpbz )
{
	static const string ftag( "MarketInfoHelper::GetCurrQuotGapByCircle_RecentPrice() " );

	if ( 0 == in_cstrZqdm.length() || 0 == in_cstrTradeCircle.length() )
	{
		return 1;
	}

	// read redis 
	std::string strCmdTmp;
	string straItoa;
	std::vector<string> vctCmd;

	// 从自己缓存的行情中读取
	vctCmd.push_back( "HMGET" );
	vctCmd.push_back( simutgw::g_Key_AStockQuotTGW_TradeVolume );

	//
	// 实时的行情
	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_maxgain";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_minfall";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_CJSL";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_ZJJG";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_OrigTime";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_TPBZ";
	vctCmd.push_back( strCmdTmp );

	//
	// 缓存历史交易圈行情
	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_CJSL_";
	strCmdTmp += in_cstrTradeCircle;
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_ZJJG_";
	strCmdTmp += in_cstrTradeCircle;
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_OrigTime_";
	strCmdTmp += in_cstrTradeCircle;
	vctCmd.push_back( strCmdTmp );

	//
	long long llRedisRes = 0;
	std::string strRedisRes;
	std::vector<StgwRedisReply> vectArray;
	size_t stElemSize = 0;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv( vctCmd, &llRedisRes, &strRedisRes,
		nullptr, &vectArray, &stElemSize );
	if ( RedisReply_array != emPcbCallRes )
	{
		EzLog::e( ftag, "redis call error! HMGET AStockQuotTGW_TradeVolume reply not array" );

		return -1;
	}

	// 已知会返回数组
	size_t iSize = vectArray.size();

	// 实时行情	
	string strMaxGain;
	string strMinFall;
	string strLive_Cjsl;
	string strLive_Cjjg; // 最近成交价
	string strLive_Hqsj;
	string strTpbz;
	bool bIsLiveDataExist = true;

	// 交易圈值
	string strCircle_Cjsl;
	string strCircle_Cjjg; // 最近成交价
	string strCircle_Hqsj;
	bool bIsCircleDataExist = true;

	// 采用的行情值
	string strOut_Cjsl;
	string strOut_Cjjg;


	if ( 8 <= iSize )
	{
		// 值存在
		int i = 0;
		strMaxGain = vectArray[i].str;
		i = 1;
		strMinFall = vectArray[i].str;
		i = 2;
		strLive_Cjsl = vectArray[i].str;
		i = 3;
		strLive_Cjjg = vectArray[i].str;
		i = 4;
		strLive_Hqsj = vectArray[i].str;
		i = 5;
		strTpbz = vectArray[i].str;

		i = 6;
		strCircle_Cjsl = vectArray[i].str;
		i = 7;
		strCircle_Cjjg = vectArray[i].str;
		i = 8;
		strCircle_Hqsj = vectArray[i].str;
	}
	else
	{
		return -1;
	}

	if ( 0 == strLive_Cjsl.length() || 0 == strLive_Cjjg.length() || 0 == strLive_Hqsj.length() || 0 == strTpbz.length() )
	{
		// 值不存在
		bIsLiveDataExist = false;
	}

	if ( 0 == strCircle_Cjsl.length() || 0 == strCircle_Cjjg.length() || 0 == strCircle_Hqsj.length() )
	{
		// 值不存在
		bIsCircleDataExist = false;
	}

	// 判断返回的数据
	if ( bIsLiveDataExist )
	{
		// 实时行情数据存在
		out_strTpbz = strTpbz;

		if ( bIsCircleDataExist )
		{
			// 交易圈行情存在
			// 比较两者的时间，取最新的那个
			int64_t i64TimeDiff = 0;

			int iRes = TimeStringUtil::CompareTime(strCircle_Hqsj, strLive_Hqsj, i64TimeDiff);
			if ( 0 > iRes )
			{
				string straItoa;
				string strDebug;
				strDebug += "unkown error when call CompareTime(";
				strDebug += strCircle_Hqsj;
				strDebug += ",";
				strDebug += strLive_Hqsj;
				strDebug += ") res=";
				strDebug += sof_string::itostr( iRes, straItoa );
				EzLog::e( ftag, strDebug );

				return -1;
			}

			if ( 0 == i64TimeDiff )
			{
				// 交易圈行情已和实时行情同步
				strOut_Cjsl = strCircle_Cjsl;
				strOut_Cjjg = strCircle_Cjjg;

				out_strHqsj = strCircle_Hqsj;
			}
			else if ( 0 < i64TimeDiff )
			{
				// 实时行情比交易圈行情更新
				strOut_Cjsl = strLive_Cjsl;
				strOut_Cjjg = strLive_Cjjg;

				out_strHqsj = strLive_Hqsj;
			}
			else
			{
				string straItoa;
				string strDebug;

				strDebug += "unkown error when call CompareTime(";
				strDebug += strCircle_Hqsj;
				strDebug += ",";
				strDebug += strLive_Hqsj;
				strDebug += ") TimeDiff=";
				strDebug += sof_string::itostr( i64TimeDiff, straItoa );
				EzLog::e( ftag, strDebug );

				// 采用实时行情
				strOut_Cjsl = strLive_Cjsl;
				strOut_Cjjg = strLive_Cjjg;

				out_strHqsj = strLive_Hqsj;
			}
		}
		else // if(bIsCircleDataExist)
		{
			// 交易圈行情不存在
			// 采用实时行情
			strOut_Cjsl = strLive_Cjsl;
			strOut_Cjjg = strLive_Cjjg;

			out_strHqsj = strLive_Hqsj;
		}
	}
	else // if( bIsLiveDataExist )
	{
		// 实时行情数据不存在
		return -1;
	}

	// 转换字符串为整型

	int iRes = Tgw_StringUtil::String2UInt64_strtoui64( strMaxGain, out_ui64mMaxGain );
	if ( 0 != iRes )
	{
		return -1;
	}

	iRes = Tgw_StringUtil::String2UInt64_strtoui64( strMinFall, out_ui64mMinFall );
	if ( 0 != iRes )
	{
		return -1;
	}

	iRes = Tgw_StringUtil::String2UInt64_strtoui64( strOut_Cjsl, out_ui64Cjsl );
	if ( 0 != iRes )
	{
		return -1;
	}

	simutgw::uint64_t_Money ui64mCjjg = 0;
	iRes = Tgw_StringUtil::String2UInt64_strtoui64( strOut_Cjjg, ui64mCjjg );
	if ( 0 != iRes )
	{
		return -1;
	}

	out_ui64Cjje = ui64mCjjg * out_ui64Cjsl;

	return 0;
}

/*
储存当前储存的交易圈实时行情容量
Return:
0 -- 储存成功
非0 -- 储存失败
*/
int MarketInfoHelper::SetCurrQuotGapByCircle(const string& in_cstrZqdm, const string& in_cstrTradeCircle,
	const uint64_t& in_cui64Cjsl, const uint64_t& in_cui64Cjje, const string& in_cstrHqsj )
{
	static const string ftag( "MarketInfoHelper::SetCurrQuotGapByCircle() " );

	// HMSET KEY_NAME FIELD1 VALUE1 ...FIELDN VALUEN
	// 将行情写入Redis
	// 使用Set写入行情结构体

	std::string strCmdTmp;
	string straItoa;
	std::vector<string> vctCmd;

	vctCmd.push_back( "HMSET" );
	vctCmd.push_back( simutgw::g_Key_AStockQuotTGW_TradeVolume );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::cjsl;
	strCmdTmp += "_";
	strCmdTmp += in_cstrTradeCircle;
	vctCmd.push_back( strCmdTmp );

	sof_string::itostr( in_cui64Cjsl, straItoa );
	vctCmd.push_back( straItoa );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::cjje;
	strCmdTmp += "_";
	strCmdTmp += in_cstrTradeCircle;
	vctCmd.push_back( strCmdTmp );

	sof_string::itostr( in_cui64Cjje, straItoa );
	vctCmd.push_back( straItoa );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::hqsj;
	strCmdTmp += "_";
	strCmdTmp += in_cstrTradeCircle;
	vctCmd.push_back( strCmdTmp );

	vctCmd.push_back( in_cstrHqsj );

	//
	long long llRedisRes = 0;
	std::string strRedisRes;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv( vctCmd, &llRedisRes, &strRedisRes,
		nullptr, nullptr, nullptr );

	bool bIsSuccees = Tgw_RedisHelper::IsRedisCmdSuccess( emPcbCallRes, llRedisRes, strRedisRes );
	if ( !bIsSuccees )
	{
		string strDebug( "Redis执行 HMSET AStockQuotTGW_TradeVolume错误，res=[" );
		strDebug += strRedisRes;
		strDebug += "].";
		EzLog::e( ftag, strDebug );
		return -1;
	}

	return 0;
}

/*
储存当前储存的交易圈实时行情容量
Return:
0 -- 储存成功
非0 -- 储存失败
*/
int MarketInfoHelper::SetCurrQuotGapByCircle( std::shared_ptr<struct simutgw::OrderMessage>& orderMsg,
	const string& in_cstrTradeCircle, const uint64_t& in_cui64Cjsl,
	const uint64_t& in_cui64Cjje, const string& in_cstrHqsj )
{
	static const string ftag( "MarketInfoHelper::SetCurrQuotGapByCircle() " );

	// HMSET KEY_NAME FIELD1 VALUE1 ...FIELDN VALUEN
	// 将行情写入Redis
	// 使用Set写入行情结构体

	std::string strCmdTmp;
	string straItoa;
	std::vector<string> vctCmd;

	vctCmd.push_back( "HMSET" );
	vctCmd.push_back( simutgw::g_Key_AStockQuotTGW_TradeVolume );

	if ( simutgw::AveragePrice == simutgw::g_Quotation_Type )
	{
		// 
		strCmdTmp = orderMsg->strStockID;
		strCmdTmp += "_";
		strCmdTmp += AStockQuotName::cjsl;
		strCmdTmp += "_";
		strCmdTmp += in_cstrTradeCircle;
		vctCmd.push_back( strCmdTmp );

		sof_string::itostr( in_cui64Cjsl, straItoa );
		vctCmd.push_back( straItoa );

		strCmdTmp = orderMsg->strStockID;
		strCmdTmp += "_";
		strCmdTmp += AStockQuotName::cjje;
		strCmdTmp += "_";
		strCmdTmp += in_cstrTradeCircle;
		vctCmd.push_back( strCmdTmp );

		sof_string::itostr( in_cui64Cjje, straItoa );
		vctCmd.push_back( straItoa );
	}
	else if ( simutgw::SellBuyPrice == simutgw::g_Quotation_Type )
	{
		//买一卖一只消耗数量，不改变价格
		strCmdTmp = orderMsg->strStockID;
		strCmdTmp += "_";
		if (0 == orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
		{
			strCmdTmp += AStockQuotName::SSL1;
		}
		else if (0 == orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
		{
			strCmdTmp += AStockQuotName::BSL1;
		}
		strCmdTmp += "_";
		strCmdTmp += in_cstrTradeCircle;
		vctCmd.push_back( strCmdTmp );

		sof_string::itostr( in_cui64Cjsl, straItoa );
		vctCmd.push_back( straItoa );

		strCmdTmp = orderMsg->strStockID;
		strCmdTmp += "_";
		if (0 == orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
		{
			strCmdTmp += AStockQuotName::SJW1;
		}
		else if (0 == orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
		{
			strCmdTmp += AStockQuotName::BJW1;
		}
		strCmdTmp += "_";
		strCmdTmp += in_cstrTradeCircle;
		vctCmd.push_back( strCmdTmp );

		if ( 0 != in_cui64Cjsl )
		{
			uint64_t ui64jg = in_cui64Cjje / in_cui64Cjsl;
			sof_string::itostr( ui64jg, straItoa );
			vctCmd.push_back( straItoa );
		}
		else
		{
			vctCmd.push_back( "0" );
		}
	}
	else if ( simutgw::RecentMatchPrice == simutgw::g_Quotation_Type )
	{
		// 最近成交价只消耗数量，不改变价格
		strCmdTmp = orderMsg->strStockID;
		strCmdTmp += "_";
		strCmdTmp += AStockQuotName::cjsl;
		strCmdTmp += "_";
		strCmdTmp += in_cstrTradeCircle;
		vctCmd.push_back( strCmdTmp );

		sof_string::itostr( in_cui64Cjsl, straItoa );
		vctCmd.push_back( straItoa );

		strCmdTmp = orderMsg->strStockID;
		strCmdTmp += "_";
		strCmdTmp += AStockQuotName::zjjg;
		strCmdTmp += "_";
		strCmdTmp += in_cstrTradeCircle;
		vctCmd.push_back( strCmdTmp );

		if ( 0 != in_cui64Cjsl )
		{
			uint64_t ui64jg = in_cui64Cjje / in_cui64Cjsl;
			sof_string::itostr( ui64jg, straItoa );
			vctCmd.push_back( straItoa );
		}
		else
		{
			vctCmd.push_back( "0" );
		}
	}
	else
	{
		// 其他行情模式
	}

	strCmdTmp = orderMsg->strStockID;
	strCmdTmp += "_";
	strCmdTmp += AStockQuotName::hqsj;
	strCmdTmp += "_";
	strCmdTmp += in_cstrTradeCircle;
	vctCmd.push_back( strCmdTmp );

	vctCmd.push_back( in_cstrHqsj );

	//
	long long llRedisRes = 0;
	std::string strRedisRes;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv( vctCmd, &llRedisRes, &strRedisRes,
		nullptr, nullptr, nullptr );

	bool bIsSuccees = Tgw_RedisHelper::IsRedisCmdSuccess( emPcbCallRes, llRedisRes, strRedisRes );
	if ( !bIsSuccees )
	{
		string strDebug( "Redis执行 HMSET AStockQuotTGW_TradeVolume错误，res=[" );
		strDebug += strRedisRes;
		strDebug += "].";
		EzLog::e( ftag, strDebug );
		return -1;
	}

	return 0;
}

/*
查询当前的静态行情信息
Return:
0 -- 成功
-1 -- 失败
*/
int MarketInfoHelper::GetCurrStaticQuot(const string& in_cstrZqdm, uint64_t& out_ui64Zrsp, string& out_strTPBZ)
{
	static const string ftag("MarketInfoHelper::GetCurrStaticQuot() ");

	string strKey(simutgw::g_Key_AStockStaticQuotation_Prefix);
	string strTradeMarket;
	StockHelper::GetStockTradeMarket(in_cstrZqdm, strTradeMarket);

	if (0 == strTradeMarket.compare(simutgw::TRADE_MARKET_SZ))
	{
		strKey += "sz_";
	}
	else if (0 == strTradeMarket.compare(simutgw::TRADE_MARKET_SH))
	{
		strKey += "sh_";
	}
	strKey += in_cstrZqdm;
	
	std::vector<string> vctCmd;
	vctCmd.push_back("GET");
	vctCmd.push_back(strKey);

	long long llRedisRes = 0;
	std::string strRedisRes;

	std::vector<StgwRedisReply> vectArray;
	size_t stElemSize = 0;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv(vctCmd, &llRedisRes, &strRedisRes,
		nullptr, &vectArray, &stElemSize);
	if (RedisReply_string != emPcbCallRes)
	{
		EzLog::e(ftag, "redis call error! reply not string");

		return -1;

	}

	// 解析json格式静态行情
	rapidjson::Document docData;
	if (docData.Parse<0>(strRedisRes.c_str()).HasParseError() || docData.IsNull())
	{
		//解析FIX消息失败或无message内容
		EzLog::e(ftag, "Parse static hq to Json failed");
		return -1;
	}

	if (!docData.HasMember("ZRSP"))
	{
		EzLog::e(ftag, "static hq has't member[ZRSP]");
		return -1;
	}

	strKey = docData["ZRSP"].GetString();
	Tgw_StringUtil::String2UInt64_strtoui64(strKey, out_ui64Zrsp);

	if (!docData.HasMember("TPBZ"))
	{
		EzLog::e(ftag, "static hq has't member[TPBZ]");
		return -1;
	}

	out_strTPBZ = docData["TPBZ"].GetString();

	return 0;
}