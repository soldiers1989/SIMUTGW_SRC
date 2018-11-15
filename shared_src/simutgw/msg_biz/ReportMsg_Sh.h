#ifndef __REPORT_MSG_SH_H__
#define __REPORT_MSG_SH_H__

#include <memory>

#include "order/define_order_msg.h"

#include "util/EzLog.h"

/*
	����ӻر������ж����Ļر��࣬
	��Ҫ�Ǹ��±�ͷ����Ϻ�sql��
	*/
class ReportMsg_Sh
{
	/*
	member
	*/


	/*
	function
	*/
public:
	ReportMsg_Sh();
	virtual ~ReportMsg_Sh();

protected:
	static src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

private:
	/*
	����һ���ر�

	Reutrn:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ProcSingleReport(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);


	//----------------------------------�Ϻ�begin-------------------------------------
	/*
	FIXЭ���ʽ����ת����FIXЭ���ʽ�Ϻ�ȷ�ϣ�����һ��sql��

	Reutrn:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int FixToSHConfirm(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	/*
	FIXЭ���ʽ���� ��JSON���ù��� ת����FIXЭ���ʽ�Ϻ�ȷ�ϣ�����һ��sql��

	Reutrn:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int FixToSHConfirm_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	/*
	FIXЭ���ʽ����ת����FIXЭ���ʽ�Ϻ��ر�������һ��sql��

	Reutrn:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int FixToSHReport(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	/*
	FIXЭ���ʽ���� ��JSON���ù��� ת����FIXЭ���ʽ�Ϻ��ر�������һ��sql��

	Reutrn:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int FixToSHReport_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	//----------------------------------�Ϻ�end-------------------------------------

	/*
	�����Ϻ�����

	@param std::string& out_strSql_confirm : д��ȷ�ϱ������

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ProcSHCancelOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	/*
	�����Ϻ����󶩵�

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	1 -- �޻ر�
	*/
	static int ProcSHErrorOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

	/*
	�����Ϻ�ȷ��

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	1 -- �޻ر�
	*/
	static int ProcSHConfirmOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::vector<std::string>& out_vectSqlStr);

public:
	/*
		�����Ϻ��ر�ҵ��
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		1 -- �޻ر�
		*/
	static int Get_SHReport(map<string, vector<string>>& out_mapUpdate);
};

#endif