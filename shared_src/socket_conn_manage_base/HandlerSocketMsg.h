#ifndef __HANDLER_SOCKET_MSG_H__
#define __HANDLER_SOCKET_MSG_H__

#include "config/conf_net_msg.h"

/*
socket message����Ϣ���桢�����
*/
class HandlerSocketMsg
{
	/*
		member
		*/
private:
	boost::mutex m_cidMsgMutex;

	// ��Ϣ����
	std::map<uint64_t, struct simutgw::SocketUserMessage> m_mapCidMsg;

public:
	HandlerSocketMsg();
	virtual ~HandlerSocketMsg();

	// ��δ�����buffer���뵽��buffer,�ٷְ�
	void AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf, std::vector<std::shared_ptr<simutgw::NET_PACKAGE> >& vecApdPack);

	// ���Ƴ�id���õ�buffer
	bool RemoveId(uint64_t cid);

	// �Ƴ�����id���õ�buffer
	void RemoveAllId(void);

private:
	// ��δ�����buffer���뵽��buffer������buffer�еİ��ٷ�һ��
	void PacketAssem(uint64_t cid, const std::vector<uint8_t>& vecApdBuf,
		std::vector<std::shared_ptr<simutgw::NET_PACKAGE> >& vecApdPack);
};

#endif
