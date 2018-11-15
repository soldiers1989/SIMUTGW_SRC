#include "HandlerSocketMsg.h"
#include "tool_net/PacketAssembler.h"

HandlerSocketMsg::HandlerSocketMsg()
{
}


HandlerSocketMsg::~HandlerSocketMsg()
{
}

// 把未处理的buffer加入到主buffer,再分包
void HandlerSocketMsg::AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf,
	std::vector<std::shared_ptr<simutgw::NET_PACKAGE> >& vecApdPack)
{
	boost::unique_lock<boost::mutex> Locker(m_cidMsgMutex);

	PacketAssem(cid, vecApdBuf, vecApdPack);
}

// 把未处理的buffer加入到主buffer，把主buffer中的包再分一次
void HandlerSocketMsg::PacketAssem(uint64_t cid, const std::vector<uint8_t>& vecApdBuf,
	std::vector<std::shared_ptr<simutgw::NET_PACKAGE> >& vecApdPack)
{
	boost::unique_lock<boost::mutex> Locker(m_mapCidMsg[cid].msgMutex);

	for (size_t st = 0; st < vecApdBuf.size(); ++st)
	{
		m_mapCidMsg[cid].vecRevBuffer.push_back(vecApdBuf[st]);
	}

	// param 3 = true;
	int iRes = PacketAssembler::RecvPackage(m_mapCidMsg[cid].vecRevBuffer, vecApdPack, true, false);
	if (-1 == iRes)
	{

	}

	// 尝试清空buffer，当超过100*1024时，清空
	if (m_mapCidMsg[cid].vecRevBuffer.size() > 100 * 1024)
	{
		m_mapCidMsg[cid].vecRevBuffer.clear();
	}

}

// 移除id所用的buffer
bool HandlerSocketMsg::RemoveId(uint64_t cid)
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
void HandlerSocketMsg::RemoveAllId(void)
{
	boost::unique_lock<boost::mutex> Locker(m_cidMsgMutex);

	m_mapCidMsg.clear();
}