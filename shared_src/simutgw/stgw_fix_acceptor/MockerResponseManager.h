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
配置json回报的管理类
*/
class MockerResponseManager
{
	//
	// member
	//
private:
	typedef std::map<std::string, std::string> mapResponse;
	// 回报配置
	// key为applid,遍历doc找到成交或撤单回报
	mapResponse m_Responses;
	// 
	boost::mutex m_mutex;

	// 配置json文件名示例：config/rsp/010.json
	// 路径前缀
	static const std::string strPathPrefix;
	// 文件名后缀
	static const std::string strNamePost;

	//
	// function
	//
public:
	MockerResponseManager();
	virtual ~MockerResponseManager();

	// 取相应的response
	// @param in_strApplId applid
	// @param out_strResponse 如果找到，则是对应的response
	// @return
	//  0 -- 成功
	//  -1 -- 失败
	int GetResponse(const std::string& in_strApplId,
		std::string& out_strResponse);


public:
	// 查找map中的response
	// @param in_strApplId applid
	// @param out_strResponse 如果找到，则是对应的response
	// @return
	//  0 -- 成功
	//  -1 -- 失败
	int FindResponseFromMemory(const std::string& in_strApplId,
		std::string& out_strResponse);

	// 载入文件中的response
	// @param in_strApplId applid
	// @return
	//  0 -- 成功
	//  -1 -- 失败
	// @note 文件名为applid.json
	int LoadResponseFromFile(const std::string& in_strApplId);

	// 增加response
	// @param in_strApplId applid
	// @param in_cDoc 读取的json response
	// @return
	//  0 -- 成功
	//  -1 -- 失败
	int AddResponse(const std::string& in_strApplId, const std::string& in_strContent);
};

#endif