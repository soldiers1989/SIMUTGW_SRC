#ifndef __SH_SETTLE_H__
#define __SH_SETTLE_H__

#include "tool_file/TgwDBFOperHelper.h"
#include "tool_mysql/MySqlCnnC602.h"

/*
�Ϻ�������
*/
class SHSettle
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

private:
	std::string m_strNow;//20180226
	std::string m_strNextDay;//20180227

	//
	// Functions
	//
public:
	SHSettle();
	virtual ~SHSettle();

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

public:
	/*
	��ʼ����Ա����
	*/
	int Init();

	/*
	�Ϻ��г�gh����
	�������ݽӿ� gh.dbf

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Gen_gh(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSettleGroupName,
		const std::string& in_strFilePath);

	/*
	�Ϻ��г�gh����dbf��ʽ
	�������ݽӿ� gh.dbf
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_gh(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	�Ϻ��г�gh����dbfȡֵ
	�������ݽӿ� gh.dbf
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_Value_gh(const map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	�Ϻ��г�bc1����
	�������ݽӿ� bc1.dbf

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Gen_bc1(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSettleGroupName,
		const std::string& in_strFilePath);

	/*
	�Ϻ��г�bc1����dbf��ʽ
	�������ݽӿ� bc1.dbf
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_bc1(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	�Ϻ��г�bc1����dbfȡֵ
	�������ݽӿ� bc1.dbf
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_Value_bc1(const map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	�Ϻ��г�zqbd����
	zqbd(֤ȯ�䶯�ļ�)
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Gen_zqbd(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strSettleGroupName, const std::string& in_strFilePath);

	/*
	�Ϻ��г�zqbd����dbf��ʽ
	zqbd(֤ȯ�䶯�ļ�)
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_zqbd(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	�Ϻ��г�zqbd����
	zqbd(֤ȯ�䶯�ļ�)
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_Value_zqbd(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	�Ϻ��г�zqye���� ��T��T+1��

	zqye(֤ȯ�������ļ�)
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Gen_zqye_inTwoDays(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strFilePath);

	/*
	�����Ϻ��г�zqye�����ļ�
	zqye(֤ȯ�������ļ�)

	@param bool bIsNextDay : �Ƿ���T+1��
	true -- ��T+1��
	false -- ����T+1��

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Gen_DBF_zqye(vector<map<string, struct MySqlCnnC602_DF::DataInRow> >& in_vecMapRowData,
		const std::string& strFilePath, bool bIsNextDay);

	/*
	�Ϻ��г�zqye00001����dbf��ʽ
	zqye(֤ȯ�������ļ�)
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_zqye(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	�Ϻ��г�zqye00001����dbfȡֵ
	zqye(֤ȯ�������ļ�)
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int DBF_Value_zqye(const map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue,
		bool bIsNextDay);
	
	/*
	�Ϻ��г�jsmx00001����dbf��ʽ
	jsmx(��һ���ν�����ϸ�ļ�)
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int SH_DBF_jsmx00001(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	�Ϻ��г�jsmx00001����dbfȡֵ
	jsmx(��һ���ν�����ϸ�ļ�)
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int SH_DBF_Value_jsmx00001(const map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	�Ϻ��г�jsmx00001����
	jsmx(��һ���ν�����ϸ�ļ�)
	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int SH_Gen_jsmx00001(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& strFilePath);
};

#endif