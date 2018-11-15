#include "OutMemoryManage.h"

#include "config/conf_msg.h"

OutMemoryManage::OutMemoryManage( void )
{
}

OutMemoryManage::~OutMemoryManage( void )
{
}

/*
���뷵����Ϣ

Param :

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int OutMemoryManage::PushBack( std::shared_ptr<struct simutgw::OrderMessage>& in_msg )
{
	static const string ftag( "OutMemoryManage::PushBack() " );

	int iRes = 0;


	if ( 0 == in_msg->strTrade_market.compare( simutgw::TRADE_MARKET_SZ ) )
	{
		// ����

		// connection 
		string sKey = in_msg->strSenderCompID;
		if ( sKey.empty() || 0 == sKey.length() )
		{
			string sDebug( "empty SenderCompID, Clordid=[" );
			sDebug += in_msg->strClordid;
			sDebug += "]";
			EzLog::e( ftag, sDebug );
			return -1;
		}

		iRes = PushBack_sz( sKey, in_msg );
	}
	else if ( 0 == in_msg->strTrade_market.compare( simutgw::TRADE_MARKET_SH ) )
	{
		// �Ϻ�
		iRes = PushBack_sh( in_msg );
	}
	else
	{
		string sDebug( "error Trade_market=[" );
		sDebug += in_msg->strTrade_market;
		sDebug += "], Clordid=[";
		sDebug += in_msg->strClordid;
		sDebug += "]";
		EzLog::e( ftag, sDebug );
		return -1;
	}

	return iRes;
}

/*
���뷵����Ϣ
����

Param :

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int OutMemoryManage::PushBack_sz( const string& in_key, std::shared_ptr<struct simutgw::OrderMessage>& in_msg )
{
	static const string ftag( "OutMemoryManage::PushBack_sz() " );

	// ���ڵ���Ϊ��map����Ҫ��
	boost::unique_lock<boost::mutex> Locker( m_mutexlock );

	int iRes = 0;

	iRes = m_sz_Buffer[in_key].PushBack( in_msg );

	return iRes;
}

/*
���뷵����Ϣ
�Ϻ�

Param :

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int OutMemoryManage::PushBack_sh( std::shared_ptr<struct simutgw::OrderMessage>& in_msg )
{
	static const string ftag( "OutMemoryManage::PushBack_sh() " );

	int iRes = 0;

	iRes = m_sh_Buffer.PushBack( in_msg );

	return iRes;
}

/*
��ȡ������Ϣ
����

Param :


Return :
0 -- ����Ϣ
1 -- ����Ϣ
-1 -- ʧ��
*/
int OutMemoryManage::PopFront_sz( const std::string& in_strConnKey, std::shared_ptr<struct simutgw::OrderMessage>& in_msg )
{
	static const string ftag( "OutMemoryManage::PopFront_sz() " );

	// ���ڵ���Ϊ��map����Ҫ��
	boost::unique_lock<boost::mutex> Locker( m_mutexlock );

	int iRes = 0;

	if ( in_strConnKey.empty() || 0 == in_strConnKey.length() )
	{
		string sDebug( "empty SenderCompID" );
		EzLog::e( ftag, sDebug );
		return -1;
	}

	std::map<std::string, MemoryStoreCell<std::shared_ptr<struct simutgw::OrderMessage>>>::iterator it = m_sz_Buffer.find( in_strConnKey );
	if ( m_sz_Buffer.end() == it )
	{
		// û�ҵ������ӵ���Ϣ
		return 1;
	}
	else
	{
		iRes = it->second.PopFront( in_msg );
	}

	return iRes;
}

/*
��ȡ������Ϣ
�Ϻ�

Param :

Return :
0 -- ����Ϣ
1 -- ����Ϣ
-1 -- ʧ��
*/
int OutMemoryManage::PopFront_sh( std::shared_ptr<struct simutgw::OrderMessage>& in_msg )
{
	static const string ftag( "OutMemoryManage::PopFront_sz() " );

	int iRes = 0;

	iRes = m_sh_Buffer.PopFront( in_msg );

	return iRes;
}

/*
��ȡ���������Ϣ
����

Param :


Return :
0 -- ����Ϣ
1 -- ����Ϣ
-1 -- ʧ��
*/
size_t OutMemoryManage::GetSize_sz( const std::string& in_strConnKey )
{
	static const string ftag( "OutMemoryManage::GetSize_sz() " );

	// ���ڵ���Ϊ��map����Ҫ��
	boost::unique_lock<boost::mutex> Locker( m_mutexlock );

	size_t stSize = 0;

	if ( in_strConnKey.empty() || 0 == in_strConnKey.length() )
	{
		string sDebug( "empty SenderCompID" );
		EzLog::e( ftag, sDebug );
		return 0;
	}

	std::map<std::string, MemoryStoreCell<std::shared_ptr<struct simutgw::OrderMessage>>>::iterator it = m_sz_Buffer.find( in_strConnKey );
	if ( m_sz_Buffer.end() == it )
	{
		// û�ҵ������ӵ���Ϣ
		return 0;
	}
	else
	{
		stSize = it->second.GetSize();
	}

	return stSize;
}

/*
��ȡ���������Ϣ
�Ϻ�

Param :


Return :
size_t
*/
size_t OutMemoryManage::GetSize_sh()
{
	static const string ftag( "OutMemoryManage::GetSize_sh() " );

	size_t stSize = m_sh_Buffer.GetSize();

	return stSize;
}