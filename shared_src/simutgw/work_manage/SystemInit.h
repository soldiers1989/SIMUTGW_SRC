#ifndef __SYSTEM_INIT_H__
#define __SYSTEM_INIT_H__

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include <vector>
#include <memory>

#include "order/define_order_msg.h"

/*
系统初始化的类
主要是检查mysql是否能连接上，
检查redis是否能连接上,
删除redis的委托和回报
将mysql中未处理的委托导入到redis中
*/
class SystemInit
{
	//
	// Members
	//

	//
	// Functions
	//
private:
	// 禁止使用
	SystemInit(void)
	{
	}

	// 禁止使用
	virtual ~SystemInit(void)
	{
	}

	/*
	使用动态库检查授权是否到期

	return:
	0 -- 未到期
	-1 -- 到期
	*/
	static int ValidateLicense_Dll();

	/*
	使用动态库检查运行模式

	return:
	0 -- 有模式文件
	-1 -- 无模式文件
	*/
	static int CheckRunMode_Dll();

	/*
	初始化日志

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step0_IniLog();

	/*
	读配置文件

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step1_CheckLicense();

	/*
	读配置文件

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step2_ReadConfig(void);

	/*
	启动mysql连接池

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step3_1_MysqlPoolInit(void);

	/*
	检查mysql

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step3_2_CheckMysql(void);

	/*
	由mysql数据库载入配置

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step3_3_LoadConfigFromDb(void);

	/*
	从file读id

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step4_1_ReadIdFromFile(void);


	/*
	从mysql表读id

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step4_2_ReadIdFromMysql(void);

	/*
	由Web取Id

	@param bool bSaveIdToDb :
	true -- 向db存ID
	flase -- 不向db存ID

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step4_3_GetIdFromWeb(bool bSaveIdToDb);

	/*
	由Web server取redis、连接等配置

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step4_4_GetConfigFromWebServer(void);

	/*
	初始化相关配置

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step5_1_InnerValuesInit(void);

	/*
	初始化相关配置

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step5_2_RedisPoolInit(void);

	/*
	检查redis

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step6_1_CheckRedis(void);

	/*
	清理外部环境

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step7_OuterClean(void);

	/*
	线程启动

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step9_ThreadsStartup(void);

	/*
		检查mysql是否连接正常
		return:
		0 -- 成功
		其他 -- 失败
		*/
	static int CheckMysql();

	/*
		检查redis是否连接正常
		return:
		0 -- 成功
		其他 -- 失败
		*/
	static int CheckRedis();

	/*
	保存id到文件

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int SaveIdToFile(void);

	/*
	保存id
	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int SaveIdFromWeb();

public:
	/*
	配置初始化
	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Simutgw_ConfigInit(void);

	/*
	检查通道策略和已收到策略同步情况

	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Step8_CheckLinkRuleSync(void);
};

#endif