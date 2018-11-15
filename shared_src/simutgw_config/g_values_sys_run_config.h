#ifndef __VALUES_SYS_RUN_CONFIG_H__
#define __VALUES_SYS_RUN_CONFIG_H__

#include <string>
#include <map>
#include <stdint.h>

#include "boost/property_tree/ptree.hpp"    
#include "boost/property_tree/ini_parser.hpp"    

#include "config_define.h"

namespace simutgw
{
	// 配置信息
	extern boost::property_tree::ptree g_ptConfig;

	// 启用处理行情线程
	extern bool g_bEnable_Quotation_Task;

	// 启用深圳委托、回报消息处理线程
	extern bool g_bEnable_Sz_Msg_Task;

	// 启用上海委托、回报消息处理线程
	extern bool g_bEnable_Sh_Msg_Task;

	// 授权检验插件
	extern std::string dll_AuthCheckName;

	/*
	撮合时高性能模式
	true -- 高性能
	false -- 非高性能

	*/
	extern volatile bool g_bHighPfm;

	/*
	Web管理模式
	0 -- 非Web管理模式
	1 -- Web管理模式
	*/
	extern volatile enum WebManMode g_iWebManMode;

	/*
	运行模式
	1 -- 压力模式
	2 -- 极简模式
	3 -- 普通模式
	*/
	extern volatile enum SysRunMode g_iRunMode;

	// 极简模式是否走json配置回报
	// true -- 是
	// false -- 否
	extern bool g_bEnable_Json_Mocker;

	// 是否检查资金股份
	// true -- 检查
	// false -- 不检查
	extern bool g_bEnable_Check_Assets;

	// 深圳STEP回报是否是Ver1.10
	// true -- 是Ver1.10
	// false -- 不是Ver1.10
	extern bool g_bSZ_Step_ver110;

	/*
	成交模式
	0 -- 默认
	1 -- 不看行情，全部成交
	2 -- 不看行情，分笔成交
	3 -- 不看行情，挂单、不成交，可撤单
	4 -- 错误单
	*/
	extern volatile enum SysMatchMode g_iMatchMode;

	// 实盘模拟行情模式
	extern enum QuotationType g_Quotation_Type;

	// 分笔成交笔数
	extern uint32_t g_ui32_Part_Match_Num;

	// 深圳连接配置
	extern std::string g_strSzConfig;

	// 数据库配置
	extern std::string g_strSQL_HostName;
	extern std::string g_strSQL_UserName;
	extern std::string g_strSQL_Password;
	extern std::string g_strSQL_CataLog;
	extern unsigned int g_uiSQL_Port;

	//
	// Redis配置
	extern std::string g_strRedis_HostName;
	extern std::string g_strRedis_Password;
	extern unsigned int g_uiRedis_Port;
	extern bool g_bRedis_requirepass;

	//是否启用服务器
	extern volatile bool g_bEnableServer;
	// 服务器listen地址
	extern std::string g_strServerIp;
	// 服务器listen端口
	extern unsigned int g_uiServerPort;

	// web管理端配置
	// web分配的id
	extern std::string g_strWeb_id;

	// web分配的id in local file
	extern std::string g_strWeb_id_local_file;

	// web分配的id in local db
	extern std::string g_strWeb_id_local_db;

	// web管理端 socket服务器地址
	extern std::string g_strSocketServerIp;
	// web管理端 socket服务器端口
	extern unsigned int g_uiSocketServerPort;
}

#endif