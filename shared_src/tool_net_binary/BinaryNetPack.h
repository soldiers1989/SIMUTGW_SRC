#ifndef __BINARY_NET_PACK_H__
#define __BINARY_NET_PACK_H__

#include <stdint.h>
#include <vector>
#include <memory>

namespace simutgw
{
	struct BinaryNetPack
	{
		// MsgType  消息类型
		uint32_t ui32MsgType;
		// BodyLength 消息体长度
		uint32_t ui32BodyLength;

		// whole message
		std::vector<uint8_t> vctBuffer;

		BinaryNetPack()
		{
			ui32MsgType = 0;
			ui32BodyLength = 0;
		}
	};
}
#endif