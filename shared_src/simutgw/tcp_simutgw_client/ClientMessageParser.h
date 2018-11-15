#ifndef __CLIENT_MESSAGE_PARSER_H__
#define __CLIENT_MESSAGE_PARSER_H__

#include <string>
#include <vector>

#include "config/conf_net_msg.h"

#include "socket_conn_manage_base/HandlerSocketMsg.h"

/*
�ͻ�����Ϣ������
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
	static int ReceiveMessageParse( HandlerSocketMsg& handlerMsg,
		uint64_t cid, std::vector<uint8_t> const &in_data,
	struct simutgw::SocketUserMessage& out_msg );

	/*
	������Ϣ����

	Param :
	const std::string& in_sendmsg : ��������Ϣ
	std::string& out_msgpack : �Ѵ�������Ϣ

	Return :
	0 -- ����ɹ�
	-1 -- ����ʧ��
	*/
	static int SendMessageParse( const std::string& in_sendmsg, std::string& out_msgpack );
};

#endif
