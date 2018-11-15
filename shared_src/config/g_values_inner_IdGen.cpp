#include "g_values_inner_IdGen.h"

namespace simutgw
{
	// Id生成器 -- 线程ID
	UidGenerator<uint64_t> g_uidThreadGen;

	// Id生成器 -- 任务ID
	UidGenerator<uint64_t> g_uidTaskGen;	
}