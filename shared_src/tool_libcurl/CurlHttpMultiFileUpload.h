#ifndef __CURL_HTTP_MULTI_FILE_UPLOAD_H__
#define __CURL_HTTP_MULTI_FILE_UPLOAD_H__

#include "util/EzLog.h"

#include <curl/curl.h>

/*
利用libcurl 进行多文件http上传
*/
class CurlHttpMultiFileUpload
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	//
	// Functions
	//
public:
	CurlHttpMultiFileUpload();
	~CurlHttpMultiFileUpload();

	int UpLoadFiles(const std::string& in_strUploadLink,
		const std::string& in_engineId,
		const std::string& in_strDay, const std::vector<std::string>& in_vctFiles,
		std::string& out_strHttpRes, std::string& out_strErrorMsg);
};

#endif