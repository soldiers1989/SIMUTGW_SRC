#include "CurlHttpMultiFileUpload.h"

#include <stdio.h>

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

using namespace std;

static const char cszExpect_buf[] = "Expect:";

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;

	std::string* str = dynamic_cast<std::string*>((std::string *)userp);
	if (NULL == str || NULL == contents)
	{
		return -1;
	}

	char* pData = (char*)contents;
	str->append(pData, size * nmemb);
	return realsize;
}

CurlHttpMultiFileUpload::CurlHttpMultiFileUpload()
	:m_scl(keywords::channel = "SimutgwTcpServer")
{

}

CurlHttpMultiFileUpload::~CurlHttpMultiFileUpload()
{

}

int CurlHttpMultiFileUpload::UpLoadFiles(const std::string& in_strUploadLink,
	const std::string& in_engineId,
	const std::string& in_strDay, const std::vector<std::string>& in_vctFiles,
	std::string& out_strHttpRes, std::string& out_strErrorMsg)
{
	static const string ftag("CurlHttpMultiFileUpload::UpLoadFiles() ");

	if (in_strUploadLink.empty())
	{
		return -1;
	}

	try
	{
		std::string strHttpRes;

		CURL *curl;
		CURLcode res;

		curl = curl_easy_init();

		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: multipart/form-data");

		/* initialize custom header list (stating that Expect: 100-continue is not
		wanted */
		headers = curl_slist_append(headers, cszExpect_buf);

		/* pass our list of custom made headers */
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_mime *multipart = curl_mime_init(curl);
		curl_mimepart *part = nullptr;

		part = curl_mime_addpart(multipart);
		curl_mime_name(part, "NODE_INFO");
		curl_mime_data(part, "master", CURL_ZERO_TERMINATED);

		part = curl_mime_addpart(multipart);
		curl_mime_name(part, "OWNED_MODULE");
		curl_mime_data(part, "MATCH", CURL_ZERO_TERMINATED);

		part = curl_mime_addpart(multipart);
		curl_mime_name(part, "OWNED_ID");
		curl_mime_data(part, in_engineId.c_str(), CURL_ZERO_TERMINATED);

		part = curl_mime_addpart(multipart);
		curl_mime_name(part, "FILE_TIME");
		curl_mime_data(part, in_strDay.c_str(), CURL_ZERO_TERMINATED);

		size_t i = 0;
		for (i = 0; i < in_vctFiles.size(); ++i)
		{
			// add file part
			part = curl_mime_addpart(multipart);
			curl_mime_name(part, "file");
			curl_mime_filedata(part, in_vctFiles[i].c_str());
		}

		/* what URL that receives this POST */
		curl_easy_setopt(curl, CURLOPT_URL, in_strUploadLink.c_str());

		/* ask libcurl to show us the verbose output */
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Set the form info */
		curl_easy_setopt(curl, CURLOPT_MIMEPOST, multipart);

		/* send all data to this function  */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strHttpRes);

		/* complete connection within 10 seconds */
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
		/*the maximum time in seconds that you allow the libcurl transfer operation to take.
		Normally, name lookups can take a considerable time and limiting operations to less than a few minutes risk aborting perfectly normal operations.
		This option may cause libcurl to use the SIGALRM signal to timeout system calls.*/
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 500L);

		/* post away! */
		res = curl_easy_perform(curl);

		if (CURLE_OK != res)
		{
			// 获取详细错误信息
			out_strErrorMsg = curl_easy_strerror(res);
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "curl_easy_perform() failed:" << out_strErrorMsg;
		}

		/* always cleanup */
		curl_easy_cleanup(curl);

		/* free the post data again */
		curl_mime_free(multipart);

		curl_slist_free_all(headers); /* free the header list */

		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "http response:\n" << strHttpRes;

		out_strHttpRes = strHttpRes;

		if (CURLE_OK != res)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
}