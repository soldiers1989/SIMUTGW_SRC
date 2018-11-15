#ifndef __SZ_SETTLE_H__
#define __SZ_SETTLE_H__

#include "tool_file/TgwDBFOperHelper.h"
#include "tool_mysql/MySqlCnnC602.h"

/*
���ڽ�����
*/
class SZSettle
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

private:
	std::string m_strDbfPostfix;
	std::string m_strNow;//20180226
	std::string m_strNextDay;//20180227

	//
	// Functions
	//
public:
	SZSettle();
	virtual ~SZSettle();

	/*
	������ϸ
	ÿ�������ID����һ��

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int MakeSettlementDetails(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strSettleGroupName, const std::string& in_strFilePath);

	/*
	�������
	��һ��

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int MakeSettlementSummary(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strFilePath);

	/*
	�����г�SJSDZ����dbf��ʽ
	�ɷݽ�����ʿ� SJSDZ.DBF
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_SJSDZ(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

protected:
	/*
	��ʼ����Ա����
	*/
	int Init();

	/*
	�����г�SJSMXn����
	������ϸ�� SJSMXn.DBF
	����SJSMX1.DBF(����A��)��SJSMX2.DBF(����B��)

	Param:
	in_strTradeType: 0 -- A�ɣ� 1 -- B��

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Gen_SJSMXn(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSettleGroupName,
		const std::string& in_strFilePath);

	/*
	�����г�SJSMXn����dbf��ʽ
	������ϸ�� SJSMXn.DBF
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_SJSMXn(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	�����г�SJSMX1����dbfȡֵ
	������ϸ�� SJSMXn.DBF
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_Value_SJSMX1(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	�����г�SJSMX2����dbfȡֵ
	������ϸ�� SJSMXn.DBF
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_Value_SJSMX2(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	�����г�SJSJG����
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Gen_SJSJG(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, 
		const std::string& in_strSettleGroupName,
		const std::string& in_strFilePath);

	/*
	�����г�SJSJG����dbf��ʽ
	��ϸ����� SJSJG.DBF
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_SJSJG(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	�����г�SJSJG����dbfȡֵ
	��ϸ����� SJSJG.DBF

	Return :
	0 -- �ɹ�
	1 -- ������������д�룬����������һ��
	��0 -- ʧ��
	*/
	int DBF_Value_SJSJG(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	�����г�SJSDZ����
	�ɷݽ�����ʿ� SJSDZ.DBF
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Gen_SJSDZ(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& strFilePath);

	/*
	���������г�SJSDZ�����ļ�����ΪT��T+1��
	�ɷݽ�����ʿ� SJSDZ.DBF

	@param bool bIsNextDay : �Ƿ���T+1��
	true -- ��T+1��
	false -- ����T+1��

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Gen_DBF_SJSDZ(vector<map<string, struct MySqlCnnC602_DF::DataInRow> >& in_vecMapRowData,
		const std::string& strFilePath, bool bIsNextDay);
	
	/*
	�����г�SJSDZ����dbfȡֵ
	�ɷݽ�����ʿ� SJSDZ.DBF
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_Value_SJSDZ(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue,
		bool bIsNextDay);

	/*
	�����г�SJSTJ����dbf��ʽ
	֤ȯ����ͳ�ƿ� SJSTJ.DBF
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int SZ_DBF_SJSTJ(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	�����г�SJSTJ����dbfȡֵ
	֤ȯ����ͳ�ƿ� SJSTJ.DBF
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int SZ_DBF_Value_SJSTJ(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

};

#endif