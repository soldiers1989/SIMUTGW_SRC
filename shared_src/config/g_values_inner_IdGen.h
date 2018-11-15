#ifndef __G_VALUES_INNER_ID_GEN_H__
#define __G_VALUES_INNER_ID_GEN_H__

#include <stdint.h>

#include "cache/UidGenerator.h"

namespace simutgw
{
	// Id������ -- �߳�ID
	extern UidGenerator<uint64_t> g_uidThreadGen;

	// Id������ -- ����ID
	extern UidGenerator<uint64_t> g_uidTaskGen;	
}

#endif