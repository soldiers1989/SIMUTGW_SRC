#include "EchoHandler.h"
#include "simutgw/tcp_simutgw_server/ProcSocketMsg.h"

#include "tool_net/PacketAssembler.h"

#include "simutgw/stgw_config/g_values_biz.h"
#include "simutgw/stgw_config/g_values_net.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"

string EchoHandler::m_strPing("Ping");
string EchoHandler::m_strPong("Pong");

EchoHandler::EchoHandler(void)
	:m_scl(keywords::channel = "EchoHandler")
{
}

EchoHandler::~EchoHandler(void)
{
}

void EchoHandler::OnServerError(int errorCode)
{
}

void EchoHandler::OnNewConnection(uint64_t cid, struct ConnectionInformation const & c)
{
	static const string ftag("EchoHandler::OnNewConnection() ");

	BOOST_LOG_SEV(m_scl, trivial::warning) << ftag
		<< "new id=" << cid << ", hostname="
		<< c.strRemoteHostName << ", ip=" << c.strRemoteIpAddress
		<< ", port=" << c.remotePortNumber;
}

void EchoHandler::OnReceiveData(uint64_t cid, std::vector<uint8_t> const &data)
{
	static const string ftag("EchoHandler::OnReceiveData() ");

	// 已分包部分
	std::vector<std::shared_ptr<simutgw::NET_PACKAGE>> vecRevDatas;

	// 添加到主buffer,并再次分包
	m_handlerMsg.AppendBuffer(cid, data, vecRevDatas);

	string strReport;
	uint64_t ui64ReportIndex = 0;
	for ( size_t i = 0; i < vecRevDatas.size(); ++i )
	{
		/* 先取ReportIndex */
		ProcSocketMsg::ProcMsg(vecRevDatas[i]->data, ui64ReportIndex, strReport);
		EzLog::i(ftag, vecRevDatas[i]->data);
		simutgw::g_SocketIOCPServer->Send(cid, 1, strReport);
		EzLog::i(ftag, strReport);
	}


	//EzLog::i(ftag, strDebug);
	//std::vector<uint8_t> vecSend;
	//for (size_t i = 0; i < m_strPong.size(); ++i)
	//{
	//	vecSend.push_back(m_strPong[i]);
	//}
	//if (0 == m_strPing.compare(strDebug))
	//{
	//	GetIocpServer().Send(cid, vecSend);
	//}
}

void EchoHandler::OnSentData(uint64_t cid, uint64_t byteTransferred)
{
	static const string ftag("EchoHandler::OnSentData() ");

	string strTran;
	TimeStringUtil::GetTimeStamp_intstr(strTran);
	EzLog::i(ftag, strTran);
	//string strTran;
	//string strDebug = "id=";
	//strDebug += sof_string::itostr(cid, strTran);
	//EzLog::i(ftag, strDebug);
}

void EchoHandler::OnClientDisconnect(uint64_t cid, int errorcode)
{
	static const string ftag("EchoHandler::OnClientDisconnect() ");

	string strTran;
	string strDebug = "id=";
	strDebug += sof_string::itostr(cid, strTran);
	EzLog::i(ftag, strDebug);

	try
	{
		GetServiceOwner().Shutdown(cid, SD_SEND);

		GetServiceOwner().Disconnect(cid);
	}
	catch ( exception const & e )
	{
		EzLog::ex(ftag, e);
		return;
	}
}

void EchoHandler::OnDisconnect(uint64_t cid, int errorcode)
{
	static const string ftag("EchoHandler::OnDisconnect() ");

	string strTran;
	string strDebug = "id=";
	strDebug += sof_string::itostr(cid, strTran);
	strDebug += " errorcode=";
	strDebug += sof_string::itostr(errorcode, strTran);
	EzLog::i(ftag, strDebug);
}

void EchoHandler::OnServerClose(int errorCode)
{
	// OnServerClose时一般是程序退出时，因为是多线程程序，可能会引用到已经释放的资源
	//static const string ftag("EchoHandler::OnServerClose() ");

	//string strTran;
	//string strDebug = " errorcode=";
	//strDebug += sof_string::itostr(errorCode, strTran);
	//EzLog::i(ftag, strDebug);
}