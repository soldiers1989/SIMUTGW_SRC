#include "HandlerSocketMsg_Binary.h"

#include "tool_net_binary/BinaryPacketAssembler.h"

HandlerSocketMsg_Binary::HandlerSocketMsg_Binary()
{
}


HandlerSocketMsg_Binary::~HandlerSocketMsg_Binary()
{
}

// 把未处理的buffer加入到主buffer,再分包
void HandlerSocketMsg_Binary::AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf,
	std::vector<std::shared_ptr<struct simutgw::BinaryNetPack> >& vecApdPack)
{
	boost::unique_lock<boost::mutex> Locker(m_cidMsgMutex);

	AppendBuffer(cid, vecApdBuf);

	PacketAssem(cid, vecApdPack);

	TryClearBuffer(cid);
}

// 把未处理的buffer加入到主buffer
void HandlerSocketMsg_Binary::AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf)
{
	boost::unique_lock<boost::mutex> Locker(m_mapCidMsg[cid].msgMutex);

	for (size_t st = 0; st < vecApdBuf.size(); ++st)
	{
		m_mapCidMsg[cid].vecRevBuffer.push_back(vecApdBuf[st]);
	}
}

// 把主buffer中的包再分一次
void HandlerSocketMsg_Binary::PacketAssem(uint64_t cid, std::vector<std::shared_ptr<struct simutgw::BinaryNetPack> >& vecApdPack)
{
	boost::unique_lock<boost::mutex> Locker(m_mapCidMsg[cid].msgMutex);

	// param 3 = true;
	BinaryPacketAssembler::RecvPackage(m_mapCidMsg[cid].vecRevBuffer, vecApdPack, true);

}

// 尝试清空buffer，当超过10*1024时，清空
void HandlerSocketMsg_Binary::TryClearBuffer(uint64_t cid)
{
	boost::unique_lock<boost::mutex> Locker(m_mapCidMsg[cid].msgMutex);

	if (m_mapCidMsg[cid].vecRevBuffer.size() > 10 * 1024)
	{
		m_mapCidMsg[cid].vecRevBuffer.clear();
	}
}

// 移除id所用的buffer
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

// 移除所有id所用的buffer
void HandlerSocketMsg_Binary::RemoveAllId(void)
{
	boost::unique_lock<boost::mutex> Locker(m_cidMsgMutex);

	m_mapCidMsg.clear();
}