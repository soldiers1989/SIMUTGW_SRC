#ifndef __SZ_CONNECTION_DEFINE_H__
#define __SZ_CONNECTION_DEFINE_H__

#include <stdint.h>
#include <string>
#include <map>
#include <memory>

/*
	深圳接口连接信息
*/
class SzConnection
{
	/*
		member
	*/
private:
	//连接是否建立
	bool m_bLogOn;

	// 是否回报同步，先要登录上才能进行回报同步
	bool m_bReportSyn;

	// 记录reportindex
	uint64_t m_ui64ReportIndex;

	// 未回报同步时，记录回报了几次
	uint64_t m_ui64QueLookUpTimes;

	// 本端ID
	std::string m_strSenderID;

	// 对端ID
	std::string m_strTargetID;

	// 平台分区号和ReportIndex对应关系
	std::shared_ptr<std::map<int, uint64_t>> m_ptrmapPartitions;

	/*
		functions
	*/
public:
	SzConnection() :m_bLogOn(false), m_bReportSyn(false), m_ui64ReportIndex(0), m_ui64QueLookUpTimes(0),
		m_ptrmapPartitions(new std::map<int, uint64_t>())
	{
	};

	~SzConnection()
	{
		m_bLogOn = false;
		m_bReportSyn = false;
		m_ui64ReportIndex = 0;
	}

	bool GetLogSta(){ return m_bLogOn; }
	void LogOn(){ m_bLogOn = true; }
	void LogOut(){ m_bLogOn = false; m_bReportSyn = false; ResetQueLkupTimes(); }

	bool GetRptSta(){ return m_bReportSyn; }
	void RptSyn(){ m_bReportSyn = true; }

	void SetRptIdx(uint64_t ui64Num){ m_ui64ReportIndex = ui64Num; m_bReportSyn = true; }
	uint64_t GetRptIdex(){ return m_ui64ReportIndex++; }

	void IncQueLkupTimes(){ ++m_ui64QueLookUpTimes; }
	uint64_t GetQueLkupTimes(){ return m_ui64QueLookUpTimes; }
	void ResetQueLkupTimes(){ m_ui64QueLookUpTimes = 0; }

	std::string GetSenderID(){ return m_strSenderID; }
	std::string GetTargetID(){ return m_strTargetID; }
	void SetSenderID(const std::string& strID){ m_strSenderID = strID; }
	void SetTargetID(const std::string& strID){ m_strTargetID = strID; }

	std::shared_ptr<std::map<int, uint64_t>> GetPartitionsMap()
	{
		return m_ptrmapPartitions;
	}
};

#endif