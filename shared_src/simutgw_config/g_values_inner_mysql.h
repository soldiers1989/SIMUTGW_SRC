#ifndef __G_VALUES_INNER_MYSQL_H__
#define __G_VALUES_INNER_MYSQL_H__

#include <stdint.h>
#include "config/g_values_inner_IdGen.h"

#include "tool_mysql/MysqlConnectionPool.h"

namespace simutgw
{	
	// MySQL���ӳأ���ΪҪ�õ����ݿ�����g_strSQL_HostName�ȣ�������������֮��
	extern MysqlConnectionPool g_mysqlPool;
}

#endif