#ifndef __ETF_HELPER_H__
#define __ETF_HELPER_H__

#include <memory>

#include "conf_etf_info.h"

/*
	深圳/上海etf的信息helper类
	*/
class ETFHelper
{
	/*
		function
		*/
public:
	ETFHelper();
	virtual ~ETFHelper();

	/*
		加载指定目录的etf文件
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int LoadETF(const std::string& in_strDir);

	/*
		加载指定目录的pcf文件
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int LoadSZPcf(const std::string& in_strDir);

	/*
		加载指定目录的上海etf公告文件
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int LoadSHETF(const std::string& in_strDir);

	/*
		查询某支etf的信息
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int Query(const std::string& in_strSecurityID,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
		打印当前存储的ETF信息的证券代码
		*/
	static int Overview();

private:
	/*
		获取指定目录底下的pcf文件名字
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int GetPCFFileNameInDir(const std::string& in_strDir, std::vector<std::string>& out_vecFileName);

	/*
	检查ShenZhen pcf文件名称的合法性

	文件命名规则为： pcf_<ETF 代码>_YYYYMMDD.xml ，其中 YYYYMMDD 为 T日日期。 示例如下：
	pcf_159901_20150115.xml
	*/
	static bool Check_SZETF_FileName(const std::string& in_strFileName);

	/*
	获取指定目录底下的上海ETF文件名字
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int Get_SHETF_FileNameInDir(const std::string& in_strDir, std::vector<std::string>& out_vecFileName);

	/*
		检查上海ETF文件名称的合法性


		*/
	static bool Check_SHETF_FileName(const std::string& in_strFileName);
};

#endif