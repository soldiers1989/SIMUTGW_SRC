#include "ETF_TradeVolume_Basket.h"

/*
冻结申购交易额

Return:
0 -- 未超过最大限制额度，冻结成功
-1 -- 冻结失败
*/
int ETF_TradeVolume_Basket::FrozeCreation( const uint64_t in_num, const std::string& in_sCustId )
{
	static const std::string ftag( "ETF_TradeVolume_Basket::FrozeCreation() " );

	boost::unique_lock<boost::mutex> Locker( m_mutex );

	// param check
	if ( 0 == in_num || in_sCustId.empty() )
	{
		return 0;
	}

	int iRes = 0;
	// 单用户用iterator
	std::map<std::string, FrozeVolume>::iterator it_CreationLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetCreationLimitPerUser;
	std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//累计申购总额限制 CreationLimit N18(2) 当天累计可申购的基金份额上限，为 0 表示没有限制，目前只能为整数
	iRes = m_CreationLimit.Froze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//单个账户累计申购总额限制 CreationLimitPerUser N18(2) 单个证券账户当天累计可申购的基金份额上限，
	//为 0 表示没有限制，目前只能为整数单个账户累计赎回总额限制
	if ( 0 != m_ui64CreationLimitPerUser )
	{
		it_CreationLimitPerUser = m_CreationLimitPerUser.find( in_sCustId );
		if ( m_CreationLimitPerUser.end() == it_CreationLimitPerUser )
		{
			// 无该用户的交易记录
			insertRet = m_CreationLimitPerUser.insert(
				std::pair<std::string, FrozeVolume>(
				in_sCustId, FrozeVolume( m_ui64CreationLimitPerUser ) ) );
			if ( !insertRet.second )
			{
				// insert fail，有问题
				std::string sDebug( "map insert failed" );

				EzLog::e( ftag, sDebug );

				return -1;
			}

			it_CreationLimitPerUser = insertRet.first;
		}

		iRes = it_CreationLimitPerUser->second.Froze( in_num );
		if ( 0 != iRes )
		{
			// Roll back
			m_CreationLimit.Defroze( in_num );

			return -1;
		}
	}

	//净申购总额限制 NetCreationLimit N18(2) 当天净申购的基金份额上限，为 0表示没有限制，目前只能为整数
	m_NetCreationLimit.Froze( in_num );
	if ( 0 != iRes )
	{
		// Roll back
		m_CreationLimit.Defroze( in_num );
		if ( 0 != m_ui64CreationLimitPerUser )
		{
			it_CreationLimitPerUser->second.Defroze( in_num );
		}

		return -1;
	}

	//单个账户净申购总额限制 NetCreationLimitPerUser N18(2) 单个证券账户当天净申购的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	if ( 0 != m_ui64NetCreationLimitPerUser )
	{
		it_NetCreationLimitPerUser = m_NetCreationLimitPerUser.find( in_sCustId );
		if ( m_NetCreationLimitPerUser.end() == it_NetCreationLimitPerUser )
		{
			// 无该用户的交易记录
			insertRet = m_NetCreationLimitPerUser.insert(
				std::pair<std::string, FrozeVolume>(
				in_sCustId, FrozeVolume( m_ui64NetCreationLimitPerUser ) ) );
			if ( !insertRet.second )
			{
				// insert fail，有问题
				std::string sDebug( "map insert failed" );

				EzLog::e( ftag, sDebug );

				return -1;
			}

			it_NetCreationLimitPerUser = insertRet.first;
		}

		iRes = it_NetCreationLimitPerUser->second.Froze( in_num );
		if ( 0 != iRes )
		{
			// Roll back
			m_CreationLimit.Defroze( in_num );
			if ( 0 != m_ui64CreationLimitPerUser )
			{
				it_CreationLimitPerUser->second.Defroze( in_num );
			}
			m_NetCreationLimit.Defroze( in_num );

			return -1;
		}
	}

	return 0;
}

/*
解除冻结申购交易额

Return:
0 -- 解除冻结成功
-1 -- 解除冻结失败
*/
int ETF_TradeVolume_Basket::DefrozeCreation( const uint64_t in_num, const std::string& in_sCustId )
{
	static const std::string ftag( "ETF_TradeVolume_Basket::DefrozeCreation() " );

	boost::unique_lock<boost::mutex> Locker( m_mutex );

	// param check
	if ( 0 == in_num || in_sCustId.empty() )
	{
		return 0;
	}

	int iRes = 0;
	// 单用户用iterator
	std::map<std::string, FrozeVolume>::iterator it_CreationLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetCreationLimitPerUser;
	//std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//累计申购总额限制 CreationLimit N18(2) 当天累计可申购的基金份额上限，为 0 表示没有限制，目前只能为整数
	iRes = m_CreationLimit.Defroze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//单个账户累计申购总额限制 CreationLimitPerUser N18(2) 单个证券账户当天累计可申购的基金份额上限，
	//为 0 表示没有限制，目前只能为整数单个账户累计赎回总额限制
	if ( 0 != m_ui64CreationLimitPerUser )
	{
		it_CreationLimitPerUser = m_CreationLimitPerUser.find( in_sCustId );
		if ( m_CreationLimitPerUser.end() == it_CreationLimitPerUser )
		{
			// 无该用户的交易记录，有问题
			std::string sDebug( "CreationLimitPerUser not find CustId=[" );
			sDebug += in_sCustId;
			sDebug += "]";

			EzLog::e( ftag, sDebug );

			return -1;
		}

		iRes = it_CreationLimitPerUser->second.Defroze( in_num );
		if ( 0 != iRes )
		{
			return -1;
		}

	}

	//净申购总额限制 NetCreationLimit N18(2) 当天净申购的基金份额上限，为 0表示没有限制，目前只能为整数
	m_NetCreationLimit.Defroze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//单个账户净申购总额限制 NetCreationLimitPerUser N18(2) 单个证券账户当天净申购的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	if ( 0 != m_ui64NetCreationLimitPerUser )
	{
		it_NetCreationLimitPerUser = m_NetCreationLimitPerUser.find( in_sCustId );
		if ( m_NetCreationLimitPerUser.end() == it_NetCreationLimitPerUser )
		{
			// 无该用户的交易记录，有问题
			std::string sDebug( "NetCreationLimitPerUser not find CustId=[" );
			sDebug += in_sCustId;
			sDebug += "]";

			EzLog::e( ftag, sDebug );

			return -1;
		}

		iRes = it_NetCreationLimitPerUser->second.Defroze( in_num );
		if ( 0 != iRes )
		{
			return -1;
		}
	}

	return 0;
}

/*
确认申购交易额

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int ETF_TradeVolume_Basket::ConfirmCreation( const uint64_t in_num, const std::string& in_sCustId )
{
	static const std::string ftag( "ETF_TradeVolume_Basket::ConfirmCreation() " );

	boost::unique_lock<boost::mutex> Locker( m_mutex );

	// param check
	if ( 0 == in_num || in_sCustId.empty() )
	{
		return 0;
	}

	int iRes = 0;
	// 单用户用iterator
	std::map<std::string, FrozeVolume>::iterator it_CreationLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetCreationLimitPerUser;
	//std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//累计申购总额限制 CreationLimit N18(2) 当天累计可申购的基金份额上限，为 0 表示没有限制，目前只能为整数
	iRes = m_CreationLimit.Confirm( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//单个账户累计申购总额限制 CreationLimitPerUser N18(2) 单个证券账户当天累计可申购的基金份额上限，
	//为 0 表示没有限制，目前只能为整数单个账户累计赎回总额限制
	if ( 0 != m_ui64CreationLimitPerUser )
	{
		it_CreationLimitPerUser = m_CreationLimitPerUser.find( in_sCustId );
		if ( m_CreationLimitPerUser.end() == it_CreationLimitPerUser )
		{
			// 无该用户的交易记录，有问题
			std::string sDebug( "CreationLimitPerUser not find CustId=[" );
			sDebug += in_sCustId;
			sDebug += "]";

			EzLog::e( ftag, sDebug );

			return -1;
		}

		iRes = it_CreationLimitPerUser->second.Confirm( in_num );
		if ( 0 != iRes )
		{
			return -1;
		}

	}

	//净申购总额限制 NetCreationLimit N18(2) 当天净申购的基金份额上限，为 0表示没有限制，目前只能为整数
	m_NetCreationLimit.Confirm( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//单个账户净申购总额限制 NetCreationLimitPerUser N18(2) 单个证券账户当天净申购的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	if ( 0 != m_ui64NetCreationLimitPerUser )
	{
		it_NetCreationLimitPerUser = m_NetCreationLimitPerUser.find( in_sCustId );
		if ( m_NetCreationLimitPerUser.end() == it_NetCreationLimitPerUser )
		{
			// 无该用户的交易记录，有问题
			std::string sDebug( "NetCreationLimitPerUser not find CustId=[" );
			sDebug += in_sCustId;
			sDebug += "]";

			EzLog::e( ftag, sDebug );

			return -1;
		}

		iRes = it_NetCreationLimitPerUser->second.Confirm( in_num );
		if ( 0 != iRes )
		{
			return -1;
		}
	}

	return 0;
}


/*
冻结赎回交易额

Return:
0 -- 未超过最大限制额度，冻结成功
-1 -- 冻结失败
*/
int ETF_TradeVolume_Basket::FrozeRedemption( const uint64_t in_num, const std::string& in_sCustId )
{
	static const std::string ftag( "ETF_TradeVolume_Basket::FrozeRedemption() " );

	boost::unique_lock<boost::mutex> Locker( m_mutex );

	// param check
	if ( 0 == in_num || in_sCustId.empty() )
	{
		return 0;
	}

	int iRes = 0;
	// 单用户用iterator
	std::map<std::string, FrozeVolume>::iterator it_RedemptionLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetRedemptionLimitPerUser;
	std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//累计赎回总额限制 RedemptionLimit N18(2) 当天累计可赎回的基金份额上限，为 0 表示没有限制， 目前只能为整数
	iRes = m_RedemptionLimit.Froze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//单个账户累计赎回总额限制 RedemptionLimitPerUser N18(2) 单个证券账户当天累计可赎回的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	if ( 0 != m_ui64RedemptionLimitPerUser )
	{
		it_RedemptionLimitPerUser = m_RedemptionLimitPerUser.find( in_sCustId );
		if ( m_RedemptionLimitPerUser.end() == it_RedemptionLimitPerUser )
		{
			// 无该用户的交易记录
			insertRet = m_RedemptionLimitPerUser.insert(
				std::pair<std::string, FrozeVolume>(
				in_sCustId, FrozeVolume( m_ui64RedemptionLimitPerUser ) ) );
			if ( !insertRet.second )
			{
				// insert fail，有问题
				std::string sDebug( "map insert failed" );

				EzLog::e( ftag, sDebug );

				return -1;
			}

			it_RedemptionLimitPerUser = insertRet.first;
		}

		iRes = it_RedemptionLimitPerUser->second.Froze( in_num );
		if ( 0 != iRes )
		{
			// Roll back
			m_RedemptionLimit.Defroze( in_num );

			return -1;
		}
	}

	//净赎回总额限制 NetRedemptionLimit N18(2) 当天净赎回的基金份额上限，为 0表示没有限制，目前只能为整数
	m_NetRedemptionLimit.Froze( in_num );
	if ( 0 != iRes )
	{
		// Roll back
		m_RedemptionLimit.Defroze( in_num );
		if ( 0 != m_ui64RedemptionLimitPerUser )
		{
			it_RedemptionLimitPerUser->second.Defroze( in_num );
		}

		return -1;
	}

	//单个账户净赎回总额限制 NetRedemptionLimitPerUser N18(2) 单个证券账户当天净赎回的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	if ( 0 != m_ui64NetRedemptionLimitPerUser )
	{
		it_NetRedemptionLimitPerUser = m_NetRedemptionLimitPerUser.find( in_sCustId );
		if ( m_NetRedemptionLimitPerUser.end() == it_NetRedemptionLimitPerUser )
		{
			// 无该用户的交易记录
			insertRet = m_NetRedemptionLimitPerUser.insert(
				std::pair<std::string, FrozeVolume>(
				in_sCustId, FrozeVolume( m_ui64NetRedemptionLimitPerUser ) ) );
			if ( !insertRet.second )
			{
				// insert fail，有问题
				std::string sDebug( "map insert failed" );

				EzLog::e( ftag, sDebug );

				return -1;
			}

			it_NetRedemptionLimitPerUser = insertRet.first;
		}

		iRes = it_NetRedemptionLimitPerUser->second.Froze( in_num );
		if ( 0 != iRes )
		{
			// Roll back
			m_RedemptionLimit.Defroze( in_num );
			if ( 0 != m_ui64RedemptionLimitPerUser )
			{
				it_RedemptionLimitPerUser->second.Defroze( in_num );
			}
			m_NetRedemptionLimit.Defroze( in_num );

			return -1;
		}
	}

	return 0;
}

/*
解除冻结赎回交易额

Return:
0 -- 解除冻结成功
-1 -- 冻结失败
*/
int ETF_TradeVolume_Basket::DefrozeRedemption( const uint64_t in_num, const std::string& in_sCustId )
{
	static const std::string ftag( "ETF_TradeVolume_Basket::DefrozeRedemption() " );

	boost::unique_lock<boost::mutex> Locker( m_mutex );

	// param check
	if ( 0 == in_num || in_sCustId.empty() )
	{
		return 0;
	}

	int iRes = 0;
	// 单用户用iterator
	std::map<std::string, FrozeVolume>::iterator it_RedemptionLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetRedemptionLimitPerUser;
	//std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//累计赎回总额限制 RedemptionLimit N18(2) 当天累计可赎回的基金份额上限，为 0 表示没有限制， 目前只能为整数
	iRes = m_RedemptionLimit.Defroze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//单个账户累计赎回总额限制 RedemptionLimitPerUser N18(2) 单个证券账户当天累计可赎回的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	if ( 0 != m_ui64RedemptionLimitPerUser )
	{
		it_RedemptionLimitPerUser = m_RedemptionLimitPerUser.find( in_sCustId );
		if ( m_RedemptionLimitPerUser.end() == it_RedemptionLimitPerUser )
		{
			// 无该用户的交易记录，有问题
			std::string sDebug( "RedemptionLimitPerUser not find CustId=[" );
			sDebug += in_sCustId;
			sDebug += "]";

			EzLog::e( ftag, sDebug );

			return -1;
		}

		iRes = it_RedemptionLimitPerUser->second.Defroze( in_num );
		if ( 0 != iRes )
		{
			return -1;
		}
	}

	//净赎回总额限制 NetRedemptionLimit N18(2) 当天净赎回的基金份额上限，为 0表示没有限制，目前只能为整数
	m_NetRedemptionLimit.Defroze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//单个账户净赎回总额限制 NetRedemptionLimitPerUser N18(2) 单个证券账户当天净赎回的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	if ( 0 != m_ui64NetRedemptionLimitPerUser )
	{
		it_NetRedemptionLimitPerUser = m_NetRedemptionLimitPerUser.find( in_sCustId );
		if ( m_NetRedemptionLimitPerUser.end() == it_NetRedemptionLimitPerUser )
		{
			// 无该用户的交易记录，有问题
			std::string sDebug( "NetRedemptionLimitPerUser not find CustId=[" );
			sDebug += in_sCustId;
			sDebug += "]";

			EzLog::e( ftag, sDebug );

			return -1;
		}

		iRes = it_NetRedemptionLimitPerUser->second.Defroze( in_num );
		if ( 0 != iRes )
		{
			return -1;
		}
	}

	return 0;
}

/*
确认赎回交易额

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int ETF_TradeVolume_Basket::ConfirmRedemption( const uint64_t in_num, const std::string& in_sCustId )
{
	static const std::string ftag( "ETF_TradeVolume_Basket::ConfirmRedemption() " );

	boost::unique_lock<boost::mutex> Locker( m_mutex );

	// param check
	if ( 0 == in_num || in_sCustId.empty() )
	{
		return 0;
	}

	int iRes = 0;
	// 单用户用iterator
	std::map<std::string, FrozeVolume>::iterator it_RedemptionLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetRedemptionLimitPerUser;
	//std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//累计赎回总额限制 RedemptionLimit N18(2) 当天累计可赎回的基金份额上限，为 0 表示没有限制， 目前只能为整数
	iRes = m_RedemptionLimit.Confirm( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//单个账户累计赎回总额限制 RedemptionLimitPerUser N18(2) 单个证券账户当天累计可赎回的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	if ( 0 != m_ui64RedemptionLimitPerUser )
	{
		it_RedemptionLimitPerUser = m_RedemptionLimitPerUser.find( in_sCustId );
		if ( m_RedemptionLimitPerUser.end() == it_RedemptionLimitPerUser )
		{
			// 无该用户的交易记录，有问题
			std::string sDebug( "RedemptionLimitPerUser not find CustId=[" );
			sDebug += in_sCustId;
			sDebug += "]";

			EzLog::e( ftag, sDebug );

			return -1;
		}

		iRes = it_RedemptionLimitPerUser->second.Confirm( in_num );
		if ( 0 != iRes )
		{
			return -1;
		}
	}

	//净赎回总额限制 NetRedemptionLimit N18(2) 当天净赎回的基金份额上限，为 0表示没有限制，目前只能为整数
	m_NetRedemptionLimit.Confirm( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//单个账户净赎回总额限制 NetRedemptionLimitPerUser N18(2) 单个证券账户当天净赎回的基金份额上限，
	//为 0 表示没有限制，目前只能为整数
	if ( 0 != m_ui64NetRedemptionLimitPerUser )
	{
		it_NetRedemptionLimitPerUser = m_NetRedemptionLimitPerUser.find( in_sCustId );
		if ( m_NetRedemptionLimitPerUser.end() == it_NetRedemptionLimitPerUser )
		{
			// 无该用户的交易记录，有问题
			std::string sDebug( "NetRedemptionLimitPerUser not find CustId=[" );
			sDebug += in_sCustId;
			sDebug += "]";

			EzLog::e( ftag, sDebug );

			return -1;
		}

		iRes = it_NetRedemptionLimitPerUser->second.Confirm( in_num );
		if ( 0 != iRes )
		{
			return -1;
		}
	}

	return 0;
}