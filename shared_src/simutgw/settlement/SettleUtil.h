#ifndef __SETTLE_UTIL_H__
#define __SETTLE_UTIL_H__

#include <stdint.h>
#include <string>

/*
清算相关功能函数
*/
namespace SettleUtil
{
	/*
	securityid 是否是深圳ETF代码

	@return:
	true -- 是深圳ETF代码
	false -- 不是
	*/
	bool IsSzEtf(const std::string& strSecurityId);


};

#endif