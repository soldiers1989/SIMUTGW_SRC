#ifndef __BINARY_NET_PACK_H__
#define __BINARY_NET_PACK_H__

#include <stdint.h>
#include <vector>
#include <memory>

namespace simutgw
{
	struct BinaryNetPack
	{
		// MsgType  ��Ϣ����
		uint32_t ui32MsgType;
		// BodyLength ��Ϣ�峤��
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