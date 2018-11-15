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
	У��ɷֹ���Ϣ
	Return:
	0 -- �Ϸ�
	-1 -- �Ƿ�
	*/
	static int Validate_ETFComponent_Info(struct simutgw::SzETFComponent& cpn);

	/*
	У��ETF���ɷֹ�֮�����Ϣ
	Return:
	0 -- �Ϸ�
	-1 -- �Ƿ�
	*/
	static int Validate_ETF_Info(const std::shared_ptr<struct simutgw::SzETF>& ptrEtf);
};

#endif