#include "MockerResponseManager.h"
#include "util/EzLog.h"
#include "util/FileHandler.h"

// ·��ǰ׺
const std::string MockerResponseManager::strPathPrefix("config\\rsp\\");
// �ļ�����׺
const std::string MockerResponseManager::strNamePost(".json");

MockerResponseManager::MockerResponseManager()
{
}


MockerResponseManager::~MockerResponseManager()
{
}


// ȡ��Ӧ��response
// @param in_strApplId applid
// @param out_strResponse ����ҵ������Ƕ�Ӧ��response
// @return
//  0 -- �ɹ�
//  -1 -- ʧ��
int MockerResponseManager::GetResponse(const std::string& in_strApplId,
	std::string& out_strResponse)
{
	// static const std::string ftag("MockerResponseManager::GetResponse() ");

	boost::unique_lock<boost::mutex> locker(m_mutex);

	int iRes = 0;
	// �����ڴ����Ƿ��ж�Ӧ��response
	iRes = FindResponseFromMemory(in_strApplId, out_strResponse);
	if (-1 == iRes)
	{
		// δ�ҵ�
		// ����Ӧ�����ļ�
		iRes = LoadResponseFromFile(in_strApplId);
		if (-1 == iRes)
		{
			// ���ļ�ʧ��
			return -1;
		}
		else
		{
			//
		}

		// ����һ��
		iRes = FindResponseFromMemory(in_strApplId, out_strResponse);
	}
	else
	{
		// �ҵ�
	}

	return iRes;
}

// ����map�е�response
// @param in_strApplId applid
// @param out_strResponse ����ҵ������Ƕ�Ӧ��response
// @return
//  0 -- �ɹ�
//  -1 -- ʧ��
int MockerResponseManager::FindResponseFromMemory(const std::string& in_strApplId,
	std::string& out_strResponse)
{
	// static const std::string ftag("MockerResponseManager::FindResponseFromMemory() ");

	// ���ڴ����Ƿ��ж�Ӧ��response
	mapResponse::iterator iterMap = m_Responses.begin();
	iterMap = m_Responses.find(in_strApplId);
	if (m_Responses.end() != iterMap)
	{
		// �ҵ�key
		out_strResponse = iterMap->second;
		return 0;
	}
	else
	{
		// δ�ҵ�key
	}

	return -1;
}

// �����ļ��е�response
// @param in_strApplId applid
// @return
//  0 -- �ɹ�
//  -1 -- ʧ��
// @note �ļ���Ϊapplid.json
int MockerResponseManager::LoadResponseFromFile(const std::string& in_strApplId)
{
	static const std::string ftag("MockerResponseManager::LoadResponseFromFile() ");
	
	std::string strFileName(strPathPrefix);
	strFileName += in_strApplId;
	strFileName += strNamePost;

	int iRes = 0;
	std::string strContent;
	// ����Ӧ�������ļ�
	FileHandler handler;
	iRes = handler.ReadFileContent(strFileName, strContent, nullptr);
	if (0 != iRes)
	{
		// ��ʧ��
		return -1;
	}

	rapidjson::Document doc;
	//rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
	if ((doc.Parse<0>(strContent.c_str())).HasParseError())
	{
		//  ����ʧ��
		std::string strError(strFileName);
		strError += "����json��ʽ";
		EzLog::e(ftag, strError);
		return -1;
	}

	AddResponse(in_strApplId, strContent);

	return 0;
}

// ����response
// @param in_strApplId applid
// @param in_cDoc ��ȡ��json response
// @return
//  0 -- �ɹ�
//  -1 -- ʧ��
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