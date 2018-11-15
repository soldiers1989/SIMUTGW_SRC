#ifndef __CLIENT_MESSAGE_PARSER_H__
#define __CLIENT_MESSAGE_PARSER_H__

#include <string>
#include <vector>

#include "config/conf_net_msg.h"

#include "socket_conn_manage_base/HandlerSocketMsg.h"

/*
客户端消息处理类
*/
class ClientMessageParser
{
	//
	// Functions
	//
public:
	ClientMessageParser();
	virtual ~ClientMessageParser();

	/*
	接收消息处理

	Param :
	HandlerSocketMsg& handlerMsg : 消息缓存、处理类句柄
	uint64_t cid : client id
	std::vector<uint8_t> const &in_data : 传入消息
	struct simutgw::SocketUserMessage& out_msg : 处理完毕消息

	Return :
	0 -- 处理成功
	-1 -- 处理失败
	*/
	static int ReceiveMessageParse( HandlerSocketMsg& handlerMsg,
		uint64_t cid, std::vector<uint8_t> const &in_data,
	struct simutgw::SocketUserMessage& out_msg );

	/*
	发送消息处理

	Param :
	const std::string& in_sendmsg : 待发送消息
	std::string& out_msgpack : 已打包完毕消息

	Return :
	0 -- 处理成功
	-1 -- 处理失败
	*/
	static int SendMessageParse( const std::string& in_sendmsg, std::string& out_msgpack );
};

#endif
