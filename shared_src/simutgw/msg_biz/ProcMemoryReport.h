#ifndef __PROC_MEMORY_REPORT_H__
#define __PROC_MEMORY_REPORT_H__

/*
处理深圳STEP接口A股和上海A股的回报类

*/

#include <map>
#include <vector>
#include <string>

class ProcMemoryReport
{
	/*
	member
	*/
private:


public:
	ProcMemoryReport(void);
	virtual ~ProcMemoryReport(void);

	/*
	发送一条上海回报或确认

	Return:
	0 -- 成功
	其他 -- 失败
	*/
	static int Send_SH_ReportOrConfirm(std::map<std::string, std::vector<std::string>>& in_mapUpdate);

	/*
		处理上海回报

		Return:
		0 -- 成功
		-1 -- 失败
		1 -- 无回报
	*/
	static int ProcSHReport();
};

#endif