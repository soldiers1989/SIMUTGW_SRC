#ifndef __SH_SETTLE_H__
#define __SH_SETTLE_H__

#include "tool_file/TgwDBFOperHelper.h"
#include "tool_mysql/MySqlCnnC602.h"

/*
上海结算类
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
	结算明细
	每个清算池ID都有一份

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int MakeSettlementDetails(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strSettleGroupName, const std::string& in_strFilePath);

	/*
	结算汇总
	做一次

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int MakeSettlementSummary(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strFilePath);

public:
	/*
	初始化成员变量
	*/
	int Init();

	/*
	上海市场gh结算
	过户数据接口 gh.dbf

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Gen_gh(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSettleGroupName,
		const std::string& in_strFilePath);

	/*
	上海市场gh结算dbf格式
	过户数据接口 gh.dbf
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_gh(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	上海市场gh结算dbf取值
	过户数据接口 gh.dbf
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_Value_gh(const map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	上海市场bc1结算
	过户数据接口 bc1.dbf

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Gen_bc1(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSettleGroupName,
		const std::string& in_strFilePath);

	/*
	上海市场bc1结算dbf格式
	过户数据接口 bc1.dbf
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_bc1(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	上海市场bc1结算dbf取值
	过户数据接口 bc1.dbf
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_Value_bc1(const map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	上海市场zqbd结算
	zqbd(证券变动文件)
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Gen_zqbd(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strSettleGroupName, const std::string& in_strFilePath);

	/*
	上海市场zqbd结算dbf格式
	zqbd(证券变动文件)
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_zqbd(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	上海市场zqbd结算
	zqbd(证券变动文件)
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_Value_zqbd(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	上海市场zqye结算 含T、T+1日

	zqye(证券余额对账文件)
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Gen_zqye_inTwoDays(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strFilePath);

	/*
	生成上海市场zqye结算文件
	zqye(证券余额对账文件)

	@param bool bIsNextDay : 是否是T+1日
	true -- 是T+1日
	false -- 不是T+1日

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Gen_DBF_zqye(vector<map<string, struct MySqlCnnC602_DF::DataInRow> >& in_vecMapRowData,
		const std::string& strFilePath, bool bIsNextDay);

	/*
	上海市场zqye00001结算dbf格式
	zqye(证券余额对账文件)
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_zqye(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	上海市场zqye00001结算dbf取值
	zqye(证券余额对账文件)
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_Value_zqye(const map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue,
		bool bIsNextDay);
	
	/*
	上海市场jsmx00001结算dbf格式
	jsmx(单一批次结算明细文件)
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int SH_DBF_jsmx00001(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	上海市场jsmx00001结算dbf取值
	jsmx(单一批次结算明细文件)
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int SH_DBF_Value_jsmx00001(const map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	上海市场jsmx00001结算
	jsmx(单一批次结算明细文件)
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int SH_Gen_jsmx00001(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& strFilePath);
};

#endif