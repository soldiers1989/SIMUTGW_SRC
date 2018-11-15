#ifndef __G_VALUES_INNER_SUBP_QUOT_H__
#define __G_VALUES_INNER_SUBP_QUOT_H__

#include "simutgw_config/g_values_inner_redis.h"

#include "simutgw_quotation/work_manage/LooperManager_child_quotation.h"

namespace simutgw
{	
	// 处理线程管理
	extern LooperManager_child_quotation g_flowManage_quota;
}

#endif