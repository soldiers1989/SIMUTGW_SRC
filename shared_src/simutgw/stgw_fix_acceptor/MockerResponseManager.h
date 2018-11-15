#ifndef __MOCKER_RESPONSE_H__
#define __MOCKER_RESPONSE_H__

#include <map>
#include <string>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "boost/thread/mutex.hpp"

/*
����json�ر��Ĺ�����
*/
class MockerResponseManager
{
	//
	// member
	//
private:
	typedef std::map<std::string, std::string> mapResponse;
	// �ر�����
	// keyΪapplid,����doc�ҵ��ɽ��򳷵��ر�
	mapResponse m_Responses;
	// 
	boost::mutex m_mutex;

	// ����json�ļ���ʾ����config/rsp/010.json
	// ·��ǰ׺
	static const std::string strPathPrefix;
	// �ļ�����׺
	static const std::string strNamePost;

	//
	// function
	//
public:
	MockerResponseManager();
	virtual ~MockerResponseManager();

	// ȡ��Ӧ��response
	// @param in_strApplId applid
	// @param out_strResponse ����ҵ������Ƕ�Ӧ��response
	// @return
	//  0 -- �ɹ�
	//  -1 -- ʧ��
	int GetResponse(const std::string& in_strApplId,
		std::string& out_strResponse);


public:
	// ����map�е�response
	// @param in_strApplId applid
	// @param out_strResponse ����ҵ������Ƕ�Ӧ��response
	// @return
	//  0 -- �ɹ�
	//  -1 -- ʧ��
	int FindResponseFromMemory(const std::string& in_strApplId,
		std::string& out_strResponse);

	// �����ļ��е�response
	// @param in_strApplId applid
	// @return
	//  0 -- �ɹ�
	//  -1 -- ʧ��
	// @note �ļ���Ϊapplid.json
	int LoadResponseFromFile(const std::string& in_strApplId);

	// ����response
	// @param in_strApplId applid
	// @param in_cDoc ��ȡ��json response
	// @return
	//  0 -- �ɹ�
	//  -1 -- ʧ��
	int AddResponse(const std::string& in_strApplId, const std::string& in_strContent);
};

#endif