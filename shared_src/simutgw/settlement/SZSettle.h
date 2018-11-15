#ifndef __SZ_SETTLE_H__
#define __SZ_SETTLE_H__

#include "tool_file/TgwDBFOperHelper.h"
#include "tool_mysql/MySqlCnnC602.h"

/*
深圳结算类
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

	/*
	深圳市场SJSDZ结算dbf格式
	股份结算对帐库 SJSDZ.DBF
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_SJSDZ(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

protected:
	/*
	初始化成员变量
	*/
	int Init();

	/*
	深圳市场SJSMXn结算
	清算明细库 SJSMXn.DBF
	包括SJSMX1.DBF(包含A股)和SJSMX2.DBF(包含B股)

	Param:
	in_strTradeType: 0 -- A股， 1 -- B股

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Gen_SJSMXn(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSettleGroupName,
		const std::string& in_strFilePath);

	/*
	深圳市场SJSMXn结算dbf格式
	清算明细库 SJSMXn.DBF
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_SJSMXn(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	深圳市场SJSMX1结算dbf取值
	清算明细库 SJSMXn.DBF
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_Value_SJSMX1(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	深圳市场SJSMX2结算dbf取值
	清算明细库 SJSMXn.DBF
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_Value_SJSMX2(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	深圳市场SJSJG结算
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Gen_SJSJG(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, 
		const std::string& in_strSettleGroupName,
		const std::string& in_strFilePath);

	/*
	深圳市场SJSJG结算dbf格式
	明细结果库 SJSJG.DBF
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_SJSJG(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	深圳市场SJSJG结算dbf取值
	明细结果库 SJSJG.DBF

	Return :
	0 -- 成功
	1 -- 此条数据无需写入，跳过处理下一条
	非0 -- 失败
	*/
	int DBF_Value_SJSJG(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

	/*
	深圳市场SJSDZ结算
	股份结算对帐库 SJSDZ.DBF
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Gen_SJSDZ(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& strFilePath);

	/*
	生成深圳市场SJSDZ结算文件，分为T、T+1日
	股份结算对帐库 SJSDZ.DBF

	@param bool bIsNextDay : 是否是T+1日
	true -- 是T+1日
	false -- 不是T+1日

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Gen_DBF_SJSDZ(vector<map<string, struct MySqlCnnC602_DF::DataInRow> >& in_vecMapRowData,
		const std::string& strFilePath, bool bIsNextDay);
	
	/*
	深圳市场SJSDZ结算dbf取值
	股份结算对帐库 SJSDZ.DBF
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int DBF_Value_SJSDZ(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue,
		bool bIsNextDay);

	/*
	深圳市场SJSTJ结算dbf格式
	证券交易统计库 SJSTJ.DBF
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int SZ_DBF_SJSTJ(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting);

	/*
	深圳市场SJSTJ结算dbf取值
	证券交易统计库 SJSTJ.DBF
	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int SZ_DBF_Value_SJSTJ(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue);

};

#endif