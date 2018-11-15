#ifndef __STGW_FIX_REPORT_H__
#define __STGW_FIX_REPORT_H__

#include <memory>

#include "StgwApplication.h"

#include "order/define_order_msg.h"

#include "util/EzLog.h"

// ȡfix�ر���Ϣ��
class StgwFixReport
{
	//
	// member
	//
protected:
	static src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	//
	// function
	//
public:
	StgwFixReport();
	virtual ~StgwFixReport();

	/*
	�������ڻر�ҵ��
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	1 -- �޻ر�
	*/
	static int Send_SZReport();

	/*
	���������ظ���ҵ��ܾ���Ϣ
	*/
	static int Send_SZ_RepeatRejectMsg(std::shared_ptr<struct simutgw::OrderMessage>& orderMsg);

private:
	/*
	����һ���ر�

	Reutrn:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ProcSingleReport(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	�������ڳ���

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ProcSZCancelOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	�������ڴ��󶩵�

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	1 -- �޻ر�
	*/
	static int ProcSZErrorOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	��������ȷ��

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	1 -- �޻ر�
	*/
	static int ProcSZConfirmOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	����һ��ί�гɹ���ִ�б���ر�,msgtype = 8
	*/
	static int ProcMsgType_8_Report(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	����һ��ί�гɹ���ִ�б���ر�,msgtype = 8
	��JSON���ù���

	Reutrn:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ProcMsgType_8_Confirm_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);

	/*
	����һ��ί�гɹ���ִ�б���ر�,msgtype = 8
	��JSON���ù���

	Reutrn:
	0 -- �ɹ�
	1 -- �޻ظ���Ϣ
	-1 -- ʧ��
	*/
	static int ProcMsgType_8_Report_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& out_vctFixReport);

	// ���ӻر�NoPartyIds�ظ���
	static int AddNoPartyIds(FIX::Message& fixReport, const std::string& strSeat,
		const std::string& strAccount, const std::string& strMarket_branchid);

	// ���ӻر�NoSecurity�ظ���
	static int AddNoSecurity(FIX::Message& fixReport,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport);

	/*
	����һ������ʧ�ܳɹ���ִ�б���ر�, msgtype = 9
	*/
	static int ProcMsgType_9_Report(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& fixReport);
};

#endif