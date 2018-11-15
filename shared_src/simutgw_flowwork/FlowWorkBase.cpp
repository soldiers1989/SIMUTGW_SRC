#include "FlowWorkBase.h"

#include <string>

FlowWorkBase::FlowWorkBase(void)
: TaskBase(0), m_iMatchType(0)
{
}

FlowWorkBase::~FlowWorkBase(void)
{
}

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
int FlowWorkBase::SetFlowMatchType(const int iType)
{
	// static const std::string fTag("FlowWorkBase::SetFlowMatchType");

	m_iMatchType = iType;

	return 0;
}
