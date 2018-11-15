#ifndef __SYS_FUNCTION_H__
#define __SYS_FUNCTION_H__

#include <vector>

#include "boost/shared_ptr.hpp"

#include "config/sys_function_base.h"

namespace simutgw
{
	/*
	控件自身必要程序回收
	*/
	void SimuTgwSelfExit(void);

	/*
	控件自身必要程序回收
	指令重启功能
	*/
	void SimuTgwSelfExit_remoterestart(void);

	/*
	结算
	@param const std::vector<std::string>& in_vctSettleGroup : 清算池别名
	@param std::string& out_strDay : 当前日期字符串
	@param std::string& out_strSettlementFilePath : 清算文件所在路径

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Simutgw_Settle(const std::vector<std::string>& in_vctSettleGroup,
		std::string& out_strDay, std::string& out_strSettlementFilePath);

	/*
	日终
	*/
	int Simutgw_DayEnd();
};

#endif