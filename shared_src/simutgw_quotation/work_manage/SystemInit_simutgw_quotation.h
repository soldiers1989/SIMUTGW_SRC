#ifndef __SYSTEM_INIT_CHILD_QUOTATION_H__
#define __SYSTEM_INIT_CHILD_QUOTATION_H__

#ifdef _MSC_VER
#include <Windows.h>
#else

#endif

#include <vector>

#include "order/define_order_msg.h"

/*
系统初始化的类
主要是检查mysql是否能连接上，
检查redis是否能连接上,
删除redis的委托和回报
将mysql中未处理的委托导入到redis中
*/
class SystemInit_simutgw_quotation
{
	//
	// Members
	//

	//
	// Functions
	//
private:
	// 禁止使用
	SystemInit_simutgw_quotation(void)
	{
	}

	// 禁止使用
	virtual ~SystemInit_simutgw_quotation(void)
	{
	}

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
	static int Step2_ReadConfig(void);

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
		检查redis是否连接正常
		return:
		0 -- 成功
		其他 -- 失败
		*/
	static int CheckRedis();

public:
	/*
	子进程行情处理 配置初始化
	return:
	0 -- 成功
	其他 -- 失败
	*/
	static int ChildP_Quotation_ConfigInit(void);

	/*
	控件自身必要程序回收
	*/
	static void SelfExit(void);

};

#endif