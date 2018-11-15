#ifndef __FLOW_WORK_BASE_H__
#define __FLOW_WORK_BASE_H__

#include "thread_pool_base/TaskBase.h"

class FlowWorkBase
	: public TaskBase
{
protected:

	/* 
	处理的flow类型
	1 -- 深圳买
	2 -- 深圳卖
	3 -- 上海买
	4 -- 上海卖

	5 -- 取深圳委托，发回报
	6 -- 取上海委托，发回报

	7 -- 读深圳redis委托
	8 -- 读上海redis委托

	*/
	int m_iMatchType;

public:
	FlowWorkBase(void);
	virtual ~FlowWorkBase(void);

	virtual int TaskProc(void) = 0;

	/*
	设置处理的flow类型
	Param:
	iType 1 -- 深圳买
	2 -- 深圳卖
	3 -- 上海买
	4 -- 上海卖

	5 -- 取深圳委托，发回报
	6 -- 取上海委托，发回报

	7 -- 读深圳redis委托
	8 -- 读上海redis委托

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	int SetFlowMatchType(const int iType);

	/*
	获取处理的flow类型
	
	Return :
	*/
	int GetFlowMatchType(void) const
	{
		return m_iMatchType;
	}
	
};

#endif