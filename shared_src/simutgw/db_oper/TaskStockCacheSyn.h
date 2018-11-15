#ifndef __TASK_STOCK_CACHE_SYN_H__
#define __TASK_STOCK_CACHE_SYN_H__

#include <string>

#include "thread_pool_base/TaskBase.h"

/*
	����ɷ���Ϣͬ��������
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
	ִ�и���sql���
	*/
	int ExcUpdateSql();
};

#endif