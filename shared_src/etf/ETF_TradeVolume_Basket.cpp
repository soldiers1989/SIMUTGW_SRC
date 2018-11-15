#include "ETF_TradeVolume_Basket.h"

/*
�����깺���׶�

Return:
0 -- δ����������ƶ�ȣ�����ɹ�
-1 -- ����ʧ��
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
	// ���û���iterator
	std::map<std::string, FrozeVolume>::iterator it_CreationLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetCreationLimitPerUser;
	std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//�ۼ��깺�ܶ����� CreationLimit N18(2) �����ۼƿ��깺�Ļ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	iRes = m_CreationLimit.Froze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//�����˻��ۼ��깺�ܶ����� CreationLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ��깺�Ļ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ���������˻��ۼ�����ܶ�����
	if ( 0 != m_ui64CreationLimitPerUser )
	{
		it_CreationLimitPerUser = m_CreationLimitPerUser.find( in_sCustId );
		if ( m_CreationLimitPerUser.end() == it_CreationLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼
			insertRet = m_CreationLimitPerUser.insert(
				std::pair<std::string, FrozeVolume>(
				in_sCustId, FrozeVolume( m_ui64CreationLimitPerUser ) ) );
			if ( !insertRet.second )
			{
				// insert fail��������
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

	//���깺�ܶ����� NetCreationLimit N18(2) ���쾻�깺�Ļ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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

	//�����˻����깺�ܶ����� NetCreationLimitPerUser N18(2) ����֤ȯ�˻����쾻�깺�Ļ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	if ( 0 != m_ui64NetCreationLimitPerUser )
	{
		it_NetCreationLimitPerUser = m_NetCreationLimitPerUser.find( in_sCustId );
		if ( m_NetCreationLimitPerUser.end() == it_NetCreationLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼
			insertRet = m_NetCreationLimitPerUser.insert(
				std::pair<std::string, FrozeVolume>(
				in_sCustId, FrozeVolume( m_ui64NetCreationLimitPerUser ) ) );
			if ( !insertRet.second )
			{
				// insert fail��������
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
��������깺���׶�

Return:
0 -- �������ɹ�
-1 -- �������ʧ��
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
	// ���û���iterator
	std::map<std::string, FrozeVolume>::iterator it_CreationLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetCreationLimitPerUser;
	//std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//�ۼ��깺�ܶ����� CreationLimit N18(2) �����ۼƿ��깺�Ļ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	iRes = m_CreationLimit.Defroze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//�����˻��ۼ��깺�ܶ����� CreationLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ��깺�Ļ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ���������˻��ۼ�����ܶ�����
	if ( 0 != m_ui64CreationLimitPerUser )
	{
		it_CreationLimitPerUser = m_CreationLimitPerUser.find( in_sCustId );
		if ( m_CreationLimitPerUser.end() == it_CreationLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼��������
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

	//���깺�ܶ����� NetCreationLimit N18(2) ���쾻�깺�Ļ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	m_NetCreationLimit.Defroze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//�����˻����깺�ܶ����� NetCreationLimitPerUser N18(2) ����֤ȯ�˻����쾻�깺�Ļ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	if ( 0 != m_ui64NetCreationLimitPerUser )
	{
		it_NetCreationLimitPerUser = m_NetCreationLimitPerUser.find( in_sCustId );
		if ( m_NetCreationLimitPerUser.end() == it_NetCreationLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼��������
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
ȷ���깺���׶�

Return:
0 -- ȷ�ϳɹ�
-1 -- ȷ��ʧ��
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
	// ���û���iterator
	std::map<std::string, FrozeVolume>::iterator it_CreationLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetCreationLimitPerUser;
	//std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//�ۼ��깺�ܶ����� CreationLimit N18(2) �����ۼƿ��깺�Ļ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	iRes = m_CreationLimit.Confirm( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//�����˻��ۼ��깺�ܶ����� CreationLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ��깺�Ļ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ���������˻��ۼ�����ܶ�����
	if ( 0 != m_ui64CreationLimitPerUser )
	{
		it_CreationLimitPerUser = m_CreationLimitPerUser.find( in_sCustId );
		if ( m_CreationLimitPerUser.end() == it_CreationLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼��������
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

	//���깺�ܶ����� NetCreationLimit N18(2) ���쾻�깺�Ļ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	m_NetCreationLimit.Confirm( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//�����˻����깺�ܶ����� NetCreationLimitPerUser N18(2) ����֤ȯ�˻����쾻�깺�Ļ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	if ( 0 != m_ui64NetCreationLimitPerUser )
	{
		it_NetCreationLimitPerUser = m_NetCreationLimitPerUser.find( in_sCustId );
		if ( m_NetCreationLimitPerUser.end() == it_NetCreationLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼��������
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
������ؽ��׶�

Return:
0 -- δ����������ƶ�ȣ�����ɹ�
-1 -- ����ʧ��
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
	// ���û���iterator
	std::map<std::string, FrozeVolume>::iterator it_RedemptionLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetRedemptionLimitPerUser;
	std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//�ۼ�����ܶ����� RedemptionLimit N18(2) �����ۼƿ���صĻ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ� Ŀǰֻ��Ϊ����
	iRes = m_RedemptionLimit.Froze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//�����˻��ۼ�����ܶ����� RedemptionLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ���صĻ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	if ( 0 != m_ui64RedemptionLimitPerUser )
	{
		it_RedemptionLimitPerUser = m_RedemptionLimitPerUser.find( in_sCustId );
		if ( m_RedemptionLimitPerUser.end() == it_RedemptionLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼
			insertRet = m_RedemptionLimitPerUser.insert(
				std::pair<std::string, FrozeVolume>(
				in_sCustId, FrozeVolume( m_ui64RedemptionLimitPerUser ) ) );
			if ( !insertRet.second )
			{
				// insert fail��������
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

	//������ܶ����� NetRedemptionLimit N18(2) ���쾻��صĻ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
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

	//�����˻�������ܶ����� NetRedemptionLimitPerUser N18(2) ����֤ȯ�˻����쾻��صĻ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	if ( 0 != m_ui64NetRedemptionLimitPerUser )
	{
		it_NetRedemptionLimitPerUser = m_NetRedemptionLimitPerUser.find( in_sCustId );
		if ( m_NetRedemptionLimitPerUser.end() == it_NetRedemptionLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼
			insertRet = m_NetRedemptionLimitPerUser.insert(
				std::pair<std::string, FrozeVolume>(
				in_sCustId, FrozeVolume( m_ui64NetRedemptionLimitPerUser ) ) );
			if ( !insertRet.second )
			{
				// insert fail��������
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
���������ؽ��׶�

Return:
0 -- �������ɹ�
-1 -- ����ʧ��
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
	// ���û���iterator
	std::map<std::string, FrozeVolume>::iterator it_RedemptionLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetRedemptionLimitPerUser;
	//std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//�ۼ�����ܶ����� RedemptionLimit N18(2) �����ۼƿ���صĻ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ� Ŀǰֻ��Ϊ����
	iRes = m_RedemptionLimit.Defroze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//�����˻��ۼ�����ܶ����� RedemptionLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ���صĻ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	if ( 0 != m_ui64RedemptionLimitPerUser )
	{
		it_RedemptionLimitPerUser = m_RedemptionLimitPerUser.find( in_sCustId );
		if ( m_RedemptionLimitPerUser.end() == it_RedemptionLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼��������
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

	//������ܶ����� NetRedemptionLimit N18(2) ���쾻��صĻ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	m_NetRedemptionLimit.Defroze( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//�����˻�������ܶ����� NetRedemptionLimitPerUser N18(2) ����֤ȯ�˻����쾻��صĻ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	if ( 0 != m_ui64NetRedemptionLimitPerUser )
	{
		it_NetRedemptionLimitPerUser = m_NetRedemptionLimitPerUser.find( in_sCustId );
		if ( m_NetRedemptionLimitPerUser.end() == it_NetRedemptionLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼��������
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
ȷ����ؽ��׶�

Return:
0 -- ȷ�ϳɹ�
-1 -- ȷ��ʧ��
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
	// ���û���iterator
	std::map<std::string, FrozeVolume>::iterator it_RedemptionLimitPerUser;
	std::map<std::string, FrozeVolume>::iterator it_NetRedemptionLimitPerUser;
	//std::pair<std::map<std::string, FrozeVolume>::iterator, bool> insertRet;

	//�ۼ�����ܶ����� RedemptionLimit N18(2) �����ۼƿ���صĻ���ݶ����ޣ�Ϊ 0 ��ʾû�����ƣ� Ŀǰֻ��Ϊ����
	iRes = m_RedemptionLimit.Confirm( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//�����˻��ۼ�����ܶ����� RedemptionLimitPerUser N18(2) ����֤ȯ�˻������ۼƿ���صĻ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	if ( 0 != m_ui64RedemptionLimitPerUser )
	{
		it_RedemptionLimitPerUser = m_RedemptionLimitPerUser.find( in_sCustId );
		if ( m_RedemptionLimitPerUser.end() == it_RedemptionLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼��������
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

	//������ܶ����� NetRedemptionLimit N18(2) ���쾻��صĻ���ݶ����ޣ�Ϊ 0��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	m_NetRedemptionLimit.Confirm( in_num );
	if ( 0 != iRes )
	{
		return -1;
	}

	//�����˻�������ܶ����� NetRedemptionLimitPerUser N18(2) ����֤ȯ�˻����쾻��صĻ���ݶ����ޣ�
	//Ϊ 0 ��ʾû�����ƣ�Ŀǰֻ��Ϊ����
	if ( 0 != m_ui64NetRedemptionLimitPerUser )
	{
		it_NetRedemptionLimitPerUser = m_NetRedemptionLimitPerUser.find( in_sCustId );
		if ( m_NetRedemptionLimitPerUser.end() == it_NetRedemptionLimitPerUser )
		{
			// �޸��û��Ľ��׼�¼��������
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