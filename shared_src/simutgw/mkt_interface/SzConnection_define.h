#ifndef __SZ_CONNECTION_DEFINE_H__
#define __SZ_CONNECTION_DEFINE_H__

#include <stdint.h>
#include <string>
#include <map>
#include <memory>

/*
	���ڽӿ�������Ϣ
*/
class SzConnection
{
	/*
		member
	*/
private:
	//�����Ƿ���
	bool m_bLogOn;

	// �Ƿ�ر�ͬ������Ҫ��¼�ϲ��ܽ��лر�ͬ��
	bool m_bReportSyn;

	// ��¼reportindex
	uint64_t m_ui64ReportIndex;

	// δ�ر�ͬ��ʱ����¼�ر��˼���
	uint64_t m_ui64QueLookUpTimes;

	// ����ID
	std::string m_strSenderID;

	// �Զ�ID
	std::string m_strTargetID;

	// ƽ̨�����ź�ReportIndex��Ӧ��ϵ
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