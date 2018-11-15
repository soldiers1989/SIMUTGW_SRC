#ifndef __PROC_MEMORY_REPORT_H__
#define __PROC_MEMORY_REPORT_H__

/*
��������STEP�ӿ�A�ɺ��Ϻ�A�ɵĻر���

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
	����һ���Ϻ��ر���ȷ��

	Return:
	0 -- �ɹ�
	���� -- ʧ��
	*/
	static int Send_SH_ReportOrConfirm(std::map<std::string, std::vector<std::string>>& in_mapUpdate);

	/*
		�����Ϻ��ر�

		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		1 -- �޻ر�
	*/
	static int ProcSHReport();
};

#endif