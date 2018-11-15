#include "Redis3_0Cnn_dll.h"
#include "tool_string/Tgw_StringUtil.h"

#include <time.h>

#include "util/EzLog.h"
#include "tool_redis/Tgw_RedisHelper.h"

#include "boost/algorithm/string.hpp"

Redis3_0Cnn_dll::Redis3_0Cnn_dll()
	: m_lTimeout_Millisecond( 10000 ), m_ptrCurrentContext( nullptr ), m_bShareMem( false ),
	m_timeLastUseTime(0)
{
}

Redis3_0Cnn_dll::~Redis3_0Cnn_dll()
{
	CleanContext();
}

/*
连接Redis Server
Return :
0 -- 连接成功
-1 -- 连接失败
*/
int Redis3_0Cnn_dll::Connect( const std::string& strSvrIp, const unsigned int uiPort,
	const long in_lTimeout_Millisecond )
{
	static const string ftag( "Redis3_0Cnn_dll::Connect() " );
	if ( strSvrIp.empty() || 0 == uiPort || 0 > in_lTimeout_Millisecond )
	{
		return -1;
	}

	CleanContext();

	m_lTimeout_Millisecond = in_lTimeout_Millisecond;

	std::shared_ptr<RedisContextPack_dll> ptrContext;

	int iRes = OpenNewConnect( strSvrIp, uiPort, in_lTimeout_Millisecond, ptrContext );
	if ( 0 != iRes )
	{
		EzLog::e( ftag, "failed!" );

		return -1;
	}

	m_ptrCurrentContext = ptrContext;

	AddConnection( ptrContext );

	return iRes;
}

/*
断开连接
Return :
*/
void Redis3_0Cnn_dll::Disconnect(void)
{
	static const string ftag("Redis3_0Cnn_dll::Disconnect() ");

	CleanContext();
}

/*
连接Redis Server
Return :
0 -- 连接成功
-1 -- 连接失败
*/
int Redis3_0Cnn_dll::OpenNewConnect( const std::string& strSvrIp, const unsigned int uiPort,
	const long in_lTimeout_Millisecond, std::shared_ptr<RedisContextPack_dll>& out_ptrContext )
{
	static const string ftag( "Redis3_0Cnn_dll::OpenNewConnect() " );
	if ( strSvrIp.empty() || 0 == uiPort || 0 > in_lTimeout_Millisecond )
	{
		return -1;
	}

	if ( NULL == Tgw_RedisHelper::F_Dll_redisConnectWithTimeout )
	{
		EzLog::e( ftag, "F_Dll_redisConnectWithTimeout() is NULL" );
		return -1;
	}

	long lSecond = in_lTimeout_Millisecond / 1000;
	long lMicrosecond = ( in_lTimeout_Millisecond % 1000 ) * 1000;

	struct timeval timeout = { lSecond, lMicrosecond };

	// 生成context package
	std::shared_ptr<RedisContextPack_dll> ptrContext( new RedisContextPack_dll() );

	ptrContext->strIP = strSvrIp;
	ptrContext->uiPort = uiPort;

	// key
	std::string strTranTmp;
	sof_string::itostr( uiPort, strTranTmp );

	ptrContext->strKey = strSvrIp;
	ptrContext->strKey += ":";
	ptrContext->strKey += strTranTmp;

	ptrContext->contextRedis = Tgw_RedisHelper::F_Dll_redisConnectWithTimeout( ptrContext->strIP.c_str(), ptrContext->uiPort, timeout );
	if ( ptrContext->contextRedis->err )
	{
		string strDebug( "Connect to redisServer failed:" );
		strDebug += ptrContext->strKey;
		strDebug += " ";
		strDebug += ptrContext->contextRedis->errstr;
		EzLog::e( ftag, strDebug );

		FreeContext( ptrContext );

		return -1;
	}

	// alive time
	ptrContext->tLastActiveTime = time( NULL );

	out_ptrContext = ptrContext;

	return 0;
}

/*
释放连接
*/
void Redis3_0Cnn_dll::CleanContext( void )
{
	static const string ftag( "Redis3_0Cnn_dll::CleanContext() " );

	std::map<std::string, std::shared_ptr<RedisContextPack_dll>>::iterator
		it = m_mapRedisContexts.begin();

	if ( NULL == Tgw_RedisHelper::F_Dll_redisFree )
	{
		EzLog::e( ftag, "F_Dll_redisFree() is NULL" );
		return;
	}

	while ( m_mapRedisContexts.end() != it )
	{
		std::shared_ptr<RedisContextPack_dll> ptrContext = it->second;

		if ( nullptr != ptrContext && nullptr != ptrContext->contextRedis )
		{
			Tgw_RedisHelper::F_Dll_redisFree( ptrContext->contextRedis );
			ptrContext->contextRedis = nullptr;
		}

		m_mapRedisContexts.erase( it );
		it = m_mapRedisContexts.begin();
	}

	m_ptrCurrentContext.reset();
}

/*
释放连接
*/
void Redis3_0Cnn_dll::FreeContext( std::shared_ptr<RedisContextPack_dll>& in_ptrContext )
{
	static const string ftag( "Redis3_0Cnn_dll::FreeContext() " );

	if ( NULL == Tgw_RedisHelper::F_Dll_redisFree )
	{
		EzLog::e( ftag, "F_Dll_redisFree() is NULL" );
		return;
	}

	if ( nullptr != in_ptrContext && nullptr != in_ptrContext->contextRedis )
	{
		Tgw_RedisHelper::F_Dll_redisFree( in_ptrContext->contextRedis );
		in_ptrContext->contextRedis = nullptr;
	}
}

/*
添加新连接进连接库
Return :
0 -- 成功
-1 -- 失败
*/
int Redis3_0Cnn_dll::AddConnection( std::shared_ptr<RedisContextPack_dll>& in_ptrContext )
{
	static const string ftag( "Redis3_0Cnn_dll::AddConnection() " );

	if ( nullptr == in_ptrContext || nullptr == in_ptrContext->contextRedis )
	{
		return -1;
	}

	// 查找现有Key是否有值
	std::map<std::string, std::shared_ptr<RedisContextPack_dll>>::iterator it
		= m_mapRedisContexts.find( in_ptrContext->strKey );
	if ( m_mapRedisContexts.end() != it )
	{
		// 先释放已有连接
		FreeContext( it->second );

		it->second = in_ptrContext;
	}
	else
	{
		m_mapRedisContexts[in_ptrContext->strKey] = in_ptrContext;
	}

	return 0;
}

/*
从连接库获取连接
Return :
0 -- 成功
-1 -- 失败
*/
int Redis3_0Cnn_dll::GetConnection( const std::string& in_key,
	std::shared_ptr<RedisContextPack_dll>& out_ptrContext )
{
	static const string ftag( "StgwRedis3_0Helper::GetConnection() " );

	// 查找现有Key是否有值
	std::map<std::string, std::shared_ptr<RedisContextPack_dll>>::iterator it
		= m_mapRedisContexts.find( in_key );
	if ( m_mapRedisContexts.end() != it )
	{
		out_ptrContext = it->second;

		return 0;
	}
	else
	{
		return -1;
	}
}

/*
执行Redis Command
Params :
const std::string strCmd :
redis command string.

long long* out_pllRes :
返回的long long值.

std::string* out_pstrRes :
返回的string值.

std::vector<string>* out_pvectArray :
返回的数组值.

Return :
返回的数据类型
enum RedisReply
{
RedisReply_error = -2, // REDIS_REPLY_ERROR 6
RedisReply_nil = -1, // REDIS_REPLY_NIL 4
RedisReply_string = 1, // REDIS_REPLY_STRING 1
RedisReply_array = 2, // REDIS_REPLY_ARRAY 2
RedisReply_integer = 3, // REDIS_REPLY_INTEGER 3
RedisReply_status = 5 // REDIS_REPLY_STATUS 5
};
*/
RedisReply Redis3_0Cnn_dll::Cmd( const std::string& strCmd,
	long long* out_pllRes, std::string* out_pstrRes, int* out_piStrLen,
	std::vector<StgwRedisReply>* out_pvectArray, size_t* out_pElemSize )
{
	static const string ftag( "StgwRedis3_0Helper::Cmd() " );
	string strDebug;

	if ( nullptr == m_ptrCurrentContext )
	{
		return RedisReply_error;
	}

	if ( NULL == Tgw_RedisHelper::F_DLL_redisCmd || NULL == Tgw_RedisHelper::F_DLL_freeReplyObject )
	{
		EzLog::e( ftag, "F_DLL_redisCmd() or F_DLL_freeReplyObject() is NULL" );
		return RedisReply_error;
	}

	redisReply_dll* reply = static_cast<redisReply_dll*>(
		Tgw_RedisHelper::F_DLL_redisCmd( m_ptrCurrentContext->contextRedis, strCmd.c_str() ) );

	if ( nullptr == reply )
	{
		string strDebug( "Execute command [" );
		strDebug += strCmd;
		strDebug += "] failure[nullptr == reply]:";
		strDebug += m_ptrCurrentContext->contextRedis->errstr;
		EzLog::e( ftag, strDebug );

		return RedisReply_error;
	}

	RedisReply emReturnRes = RedisReply_error;

	string sReplyString;

	// in case redis cluster, key moved.
	// error: MOVED 15495 192.168.198.130:7002
	if ( simutgw::redis_REDIS_REPLY_ERROR == reply->type )
	{
		sReplyString = reply->str;

		std::string strMoved_IpPort;
		std::string strMoved_Ip;
		unsigned int uiMoved_Port;

		int iRes = CheckIsClusterMoved( sReplyString, strMoved_IpPort, strMoved_Ip, uiMoved_Port );
		if ( 0 == iRes )
		{
			// Need resend because of redis clusted moved error.
			std::shared_ptr<RedisContextPack_dll> ptrContext;

			iRes = ClusterMovedReconnect( strMoved_IpPort, strMoved_Ip, uiMoved_Port, ptrContext );
			if ( 0 == iRes )
			{
				// Reconnect success.
				m_ptrCurrentContext = ptrContext;

				// free previous reply
				Tgw_RedisHelper::F_DLL_freeReplyObject( reply );
				reply = nullptr;

				reply = static_cast<redisReply_dll*>( Tgw_RedisHelper::F_DLL_redisCmd( m_ptrCurrentContext->contextRedis, strCmd.c_str() ) );

				if ( nullptr == reply )
				{
					string strDebug( "Execute command [" );
					strDebug += strCmd;
					strDebug += "] failure[nullptr == reply]:";
					strDebug += m_ptrCurrentContext->contextRedis->errstr;
					EzLog::e( ftag, strDebug );

					return RedisReply_error;
				}
			}
			else
			{
				// Reconnect failed
				string strDebug( "Moved Reconnect [" );
				strDebug += strMoved_IpPort;
				strDebug += "] failed";
				EzLog::e( ftag, strDebug );
			}
		}
	}

	switch ( reply->type )
	{
	case simutgw::redis_REDIS_REPLY_STRING: // 1
		// 返回字符串标识。可以通过reply->str得到具体值，通过reply->len得到信息长度。

		emReturnRes = RedisReply_string;

		if ( nullptr != out_piStrLen )
		{
			*out_piStrLen = reply->len;
		}

		if ( nullptr != out_pstrRes )
		{
			*out_pstrRes = reply->str;
		}
		break;

	case simutgw::redis_REDIS_REPLY_ARRAY: // 2
		// 返回数据集标识。数据集中元素的数目可以通过reply->elements获得，每个元素是个redisReply对象，
		// 元素值可以通过reply->element[..index..].*形式获得，用在获取多个数据结果的操作。

		emReturnRes = RedisReply_array;

		if ( nullptr != out_pElemSize )
		{
			*out_pElemSize = reply->elements;
		}

		if ( nullptr != out_pvectArray )
		{
			size_t i = 0;
			for ( i = 0; i < reply->elements; ++i )
			{
				StgwRedisReply arrayElem;

				int iTypeElem = reply->element[i]->type;

				switch ( iTypeElem )
				{
				case simutgw::redis_REDIS_REPLY_STRING: // 1
					// 返回字符串标识。可以通过reply->str得到具体值，通过reply->len得到信息长度。
					arrayElem.type = RedisReply_string;

					arrayElem.str = reply->element[i]->str;
					break;

				case simutgw::redis_REDIS_REPLY_INTEGER: // 3
					// 返回整型标识。可以通过reply->integer变量得到类型为long long的值。
					arrayElem.type = RedisReply_integer;

					arrayElem.integer = reply->element[i]->integer;
					break;

				case simutgw::redis_REDIS_REPLY_NIL: // 4
					// 返回nil对象，说明不存在要访问的数据。
					arrayElem.type = RedisReply_nil;
					break;

				default:
					arrayElem.type = RedisReply_nil;
					break;
				}

				out_pvectArray->push_back( arrayElem );
			}
		}
		break;

	case simutgw::redis_REDIS_REPLY_INTEGER: // 3
		// 返回整型标识。可以通过reply->integer变量得到类型为long long的值。

		emReturnRes = RedisReply_integer;

		if ( nullptr != out_pllRes )
		{
			*out_pllRes = reply->integer;
		}
		break;

	case simutgw::redis_REDIS_REPLY_NIL: // 4
		// 返回nil对象，说明不存在要访问的数据。

		emReturnRes = RedisReply_nil;
		break;

	case simutgw::redis_REDIS_REPLY_STATUS: // 5
		// 返回执行结果为状态的命令。比如set命令的返回值的类型是REDIS_REPLY_STATUS，然后只有当返回信息是"OK"时，
		// 才表示该命令执行成功。可以通过reply->str得到文字信息，通过reply->len得到信息长度。

		sReplyString = reply->str;

		if ( nullptr != out_piStrLen )
		{
			*out_piStrLen = reply->len;
		}

		if ( 0 == sReplyString.compare( "OK" ) )
		{
			emReturnRes = RedisReply_status;

			if ( nullptr != out_pstrRes )
			{
				*out_pstrRes = sReplyString;
			}
		}
		else
		{
			emReturnRes = RedisReply_error;

			strDebug = "Execute command [";
			strDebug += strCmd;
			strDebug += "] failure[ReplyString=";
			strDebug += sReplyString;
			strDebug += "]:";
			strDebug += m_ptrCurrentContext->contextRedis->errstr;
			EzLog::e( ftag, strDebug );
		}
		break;

	case simutgw::redis_REDIS_REPLY_ERROR: // 6
		// 返回错误。错误信息可以通过reply->str得到文字信息，通过reply->len得到信息长度。

		emReturnRes = RedisReply_error;

		if ( nullptr != out_piStrLen )
		{
			*out_piStrLen = reply->len;
		}

		if ( nullptr != out_pstrRes )
		{
			*out_pstrRes = reply->str;
		}

		strDebug = "Failed to execute command [";
		strDebug += strCmd;
		strDebug += "] failure[REDIS_REPLY_ERROR]:";
		strDebug += reply->str;
		EzLog::e( ftag, strDebug );
		break;

	default:
		emReturnRes = RedisReply_error;

		strDebug = "Failed to execute command [";
		strDebug += strCmd;
		strDebug += "] failure[case default=";

		std::string strNum;
		sof_string::itostr( reply->type, strNum );
		strDebug += strNum;
		strDebug += "]";

		EzLog::e( ftag, strDebug );
		break;
	}

	Tgw_RedisHelper::F_DLL_freeReplyObject( reply );

	return emReturnRes;
}

/*
执行Redis redisCommandArgv
Params :

Return :
redisReply*
*/
redisReply_dll* Redis3_0Cnn_dll::Send_redisCommandArgv( const vector<string>& vctArgs )
{
	static const string ftag( "Redis3_0Cnn_dll::Send_redisCommandArgv() " );
	string strDebug;

	if ( nullptr == m_ptrCurrentContext )
	{
		return nullptr;
	}

	if ( NULL == Tgw_RedisHelper::F_DLL_redisCommandArgv || NULL == Tgw_RedisHelper::F_DLL_freeReplyObject )
	{
		EzLog::e( ftag, "F_DLL_redisCommandArgv() or F_DLL_freeReplyObject() is NULL" );
		return nullptr;
	}

	int nArgs = (int)vctArgs.size();

	char **pszArgs = new char *[nArgs];
	size_t *pnArgsLen = new size_t[nArgs];

	size_t i = 0;
	for ( i = 0; i < nArgs; ++i )
	{
		size_t iElemLen = vctArgs[i].size();
		pnArgsLen[i] = iElemLen;
		if ( m_bShareMem )
		{
			pszArgs[i] = (char*) vctArgs[i].data();
		}
		else
		{
			pszArgs[i] = new char[iElemLen + 1];
#ifdef _MSC_VER
			memcpy_s( pszArgs[i], iElemLen, vctArgs[i].data(), iElemLen );
#else
			memcpy(pszArgs[i], vctArgs[i].data(), iElemLen);
#endif
			pszArgs[i][iElemLen] = '\0';
		}
	}

	redisReply_dll* reply = static_cast<redisReply_dll*>( Tgw_RedisHelper::F_DLL_redisCommandArgv( m_ptrCurrentContext->contextRedis, nArgs,
		(const char **) pszArgs, (const size_t *) pnArgsLen ) );

	// free
	if ( !m_bShareMem )
	{
		for ( i = 0; i < nArgs; ++i )
		{
			delete[] pszArgs[i];
		}

	}
	delete[] pszArgs;
	pszArgs = nullptr;

	delete[] pnArgsLen;
	pnArgsLen = nullptr;

	return reply;
}

/*
执行Redis Command
Params :
const vector<string>& vctArgs :
redis command vector.

long long* out_pllRes :
返回的long long值.

std::string* out_pstrRes :
返回的string值.

int* out_piStrLen :
Length of string.

std::vector<string>* out_pvectArray :
返回的数组值.

size_t* out_pElemSize :
number of elements, for REDIS_REPLY_ARRAY.

Return :
返回的数据类型
enum RedisReply
{
RedisReply_error = -2, // REDIS_REPLY_ERROR 6
RedisReply_nil = -1, // REDIS_REPLY_NIL 4
RedisReply_string = 1, // REDIS_REPLY_STRING 1
RedisReply_array = 2, // REDIS_REPLY_ARRAY 2
RedisReply_integer = 3, // REDIS_REPLY_INTEGER 3
RedisReply_status = 5 // REDIS_REPLY_STATUS 5
};
*/
RedisReply Redis3_0Cnn_dll::CmdArgv( const vector<string>& vctArgs,
	long long* out_pllRes, std::string* out_pstrRes, int* out_piStrLen,
	std::vector<StgwRedisReply>* out_pvectArray, size_t* out_pElemSize )
{
	static const string ftag( "StgwRedis3_0Helper::CmdArgv() " );
	string strDebug;

	if ( nullptr == m_ptrCurrentContext )
	{
		return RedisReply_error;
	}

	if ( NULL == Tgw_RedisHelper::F_DLL_freeReplyObject )
	{
		EzLog::e( ftag, "F_DLL_freeReplyObject() is NULL" );
		return RedisReply_error;
	}

	size_t nArgs = vctArgs.size();
	size_t i = 0;

	redisReply_dll* reply = Send_redisCommandArgv( vctArgs );

	if ( nullptr == reply )
	{
		string strDebug( "Execute command [" );
		for ( i = 0; i < nArgs; ++i )
		{
			strDebug += vctArgs[i];
			strDebug += " ";
		}
		strDebug += "] failure[nullptr == reply]:";
		strDebug += m_ptrCurrentContext->contextRedis->errstr;
		EzLog::e( ftag, strDebug );

		return RedisReply_error;
	}

	RedisReply emReturnRes = RedisReply_error;

	string sReplyString;

	// in case redis cluster, key moved.
	// error: MOVED 15495 192.168.198.130:7002
	if ( simutgw::redis_REDIS_REPLY_ERROR == reply->type )
	{
		sReplyString = reply->str;

		std::string strMoved_IpPort;
		std::string strMoved_Ip;
		unsigned int uiMoved_Port;

		int iRes = CheckIsClusterMoved( sReplyString, strMoved_IpPort, strMoved_Ip, uiMoved_Port );
		if ( 0 == iRes )
		{
			// Need resend because of redis clusted moved error.

			std::shared_ptr<RedisContextPack_dll> ptrContext;

			iRes = ClusterMovedReconnect( strMoved_IpPort, strMoved_Ip, uiMoved_Port, ptrContext );
			if ( 0 == iRes )
			{
				// Reconnect success.
				m_ptrCurrentContext = ptrContext;

				// free previous reply
				Tgw_RedisHelper::F_DLL_freeReplyObject( reply );
				reply = nullptr;

				reply = Send_redisCommandArgv( vctArgs );

				if ( nullptr == reply )
				{
					string strDebug( "Execute command [" );
					for ( i = 0; i < nArgs; ++i )
					{
						strDebug += vctArgs[i];
						strDebug += " ";
					}
					strDebug += "] failure[nullptr == reply]:";
					strDebug += m_ptrCurrentContext->contextRedis->errstr;
					EzLog::e( ftag, strDebug );

					return RedisReply_error;
				}
			}
			else
			{
				// Reconnect failed
				string strDebug( "Moved Reconnect [" );
				strDebug += strMoved_IpPort;
				strDebug += "] failed";
				EzLog::e( ftag, strDebug );
			}
		}
	}

	switch ( reply->type )
	{
	case simutgw::redis_REDIS_REPLY_STRING: // 1
		// 返回字符串标识。可以通过reply->str得到具体值，通过reply->len得到信息长度。

		emReturnRes = RedisReply_string;

		if ( nullptr != out_piStrLen )
		{
			*out_piStrLen = reply->len;
		}

		if ( nullptr != out_pstrRes )
		{
			*out_pstrRes = reply->str;
		}
		break;

	case simutgw::redis_REDIS_REPLY_ARRAY: // 2
		// 返回数据集标识。数据集中元素的数目可以通过reply->elements获得，每个元素是个redisReply对象，
		// 元素值可以通过reply->element[..index..].*形式获得，用在获取多个数据结果的操作。

		emReturnRes = RedisReply_array;

		if ( nullptr != out_pElemSize )
		{
			*out_pElemSize = reply->elements;
		}

		if ( nullptr != out_pvectArray )
		{
			for ( i = 0; i < reply->elements; ++i )
			{
				StgwRedisReply arrayElem;

				int iTypeElem = reply->element[i]->type;

				switch ( iTypeElem )
				{
				case simutgw::redis_REDIS_REPLY_STRING: // 1
					// 返回字符串标识。可以通过reply->str得到具体值，通过reply->len得到信息长度。

					arrayElem.type = RedisReply_string;

					arrayElem.str = reply->element[i]->str;
					break;

				case simutgw::redis_REDIS_REPLY_INTEGER: // 3
					// 返回整型标识。可以通过reply->integer变量得到类型为long long的值。

					arrayElem.type = RedisReply_integer;

					arrayElem.integer = reply->element[i]->integer;
					break;

				case simutgw::redis_REDIS_REPLY_NIL: // 4
					// 返回nil对象，说明不存在要访问的数据。

					arrayElem.type = RedisReply_nil;
					break;


				default:
					arrayElem.type = RedisReply_nil;
					break;
				}

				out_pvectArray->push_back( arrayElem );
			}
		}
		break;

	case simutgw::redis_REDIS_REPLY_INTEGER: // 3
		// 返回整型标识。可以通过reply->integer变量得到类型为long long的值。

		emReturnRes = RedisReply_integer;

		if ( nullptr != out_pllRes )
		{
			*out_pllRes = reply->integer;
		}
		break;

	case simutgw::redis_REDIS_REPLY_NIL: // 4
		// 返回nil对象，说明不存在要访问的数据。

		emReturnRes = RedisReply_nil;
		break;

	case simutgw::redis_REDIS_REPLY_STATUS: // 5
		// 返回执行结果为状态的命令。比如set命令的返回值的类型是REDIS_REPLY_STATUS，然后只有当返回信息是"OK"时，
		// 才表示该命令执行成功。可以通过reply->str得到文字信息，通过reply->len得到信息长度。
		sReplyString = reply->str;

		if ( nullptr != out_piStrLen )
		{
			*out_piStrLen = reply->len;
		}

		if ( 0 == sReplyString.compare( "OK" ) || 0 == sReplyString.compare( "ok" ) )
		{
			emReturnRes = RedisReply_status;

			if ( nullptr != out_pstrRes )
			{
				*out_pstrRes = sReplyString;
			}
		}
		else
		{
			emReturnRes = RedisReply_error;

			strDebug = "Execute command [";
			for ( i = 0; i < nArgs; ++i )
			{
				strDebug += vctArgs[i];
				strDebug += " ";
			}
			strDebug += "] failure[ReplyString=";
			strDebug += sReplyString;
			strDebug += "]:";
			strDebug += m_ptrCurrentContext->contextRedis->errstr;
			EzLog::e( ftag, strDebug );
		}
		break;

	case simutgw::redis_REDIS_REPLY_ERROR: // 6
		// 返回错误。错误信息可以通过reply->str得到文字信息，通过reply->len得到信息长度。

		emReturnRes = RedisReply_error;

		if ( nullptr != out_piStrLen )
		{
			*out_piStrLen = reply->len;
		}

		if ( nullptr != out_pstrRes )
		{
			*out_pstrRes = reply->str;
		}

		strDebug = "Failed to execute command [";
		for ( i = 0; i < nArgs; ++i )
		{
			strDebug += vctArgs[i];
			strDebug += " ";
		}
		strDebug += "] failure[REDIS_REPLY_ERROR]:";
		strDebug += reply->str;
		EzLog::e( ftag, strDebug );
		break;

	default:
		emReturnRes = RedisReply_error;

		strDebug = "Failed to execute command [";
		for ( i = 0; i < nArgs; ++i )
		{
			strDebug += vctArgs[i];
			strDebug += " ";
		}
		strDebug += "] failure[case default=";
		std::string strNum;
		sof_string::itostr( reply->type, strNum );
		strDebug += strNum;
		strDebug += "]";
		EzLog::e( ftag, strDebug );
		break;
	}

	Tgw_RedisHelper::F_DLL_freeReplyObject( reply );

	return emReturnRes;
}

/*
check if redis cluster moved
error messages are like : MOVED 15495 192.168.198.130:7002
Return :
0 -- is cluster moved
-1 -- not cluster moved
*/
int Redis3_0Cnn_dll::CheckIsClusterMoved( const std::string& in_strErrMsg,
	std::string& out_strIpPort, std::string& out_strIp, unsigned int& out_uiPort )
{
	static const string ftag( "StgwRedis3_0Helper::CheckIsClusterMoved() " );

	// MOVED key word
	static const std::string strMOVED( "MOVED" );
	// :
	static const std::string strColon( ":" );
	string strDebug;

	// MOVED 15495 192.168.198.130:7002 有可能的最小长度为17
	if ( 17 > in_strErrMsg.length() )
	{
		return -1;
	}

	std::string::size_type stFindPos = in_strErrMsg.find( strMOVED );

	if ( std::string::npos == stFindPos || 0 != stFindPos )
	{
		return -1;
	}

	std::vector<std::string> vectSegement;
	// MOVED 15495 192.168.198.130:7002 使用空格分隔
	boost::split( vectSegement, in_strErrMsg, boost::is_any_of( " " ) );

	if ( 3 > vectSegement.size() )
	{
		return -1;
	}
	// 192.168.198.130:7002
	std::string strIpPort = vectSegement[2];

	// 寻找冒号位置
	stFindPos = strIpPort.find( strColon );

	// 确保找到的位置有效
	if ( std::string::npos == stFindPos || 0 == stFindPos || strIpPort.length() < ( stFindPos + 1 ) )
	{
		return -1;
	}

	std::string strIp = strIpPort.substr( 0, stFindPos );
	std::string strPort = strIpPort.substr( stFindPos + 1 );

	// 转换port端口号
	long lPort = 0;
	int iRes = Tgw_StringUtil::String2Long_atol( strPort, lPort );
	if ( 0 != iRes )
	{
		return -1;
	}

	out_strIpPort = strIpPort;
	out_strIp = strIp;
	out_uiPort = (unsigned int) lPort;

	return 0;
}

/*
redis cluster moved, reconnect to another node.

Return :
0 -- reconnect success
-1 -- reconnect failed
*/
int Redis3_0Cnn_dll::ClusterMovedReconnect( const std::string& in_strIpPort,
	const std::string& in_strIp, const unsigned int& in_uiPort,
	std::shared_ptr<RedisContextPack_dll>& out_ptrContext )
{
	static const string ftag( "StgwRedis3_0Helper::ClusterMovedReconnect() " );

	std::shared_ptr<RedisContextPack_dll> ptrContext;

	int iRes = GetConnection( in_strIpPort, ptrContext );

	if ( 0 == iRes )
	{
		// 有以IpPort为key的连接

		out_ptrContext = ptrContext;
		return 0;
	}

	// 没有以IpPort为key的连接
	// 创建新的连接
	iRes = OpenNewConnect( in_strIp, in_uiPort, m_lTimeout_Millisecond, ptrContext );
	if ( 0 != iRes )
	{
		EzLog::e( ftag, "failed!" );

		return -1;
	}

	AddConnection( ptrContext );

	out_ptrContext = ptrContext;
	return 0;
}