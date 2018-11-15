#include "ClientMessageParser.h"

#include "tool_net/PacketAssembler.h"

using namespace std;

ClientMessageParser::ClientMessageParser()
{
}


ClientMessageParser::~ClientMessageParser()
{
}

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
int ClientMessageParser::ReceiveMessageParse( HandlerSocketMsg& handlerMsg, uint64_t cid,
	std::vector<uint8_t> const &in_data, struct simutgw::SocketUserMessage& out_msg )
{
	// static const string ftag( " ClientMessageParser::ReceiveMessageParse() " );

	out_msg.vecRevBuffer = in_data;

	int iRes = 0;
	iRes = PacketAssembler::RecvPackage( out_msg.vecRevBuffer, out_msg.vecRevDatas, false, false );
	if ( 0 > iRes )
	{
		// 解析包失败

	}
	else
	{
		// 添加到主buffer,并再次分包
		handlerMsg.AppendBuffer( cid, out_msg.vecRevBuffer, out_msg.vecRevDatas );
	}


	return 0;
}

/*
发送消息处理

Param :
const std::string& in_sendmsg : 待发送消息
std::string& out_msgpack : 已打包完毕消息

Return :
0 -- 处理成功
-1 -- 处理失败
*/
int ClientMessageParser::SendMessageParse( const std::string& in_sendmsg, std::string& out_msgpack )
{
	// static const string ftag( " ClientMessageParser::SendMessageParse() " );

	out_msgpack = in_sendmsg;

	return 0;
}