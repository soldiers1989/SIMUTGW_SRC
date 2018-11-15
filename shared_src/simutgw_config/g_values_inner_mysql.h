#ifndef __G_VALUES_INNER_MYSQL_H__
#define __G_VALUES_INNER_MYSQL_H__

#include <stdint.h>
#include "config/g_values_inner_IdGen.h"

#include "tool_mysql/MysqlConnectionPool.h"

namespace simutgw
{	
	// MySQL连接池，因为要用到数据库配置g_strSQL_HostName等，所以声明在其之后
	extern MysqlConnectionPool g_mysqlPool;
}

#endif