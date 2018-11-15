#include "MockerResponseManager.h"
#include "util/EzLog.h"
#include "util/FileHandler.h"

// 路径前缀
const std::string MockerResponseManager::strPathPrefix("config\\rsp\\");
// 文件名后缀
const std::string MockerResponseManager::strNamePost(".json");

MockerResponseManager::MockerResponseManager()
{
}


MockerResponseManager::~MockerResponseManager()
{
}


// 取相应的response
// @param in_strApplId applid
// @param out_strResponse 如果找到，则是对应的response
// @return
//  0 -- 成功
//  -1 -- 失败
int MockerResponseManager::GetResponse(const std::string& in_strApplId,
	std::string& out_strResponse)
{
	// static const std::string ftag("MockerResponseManager::GetResponse() ");

	boost::unique_lock<boost::mutex> locker(m_mutex);

	int iRes = 0;
	// 先找内存中是否有对应的response
	iRes = FindResponseFromMemory(in_strApplId, out_strResponse);
	if (-1 == iRes)
	{
		// 未找到
		// 读相应配置文件
		iRes = LoadResponseFromFile(in_strApplId);
		if (-1 == iRes)
		{
			// 读文件失败
			return -1;
		}
		else
		{
			//
		}

		// 再找一次
		iRes = FindResponseFromMemory(in_strApplId, out_strResponse);
	}
	else
	{
		// 找到
	}

	return iRes;
}

// 查找map中的response
// @param in_strApplId applid
// @param out_strResponse 如果找到，则是对应的response
// @return
//  0 -- 成功
//  -1 -- 失败
int MockerResponseManager::FindResponseFromMemory(const std::string& in_strApplId,
	std::string& out_strResponse)
{
	// static const std::string ftag("MockerResponseManager::FindResponseFromMemory() ");

	// 找内存中是否有对应的response
	mapResponse::iterator iterMap = m_Responses.begin();
	iterMap = m_Responses.find(in_strApplId);
	if (m_Responses.end() != iterMap)
	{
		// 找到key
		out_strResponse = iterMap->second;
		return 0;
	}
	else
	{
		// 未找到key
	}

	return -1;
}

// 载入文件中的response
// @param in_strApplId applid
// @return
//  0 -- 成功
//  -1 -- 失败
// @note 文件名为applid.json
int MockerResponseManager::LoadResponseFromFile(const std::string& in_strApplId)
{
	static const std::string ftag("MockerResponseManager::LoadResponseFromFile() ");
	
	std::string strFileName(strPathPrefix);
	strFileName += in_strApplId;
	strFileName += strNamePost;

	int iRes = 0;
	std::string strContent;
	// 读相应的配置文件
	FileHandler handler;
	iRes = handler.ReadFileContent(strFileName, strContent, nullptr);
	if (0 != iRes)
	{
		// 打开失败
		return -1;
	}

	rapidjson::Document doc;
	//rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
	if ((doc.Parse<0>(strContent.c_str())).HasParseError())
	{
		//  解析失败
		std::string strError(strFileName);
		strError += "不是json格式";
		EzLog::e(ftag, strError);
		return -1;
	}

	AddResponse(in_strApplId, strContent);

	return 0;
}

// 增加response
// @param in_strApplId applid
// @param in_cDoc 读取的json response
// @return
//  0 -- 成功
//  -1 -- 失败
int MockerResponseManager::AddResponse(const std::string& in_strApplId,
	const std::string& in_strContent)
{
	// static const std::string ftag("MockerResponseManager::AddResponse() ");

	mapResponse::iterator iterMap = m_Responses.begin();
	iterMap = m_Responses.find(in_strApplId);
	if (m_Responses.end() != iterMap)
	{
		m_Responses[in_strApplId] = in_strContent;
	}
	else
	{
		m_Responses.insert(make_pair(in_strApplId, in_strContent));
	}

	return 0;
}