#include "g_values_inner_IdGen.h"

namespace simutgw
{
	// Id������ -- �߳�ID
	UidGenerator<uint64_t> g_uidThreadGen;

	// Id������ -- ����ID
	UidGenerator<uint64_t> g_uidTaskGen;	
}