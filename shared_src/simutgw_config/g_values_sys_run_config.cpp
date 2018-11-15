#include "g_values_sys_run_config.h"

namespace simutgw
{
	// 配置信息
	boost::property_tree::ptree g_ptConfig;

	// 启用处理行情线程
	bool g_bEnable_Quotation_Task(false);

	// 启用深圳委托、回报消息处理、从redis读委托线程
	bool g_bEnable_Sz_Msg_Task(false);

	// 启用上海委托、回报消息处理、从redis读委托线程
	bool g_bEnable_Sh_Msg_Task(false);

	// 授权检验插件
	std::string dll_AuthCheckName("dll_authcheck");

	volatile bool g_bHighPfm = true;

	/*
	Web管理模式
	0 -- 非Web管理模式
	1 -- Web管理模式
	*/
	volatile WebManMode g_iWebManMode = WebManMode::LocalMode;

	/*
	运行模式
	1 -- 压力模式
	2 -- 极简模式
	3 -- 普通模式
	*/
	volatile SysRunMode g_iRunMode = SysRunMode::NormalMode;

	// 极简模式是否走json配置回报
	// true -- 是
	// false -- 否
	bool g_bEnable_Json_Mocker(false);

	// 是否检查资金股份
	// true -- 检查
	// false -- 不检查
	bool g_bEnable_Check_Assets(false);

	// 深圳STEP回报是否是Ver1.10
	// true -- 是Ver1.10
	// false -- 不是Ver1.10
	bool g_bSZ_Step_ver110(false);

	/*
	成交模式
	0 -- 默认
	1 -- 不看行情，全部成交
	2 -- 不看行情，分笔成交
	3 -- 不看行情，挂单、不成交，可撤单
	4 -- 错误单
	*/
	volatile SysMatchMode g_iMatchMode = SysMatchMode::SimulMatchAll;

	// 实盘模拟行情模式
	QuotationType g_Quotation_Type = QuotationType::AveragePrice;

	// 分笔成交笔数
	uint32_t g_ui32_Part_Match_Num = 2;

	// 深圳连接配置
	std::string g_strSzConfig;

	// 数据库配置
	std::string g_strSQL_HostName("127.0.0.1");
	std::string g_strSQL_UserName("admin");
	std::string g_strSQL_Password("simutgw");
	unsigned int g_uiSQL_Port = 3306;

	std::string g_strSQL_CataLog("simutgw");

	// Redis配置
	std::string g_strRedis_HostName("127.0.0.1");
	std::string g_strRedis_Password("");
	unsigned int g_uiRedis_Port = 6379;
	bool g_bRedis_requirepass = false;

	//是否启用服务器
	volatile bool g_bEnableServer = false;
	// 服务器listen地址
	std::string g_strServerIp = "127.0.0.1";
	// 服务器listen端口
	unsigned int g_uiServerPort = 50000;

	// web管理端配置
	// web分配的id
	std::string g_strWeb_id("");

	// web分配的id in local file
	std::string g_strWeb_id_local_file("");

	// web分配的id in local db
	std::string g_strWeb_id_local_db("");

	// web管理端 socket服务器地址
	std::string g_strSocketServerIp = "192.168.60.144";
	// web管理端 socket服务器端口
	unsigned int g_uiSocketServerPort = 10005;
}