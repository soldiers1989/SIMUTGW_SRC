#include "Tgw_RedisHelper.h"

#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/sof_string.h"

#include "simutgw/stgw_config/g_values_inner.h"

using namespace std;

const string Tgw_RedisHelper::g_RplyRes_OK("OK");
// 8小时过期时间
const string Tgw_RedisHelper::g_Expire_8Hour("28800");

#ifdef _MSC_VER
HINSTANCE Tgw_RedisHelper::m_hDLL = NULL;
#else
void* Tgw_RedisHelper::m_hDLL = NULL;
#endif

Tgw_RedisHelper::pfDll_redisConnectWithTimeout
Tgw_RedisHelper::F_Dll_redisConnectWithTimeout = nullptr;

Tgw_RedisHelper::pfDll_redisFree
Tgw_RedisHelper::F_Dll_redisFree = nullptr;

Tgw_RedisHelper::pfDLL_redisCmd
Tgw_RedisHelper::F_DLL_redisCmd = nullptr;

Tgw_RedisHelper::pfDLL_redisCommandArgv
Tgw_RedisHelper::F_DLL_redisCommandArgv = nullptr;

Tgw_RedisHelper::pfDLL_freeReplyObject
Tgw_RedisHelper::F_DLL_freeReplyObject = nullptr;

Tgw_RedisHelper::Tgw_RedisHelper(void)
{
}

Tgw_RedisHelper::~Tgw_RedisHelper(void)
{
}

/*
Load hiredis wrapper dll.
*/
int Tgw_RedisHelper::LoadHiredisLibrary(void)
{
	static const string ftag("Tgw_RedisHelper::LoadHiredisLibrary() ");

#ifdef _MSC_VER
	try
	{
		//加载动态链接库hiredis_wrapper.dll文件
		m_hDLL = LoadLibrary("hiredis_wrapper.dll");
		if (NULL == m_hDLL)
		{
			DWORD dw = GetLastError();
			std::string strTran;
			std::string strDebug("couldn't load hiredis_wrapper.dll, error=");
			strDebug += sof_string::itostr((uint64_t)dw, strTran);

			char szCurrDir[260];
			GetCurrentDirectory(260, szCurrDir);
			szCurrDir[260 - 1] = '\0';

			strDebug += ", current directory=[";
			strDebug += szCurrDir;
			strDebug += "]";

			EzLog::e(ftag, strDebug);

			return -1;
		}

		// Load functions addresses.
		//
		F_Dll_redisConnectWithTimeout = (pfDll_redisConnectWithTimeout)GetProcAddress(m_hDLL, "Dll_redisConnectWithTimeout");
		if (NULL == F_Dll_redisConnectWithTimeout)
		{
			DWORD dw = GetLastError();
			std::string strTran;
			std::string strDebug("couldn't load hiredis_wrapper.dll in Dll_redisConnectWithTimeout(), error=");
			strDebug += sof_string::itostr((uint64_t)dw, strTran);
			EzLog::e(ftag, strDebug);

			FreeHiredisLibrary();
			return -1;
		}

		//
		F_Dll_redisFree = (pfDll_redisFree)GetProcAddress(m_hDLL, "Dll_redisFree");
		if (NULL == F_Dll_redisFree)
		{
			DWORD dw = GetLastError();
			std::string strTran;
			std::string strDebug("couldn't load hiredis_wrapper.dll in Dll_redisFree(), error=");
			strDebug += sof_string::itostr((uint64_t)dw, strTran);
			EzLog::e(ftag, strDebug);

			FreeHiredisLibrary();
			return -1;
		}

		//
		F_DLL_redisCmd = (pfDLL_redisCmd)GetProcAddress(m_hDLL, "DLL_redisCmd");
		if (NULL == F_DLL_redisCmd)
		{
			DWORD dw = GetLastError();
			std::string strTran;
			std::string strDebug("couldn't load hiredis_wrapper.dll in DLL_redisCmd(), error=");
			strDebug += sof_string::itostr((uint64_t)dw, strTran);
			EzLog::e(ftag, strDebug);

			FreeHiredisLibrary();
			return -1;
		}

		//
		F_DLL_redisCommandArgv = (pfDLL_redisCommandArgv)GetProcAddress(m_hDLL, "DLL_redisCommandArgv");
		if (NULL == F_DLL_redisCommandArgv)
		{
			DWORD dw = GetLastError();
			std::string strTran;
			std::string strDebug("couldn't load hiredis_wrapper.dll in DLL_redisCommandArgv(), error=");
			strDebug += sof_string::itostr((uint64_t)dw, strTran);
			EzLog::e(ftag, strDebug);

			FreeHiredisLibrary();
			return -1;
		}

		//
		F_DLL_freeReplyObject = (pfDLL_freeReplyObject)GetProcAddress(m_hDLL, "DLL_freeReplyObject");
		if (NULL == F_DLL_freeReplyObject)
		{
			DWORD dw = GetLastError();
			std::string strTran;
			std::string strDebug("couldn't load hiredis_wrapper.dll in DLL_freeReplyObject(), error=");
			strDebug += sof_string::itostr((uint64_t)dw, strTran);
			EzLog::e(ftag, strDebug);

			FreeHiredisLibrary();
			return -1;
		}
	}
	catch (...)
	{
		DWORD dw = GetLastError();
		std::string strTran;
		std::string strDebug("unkown exception, error=");
		strDebug += sof_string::itostr((uint64_t)dw, strTran);
		EzLog::e(ftag, strDebug);

		return -1;
	}
#else
	try
	{
		//加载动态链接库dll_AuthCheck.dll文件
		m_hDLL = dlopen("libhiredis_wrapper.so", RTLD_LAZY);
		if (NULL == m_hDLL)
		{
			std::string strDebug("couldn't load libhiredis_wrapper.so, error=");
			strDebug += dlerror();

			EzLog::e(ftag, strDebug);

			return -1;
		}

		// Load functions addresses.
		//
		F_Dll_redisConnectWithTimeout = (pfDll_redisConnectWithTimeout)dlsym(m_hDLL, "Dll_redisConnectWithTimeout");
		if (NULL == F_Dll_redisConnectWithTimeout)
		{
			std::string strDebug("couldn't load hiredis_wrapper.dll in Dll_redisConnectWithTimeout(), error=");
			strDebug += dlerror();
			EzLog::e(ftag, strDebug);

			FreeHiredisLibrary();
			return -1;
		}

		//
		F_Dll_redisFree = (pfDll_redisFree)dlsym(m_hDLL, "Dll_redisFree");
		if (NULL == F_Dll_redisFree)
		{
			std::string strDebug("couldn't load hiredis_wrapper.dll in Dll_redisFree(), error=");
			strDebug += dlerror();
			EzLog::e(ftag, strDebug);

			FreeHiredisLibrary();
			return -1;
		}

		//
		F_DLL_redisCmd = (pfDLL_redisCmd)dlsym(m_hDLL, "DLL_redisCmd");
		if (NULL == F_DLL_redisCmd)
		{
			std::string strDebug("couldn't load hiredis_wrapper.dll in DLL_redisCmd(), error=");
			strDebug += dlerror();
			EzLog::e(ftag, strDebug);

			FreeHiredisLibrary();
			return -1;
		}

		//
		F_DLL_redisCommandArgv = (pfDLL_redisCommandArgv)dlsym(m_hDLL, "DLL_redisCommandArgv");
		if (NULL == F_DLL_redisCommandArgv)
		{
			std::string strDebug("couldn't load hiredis_wrapper.dll in DLL_redisCommandArgv(), error=");
			strDebug += dlerror();
			EzLog::e(ftag, strDebug);

			FreeHiredisLibrary();
			return -1;
		}

		//
		F_DLL_freeReplyObject = (pfDLL_freeReplyObject)dlsym(m_hDLL, "DLL_freeReplyObject");
		if (NULL == F_DLL_freeReplyObject)
		{
			std::string strDebug("couldn't load hiredis_wrapper.dll in DLL_freeReplyObject(), error=");
			strDebug += dlerror();
			EzLog::e(ftag, strDebug);

			FreeHiredisLibrary();
			return -1;
		}
	}
	catch (...)
	{
		int iErr = errno;
		std::string strTran;
		std::string strDebug("unkown exception, error=");
		strDebug += sof_string::itostr(iErr, strTran);
		strDebug += " serror=";
		strDebug += strerror(iErr);
		EzLog::e(ftag, strDebug);

		return -1;
	}

#endif

	return 0;
}

/*
Unload hiredis wrapper dll.
*/
void Tgw_RedisHelper::FreeHiredisLibrary(void)
{
#ifdef _MSC_VER
	//卸载dll文件；
	if (NULL != m_hDLL)
	{
		FreeLibrary(m_hDLL);
	}
#else
	//卸载dll文件；
	if (NULL != m_hDLL)
	{
		dlclose(m_hDLL);
	}	
#endif

	m_hDLL = NULL;
	F_Dll_redisConnectWithTimeout = nullptr;
	F_Dll_redisFree = nullptr;
	F_DLL_redisCmd = nullptr;
	F_DLL_redisCommandArgv = nullptr;
	F_DLL_freeReplyObject = nullptr;
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
RedisReply Tgw_RedisHelper::RunCmd(const std::string& in_strRedisCmd,
	long long* out_pllRes, std::string* out_pstrRes, int* out_piStrLen,
	std::vector<StgwRedisReply>* out_pvectArray, size_t* out_pElemSize)
{
	static const string ftag("Tgw_RedisHelper::RunCmd() ");

	// 进行Redis操作
	string strRedisCmd(in_strRedisCmd);

	string strDebug;

	std::shared_ptr<Redis3_0Cnn_dll> predis = simutgw::g_redisPool.GetConnection();
	if (nullptr == predis)
	{
		std::string strDebug("nullptr redis");
		EzLog::e(ftag, strDebug);

		simutgw::g_redisPool.ReleaseConnection(predis);
		return RedisReply_error;
	}

	long long llRedisRes_ll = 0;
	std::string strRedisRes_str;
	int iRedisRes_StrLen = 0;
	std::vector<StgwRedisReply> vectRedisRes_Array;
	size_t stRedisRes_ElemSize = 0;

	RedisReply emCallRes = predis->Cmd(strRedisCmd, &llRedisRes_ll, &strRedisRes_str, &iRedisRes_StrLen,
		&vectRedisRes_Array, &stRedisRes_ElemSize);

	/*
	// 消息过滤
	if( !EzLog::LogLvlFilter(trace) )
	{
	string strDebug("PCB call  RedisName[");
	strDebug += simutgw::RedisName;
	strDebug += "] RedisCmd[";
	strDebug += g_RedisCMD;
	strDebug += "] Cmd[";
	strDebug += strRedisCmd;
	strDebug += "] Res[";
	strDebug += strRes;
	strDebug += "]";
	EzLog::Out( ftag, trace, strDebug );
	}
	*/

	switch (emCallRes)
	{
	case RedisReply_error:
		strDebug = "redis call error! Cmd[";
		strDebug += strRedisCmd;
		strDebug += "] Res[";
		strDebug += strRedisRes_str;
		strDebug += "]";

		EzLog::e(ftag, strDebug);

#ifdef _MSC_VER
		Sleep(30000);
#else		
		usleep(30000 * 1000L);
#endif

		exit(3);
		break;

	case RedisReply_nil:

		break;

	case RedisReply_string:
		if (nullptr != out_piStrLen)
		{
			*out_piStrLen = iRedisRes_StrLen;
		}

		if (nullptr != out_pstrRes)
		{
			*out_pstrRes = strRedisRes_str;
		}

		break;

	case RedisReply_array:
		if (nullptr != out_pElemSize)
		{
			*out_pElemSize = stRedisRes_ElemSize;
		}
		if (nullptr != out_pvectArray)
		{
			out_pvectArray->swap(vectRedisRes_Array);
		}
		break;

	case RedisReply_integer:
		if (nullptr != out_pllRes)
		{
			*out_pllRes = llRedisRes_ll;
		}
		break;

	case RedisReply_status:
		if (nullptr != out_piStrLen)
		{
			*out_piStrLen = iRedisRes_StrLen;
		}

		if (nullptr != out_pstrRes)
		{
			*out_pstrRes = strRedisRes_str;
		}
		break;

	default:
		strDebug = "Failed to execute command [";
		strDebug += strRedisCmd;
		strDebug += "] failure[case default=";

		std::string strNum;
		sof_string::itostr(emCallRes, strNum);
		strDebug += strNum;
		strDebug += "]";

		EzLog::e(ftag, strDebug);
		break;
	}

	simutgw::g_redisPool.ReleaseConnection(predis);
	return emCallRes;
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
RedisReply Tgw_RedisHelper::RunCmdArgv(const vector<string>& vctArgs,
	long long* out_pllRes, std::string* out_pstrRes, int* out_piStrLen,
	std::vector<StgwRedisReply>* out_pvectArray, size_t* out_pElemSize)
{
	static const string ftag("Tgw_RedisHelper::CmdArgv() ");

	// 进行Redis操作

	string strDebug;

	std::shared_ptr<Redis3_0Cnn_dll> predis = simutgw::g_redisPool.GetConnection();
	if (nullptr == predis)
	{
		std::string strDebug("nullptr redis");
		EzLog::e(ftag, strDebug);

		simutgw::g_redisPool.ReleaseConnection(predis);
		return RedisReply_error;
	}

	long long llRedisRes_ll = 0;
	std::string strRedisRes_str;
	int iRedisRes_StrLen = 0;
	std::vector<StgwRedisReply> vectRedisRes_Array;
	size_t stRedisRes_ElemSize = 0;

	RedisReply emCallRes = predis->CmdArgv(vctArgs, &llRedisRes_ll, &strRedisRes_str, &iRedisRes_StrLen,
		&vectRedisRes_Array, &stRedisRes_ElemSize);

	/*
	// 消息过滤
	if( !EzLog::LogLvlFilter(trace) )
	{
	string strDebug("PCB call  RedisName[");
	strDebug += simutgw::RedisName;
	strDebug += "] RedisCmd[";
	strDebug += g_RedisCMD;
	strDebug += "] Cmd[";
	strDebug += strRedisCmd;
	strDebug += "] Res[";
	strDebug += strRes;
	strDebug += "]";
	EzLog::Out( ftag, trace, strDebug );
	}
	*/

	switch (emCallRes)
	{
	case RedisReply_error:
		strDebug = "redis call error! Res[";
		strDebug += strRedisRes_str;
		strDebug += "]";

		EzLog::e(ftag, strDebug);

#ifdef _MSC_VER
		Sleep(30000);
#else		
		usleep(30000 * 1000L);
#endif
		exit(3);
		break;

	case RedisReply_nil:

		break;

	case RedisReply_string:
		if (nullptr != out_piStrLen)
		{
			*out_piStrLen = iRedisRes_StrLen;
		}

		if (nullptr != out_pstrRes)
		{
			*out_pstrRes = strRedisRes_str;
		}

		break;

	case RedisReply_array:
		if (nullptr != out_pElemSize)
		{
			*out_pElemSize = stRedisRes_ElemSize;
		}
		if (nullptr != out_pvectArray)
		{
			out_pvectArray->swap(vectRedisRes_Array);
		}
		break;

	case RedisReply_integer:
		if (nullptr != out_pllRes)
		{
			*out_pllRes = llRedisRes_ll;
		}
		break;

	case RedisReply_status:
		if (nullptr != out_piStrLen)
		{
			*out_piStrLen = iRedisRes_StrLen;
		}

		if (nullptr != out_pstrRes)
		{
			*out_pstrRes = strRedisRes_str;
		}
		break;

	default:
		strDebug = "Failed to execute failure[case default=";

		std::string strNum;
		sof_string::itostr(emCallRes, strNum);
		strDebug += strNum;
		strDebug += "]";

		EzLog::e(ftag, strDebug);
		break;
	}

	simutgw::g_redisPool.ReleaseConnection(predis);
	return emCallRes;
}

/*
判断Redis的返回结果
Params:
const RedisReply in_emReplyType :
enum RedisReply
{
RedisReply_error = -2, // REDIS_REPLY_ERROR 6
RedisReply_nil = -1, // REDIS_REPLY_NIL 4
RedisReply_string = 1, // REDIS_REPLY_STRING 1
RedisReply_array = 2, // REDIS_REPLY_ARRAY 2
RedisReply_integer = 3, // REDIS_REPLY_INTEGER 3
RedisReply_status = 5 // REDIS_REPLY_STATUS 5
};

Return :
true -- 操作成功
false -- 操作失败
*/
bool Tgw_RedisHelper::IsRedisCmdSuccess(const RedisReply in_emReplyType,
	const long long in_llRes, const string& in_strRedisRes)
{
	static const string ftag("Tgw_RedisHelper::IsRedisCmdSuccess() ");

	string strRedisValue = in_strRedisRes;

	switch (in_emReplyType)
	{
	case RedisReply_error:
	case RedisReply_nil:
		return false;

		break;

	case RedisReply_string:
	case RedisReply_array:
		return true;
		break;

	case RedisReply_integer:
		/*Integer reply, specifically:
1 if the timeout was set.
0 if key does not exist.*/
		if (0 == in_llRes)
		{
			return false;
		}

		return true;
		break;

	case RedisReply_status:
		if (0 == strRedisValue.length())
		{
			return false;
		}

		if (0 == Tgw_RedisHelper::g_RplyRes_OK.compare(strRedisValue))
		{
			return true;
		}
		else
		{
			return false;
		}
		break;

	default:
		return false;
		break;
	}

}

/*
查看redis的队列深度
Return:
>=0 -- 成功
-1 -- 失败
*/
int Tgw_RedisHelper::GetLength(const string& in_strQueueName)
{
	static const string ftag("Tgw_RedisHelper::GetLength() ");

	//
	std::vector<string> vctCmd;

	// 长度命令
	vctCmd.push_back("LLEN");
	vctCmd.push_back(in_strQueueName);

	//
	long long llRedisRes = 0;
	std::string strRedisRes;

	RedisReply emPcbCallRes = Tgw_RedisHelper::RunCmdArgv(vctCmd, &llRedisRes, &strRedisRes,
		nullptr, nullptr, nullptr);

	if (RedisReply::RedisReply_integer != emPcbCallRes)
	{
		// 执行失败
		string strDebug("Redis执行 LLEN ");
		strDebug += in_strQueueName;
		strDebug += " 错误，res = [";
		strDebug += strRedisRes;
		strDebug += "].";
		EzLog::e(ftag, strDebug);

		return -1;
	}

	return (int)llRedisRes;
}