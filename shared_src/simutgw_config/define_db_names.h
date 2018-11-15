#ifndef __DEFINE_DB_NAMES_H__
#define __DEFINE_DB_NAMES_H__

#include <memory>
#include <stdint.h>  
#include <string>

/*
定义数据库表名
*/
namespace simutgw
{
	//
	// Members
	//
	
	// 上海成交规则参数配置
	static const std::string g_DBTable_match_rule_sh("`match_rule_sh`");

	// 深圳成交规则参数配置
	static const std::string g_DBTable_match_rule_sz("`match_rule_sz`");

	// 上海、深圳ETF PCF文件信息，结构采用深圳的信息标准，参考文档《深圳证券交易所数据文件交换接口规范（Ver1.05）.pdf》，《IS101 上海证券交易所竞价撮合平台市场参与者接口规格说明书1.38版_20180522.docx》
	static const std::string g_DBTable_etf_info("`etf_info`");
	
	// 上海、深圳ETF成份股列表 Components，结构采用深圳的信息标准，参考文档《深圳证券交易所数据文件交换接口规范（Ver1.05）.pdf》，《IS101 上海证券交易所竞价撮合平台市场参与者接口规格说明书1.38版_20180522.docx》
	static const std::string g_DBTable_etf_component("`etf_component`");
};

#endif
