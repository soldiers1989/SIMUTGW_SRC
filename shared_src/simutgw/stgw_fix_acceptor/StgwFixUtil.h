#ifndef __STGW_FIX_UTIL_H__
#define __STGW_FIX_UTIL_H__

#include "simutgw/stgw_fix_acceptor/StgwApplication.h"

/*
fix消息工具类

主要函数
@function GetNopartyIds 解析NopartyIds中的席位、账号、营业部代码
*/
namespace StgwFixUtil
{
	//
	// function
	//
	// 取nopartyids重复组的数据，包括账号、席位和营业部代码
	// @param message fix消息
	// @param out_strSeat 取得的席位
	// @param out_strAccount 取得的账号
	// @param out_strMarket_branchid 取得的营业部代码
	// @return
	// 0 -- 成功
	// -1 -- 失败
	int GetNopartyIds(const FIX::FieldMap& message,
		std::string& out_strSeat,
		std::string& out_strAccount, 
		std::string& out_strMarket_branchid);

	// 取字段内容
	// @param message fix消息
	// @param field 字段key
	// @param strValue 内容
	int GetStringField(const FIX::Message& message,
		int field, std::string& strValue);

	// set field
	// @param fixReport fix回报
	// @param field 字段key
	// @param strValue 内容
	int SetField(FIX::Message& fixReport,
		int field, const std::string& strValue);

	// set field
	int SetField(FIX::FieldMap& fixReport,
		int field, const std::string& strValue);
};

#endif