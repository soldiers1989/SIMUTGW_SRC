#include "ETF_TradeVolume.h"

/*
冻结申购交易额

Return:
0 -- 未超过最大限制额度，冻结成功
-1 -- 冻结失败
*/
int ETF_TradeVolume::FrozeCreation( const uint64_t in_num,
	const std::string& in_sSecurityId, const std::string& in_sCustId,
	std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf )
{
	static const std::string ftag( "ETF_TradeVolume::FrozeCreation() " );

	int iRes = 0;
	std::map<std::string, ETF_TradeVolume_Basket>::iterator it
		= m_etfsTv.find( in_sSecurityId );
	std::pair<std::map<std::string, ETF_TradeVolume_Basket>::iterator, bool> insertRet;

	if ( m_etfsTv.end() == it )
	{
		// 无该ETF的交易记录
		insertRet = m_etfsTv.insert(
			std::pair<std::string, ETF_TradeVolume_Basket>(
			in_sCustId, ETF_TradeVolume_Basket
			( in_ptrEtf->ui64CreationLimit, in_ptrEtf->ui64RedemptionLimit,
			in_ptrEtf->ui64CreationLimitPerUser, in_ptrEtf->ui64RedemptionLimitPerUser,
			in_ptrEtf->ui64NetCreationLimit, in_ptrEtf->ui64NetRedemptionLimit,
			in_ptrEtf->ui64NetCreationLimitPerUser, in_ptrEtf->ui64NetRedemptionLimitPerUser )
			) );
		if ( !insertRet.second )
		{
			// insert fail，有问题
			std::string sDebug( "map insert failed" );

			EzLog::e( ftag, sDebug );

			return -1;
		}

		it = insertRet.first;
	}

	iRes = it->second.FrozeCreation( in_num, in_sCustId );
	if ( 0 != iRes )
	{
		return -1;
	}

	return 0;
}

/*
解除冻结申购交易额

Return:
0 -- 解除冻结成功
-1 -- 冻结失败
*/
int ETF_TradeVolume::DefrozeCreation( const uint64_t in_num,
	const std::string& in_sSecurityId, const std::string& in_sCustId )
{
	static const std::string ftag( "ETF_TradeVolume::DefrozeCreation() " );

	int iRes = 0;
	std::map<std::string, ETF_TradeVolume_Basket>::iterator it
		= m_etfsTv.find( in_sSecurityId );

	if ( m_etfsTv.end() == it )
	{
		// 无该ETF的交易记录
		std::pair<std::map<std::string, ETF_TradeVolume_Basket>::iterator, bool>
			insertRet = m_etfsTv.insert(
			std::pair<std::string, ETF_TradeVolume_Basket>(
			in_sCustId, ETF_TradeVolume_Basket() ) );
		if ( !insertRet.second )
		{
			// insert fail，有问题
			std::string sDebug( "map insert failed" );

			EzLog::e( ftag, sDebug );

			return -1;
		}

		it = insertRet.first;
	}

	iRes = it->second.DefrozeCreation( in_num, in_sCustId );
	if ( 0 != iRes )
	{
		return -1;
	}

	return 0;
}

/*
确认申购交易额

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int ETF_TradeVolume::ConfirmCreation( const uint64_t in_num,
	const std::string& in_sSecurityId, const std::string& in_sCustId )
{
	static const std::string ftag( "ETF_TradeVolume::ConfirmCreation() " );

	int iRes = 0;
	std::map<std::string, ETF_TradeVolume_Basket>::iterator it
		= m_etfsTv.find( in_sSecurityId );

	if ( m_etfsTv.end() == it )
	{
		// 无该ETF的交易记录
		std::pair<std::map<std::string, ETF_TradeVolume_Basket>::iterator, bool>
			insertRet = m_etfsTv.insert(
			std::pair<std::string, ETF_TradeVolume_Basket>(
			in_sCustId, ETF_TradeVolume_Basket() ) );
		if ( !insertRet.second )
		{
			// insert fail，有问题
			std::string sDebug( "map insert failed" );

			EzLog::e( ftag, sDebug );

			return -1;
		}

		it = insertRet.first;
	}

	iRes = it->second.ConfirmCreation( in_num, in_sCustId );
	if ( 0 != iRes )
	{
		return -1;
	}

	return 0;
}

/*
冻结赎回交易额

Return:
0 -- 未超过最大限制额度，冻结成功
-1 -- 冻结失败
*/
int ETF_TradeVolume::FrozeRedemption( const uint64_t in_num,
	const std::string& in_sSecurityId, const std::string& in_sCustId,
	std::shared_ptr<struct simutgw::SzETF>& in_ptrEtf )
{
	static const std::string ftag( "ETF_TradeVolume::FrozeRedemption() " );

	int iRes = 0;
	std::map<std::string, ETF_TradeVolume_Basket>::iterator it
		= m_etfsTv.find( in_sSecurityId );
	std::pair<std::map<std::string, ETF_TradeVolume_Basket>::iterator, bool>
		insertRet;

	if ( m_etfsTv.end() == it )
	{
		// 无该ETF的交易记录
		insertRet = m_etfsTv.insert(
			std::pair<std::string, ETF_TradeVolume_Basket>(
			in_sCustId, ETF_TradeVolume_Basket( in_ptrEtf->ui64CreationLimit, in_ptrEtf->ui64RedemptionLimit,
			in_ptrEtf->ui64CreationLimitPerUser, in_ptrEtf->ui64RedemptionLimitPerUser,
			in_ptrEtf->ui64NetCreationLimit, in_ptrEtf->ui64NetRedemptionLimit,
			in_ptrEtf->ui64NetCreationLimitPerUser, in_ptrEtf->ui64NetRedemptionLimitPerUser )
			) );
		if ( !insertRet.second )
		{
			// insert fail，有问题
			std::string sDebug( "map insert failed" );

			EzLog::e( ftag, sDebug );

			return -1;
		}

		it = insertRet.first;
	}

	iRes = it->second.FrozeRedemption( in_num, in_sCustId );
	if ( 0 != iRes )
	{
		return -1;
	}

	return 0;
}


/*
解除冻结赎回交易额

Return:
0 -- 解除冻结成功
-1 -- 解除冻结失败
*/
int ETF_TradeVolume::DefrozeRedemption( const uint64_t in_num,
	const std::string& in_sSecurityId, const std::string& in_sCustId )

{
	static const std::string ftag( "ETF_TradeVolume::DefrozeRedemption() " );

	int iRes = 0;
	std::map<std::string, ETF_TradeVolume_Basket>::iterator it
		= m_etfsTv.find( in_sSecurityId );

	if ( m_etfsTv.end() == it )
	{
		// 无该ETF的交易记录
		std::pair<std::map<std::string, ETF_TradeVolume_Basket>::iterator, bool>
			insertRet = m_etfsTv.insert(
			std::pair<std::string, ETF_TradeVolume_Basket>(
			in_sCustId, ETF_TradeVolume_Basket() ) );
		if ( !insertRet.second )
		{
			// insert fail，有问题
			std::string sDebug( "map insert failed" );

			EzLog::e( ftag, sDebug );

			return -1;
		}

		it = insertRet.first;
	}

	iRes = it->second.DefrozeRedemption( in_num, in_sCustId );
	if ( 0 != iRes )
	{
		return -1;
	}

	return 0;
}

/*
确认赎回交易额

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int ETF_TradeVolume::ConfirmRedemption( const uint64_t in_num,
	const std::string& in_sSecurityId, const std::string& in_sCustId )

{
	static const std::string ftag( "ETF_TradeVolume::ConfirmRedemption() " );

	int iRes = 0;
	std::map<std::string, ETF_TradeVolume_Basket>::iterator it
		= m_etfsTv.find( in_sSecurityId );

	if ( m_etfsTv.end() == it )
	{
		// 无该ETF的交易记录
		std::pair<std::map<std::string, ETF_TradeVolume_Basket>::iterator, bool>
			insertRet = m_etfsTv.insert(
			std::pair<std::string, ETF_TradeVolume_Basket>(
			in_sCustId, ETF_TradeVolume_Basket() ) );
		if ( !insertRet.second )
		{
			// insert fail，有问题
			std::string sDebug( "map insert failed" );

			EzLog::e( ftag, sDebug );

			return -1;
		}

		it = insertRet.first;
	}

	iRes = it->second.ConfirmRedemption( in_num, in_sCustId );
	if ( 0 != iRes )
	{
		return -1;
	}

	return 0;
}
