#ifndef __VALIDATE_ETF_H__
#define __VALIDATE_ETF_H__

#include <memory>

#include "etf/conf_etf_info.h"

class ValidateETF
{
public:
	ValidateETF();
	virtual ~ValidateETF();

	/*
	校验成分股信息
	Return:
	0 -- 合法
	-1 -- 非法
	*/
	static int Validate_ETFComponent_Info(struct simutgw::SzETFComponent& cpn);

	/*
	校验ETF除成分股之外的信息
	Return:
	0 -- 合法
	-1 -- 非法
	*/
	static int Validate_ETF_Info(const std::shared_ptr<struct simutgw::SzETF>& ptrEtf);
};

#endif