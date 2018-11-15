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
��ȡ��ǰ�����ʵʱ����
Return:
0 -- ��ȡ�ɹ�
��0 -- ��ȡʧ��
*/
int MarketInfoHelper::GetCurrentQuotationByStockId( const string& in_cstrZqdm,
	uint64_t& out_ui64Cjsl, uint64_t& out_ui64Cjje, string& out_strHqsj )
{
	static const string ftag( "MarketInfoHelper::GetCurrentQuotationByStockId() " );

	std::string strCmdTmp;
	string straItoa;
	std::vector<string> vctCmd;

	// ���Լ�����������ж�ȡ	
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

	// ��֪�᷵������
	size_t iSize = vectArray.size();

	if ( 3 > iSize )
	{
		// ֵ������
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
		// ֵ������
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
��ȡ��ǰ����Ľ���Ȧʵʱ��������
�����ǰ����Ȧ����û�У���ȡ�������ģ����������Ҳû�У��򷵻�ʧ��
Return:
0 -- ��ȡ�ɹ�
��0 -- ��ȡʧ��
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
��ȡ��ǰ����Ľ���Ȧʵʱ��������
�����ǰ����Ȧ����û�У���ȡ�������ģ����������Ҳû�У��򷵻�ʧ��

����ξ���

Return:
0 -- ��ȡ�ɹ�
��0 -- ��ȡʧ��
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

	// ���Լ�����������ж�ȡ
	vctCmd.push_back( "HMGET" );
	vctCmd.push_back( simutgw::g_Key_AStockQuotTGW_TradeVolume );

	//
	// ʵʱ������
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
	// ������ʷ����Ȧ����
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

	// ��֪�᷵������
	size_t iSize = vectArray.size();

	// ʵʱ����	
	string strMaxGain;
	string strMinFall;
	string strLive_Cjsl;
	string strLive_Cjje;
	string strLive_Hqsj;
	string strTpbz;
	bool bIsLiveDataExist = true;

	// ����Ȧֵ
	string strCircle_Cjsl;
	string strCircle_Cjje;
	string strCircle_Hqsj;
	bool bIsCircleDataExist = true;

	// ���õ�����ֵ
	string strOut_Cjsl;
	string strOut_Cjje;


	if ( 8 <= iSize )
	{
		// ֵ����
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
		// ֵ������
		bIsLiveDataExist = false;
	}

	if ( 0 == strCircle_Cjsl.length() || 0 == strCircle_Cjje.length() || 0 == strCircle_Hqsj.length() )
	{
		// ֵ������
		bIsCircleDataExist = false;
	}

	// �жϷ��ص�����
	if ( bIsLiveDataExist )
	{
		// ʵʱ�������ݴ���
		out_strTpbz = strTpbz;

		if ( bIsCircleDataExist )
		{
			// ����Ȧ�������
			// �Ƚ����ߵ�ʱ�䣬ȡ���µ��Ǹ�
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
				// ����Ȧ�����Ѻ�ʵʱ����ͬ��
				strOut_Cjsl = strCircle_Cjsl;
				strOut_Cjje = strCircle_Cjje;

				out_strHqsj = strCircle_Hqsj;
			}
			else if ( 0 < i64TimeDiff )
			{
				// ʵʱ����Ƚ���Ȧ�������
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

				// ����ʵʱ����
				strOut_Cjsl = strLive_Cjsl;
				strOut_Cjje = strLive_Cjje;

				out_strHqsj = strLive_Hqsj;
			}
		}
		else // if(bIsCircleDataExist)
		{
			// ����Ȧ���鲻����
			// ����ʵʱ����
			strOut_Cjsl = strLive_Cjsl;
			strOut_Cjje = strLive_Cjje;

			out_strHqsj = strLive_Hqsj;
		}
	}
	else // if( bIsLiveDataExist )
	{
		// ʵʱ�������ݲ�����
		return -1;
	}

	// ת���ַ���Ϊ����

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
��ȡ��ǰ����Ľ���Ȧʵʱ��������
�����ǰ����Ȧ����û�У���ȡ�������ģ����������Ҳû�У��򷵻�ʧ��

��һ��һ

Return:
0 -- ��ȡ�ɹ�
��0 -- ��ȡʧ��
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

	// ���Լ�����������ж�ȡ
	vctCmd.push_back( "HMGET" );
	vctCmd.push_back( simutgw::g_Key_AStockQuotTGW_TradeVolume );

	//
	// ʵʱ������
	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_maxgain";
	vctCmd.push_back( strCmdTmp );

	strCmdTmp = in_cstrZqdm;
	strCmdTmp += "_minfall";
	vctCmd.push_back( strCmdTmp );

	if (0 == in_strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == in_strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
	{
		// ��ȡ������
		strCmdTmp = in_cstrZqdm;
		strCmdTmp += "_SSL1";
		vctCmd.push_back( strCmdTmp );

		strCmdTmp = in_cstrZqdm;
		strCmdTmp += "_SJW1";
		vctCmd.push_back( strCmdTmp );
	}
	else if (0 == in_strSide.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == in_strSide.compare(simutgw::STEPMSG_SIDE_SELL_2))
	{
		// ��ȡ������
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
	// ������ʷ����Ȧ����
	if (0 == in_strSide.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == in_strSide.compare(simutgw::STEPMSG_SIDE_BUY_1))
	{
		// ��ȡ������
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
		// ��ȡ������
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

	// ��֪�᷵������
	size_t iSize = vectArray.size();

	// ʵʱ����	
	string strMaxGain;
	string strMinFall;
	string strLive_Cjsl; // ��һ��һ����
	string strLive_Cjjg; // ��һ��һ�۸�
	string strLive_Hqsj;
	string strTpbz;
	bool bIsLiveDataExist = true;

	// ����Ȧֵ
	string strCircle_Cjsl; // ��һ��һ����
	string strCircle_Cjjg; // ��һ��һ�۸�
	string strCircle_Hqsj;
	bool bIsCircleDataExist = true;

	// ���õ�����ֵ
	string strOut_Cjsl;
	string strOut_Cjjg;

	if ( 8 <= iSize )
	{
		// ֵ����
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
		// ֵ������
		bIsLiveDataExist = false;
	}

	if ( 0 == strCircle_Cjsl.length() || 0 == strCircle_Cjjg.length() || 0 == strCircle_Hqsj.length() )
	{
		// ֵ������
		bIsCircleDataExist = false;
	}

	// �жϷ��ص�����
	if ( bIsLiveDataExist )
	{
		// ʵʱ�������ݴ���
		out_strTpbz = strTpbz;

		if ( bIsCircleDataExist )
		{
			// ����Ȧ�������
			// �Ƚ����ߵ�ʱ�䣬ȡ���µ��Ǹ�
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
				// ����Ȧ�����Ѻ�ʵʱ����ͬ��
				strOut_Cjsl = strCircle_Cjsl;
				strOut_Cjjg = strCircle_Cjjg;

				out_strHqsj = strCircle_Hqsj;
			}
			else if ( 0 < i64TimeDiff )
			{
				// ʵʱ����Ƚ���Ȧ�������
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

				// ����ʵʱ����
				strOut_Cjsl = strLive_Cjsl;
				strOut_Cjjg = strLive_Cjjg;

				out_strHqsj = strLive_Hqsj;
			}
		}
		else // if(bIsCircleDataExist)
		{
			// ����Ȧ���鲻����
			// ����ʵʱ����
			strOut_Cjsl = strLive_Cjsl;
			strOut_Cjjg = strLive_Cjjg;

			out_strHqsj = strLive_Hqsj;
		}
	}
	else // if( bIsLiveDataExist )
	{
		// ʵʱ�������ݲ�����
		return -1;
	}

	// ת���ַ���Ϊ����

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
��ȡ��ǰ����Ľ���Ȧʵʱ��������
�����ǰ����Ȧ����û�У���ȡ�������ģ����������Ҳû�У��򷵻�ʧ��

����ɽ���

Return:
0 -- ��ȡ�ɹ�
��0 -- ��ȡʧ��
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

	// ���Լ�����������ж�ȡ
	vctCmd.push_back( "HMGET" );
	vctCmd.push_back( simutgw::g_Key_AStockQuotTGW_TradeVolume );

	//
	// ʵʱ������
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
	// ������ʷ����Ȧ����
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

	// ��֪�᷵������
	size_t iSize = vectArray.size();

	// ʵʱ����	
	string strMaxGain;
	string strMinFall;
	string strLive_Cjsl;
	string strLive_Cjjg; // ����ɽ���
	string strLive_Hqsj;
	string strTpbz;
	bool bIsLiveDataExist = true;

	// ����Ȧֵ
	string strCircle_Cjsl;
	string strCircle_Cjjg; // ����ɽ���
	string strCircle_Hqsj;
	bool bIsCircleDataExist = true;

	// ���õ�����ֵ
	string strOut_Cjsl;
	string strOut_Cjjg;


	if ( 8 <= iSize )
	{
		// ֵ����
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
		// ֵ������
		bIsLiveDataExist = false;
	}

	if ( 0 == strCircle_Cjsl.length() || 0 == strCircle_Cjjg.length() || 0 == strCircle_Hqsj.length() )
	{
		// ֵ������
		bIsCircleDataExist = false;
	}

	// �жϷ��ص�����
	if ( bIsLiveDataExist )
	{
		// ʵʱ�������ݴ���
		out_strTpbz = strTpbz;

		if ( bIsCircleDataExist )
		{
			// ����Ȧ�������
			// �Ƚ����ߵ�ʱ�䣬ȡ���µ��Ǹ�
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
				// ����Ȧ�����Ѻ�ʵʱ����ͬ��
				strOut_Cjsl = strCircle_Cjsl;
				strOut_Cjjg = strCircle_Cjjg;

				out_strHqsj = strCircle_Hqsj;
			}
			else if ( 0 < i64TimeDiff )
			{
				// ʵʱ����Ƚ���Ȧ�������
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

				// ����ʵʱ����
				strOut_Cjsl = strLive_Cjsl;
				strOut_Cjjg = strLive_Cjjg;

				out_strHqsj = strLive_Hqsj;
			}
		}
		else // if(bIsCircleDataExist)
		{
			// ����Ȧ���鲻����
			// ����ʵʱ����
			strOut_Cjsl = strLive_Cjsl;
			strOut_Cjjg = strLive_Cjjg;

			out_strHqsj = strLive_Hqsj;
		}
	}
	else // if( bIsLiveDataExist )
	{
		// ʵʱ�������ݲ�����
		return -1;
	}

	// ת���ַ���Ϊ����

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
���浱ǰ����Ľ���Ȧʵʱ��������
Return:
0 -- ����ɹ�
��0 -- ����ʧ��
*/
int MarketInfoHelper::SetCurrQuotGapByCircle(const string& in_cstrZqdm, const string& in_cstrTradeCircle,
	const uint64_t& in_cui64Cjsl, const uint64_t& in_cui64Cjje, const string& in_cstrHqsj )
{
	static const string ftag( "MarketInfoHelper::SetCurrQuotGapByCircle() " );

	// HMSET KEY_NAME FIELD1 VALUE1 ...FIELDN VALUEN
	// ������д��Redis
	// ʹ��Setд������ṹ��

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
		string strDebug( "Redisִ�� HMSET AStockQuotTGW_TradeVolume����res=[" );
		strDebug += strRedisRes;
		strDebug += "].";
		EzLog::e( ftag, strDebug );
		return -1;
	}

	return 0;
}

/*
���浱ǰ����Ľ���Ȧʵʱ��������
Return:
0 -- ����ɹ�
��0 -- ����ʧ��
*/
int MarketInfoHelper::SetCurrQuotGapByCircle( std::shared_ptr<struct simutgw::OrderMessage>& orderMsg,
	const string& in_cstrTradeCircle, const uint64_t& in_cui64Cjsl,
	const uint64_t& in_cui64Cjje, const string& in_cstrHqsj )
{
	static const string ftag( "MarketInfoHelper::SetCurrQuotGapByCircle() " );

	// HMSET KEY_NAME FIELD1 VALUE1 ...FIELDN VALUEN
	// ������д��Redis
	// ʹ��Setд������ṹ��

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
		//��һ��һֻ�������������ı�۸�
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
		// ����ɽ���ֻ�������������ı�۸�
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
		// ��������ģʽ
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
		string strDebug( "Redisִ�� HMSET AStockQuotTGW_TradeVolume����res=[" );
		strDebug += strRedisRes;
		strDebug += "].";
		EzLog::e( ftag, strDebug );
		return -1;
	}

	return 0;
}

/*
��ѯ��ǰ�ľ�̬������Ϣ
Return:
0 -- �ɹ�
-1 -- ʧ��
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

	// ����json��ʽ��̬����
	rapidjson::Document docData;
	if (docData.Parse<0>(strRedisRes.c_str()).HasParseError() || docData.IsNull())
	{
		//����FIX��Ϣʧ�ܻ���message����
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