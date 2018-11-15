#include "HandlerSocketMsg_Binary.h"

#include "tool_net_binary/BinaryPacketAssembler.h"

HandlerSocketMsg_Binary::HandlerSocketMsg_Binary()
{
}


HandlerSocketMsg_Binary::~HandlerSocketMsg_Binary()
{
}

// ��δ�����buffer���뵽��buffer,�ٷְ�
void HandlerSocketMsg_Binary::AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf,
	std::vector<std::shared_ptr<struct simutgw::BinaryNetPack> >& vecApdPack)
{
	boost::unique_lock<boost::mutex> Locker(m_cidMsgMutex);

	AppendBuffer(cid, vecApdBuf);

	PacketAssem(cid, vecApdPack);

	TryClearBuffer(cid);
}

// ��δ�����buffer���뵽��buffer
void HandlerSocketMsg_Binary::AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf)
{
	boost::unique_lock<boost::mutex> Locker(m_mapCidMsg[cid].msgMutex);

	for (size_t st = 0; st < vecApdBuf.size(); ++st)
	{
		m_mapCidMsg[cid].vecRevBuffer.push_back(vecApdBuf[st]);
	}
}

// ����buffer�еİ��ٷ�һ��
void HandlerSocketMsg_Binary::PacketAssem(uint64_t cid, std::vector<std::shared_ptr<struct simutgw::BinaryNetPack> >& vecApdPack)
{
	boost::unique_lock<boost::mutex> Locker(m_mapCidMsg[cid].msgMutex);

	// param 3 = true;
	BinaryPacketAssembler::RecvPackage(m_mapCidMsg[cid].vecRevBuffer, vecApdPack, true);

}

// �������buffer��������10*1024ʱ�����
void HandlerSocketMsg_Binary::TryClearBuffer(uint64_t cid)
{
	boost::unique_lock<boost::mutex> Locker(m_mapCidMsg[cid].msgMutex);

	if (m_mapCidMsg[cid].vecRevBuffer.size() > 10 * 1024)
	{
		m_mapCidMsg[cid].vecRevBuffer.clear();
	}
}

// �Ƴ�id���õ�buffer
bool HandlerSocketMsg_Binary::RemoveId(uint64_t cid)
{
	boost::unique_lock<boost::mutex> Locker(m_cidMsgMutex);

	if (0 < m_mapCidMsg.erase(cid))
	{
		return true;
	}
	else
	{
		return false;
	}
}

// �Ƴ�����id���õ�buffer
void HandlerSocketMsg_Binary::RemoveAllId(void)
{
	boost::unique_lock<boost::mutex> Locker(m_cidMsgMutex);

	m_mapCidMsg.clear();
}