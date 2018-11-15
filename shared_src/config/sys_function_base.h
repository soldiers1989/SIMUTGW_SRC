#ifndef __SYS_FUNCTION_BASE_H__
#define __SYS_FUNCTION_BASE_H__

/*
跨进程的定义

*/

namespace simutgw
{
	/*
	程序异常退出
	*/
	void ErrorExit(const int exitcode);

	// 标准sleep
	void Simutgw_Sleep(void);
};

#endif