#ifndef __SETTLE_UTIL_H__
#define __SETTLE_UTIL_H__

#include <stdint.h>
#include <string>

/*
������ع��ܺ���
*/
namespace SettleUtil
{
	/*
	securityid �Ƿ�������ETF����

	@return:
	true -- ������ETF����
	false -- ����
	*/
	bool IsSzEtf(const std::string& strSecurityId);


};

#endif