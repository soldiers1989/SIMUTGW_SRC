#ifndef __HANDLER_SOCKET_MSG_BINARY_H__
#define __HANDLER_SOCKET_MSG_BINARY_H__

#include <vector>
#include <string>
#include "boost/thread/mutex.hpp"

#include "tool_net_binary/BinaryNetPack.h"

/*
socket�û�����Ϣ����
������
δ�ְ����ֺ��ѷְ�����
*/
namespace simutgw
{
	struct SocketUserMessage_Binary
	{
		boost::mutex msgMutex;

		// δ�ְ�����
		std::vector<uint8_t> vecRevBuffer;

		// �ѷְ�����
		std::vector<std::shared_ptr<struct simutgw::BinaryNetPack>> vecRevDatas;
	};
};

/*
socket message����Ϣ���桢�����
*/
class HandlerSocketMsg_Binary
{
	/*
		member
		*/
private:
	boost::mutex m_cidMsgMutex;

	// ��Ϣ����
	std::map<uint64_t, struct simutgw::SocketUserMessage_Binary> m_mapCidMsg;

public:
	HandlerSocketMsg_Binary();
	virtual ~HandlerSocketMsg_Binary();

	// ��δ�����buffer���뵽��buffer,�ٷְ�
	void AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf, std::vector<std::shared_ptr<struct simutgw::BinaryNetPack> >& vecApdPack);

	// ���Ƴ�id���õ�buffer
	bool RemoveId(uint64_t cid);

	// �Ƴ�����id���õ�buffer
	void RemoveAllId(void);

private:

	// ��δ�����buffer���뵽��buffer
	void AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf);
	
	// ����buffer�еİ��ٷ�һ��
	void PacketAssem(uint64_t cid, std::vector<std::shared_ptr<struct simutgw::BinaryNetPack> >& vecApdPack);

	// �������buffer��������10*1024ʱ�����
	void TryClearBuffer(uint64_t cid);
};

#endif
