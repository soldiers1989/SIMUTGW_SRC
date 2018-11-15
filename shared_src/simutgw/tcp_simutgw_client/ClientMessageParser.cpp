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
������Ϣ����

Param :
HandlerSocketMsg& handlerMsg : ��Ϣ���桢��������
uint64_t cid : client id
std::vector<uint8_t> const &in_data : ������Ϣ
struct simutgw::SocketUserMessage& out_msg : ���������Ϣ

Return :
0 -- ����ɹ�
-1 -- ����ʧ��
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
		// ������ʧ��

	}
	else
	{
		// ��ӵ���buffer,���ٴηְ�
		handlerMsg.AppendBuffer( cid, out_msg.vecRevBuffer, out_msg.vecRevDatas );
	}


	return 0;
}

/*
������Ϣ����

Param :
const std::string& in_sendmsg : ��������Ϣ
std::string& out_msgpack : �Ѵ�������Ϣ

Return :
0 -- ����ɹ�
-1 -- ����ʧ��
*/
int ClientMessageParser::SendMessageParse( const std::string& in_sendmsg, std::string& out_msgpack )
{
	// static const string ftag( " ClientMessageParser::SendMessageParse() " );

	out_msgpack = in_sendmsg;

	return 0;
}