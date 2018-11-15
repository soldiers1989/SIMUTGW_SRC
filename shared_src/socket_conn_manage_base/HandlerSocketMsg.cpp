#include "HandlerSocketMsg.h"
#include "tool_net/PacketAssembler.h"

HandlerSocketMsg::HandlerSocketMsg()
{
}


HandlerSocketMsg::~HandlerSocketMsg()
{
}

// ��δ�����buffer���뵽��buffer,�ٷְ�
void HandlerSocketMsg::AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf,
	std::vector<std::shared_ptr<simutgw::NET_PACKAGE> >& vecApdPack)
{
	boost::unique_lock<boost::mutex> Locker(m_cidMsgMutex);

	PacketAssem(cid, vecApdBuf, vecApdPack);
}

// ��δ�����buffer���뵽��buffer������buffer�еİ��ٷ�һ��
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

	// �������buffer��������100*1024ʱ�����
	if (m_mapCidMsg[cid].vecRevBuffer.size() > 100 * 1024)
	{
		m_mapCidMsg[cid].vecRevBuffer.clear();
	}

}

// �Ƴ�id���õ�buffer
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

// �Ƴ�����id���õ�buffer
void HandlerSocketMsg::RemoveAllId(void)
{
	boost::unique_lock<boost::mutex> Locker(m_cidMsgMutex);

	m_mapCidMsg.clear();
}