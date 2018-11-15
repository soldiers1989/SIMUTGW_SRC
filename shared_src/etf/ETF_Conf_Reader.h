#ifndef __ETF_CONF_READER_H__
#define __ETF_CONF_READER_H__

#include <string>
#include <vector>
#include <memory>

#include "conf_etf_info.h"

/*
	etf配置读取
*/
class ETF_Conf_Reader
{
public:
	ETF_Conf_Reader();
	virtual ~ETF_Conf_Reader();

	/*
		读取深圳pcf文件
		Return:
		0 -- 成功
		-1 -- 失败
	*/
	static int Read_SZ_ETF_Pcf_Conf(const std::string& in_strFileName);

	/*
		读取上海pcf文件
		Return:
		0 -- 成功
		-1 -- 失败
	*/
	static int Read_SH_ETF_Conf(const std::string& in_strFileName);

private:
	/*
		解析深圳pcf文件
		Return:
		0 -- 成功
		-1 -- 失败
	*/
	static int Parse_Pcf_XML_And_Store(const std::string& in_strXMLContent);

	/*
		将pcf内容中除成分股之外的内容存入结构体
		Return:
		0 -- 成功
		-1 -- 失败
	*/
	static int Set_ETF_Info(std::shared_ptr<struct simutgw::SzETF>& ptrEtf,
		const std::string& in_strKey, const std::string& in_strValue);

	/*
		Set成分股信息
		Return:
		0 -- 成功
		-1 -- 失败
	*/
	static int Set_ETFComponent_Info(struct simutgw::SzETFComponent& cpn,
		const std::string& in_strKey, const std::string& in_strValue);

	/*
	解析上海ETF文件
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int Parse_SH_ETF_And_Store(const std::string& in_strETFContent);

	/*
	将SH ETF内容中除成分股之外的内容存入结构体
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int Set_SH_ETF_Info(std::shared_ptr<struct simutgw::SzETF>& ptrEtf,
		const std::string& in_strKey, const std::string& in_strValue);

	/*
	Set SH成分股信息
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int Set_SHETFComponent_Info(struct simutgw::SzETFComponent& cpn,
		const std::vector<std::string>& vecComponent);

	/*
		添加ETF信息
	*/
	static void Add_ETF_Info(std::shared_ptr<struct simutgw::SzETF>& ptrEtf);
};

#endif