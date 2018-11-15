#include "EventBaseHandler.h"

#include <string.h>

#include "tool_string/sof_string.h"
#include "util/EzLog.h"

#include "socket_conn_manage_base/ConnectionInformation.h"

using namespace std;

EventBaseHandler::EventBaseHandler(void)
{
}

EventBaseHandler::~EventBaseHandler(void)
{
}

void EventBaseHandler::OnReceiveData(uint64_t cid, std::vector<uint8_t> const &data)
{
	static const string ftag("EventBaseHandler::OnReceiveData() ");

	string strTran;
	string strDebug = "id=";
	strDebug += sof_string::itostr(cid, strTran);
	strDebug += " data=[";
	std::vector<uint8_t>::const_iterator it;
	for (it = data.begin(); it != data.end(); ++it)
	{
		strDebug += *it;
		strDebug += ",";
	}
	strDebug += "]";
	EzLog::i(ftag, strDebug);
}

void EventBaseHandler::OnSentData(uint64_t cid, uint64_t byteTransferred)
{
	static const string ftag("EventBaseHandler::OnSentData() ");

	string strTran;
	string strDebug = "id=";
	strDebug += sof_string::itostr(cid, strTran);
	strDebug += ", byteTransferred=";
	strDebug += sof_string::itostr(byteTransferred, strTran);
	EzLog::t(ftag, strDebug);
}

void EventBaseHandler::OnClientDisconnect(uint64_t cid, int errorcode)
{
	static const string ftag("EventBaseHandler::OnClientDisconnect() ");

	string strTran;
	string strDebug = "id=";
	strDebug += sof_string::itostr(cid, strTran);
	strDebug += ", error=";
	strDebug += sof_string::itostr(errorcode, strTran);
	strDebug += ", serror=";
	strDebug += strerror(errorcode);
	EzLog::i(ftag, strDebug);
}

void EventBaseHandler::OnDisconnect(uint64_t cid, int errorcode)
{
	static const string ftag("EventBaseHandler::OnDisconnect() ");

	string strTran;
	string strDebug = "id=";
	strDebug += sof_string::itostr(cid, strTran);
	strDebug += ", error=";
	strDebug += sof_string::itostr(errorcode, strTran);
	strDebug += ", serror=";
	strDebug += strerror(errorcode);
	EzLog::i(ftag, strDebug);
}

void EventBaseHandler::OnServerClose(int errorCode)
{
	// OnServerClose时一般是程序退出时，因为是多线程程序，可能会引用到已经释放的资源
	/*
	static const string ftag("EventBaseHandler::OnServerClose() ");

	string strTran;
	string strDebug = "error=";
	strDebug += sof_string::itostr(errorCode, strTran);
	strDebug += ", serror=";
	strDebug += strerror(errorCode);
	EzLog::i(ftag, strDebug);
	*/
}

void EventBaseHandler::OnServerError(int errorCode)
{
	static const string ftag("EventBaseHandler::OnServerError() ");

	string strTran;
	string strDebug = "error=";
	strDebug += sof_string::itostr(errorCode, strTran);
	strDebug += ", serror=";
	strDebug += strerror(errorCode);
	EzLog::i(ftag, strDebug);
}
