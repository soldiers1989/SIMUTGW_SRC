#ifndef __CLIENTER_H__
#define __CLIENTER_H__

#include <string>

#include "boost/thread/mutex.hpp"

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

#ifdef _MSC_VER
#include "simutgw/win_iocp_simutgw_client/ClientHandler.h"
#include "simutgw/win_iocp_simutgw_client/ClientSocket.h"
#else
#include "linux_epoll_client/EpollClient_Core.h"
#include "simutgw/linux_tcp_simutgw_client/LnxTcpHandler_Simutgw_Client.h"
#endif

#include "ClientReconnectGapper.h"

/*
客户端业务程序
*/
class Clienter : public SocketServiceBase
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	struct SendBuffer
	{
		uint64_t SendSeq;
		int Response;
		std::string Msg;
		std::string respText;
	};

	// 
#ifdef _MSC_VER
	std::shared_ptr<IocpEventHandler> m_handler;
	std::shared_ptr<ClientSocket> m_clientSocket;
#else
	std::shared_ptr<EpollClient_EventHandler> m_handler;
	std::shared_ptr<EpollClient_Core> m_clientSocket;
#endif

	// server ip
	std::string m_strServerIp;
	// server port
	u_short m_uiServerPort;

	// 是否已连线
	// true -- 连线
	// flase -- 断线
	bool m_bIsConnected;

	ClientReconnectGapper reconnGapper;

	// mutex for client reconnect.
	boost::mutex m_mutexReconnect;

	uint64_t m_ui64MsgSeq;
	// mutex for message sequence.
	boost::mutex m_mutexSeq;

	// mutex for response feed back.
	boost::mutex m_mutexResp;

	// 已发送消息列表
	map<uint64_t, struct SendBuffer> m_mapSendBuffer;

	// 收到返回消息总数
	uint64_t m_ui64ResponseReceived;

	// 收到的消息 成功总数
	uint64_t m_ui64ResponseSuccess;
	// 收到的消息 失败总数
	uint64_t m_ui64ResponseFailed;

	//
	// Functions
	//
public:
	Clienter();
	virtual ~Clienter();

	/*
	获取发送消息的sequence
	*/
	uint64_t GetMsgSeq(void)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutexSeq);

		return m_ui64MsgSeq++;
	}

	/*
	设置客户端登录状态

	@param bool bIsConnected : 是否已连线
	true -- 连线
	false -- 断线
	*/
	void SetConnectStatus(bool bIsConnected)
	{
		m_bIsConnected = bIsConnected;

		if (!m_bIsConnected)
		{
			reconnGapper.RecordDisconnectTime();
		}
	}

	/*
	与server建立socket tcp连接

	Param :
	const string& in_ip : Server Ip address.
	const u_short in_port : Server Listen port.

	Return :
	0 -- 连接成功
	-1 -- 连接失败
	*/
	int Connect(const string& in_ip, const u_short in_port);

	/*
	与server重新建立socket tcp连接

	@param :

	@return :
	0 -- 连接成功
	-1 -- 连接失败
	*/
	int Reconnect(void);

	//!***************************************************************************
	//! @details
	//! Fully disconnect from a connected client. Once all outstanding sends
	//! are completed, a corresponding OnDisconnect callback will be invoked.
	//!
	//! check class SocketServiceBase::Disconnect()
	//!***************************************************************************
	virtual void Disconnect(uint64_t cid);

	//!***************************************************************************
	//! @details
	//! Shutdown certain operation on the socket.
	//!
	//! check class SocketServiceBase::Shutdown()
	//!***************************************************************************
	virtual void Shutdown(uint64_t cid, int how);

	/*
	send message to server

	@param const std::string msgKeyValue : message key value
	const std::string* in_pstrOrigseqnum : 回报消息用 原始序号
	@param rapidjson::Document& docValue : value message
	@param trivial::severity_level in_logLvl : log level
	@param bool bNeedLog : 是否需要记日志
	*/
	int SendMsgToServer(const std::string& msgKeyValue, const std::string* in_pstrOrigseqnum,
		rapidjson::Document& in_docValue, trivial::severity_level in_logLvl, bool bNeedLog = true);

	/*
	向Server注册
	@Return
	0 -- 注册成功
	-1 -- 注册失败
	*/
	int SendMsg_RegisterToServer();

	/*
	从Server获取启动参数
	@Return
	0 -- 获取成功
	-1 -- 获取失败
	*/
	int SendMsg_GetParamFromServer();

	/*
	从Server获取MatchRule

	@Return
	0 -- 成功
	-1 -- 失败
	*/
	int SendMsg_GetMatchRuleContentFromServer(std::vector<uint64_t>& in_vctNeedFetchRule_Sh,
		std::vector<uint64_t>& in_vctNeedFetchRule_Sz);

	/*
	处理从server收到的消息

	@param
	const std::string& in_cstrMsg 业务消息

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcessReceivedData(const std::string& in_cstrMsg);

protected:
	/*
	处理注册回报消息
	@param
	rapidjson::Value& in_jsonvalue json消息中的value对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcReqMsg_RegisterReport(rapidjson::Value& in_jsonvalue);

	/*
	处理获取参数回报消息
	@param
	rapidjson::Value& in_jsonvalue json消息中的value对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcReqMsg_ParamReport(rapidjson::Value& in_jsonvalue);

	/*
	处理消息 服务端心跳检查
	@param
	rapidjson::Value& in_jsonvalue json消息中的value对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcReqMsg_EngineStateCheck(const std::string& in_cstrOrigseqnum,
		rapidjson::Value& in_jsonvalue);

	/*
	处理重启请求消息
	@param
	const std::string& in_cstrOrigseqnum 消息序号
	rapidjson::Value& in_jsonvalue json消息中的value对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcReqMsg_RestartRequest(const std::string& in_cstrOrigseqnum,
		rapidjson::Value& in_jsonvalue);

	/*
	处理更改模式请求消息
	@param
	const std::string& in_cstrOrigseqnum 消息序号
	rapidjson::Value& in_jsonvalue json消息中的value对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcReqMsg_SwitchModeRequest(const std::string& in_cstrOrigseqnum,
		rapidjson::Value& in_jsonvalue);

	/*
	处理 通道策略改变请求 消息
	@param
	rapidjson::Value& in_jsonvalue json消息中的value对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcReqMsg_ChangeLinkStrategy(const std::string& in_cstrOrigseqnum,
		rapidjson::Value& in_jsonvalue);

	/*
	获取 通道策略改变内容

	@param rapidjson::Value& in_jsonvalue : json消息中的value对象
	@param rapidjson::Document& io_docResp : 回报数组
	@param rapidjson::Document::AllocatorType& in_allocResp: 回报数组

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int Get_ChangeLinkStrategy(rapidjson::Value& in_jsonvalue, rapidjson::Document& io_docResp,
		rapidjson::Document::AllocatorType& in_allocResp);

	/*
	获取 通道和成交规则配置的绑定关系

	@param rapidjson::Value& in_jsonvalue : json消息中的value对象
	@param rapidjson::Document& io_docResp : 回报数组
	@param rapidjson::Document::AllocatorType& in_allocResp: 回报数组

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int Get_LinkMatchRuleRelations(rapidjson::Value& in_jsonvalue, rapidjson::Document& io_docResp,
		rapidjson::Document::AllocatorType& in_allocResp);

	/*
	取系统参数信息

	@param
	rapidjson::Value& in_jsonvalue json消息中的sysConfig对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int GetSysConfig(rapidjson::Value& in_jsonvalue);

	/*
	取深圳接口参数

	@param
	rapidjson::Value& in_jsonvalue json消息中的szConn对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int GetSzConnConfig(rapidjson::Value& in_jsonvalue);

	/*
	取上海接口参数

	@param
	rapidjson::Value& in_jsonvalue json消息中的shConn对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int GetShConnConfig(rapidjson::Value& in_jsonvalue);

	/*
	处理 Etf信息 消息
	@param
	rapidjson::Value& in_jsonvalue json消息中的value对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcReqMsg_Etfinfo(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue);

	/*
	处理 成交配置改变 消息
	@param
	rapidjson::Value& in_jsonvalue json消息中的value对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcReqMsg_MatchRule(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue);

	/*
	处理 通道对应成交规则配置改变请求 消息
	@param
	rapidjson::Value& in_jsonvalue json消息中的value对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcReqMsg_ChangeLinkRules(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue);

	/*
	处理 生成清算文件 消息
	@param
	rapidjson::Value& in_jsonvalue json消息中的value对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcReqMsg_SettleAccounts(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue);

	/*
	处理 日终收盘 消息
	@param
	rapidjson::Value& in_jsonvalue json消息中的value对象

	@Return
	0 -- 处理成功
	-1 -- 失败
	*/
	int ProcReqMsg_DayEnd(const std::string& in_cstrOrigseqnum, rapidjson::Value& in_jsonvalue);
};

#endif