#ifndef __HANDLER_SOCKET_MSG_BINARY_H__
#define __HANDLER_SOCKET_MSG_BINARY_H__

#include <vector>
#include <string>
#include "boost/thread/mutex.hpp"

#include "tool_net_binary/BinaryNetPack.h"

/*
socket用户的消息缓存
包括：
未分包部分和已分包部分
*/
namespace simutgw
{
	struct SocketUserMessage_Binary
	{
		boost::mutex msgMutex;

		// 未分包部分
		std::vector<uint8_t> vecRevBuffer;

		// 已分包部分
		std::vector<std::shared_ptr<struct simutgw::BinaryNetPack>> vecRevDatas;
	};
};

/*
socket message的消息缓存、解包类
*/
class HandlerSocketMsg_Binary
{
	/*
		member
		*/
private:
	boost::mutex m_cidMsgMutex;

	// 消息缓存
	std::map<uint64_t, struct simutgw::SocketUserMessage_Binary> m_mapCidMsg;

public:
	HandlerSocketMsg_Binary();
	virtual ~HandlerSocketMsg_Binary();

	// 把未处理的buffer加入到主buffer,再分包
	void AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf, std::vector<std::shared_ptr<struct simutgw::BinaryNetPack> >& vecApdPack);

	// 把移除id所用的buffer
	bool RemoveId(uint64_t cid);

	// 移除所有id所用的buffer
	void RemoveAllId(void);

private:

	// 把未处理的buffer加入到主buffer
	void AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf);
	
	// 把主buffer中的包再分一次
	void PacketAssem(uint64_t cid, std::vector<std::shared_ptr<struct simutgw::BinaryNetPack> >& vecApdPack);

	// 尝试清空buffer，当超过10*1024时，清空
	void TryClearBuffer(uint64_t cid);
};

#endif
