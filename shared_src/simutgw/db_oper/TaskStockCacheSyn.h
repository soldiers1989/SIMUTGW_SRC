#ifndef __TASK_STOCK_CACHE_SYN_H__
#define __TASK_STOCK_CACHE_SYN_H__

#include <string>

#include "thread_pool_base/TaskBase.h"

/*
	缓存股份信息同步任务类
*/
class TaskStockCacheSyn :public TaskBase
{
	//
	// member
	//
private:
	std::string m_strUpdateSql;
	
	//
	// function
	//
public:
	explicit TaskStockCacheSyn(const unsigned int uiId);
	virtual ~TaskStockCacheSyn();

	int SetSql(const std::string& in_strSql)
	{
		m_strUpdateSql = in_strSql;
		return 0;
	}

	virtual int TaskProc(void);

private:
	/*
	执行更新sql语句
	*/
	int ExcUpdateSql();
};

#endif