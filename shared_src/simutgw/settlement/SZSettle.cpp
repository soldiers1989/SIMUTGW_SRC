#include "SZSettle.h"

#include <memory>

#include "boost/filesystem.hpp"
#include "boost/date_time.hpp"

#include "config/conf_msg.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "order/define_order_msg.h"

#include "tool_file/TgwDBFOperHelper.h"
#include "tool_file/FileOperHelper.h"

#include "tool_string/Tgw_StringUtil.h"

#include "tool_mysql/MysqlConnectionPool.h"

#include "SettleUtil.h"

SZSettle::SZSettle()
	: m_scl(keywords::channel = "SHSettle"), m_strDbfPostfix(".dbf")
{
	Init();
}


SZSettle::~SZSettle()
{
}

/*
初始化成员变量
*/
int SZSettle::Init()
{
	boost::gregorian::date CurrentDate = boost::gregorian::day_clock::local_day();
	boost::gregorian::date_duration dra(1);

	m_strNow = boost::gregorian::to_iso_string(CurrentDate);
	m_strNextDay = boost::gregorian::to_iso_string(CurrentDate + dra);
	m_strDbfPostfix = ".dbf";

	return 0;
}

/*
结算明细
每个清算池ID都有一份

Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::MakeSettlementDetails(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::string& in_strSettleGroupName, const std::string& in_strFilePath)
{
	static const string ftag("SZSettle::MakeSettlementDetails() ");

	int iRes = Gen_SJSMXn(in_mysqlConn, in_strSettleGroupName, in_strFilePath);
	if (0 != iRes)
	{
		return -1;
	}

	iRes = Gen_SJSJG(in_mysqlConn, in_strSettleGroupName, in_strFilePath);
	if (0 != iRes)
	{
		return -1;
	}

	return 0;
}

/*
结算汇总
做一次

Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::MakeSettlementSummary(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strFilePath)
{
	static const string ftag("SZSettle::MakeSettlementSummary() ");

	int iRes = Gen_SJSDZ(in_mysqlConn, in_strFilePath);
	if (0 != iRes)
	{
		return -1;
	}

	return 0;
}


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
int SZSettle::Gen_SJSMXn(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::string& in_strSettleGroupName,
	const std::string& in_strFilePath)
{
	static const string ftag("SZSettle::Gen_SJSMXn() ");

	int iReturn = 0;

	string strQuer("SELECT *,DATE_FORMAT(`trade_time`,'%Y%m%d%H%i%s.000') AS oper_time "
		"FROM `order_match` WHERE trade_market=102 AND ");
	if (in_strSettleGroupName.empty())
	{
		// 无清算池别名
		strQuer += "`settle_group` IS NULL ";
	}
	else
	{
		strQuer += "`settle_group`=\"";
		strQuer += in_strSettleGroupName;
		strQuer += "\" ";
	}
	strQuer += "ORDER BY trade_time ASC";

	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	vector<map<string, struct MySqlCnnC602_DF::DataInRow> > vecAMapRowData;
	vector<map<string, struct MySqlCnnC602_DF::DataInRow> > vecBMapRowData;

	// 查询出所有的深圳市场成交记录
	int iRes = in_mysqlConn->Query(strQuer, &pResultSet, ulAffectedRows);
	if (1 == iRes)
	{
		// select
		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		while ((0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData)))
		{
			std::string strTradeType(mapRowData["trade_type"].strValue);
			int iTradeType = 0;
			Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);

			// 根据股票交易类型划分到不同批次的清算文件
			if (simutgw::TADE_TYPE::a_trade == iTradeType
				|| simutgw::TADE_TYPE::margin_cash == iTradeType
				|| simutgw::TADE_TYPE::margin_stock == iTradeType
				|| simutgw::TADE_TYPE::etf_buy == iTradeType
				|| simutgw::TADE_TYPE::etf_sell == iTradeType
				|| simutgw::TADE_TYPE::etf_crt == iTradeType
				|| simutgw::TADE_TYPE::etf_rdp == iTradeType)
			{
				// A股
				vecAMapRowData.push_back(mapRowData);
			}
			else if (simutgw::b_trade == iTradeType)
			{
				// B股
				vecBMapRowData.push_back(mapRowData);
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Unknown trade_type=[" << mapRowData["trade_type"].strValue;
			}
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "执行" << strQuer << "错误res=" << iRes;

		return -1;
	}

	// 释放
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;

	string strA_FileName(in_strFilePath);
	strA_FileName += "/";
	if (!in_strSettleGroupName.empty())
	{
		// 有清算池别名	
		strA_FileName += in_strSettleGroupName;
		strA_FileName += "_";
	}
	strA_FileName += "SJSMX1_";
	strA_FileName += m_strNow;
	strA_FileName += ".dbf";

	string strB_FileName(in_strFilePath);
	strB_FileName += "/";
	if (!in_strSettleGroupName.empty())
	{
		// 有清算池别名	
		strB_FileName += in_strSettleGroupName;
		strB_FileName += "_";
	}
	strB_FileName += "SJSMX2_";
	strB_FileName += m_strNow;
	strB_FileName += ".dbf";

	// 再生成清算文件
	TgwDBFOperHelper BdbfWriter;

	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	// 不同批次的结构相同
	DBF_SJSMXn(vecSetting);

	// 第一批次
	if (0 != vecAMapRowData.size())
	{
		TgwDBFOperHelper adbfWriter;

		if (!boost::filesystem::exists(strA_FileName))
		{
			// 不存在则创建
			iReturn = adbfWriter.Create(strA_FileName, vecSetting);
			if (0 != iReturn)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "adbfWriter.Create failed file=" << strA_FileName;
				return -1;
			}
		}
		iReturn = adbfWriter.Open(strA_FileName);
		if (0 != iReturn)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "adbfWriter.Open failed file=" << strA_FileName;
			return -1;
		}

		map<string, struct TgwDBFOperHelper_DF::DataInRow> mapDBFValue;
		for (size_t st = 0; st < vecAMapRowData.size(); ++st)
		{
			iReturn = DBF_Value_SJSMX1(vecAMapRowData[st], mapDBFValue);
			if (0 != iReturn)
			{
				adbfWriter.Close();

				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "DBF_Value_SJSMX1 failed file=" << strA_FileName;

				return -1;
			}

			iReturn = adbfWriter.Append(mapDBFValue);
			if (0 != iReturn)
			{
				adbfWriter.Close();

				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "adbfWriter.Append failed file=" << strA_FileName;
				return -1;
			}
		}

		adbfWriter.Close();
	}
	else
	{
		if (in_strSettleGroupName.empty())
		{
			// 无清算池别名
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 SJSMX1.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 " << in_strSettleGroupName << "_SJSMX1.dbf";
		}
	}

	// 第二批次
	if (0 != vecBMapRowData.size())
	{
		TgwDBFOperHelper bdbfWriter;

		if (!boost::filesystem::exists(strB_FileName))
		{
			// 不存在则创建
			iReturn = bdbfWriter.Create(strB_FileName, vecSetting);
			if (0 != iReturn)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "bdbfWriter.Create failed file=" << strB_FileName;
				return -1;
			}
		}
		iReturn = bdbfWriter.Open(strB_FileName);
		if (0 != iReturn)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "bdbfWriter.Open failed file=" << strB_FileName;
			return -1;
		}

		map<string, struct TgwDBFOperHelper_DF::DataInRow> mapDBFValue;
		for (size_t st = 0; st < vecBMapRowData.size(); ++st)
		{
			iReturn = DBF_Value_SJSMX2(vecBMapRowData[st], mapDBFValue);
			if (0 != iReturn)
			{
				bdbfWriter.Close();

				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "DBF_Value_SJSMX2 failed file=" << strB_FileName;
				return -1;
			}

			iReturn = bdbfWriter.Append(mapDBFValue);
			if (0 != iReturn)
			{
				bdbfWriter.Close();

				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "bdbfWriter.Append failed file=" << strB_FileName;
				return -1;
			}
		}

		bdbfWriter.Close();
	}
	else
	{
		if (in_strSettleGroupName.empty())
		{
			// 无清算池别名
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 SJSMX2.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 " << in_strSettleGroupName << "_SJSMX2.dbf";
		}
	}

	return iReturn;
}

/*
深圳市场SJSMXn结算dbf格式
Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::DBF_SJSMXn(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SZSettle::DBF_SJSMXn() ");

	struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;
	//序号 字段名 字段描述 类型 长度 备注
	//	1 MXJSZH 结算账号 C 6
	Column.strName = "MXJSZH";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//2 MXBFZH 备付金账户 C 25
	Column.strName = "MXBFZH";
	Column.cType = 'C';
	Column.uWidth = 25;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//3 MXSJLX 数据类型 C 2
	Column.strName = "MXSJLX";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//4 MXYWLB 业务类别 C 4
	Column.strName = "MXYWLB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//5 MXZQDM 证券代码 C 8
	Column.strName = "MXZQDM";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//6 MXJYDY 交易单元 C 6
	Column.strName = "MXJYDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//7 MXTGDY 托管单元 C 6
	Column.strName = "MXTGDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//8 MXZQZH 证券账户代码 C 20
	Column.strName = "MXZQZH";
	Column.cType = 'C';
	Column.uWidth = 20;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//9		MXDDBH 客户订单编号/申请单号 C 24
	Column.strName = "MXDDBH";
	Column.cType = 'C';
	Column.uWidth = 24;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//10 MXYYB 营业部代码 C 4
	Column.strName = "MXYYB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//11 MXZXBH 执行编号 C 16
	Column.strName = "MXZXBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//12 MXYWLSH 业务流水号 C 16
	Column.strName = "MXYWLSH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//13 MXCJSL 成交数量 N 15,2
	Column.strName = "MXCJSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//14 MXQSSL 清算数量 N 15,2
	Column.strName = "MXQSSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//15 MXCJJG 成交价格 N 13,4
	Column.strName = "MXCJJG";
	Column.cType = 'N';
	Column.uWidth = 13;
	Column.uDecWidth = 4;
	io_vecSetting.push_back(Column);

	//16 MXQSJG 清算价格 N 18,9
	Column.strName = "MXQSJG";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 9;
	io_vecSetting.push_back(Column);

	//17 MXXYJY 信用交易标识 C 1
	Column.strName = "MXXYJY";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//18 MXPCBS 平仓标识 C 1
	Column.strName = "MXPCBS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//19 MXZQLB 证券类别 C 2
	Column.strName = "MXZQLB";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//20 MXZQZL 证券子类别 C 2
	Column.strName = "MXZQZL";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//21 MXGFXZ 股份性质 C 2
	Column.strName = "MXGFXZ";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//22 MXJSFS 交收方式 C 1
	Column.strName = "MXJSFS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//23 MXHBDH 货币代号 C 3
	Column.strName = "MXHBDH";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);
	//24 MXQSBJ 清算本金 N 17,2
	Column.strName = "MXQSBJ";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//25 MXYHS 印花税 N 12,2
	Column.strName = "MXYHS";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//26 MXJYJSF 交易经手费 N 12,2
	Column.strName = "MXJYJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//27 MXJGGF 监管规费 N 12,2
	Column.strName = "MXJGGF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//28 MXGHF 过户费 N 12,2
	Column.strName = "MXGHF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//29 MXJSF 结算费 N 12,2
	Column.strName = "MXJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//30 MXSXF 手续费 N 12,2
	Column.strName = "MXSXF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//31 MXQSYJ 结算参与人佣金 N 12,2 结 算 参与 人 系统自用
	Column.strName = "MXQSYJ";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//32 MXQTFY 其他费用 N 12,2
	Column.strName = "MXQTFY";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//33 MXZJJE 资金金额 N 17,2
	Column.strName = "MXZJJE";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//34 MXSFJE 收付净额 N 18,2
	Column.strName = "MXSFJE";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//35 MXCJRQ 成交日期 C 8 CCYYMMDD
	Column.strName = "MXCJRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//36 MXQSRQ 清算日期 C 8 CCYYMMDD
	Column.strName = "MXQSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//37 MXJSRQ 交收日期 C 8 CCYYMMDD
	Column.strName = "MXJSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//38 MXFSRQ 发送日期 C 8 CCYYMMDD
	Column.strName = "MXFSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//39 MXQTRQ 其它日期 C 8 CCYYMMDD
	Column.strName = "MXQTRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//40 MXSCDM 市场代码 C 2 保留字段
	Column.strName = "MXSCDM";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//41 MXJYFS 交易方式 C 2
	Column.strName = "MXJYFS";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//42 MXZQDM2 证券代码2 C 8
	Column.strName = "MXZQDM2";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//43 MXTGDY2 托管单元2 C 6
	Column.strName = "MXTGDY2";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//44 MXDDBH2 订单编号2 C 16 保留字段
	Column.strName = "MXDDBH2";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//45 MXCWDH 错误代号 C 4
	Column.strName = "MXCWDH";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//46 MXPPHM 匹配号码 C 10 备用
	Column.strName = "MXPPHM";
	Column.cType = 'C';
	Column.uWidth = 10;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//47 MXFJSM 附加说明 C 30
	Column.strName = "MXFJSM";
	Column.cType = 'C';
	Column.uWidth = 30;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//48 MXBYBZ 备用标志C 1 结 算 参与 人 系统自用
	Column.strName = "MXBYBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	return 0;
}

/*
深圳市场SJSMX1结算dbf取值
清算明细库 SJSMXn.DBF
Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::DBF_Value_SJSMX1(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, struct TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SZSettle::DBF_Value_SJSMX1() ");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.end();

		//
		//
		std::string strTradeType;		
		string strSecurityid;
		// 组合券代码
		string strSecurity_id2;
		// 是否是Sz ETF交易
		bool bIsSzEtfTrade = false;
		// 是否是Sz ETF代码
		bool bIsSzEtf_securityid = false;
		double dValue = 0.0;

		//
		//

		//序号 字段名 字段描述 类型 长度 备注
		//	1 MXJSZH 结算账号 C 6
		dbfRow.iType = 0;
		dbfRow.strValue = "stgw01";
		out_mapDBFValue["MXJSZH"] = dbfRow;

		//2 MXBFZH 备付金账户 C 25
		dbfRow.iType = 0;
		dbfRow.strValue = "bfzh_12345";
		out_mapDBFValue["MXBFZH"] = dbfRow;

		//3 MXSJLX 数据类型 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["MXSJLX"] = dbfRow;

		//
		citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			strSecurityid = citRow->second.strValue;
		}
		else
		{
			// 未找到该值
			strSecurityid = "";
		}

		citRow = in_mapRowData.find("security_id2");
		if (citRow != in_mapRowData.end())
		{
			strSecurity_id2 = citRow->second.strValue;
		}
		else
		{
			// 未找到该值
			strSecurity_id2 = "";
		}

		// 判断是否是组合券
		if (strSecurity_id2.empty())
		{
			// 不是组合券
			bIsSzEtf_securityid = true;
		}
		else
		{
			// 是组合券
			bIsSzEtf_securityid = false;
		}

		//4 MXYWLB 业务类别 C 4
		dbfRow.iType = 0;
		citRow = in_mapRowData.find("trade_type");
		if (citRow != in_mapRowData.end())
		{
			int iTradeType = 0;
			strTradeType = citRow->second.strValue;
			Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);

			// 根据股票交易类型划分到不同批次的清算文件
			if (simutgw::TADE_TYPE::a_trade == iTradeType
				|| simutgw::TADE_TYPE::margin_cash == iTradeType
				|| simutgw::TADE_TYPE::margin_stock == iTradeType
				|| simutgw::TADE_TYPE::etf_buy == iTradeType
				|| simutgw::TADE_TYPE::etf_sell == iTradeType)
			{
				// JY00 集中竞价平台交易， 综合金融服务平台交易（担保交收模式）
				dbfRow.strValue = "JY00";
			}
			else if (simutgw::TADE_TYPE::etf_crt == iTradeType)
			{
				// 是ETF交易
				bIsSzEtfTrade = true;
				if (bIsSzEtf_securityid)
				{
					// 是ETF代码
					// SGSF 申购份额，包括单市场ETF
					dbfRow.strValue = "SGSF";
				}
				else
				{
					// 是组合证券代码
					// SGSQ 申购组合券，包括单市场ETF
					dbfRow.strValue = "SGSQ";
				}
			}
			else if (simutgw::TADE_TYPE::etf_rdp == iTradeType)
			{
				// 是ETF交易
				bIsSzEtfTrade = true;
				if (bIsSzEtf_securityid)
				{
					// 是ETF代码
					// SHSF 赎回份额，包括单市场ETF
					dbfRow.strValue = "SHSF";
				}
				else
				{
					// 是组合证券代码
					// SHSQ 赎回组合券，包括单市场ETF
					dbfRow.strValue = "SHSQ";
				}
			}
			else
			{
				dbfRow.strValue = "JY00";
			}
		}
		else
		{
			// 未找到该值

		}
		out_mapDBFValue["MXYWLB"] = dbfRow;

		//5 MXZQDM 证券代码 C 8
		dbfRow.iType = 0;
		dbfRow.strValue = strSecurityid;
		out_mapDBFValue["MXZQDM"] = dbfRow;

		//6 MXJYDY 交易单元 C 6
		citRow = in_mapRowData.find("security_seat");
		if (in_mapRowData.end() != citRow)
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXJYDY"] = dbfRow;
		}
		else
		{
			// 未找到该值

		}

		//7 MXTGDY 托管单元 C 6 同上
		out_mapDBFValue["MXTGDY"] = dbfRow;

		//8 MXZQZH 证券账户代码 C 20
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXZQZH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//9		MXDDBH 客户订单编号/申请单号 C 24
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXDDBH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//10 MXYYB 营业部代码 C 4
		citRow = in_mapRowData.find("market_branchid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXYYB"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//11 MXZXBH 执行编号 C 16
		citRow = in_mapRowData.find("execid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXZXBH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//12 MXYWLSH 业务流水号 C 16
		citRow = in_mapRowData.find("orderid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXYWLSH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//13 MXCJSL 成交数量 N 15,2
		citRow = in_mapRowData.find("match_qty");
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	如果是卖，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::String2Double_atof(strValue, dbfRow.dValue);

			out_mapDBFValue["MXCJSL"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//14 MXQSSL 清算数量 N 15,2
		if (bIsSzEtfTrade)
		{
			// ETF交易
		}
		else
		{
			// 非ETF交易
			out_mapDBFValue["MXQSSL"] = dbfRow;
		}


		//15 MXCJJG 成交价格 N 13,4
		if (bIsSzEtfTrade)
		{
			// ETF交易
		}
		else
		{
			// 非ETF交易
			citRow = in_mapRowData.find("match_price");
			if (citRow != in_mapRowData.end())
			{
				dbfRow.iType = 3;

				Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 4);

				out_mapDBFValue["MXCJJG"] = dbfRow;
			}
			else
			{
				// 未找到该值
			}
		}

		//16 MXQSJG 清算价格 N 18,9
		if (bIsSzEtfTrade)
		{
			// ETF交易
		}
		else
		{
			// 非ETF交易
			out_mapDBFValue["MXQSJG"] = dbfRow;
		}

		//17 MXXYJY 信用交易标识 C 1
		if (bIsSzEtfTrade)
		{
			// ETF交易
		}
		else
		{
			// 非ETF交易
			citRow = in_mapRowData.find("trade_type");
			if (citRow != in_mapRowData.end())
			{
				dbfRow.iType = 0;

				std::string strTradeType(citRow->second.strValue);
				int iTradeType = 0;
				Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);
				if (simutgw::margin_cash == iTradeType)
				{  // 融资交易--融资买入 买券还券
					dbfRow.strValue = "1";
				}
				else if (simutgw::margin_stock == iTradeType)
				{  // 融券交易--融券卖出 卖券还款
					dbfRow.strValue = "2";
				}
				else
				{
					dbfRow.strValue.clear();
				}

				out_mapDBFValue["MXXYJY"] = dbfRow;
			}
			else
			{
				// 未找到该值
			}
		}

		//18 MXPCBS 平仓标识 C 1

		//19 MXZQLB 证券类别 C 2
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF交易
			dbfRow.strValue = "EF";
		}
		else
		{
			// 非ETF交易			
			dbfRow.strValue = "00";
		}
		out_mapDBFValue["MXZQLB"] = dbfRow;

		//20 MXZQZL 证券子类别 C 2		
		if (bIsSzEtfTrade)
		{
			// ETF交易
			dbfRow.iType = 0;
			dbfRow.strValue = "0";
			// EF 0 单市场 ETF
			out_mapDBFValue["MXZQZL"] = dbfRow;
		}
		else
		{
			// 非ETF交易			
		}

		//21 MXGFXZ 股份性质 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["MXGFXZ"] = dbfRow;

		//22 MXJSFS 交收方式 C 1
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF交易
			if (bIsSzEtf_securityid)
			{
				// 是ETF代码
				dbfRow.strValue = "Y";
			}
			else
			{
				dbfRow.strValue = "A";
			}
		}
		else
		{
			// 非ETF交易		
			dbfRow.strValue = "A";
		}
		out_mapDBFValue["MXJSFS"] = dbfRow;

		//23 MXHBDH 货币代号 C 3
		dbfRow.iType = 0;
		dbfRow.strValue = "RMB";
		out_mapDBFValue["MXHBDH"] = dbfRow;

		//
		//
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			string strValue;

			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1) 
				|| 0 == citRowSide->second.strValue.compare("D"))
			{
				//	如果是买，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dValue, 2);
		}
		else
		{
			// 未找到该值
		}

		//24 MXQSBJ 清算本金 N 17,2	
		if (bIsSzEtfTrade)
		{
			// ETF交易
			if (bIsSzEtf_securityid)
			{
				// 是ETF代码		

			}
			else
			{
			}
		}
		else
		{
			// 非ETF交易		
			dbfRow.iType = 3;
			dbfRow.dValue = dValue;
			out_mapDBFValue["MXQSBJ"] = dbfRow;
		}

		//25 MXYHS 印花税 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXYHS"] = dbfRow;

		//26 MXJYJSF 交易经手费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJYJSF"] = dbfRow;

		//27 MXJGGF 监管规费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJGGF"] = dbfRow;

		//28 MXGHF 过户费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXGHF"] = dbfRow;

		//29 MXJSF 结算费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJSF"] = dbfRow;

		//30 MXSXF 手续费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXSXF"] = dbfRow;

		//31 MXQSYJ 结算参与人佣金 N 12,2 结 算 参与 人 系统自用
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXQSYJ"] = dbfRow;

		//32 MXQTFY 其他费用 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXQTFY"] = dbfRow;

		//33 MXZJJE 资金金额 N 17,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXZJJE"] = dbfRow;

		//34 MXSFJE 收付净额 N 18,2
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			string strValue;
			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1) 
				|| 0 == citRowSide->second.strValue.compare("D"))
			{
				//	如果是买，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dValue, 2);


		}
		else
		{
			// 未找到该值
		}

		if (bIsSzEtfTrade)
		{
			// ETF交易
			if (bIsSzEtf_securityid)
			{
				// 是ETF代码		

			}
			else
			{
			}
		}
		else
		{
			// 非ETF交易
			dbfRow.iType = 3;
			dbfRow.dValue = dValue;
			out_mapDBFValue["MXSFJE"] = dbfRow;
		}

		//35 MXCJRQ 成交日期 C 8 CCYYMMDD T日
		boost::gregorian::date CurrentDate;
		citRow = in_mapRowData.find("oper_time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);

			CurrentDate = boost::gregorian::from_undelimited_string(dbfRow.strValue);

			out_mapDBFValue["MXCJRQ"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//36 MXQSRQ 清算日期 C 8 CCYYMMDD T日
		out_mapDBFValue["MXQSRQ"] = dbfRow;

		//37 MXJSRQ 交收日期 C 8 CCYYMMDD T+1日
		dbfRow.iType = 0;
		boost::gregorian::date_duration dra(1);
		CurrentDate += dra;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		CurrentDate -= dra;

		out_mapDBFValue["MXJSRQ"] = dbfRow;

		//38 MXFSRQ 发送日期 C 8 CCYYMMDD T日
		dbfRow.iType = 0;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		out_mapDBFValue["MXFSRQ"] = dbfRow;

		//39 MXQTRQ 其它日期 C 8 CCYYMMDD

		//40 MXSCDM 市场代码 C 2 保留字段

		//41 MXJYFS 交易方式 C 2
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF交易
			dbfRow.strValue = "03";
		}
		else
		{
			// 非ETF交易
			dbfRow.strValue = "01";
		}
		out_mapDBFValue["MXJYFS"] = dbfRow;

		//42 MXZQDM2 证券代码2 C 8
		if (bIsSzEtfTrade)
		{
			// ETF交易
			if (bIsSzEtf_securityid)
			{
				// 是ETF代码
			}
			else
			{
				dbfRow.iType = 0;
				dbfRow.strValue = strSecurity_id2;

				out_mapDBFValue["MXZQDM2"] = dbfRow;
			}
		}
		else
		{
			// 非ETF交易
		}

		//43 MXTGDY2 托管单元2 C 6

		//44 MXDDBH2 订单编号2 C 16 保留字段

		//45 MXCWDH 错误代号 C 4

		//46 MXPPHM 匹配号码 C 10 备用

		//47 MXFJSM 附加说明 C 30

		//48 MXBYBZ 备用标志C 1 结 算 参与 人 系统自用
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

/*
深圳市场SJSMX2结算dbf取值
清算明细库 SJSMXn.DBF
Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::DBF_Value_SJSMX2(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, struct TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SZSettle::DBF_Value_SJSMX2() ");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;

		//序号 字段名 字段描述 类型 长度 备注
		//	1 MXJSZH 结算账号 C 6

		//2 MXBFZH 备付金账户 C 25

		//3 MXSJLX 数据类型 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "02";
		out_mapDBFValue["MXSJLX"] = dbfRow;

		//4 MXYWLB 业务类别 C 4
		dbfRow.iType = 0;
		dbfRow.strValue = "BG1C";
		out_mapDBFValue["MXYWLB"] = dbfRow;

		//5 MXZQDM 证券代码 C 8
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXZQDM"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//6 MXJYDY 交易单元 C 6
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXJYDY"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//7 MXTGDY 托管单元 C 6 同上
		out_mapDBFValue["MXTGDY"] = dbfRow;

		//8 MXZQZH 证券账户代码 C 20
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXZQZH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//9		MXDDBH 客户订单编号/申请单号 C 24
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXDDBH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//10 MXYYB 营业部代码 C 4
		citRow = in_mapRowData.find("market_branchid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXYYB"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//11 MXZXBH 执行编号 C 16
		citRow = in_mapRowData.find("execid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXZXBH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//12 MXYWLSH 业务流水号 C 16
		citRow = in_mapRowData.find("orderid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXYWLSH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//13 MXCJSL 成交数量 N 15,2
		citRow = in_mapRowData.find("match_qty");
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	如果是卖，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::String2Double_atof(strValue, dbfRow.dValue);

			out_mapDBFValue["MXCJSL"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//14 MXQSSL 清算数量 N 15,2
		out_mapDBFValue["MXQSSL"] = dbfRow;

		//15 MXCJJG 成交价格 N 13,4
		citRow = in_mapRowData.find("match_price");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;

			Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 4);

			out_mapDBFValue["MXCJJG"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//16 MXQSJG 清算价格 N 18,9
		out_mapDBFValue["MXQSJG"] = dbfRow;

		//17 MXXYJY 信用交易标识 C 1

		//18 MXPCBS 平仓标识 C 1

		//19 MXZQLB 证券类别 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "20";
		out_mapDBFValue["MXZQLB"] = dbfRow;

		//20 MXZQZL 证券子类别 C 2

		//21 MXGFXZ 股份性质 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["MXZQLB"] = dbfRow;

		//22 MXJSFS 交收方式 C 1
		dbfRow.iType = 0;
		dbfRow.strValue = "A";
		out_mapDBFValue["MXJSFS"] = dbfRow;

		//23 MXHBDH 货币代号 C 3
		dbfRow.iType = 0;
		dbfRow.strValue = "HKD";
		out_mapDBFValue["MXHBDH"] = dbfRow;

		//24 MXQSBJ 清算本金 N 17,2
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1))
			{
				//	如果是买，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dbfRow.dValue, 2);

			out_mapDBFValue["MXQSBJ"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}


		//25 MXYHS 印花税 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXYHS"] = dbfRow;

		//26 MXJYJSF 交易经手费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJYJSF"] = dbfRow;

		//27 MXJGGF 监管规费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJGGF"] = dbfRow;

		//28 MXGHF 过户费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXGHF"] = dbfRow;

		//29 MXJSF 结算费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJSF"] = dbfRow;

		//30 MXSXF 手续费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXSXF"] = dbfRow;

		//31 MXQSYJ 结算参与人佣金 N 12,2 结 算 参与 人 系统自用
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXQSYJ"] = dbfRow;

		//32 MXQTFY 其他费用 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXQTFY"] = dbfRow;

		//33 MXZJJE 资金金额 N 17,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXZJJE"] = dbfRow;

		//34 MXSFJE 收付净额 N 18,2
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1))
			{
				//	如果是买，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dbfRow.dValue, 2);

			out_mapDBFValue["MXSFJE"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//35 MXCJRQ 成交日期 C 8 CCYYMMDD T日
		string strDate;
		boost::gregorian::date CurrentDate;
		citRow = in_mapRowData.find("oper_time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			strDate = citRow->second.strValue.substr(0, 8);
			CurrentDate = boost::gregorian::from_undelimited_string(strDate);

			dbfRow.strValue = strDate;

			out_mapDBFValue["MXCJRQ"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//36 MXQSRQ 清算日期 C 8 CCYYMMDD T日
		dbfRow.iType = 0;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);

		out_mapDBFValue["MXQSRQ"] = dbfRow;

		//37 MXJSRQ 交收日期 C 8 CCYYMMDD T+3日
		dbfRow.iType = 0;
		boost::gregorian::date_duration dra(3);
		CurrentDate += dra;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		CurrentDate -= dra;

		out_mapDBFValue["MXJSRQ"] = dbfRow;

		//38 MXFSRQ 发送日期 C 8 CCYYMMDD T日
		dbfRow.iType = 0;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);

		out_mapDBFValue["MXFSRQ"] = dbfRow;

		//39 MXQTRQ 其它日期 C 8 CCYYMMDD T日
		out_mapDBFValue["MXQTRQ"] = dbfRow;

		//40 MXSCDM 市场代码 C 2 保留字段

		//41 MXJYFS 交易方式 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["MXJYFS"] = dbfRow;

		//42 MXZQDM2 证券代码2 C 8

		//43 MXTGDY2 托管单元2 C 6
		dbfRow.iType = 0;
		dbfRow.strValue = out_mapDBFValue["MXTGDY"].strValue;
		out_mapDBFValue["MXTGDY2"] = dbfRow;

		//44 MXDDBH2 订单编号2 C 16 保留字段

		//45 MXCWDH 错误代号 C 4

		//46 MXPPHM 匹配号码 C 10 备用

		//47 MXFJSM 附加说明 C 30

		//48 MXBYBZ 备用标志C 1 结 算 参与 人 系统自用
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

/*
深圳市场SJSJG结算
Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::Gen_SJSJG(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::string& in_strSettleGroupName,
	const std::string& in_strFilePath)
{
	static const string ftag("SZSettle::Gen_SJSJG() ");

	int iReturn = 0;

	string strQuer("SELECT *,DATE_FORMAT(`trade_time`,'%Y%m%d%H%i%s.000') AS oper_time "
		"FROM `order_match` WHERE trade_market=102 AND ");
	if (in_strSettleGroupName.empty())
	{
		// 无清算池别名
		strQuer += "`settle_group` IS NULL ";
	}
	else
	{
		strQuer += "`settle_group`=\"";
		strQuer += in_strSettleGroupName;
		strQuer += "\" ";
	}
	strQuer += "ORDER BY trade_time ASC";

	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	vector<map<string, struct MySqlCnnC602_DF::DataInRow> > vecMapRowData;

	// 查询出所有的深圳市场成交记录
	int iRes = in_mysqlConn->Query(strQuer, &pResultSet, ulAffectedRows);
	if (1 == iRes)
	{
		// select
		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		while ((0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData)))
		{
			std::string strTradeType(mapRowData["trade_type"].strValue);
			int iTradeType = 0;
			Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);

			/* 过滤出担保业务 ???
			*SJSJG.DBF 接口文件（不分批次），表示明细结果，其内容包含 SJSMXn.DBF
			*中非担保类清算明细的逐笔交收结果、担保类业务交收违约待处置结果以及登记
			*存管类股份和资金明细。
			*/
			if (iTradeType >= simutgw::TADE_TYPE::etf_crt)
			{
				vecMapRowData.push_back(mapRowData);
			}
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "执行" << strQuer << "错误res=" << iRes;

		return -1;
	}

	// 释放
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;

	string strFileName(in_strFilePath);
	strFileName += "/";
	if (!in_strSettleGroupName.empty())
	{
		// 有清算池别名	
		strFileName += in_strSettleGroupName;
		strFileName += "_";
	}
	strFileName += "SJSJG_";
	strFileName += m_strNow;
	strFileName += ".dbf";

	// 再生成清算文件
	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	DBF_SJSJG(vecSetting);

	if (0 != vecMapRowData.size())
	{
		TgwDBFOperHelper dbfWriter;

		if (!boost::filesystem::exists(strFileName))
		{
			// 不存在则创建
			iReturn = dbfWriter.Create(strFileName, vecSetting);
			if (0 != iReturn)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "dbfWriter.Create failed file=" << strFileName;
				return -1;
			}
		}
		iReturn = dbfWriter.Open(strFileName);
		if (0 != iReturn)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "dbfWriter.Open failed file=" << strFileName;
			return -1;
		}

		map<string, struct TgwDBFOperHelper_DF::DataInRow> mapDBFValue;
		for (size_t st = 0; st < vecMapRowData.size(); ++st)
		{
			iReturn = DBF_Value_SJSJG(vecMapRowData[st], mapDBFValue);
			if (-1 == iReturn)
			{
				dbfWriter.Close();

				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "DBF_Value_SJSJG failed file=" << strFileName;

				return -1;
			}
			else if (0 == iReturn)
			{
				iReturn = dbfWriter.Append(mapDBFValue);
				if (0 != iReturn)
				{
					dbfWriter.Close();

					BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "dbfWriter.Append failed file=" << strFileName;
					return -1;
				}
			}
		}

		dbfWriter.Close();
	}
	else
	{
		if (in_strSettleGroupName.empty())
		{
			// 无清算池别名
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 SJSJG.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 " << in_strSettleGroupName << "_SJSJG.dbf";
		}
	}

	return iReturn;
}


/*
深圳市场SJSJG结算dbf格式
明细结果库 SJSJG.DBF
Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::DBF_SJSJG(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SZSettle::DBF_SJSJG() ");

	struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;
	//序号 字段名 字段描述 类型 长度 备注
	//	1 JGJSZH 结算账号 C 6
	Column.strName = "JGJSZH";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//2 JGBFZH 备付金账户 C 25
	Column.strName = "JGBFZH";
	Column.cType = 'C';
	Column.uWidth = 25;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//3 JGSJLX 数据类型 C 2
	Column.strName = "JGSJLX";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//4 JGYWLB 业务类别 C 4
	Column.strName = "JGYWLB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//5 JGZQDM 证券代码 C 8
	Column.strName = "JGZQDM";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//6 JGJYDY 交易单元 C 6
	Column.strName = "JGJYDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//7 JGTGDY 托管单元 C 6
	Column.strName = "JGTGDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//8 JGZQZH 证券账户号码 C 20
	Column.strName = "JGZQZH";
	Column.cType = 'C';
	Column.uWidth = 20;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//9 JGDDBH 客户订单编号 C 24
	Column.strName = "JGDDBH";
	Column.cType = 'C';
	Column.uWidth = 24;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//10 JGYYB 营业部代码 C 4
	Column.strName = "JGYYB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//11 JGZXBH 执行编号 C 16
	Column.strName = "JGZXBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//12 JGYWLSH 业务流水号 C 16
	Column.strName = "JGYWLSH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//13 JGCJSL 成交数量 N 15,2
	Column.strName = "JGCJSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//14 JGQSSL 清算数量 N 15,2
	Column.strName = "JGQSSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//15 JGJSSL 交收数量 N 15,2
	Column.strName = "JGJSSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//16 JGCJJG 成交价格 N 13,4
	Column.strName = "JGCJJG";
	Column.cType = 'N';
	Column.uWidth = 13;
	Column.uDecWidth = 4;
	io_vecSetting.push_back(Column);

	//17 JGQSJG 清算价格 N 18,9
	Column.strName = "JGQSJG";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 9;
	io_vecSetting.push_back(Column);

	//18 JGXYJY 信用交易标识 C 1
	Column.strName = "JGXYJY";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//19 JGPCBS 平仓标识 C 1
	Column.strName = "JGPCBS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//20 JGZQLB 证券类别 C 2
	Column.strName = "JGZQLB";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//21 JGZQZL 证券子类 C 2
	Column.strName = "JGZQZL";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//22 JGGFXZ 股份性质 C 2
	Column.strName = "JGGFXZ";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//23 JGLTLX 流通类型 C 1
	Column.strName = "JGLTLX";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//24 JGJSFS 交收方式 C 1
	Column.strName = "JGJSFS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//25 JGHBDH 货币代号 C 3
	Column.strName = "JGHBDH";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//26 JGQSBJ 清算本金 N 17,2
	Column.strName = "JGQSBJ";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//27 JGYHS 印花税 N 12,2
	Column.strName = "JGYHS";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//28 JGJYJSF 交易经手费 N 12,2
	Column.strName = "JGJYJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//29 JGJGGF 监管规费 N 12,2
	Column.strName = "JGJGGF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//30 JGGHF 过户费 N 12,2
	Column.strName = "JGGHF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//31 JGJSF 结算费 N 12,2
	Column.strName = "JGJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//32 JGSXF 手续费 N 12,2
	Column.strName = "JGSXF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//33 JGQSYJ 结算参与人佣金 N 12,2 结算参与人系统自用
	Column.strName = "JGQSYJ";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//34 JGQTFY 其它费用 N 12,2
	Column.strName = "JGQTFY";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//35 JGZJJE 资金金额 N 17,2
	Column.strName = "JGZJJE";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//36 JGSFJE 收付净额 N 18,2
	Column.strName = "JGSFJE";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//37 JGJSBZ 交收标志 C 1
	Column.strName = "JGJSBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//38 JGZYDH 摘要代号 C 4
	Column.strName = "JGZYDH";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//39 JGCJRQ 成交日期 C 8 CCYYMMDD
	Column.strName = "JGCJRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//40 JGQSRQ 清算日期 C 8 CCYYMMDD
	Column.strName = "JGQSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//41 JGJSRQ 交收日期 C 8 CCYYMMDD
	Column.strName = "JGJSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//42 JGFSRQ 发送日期 C 8 CCYYMMDD
	Column.strName = "JGFSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//43 JGQTRQ 其它日期 C 8 CCYYMMDD
	Column.strName = "JGQTRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//44 JGSCDM 市场代码 C 2
	Column.strName = "JGSCDM";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//45 JGJYFS 交易方式 C 2
	Column.strName = "JGJYFS";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//46 JGZQDM2 证券代码 2 C 8
	Column.strName = "JGZQDM2";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//47 JGTGDY2 托管单元 2 C 6
	Column.strName = "JGTGDY2";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//48 JGDDBH2 订单编号 2 C 16 保留字段
	Column.strName = "JGDDBH2";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//49 JGFJSM 附加说明 C 100
	Column.strName = "JGFJSM";
	Column.cType = 'C';
	Column.uWidth = 100;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//50 JGBYBZ 备用标志 C 1 结算参与人系统自用
	Column.strName = "JGBYBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	return 0;
}

/*
深圳市场SJSJG结算dbf取值
明细结果库 SJSJG.DBF

Return :
0 -- 成功
1 -- 此条数据无需写入，跳过处理下一条
非0 -- 失败
*/
int SZSettle::DBF_Value_SJSJG(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SZSettle::DBF_Value_SJSJG() ");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.end();

		//
		//
		std::string strTradeType;		
		string strSecurityid;
		// 组合券代码
		string strSecurity_id2;
		// 是否是Sz ETF交易
		bool bIsSzEtfTrade = false;
		// 是否是Sz ETF代码
		bool bIsSzEtf_securityid = false;
		double dValue = 0.0;

		//序号 字段名 字段描述 类型 长度 备注
		//	1 JGJSZH 结算账号 C 6
		dbfRow.iType = 0;
		dbfRow.strValue = "stgw01";
		out_mapDBFValue["JGJSZH"] = dbfRow;

		//2 JGBFZH 备付金账户 C 25
		dbfRow.iType = 0;
		dbfRow.strValue = "bfzh_12345";
		out_mapDBFValue["JGBFZH"] = dbfRow;

		//3 JGSJLX 数据类型 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["JGSJLX"] = dbfRow;

		//
		citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			strSecurityid = citRow->second.strValue;
		}
		else
		{
			// 未找到该值
			strSecurityid = "";
		}

		citRow = in_mapRowData.find("security_id2");
		if (citRow != in_mapRowData.end())
		{
			strSecurity_id2 = citRow->second.strValue;
		}
		else
		{
			// 未找到该值
			strSecurity_id2 = "";
		}

		// 判断是否是组合券
		if (strSecurity_id2.empty())
		{
			// 不是组合券
			bIsSzEtf_securityid = true;
		}
		else
		{
			// 是组合券
			bIsSzEtf_securityid = false;
		}

		//4 JGYWLB 业务类别 C 4
		dbfRow.iType = 0;
		citRow = in_mapRowData.find("trade_type");
		if (citRow != in_mapRowData.end())
		{
			int iTradeType = 0;
			
			strTradeType = citRow->second.strValue;
			Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);

			// 根据股票交易类型划分到不同批次的清算文件
			if (simutgw::TADE_TYPE::a_trade == iTradeType
				|| simutgw::TADE_TYPE::margin_cash == iTradeType
				|| simutgw::TADE_TYPE::margin_stock == iTradeType
				|| simutgw::TADE_TYPE::etf_buy == iTradeType
				|| simutgw::TADE_TYPE::etf_sell == iTradeType)
			{
				// JY00	集中交易或综合协议交易（担保交收）交收违约的待处分证券回报
				dbfRow.strValue = "JY00";
			}
			else if (simutgw::TADE_TYPE::etf_crt == iTradeType)
			{
				// 是ETF交易
				bIsSzEtfTrade = true;
				if (bIsSzEtf_securityid)
				{
					// 是ETF代码
					// EFXC ETF申购赎回现金差额
					dbfRow.strValue = "EFXC";
				}
				else
				{
					// 是组合证券代码
					return 1;
				}
			}
			else if (simutgw::TADE_TYPE::etf_rdp == iTradeType)
			{
				// 是ETF交易
				bIsSzEtfTrade = true;
				if (bIsSzEtf_securityid)
				{
					// 是ETF代码
					// EFXC ETF申购赎回现金差额
					dbfRow.strValue = "EFXC";
				}
				else
				{
					// 是组合证券代码
					return 1;
				}
			}
			else
			{
				dbfRow.strValue = "JY00";
			}
		}
		else
		{
			// 未找到该值

		}
		out_mapDBFValue["JGYWLB"] = dbfRow;

		//5 JGZQDM 证券代码 C 8
		dbfRow.iType = 0;
		dbfRow.strValue = strSecurityid;
		out_mapDBFValue["JGZQDM"] = dbfRow;

		//6 JGJYDY 交易单元 C 6
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGJYDY"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//7 JGTGDY 托管单元 C 6
		out_mapDBFValue["JGTGDY"] = dbfRow;

		//8 JGZQZH 证券账户号码 C 20
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGZQZH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//9 JGDDBH 客户订单编号 C 24
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGDDBH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//10 JGYYB 营业部代码 C 4
		citRow = in_mapRowData.find("market_branchid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGYYB"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//11 JGZXBH 执行编号 C 16
		citRow = in_mapRowData.find("execid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGZXBH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//12 JGYWLSH 业务流水号 C 16
		citRow = in_mapRowData.find("orderid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGYWLSH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//13 JGCJSL 成交数量 N 15,2
		citRow = in_mapRowData.find("match_qty");
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	如果是卖，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::String2Double_atof(strValue, dbfRow.dValue);

			out_mapDBFValue["JGCJSL"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//14 JGQSSL 清算数量 N 15,2
		out_mapDBFValue["JGQSSL"] = dbfRow;

		//15 JGJSSL 交收数量 N 15, 2
		out_mapDBFValue["JGJSSL"] = dbfRow;

		//16 JGCJJG 成交价格 N 13,4
		citRow = in_mapRowData.find("match_price");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;

			Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 4);

			out_mapDBFValue["JGCJJG"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//17 JGQSJG 清算价格 N 18,9
		out_mapDBFValue["JGQSJG"] = dbfRow;

		//18 JGXYJY 信用交易标识 C 1
		citRow = in_mapRowData.find("trade_type");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;

			std::string strTradeType(citRow->second.strValue);
			int iTradeType = 0;
			Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);
			if (simutgw::margin_cash == iTradeType)
			{  // 融资交易--融资买入 买券还券
				dbfRow.strValue = "1";
			}
			else if (simutgw::margin_stock == iTradeType)
			{  // 融券交易--融券卖出 卖券还款
				dbfRow.strValue = "2";
			}
			else
			{
				dbfRow.strValue.clear();
			}

			out_mapDBFValue["JGXYJY"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//19 JGPCBS 平仓标识 C 1

		//20 JGZQLB 证券类别 C 2
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF交易
			dbfRow.strValue = "EF";
		}
		else
		{
			// 非ETF交易			
			dbfRow.strValue = "00";
		}
		out_mapDBFValue["JGZQLB"] = dbfRow;

		//21 JGZQZL 证券子类 C 2
		if (bIsSzEtfTrade)
		{
			// ETF交易
			dbfRow.iType = 0;
			dbfRow.strValue = "0";
			// EF 0 单市场 ETF
			out_mapDBFValue["JGZQZL"] = dbfRow;
		}
		else
		{
			// 非ETF交易			
		}

		//22 JGGFXZ 股份性质 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["JGGFXZ"] = dbfRow;

		//23 JGLTLX 流通类型 C 1

		//24 JGJSFS 交收方式 C 1
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF交易
			if (bIsSzEtf_securityid)
			{
				// 是ETF代码
				dbfRow.strValue = "Z";
			}
			else
			{
			}
		}
		else
		{
			// 非ETF交易		
			dbfRow.strValue = "A";
		}
		out_mapDBFValue["JGJSFS"] = dbfRow;

		//25 JGHBDH 货币代号 C 3
		dbfRow.iType = 0;
		dbfRow.strValue = "RMB";
		out_mapDBFValue["JGHBDH"] = dbfRow;

		//
		//
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1))
			{
				//	如果是买，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dValue, 2);


		}
		else
		{
			// 未找到该值
		}

		//26 JGQSBJ 清算本金 N 17, 2
		dbfRow.iType = 3;
		dbfRow.dValue = dValue;
		out_mapDBFValue["JGQSBJ"] = dbfRow;

		//27 JGYHS 印花税 N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGYHS"] = dbfRow;

		//28 JGJYJSF 交易经手费 N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJYJSF"] = dbfRow;

		//29 JGJGGF 监管规费 N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJGGF"] = dbfRow;

		//30 JGGHF 过户费 N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGGHF"] = dbfRow;

		//31 JGJSF 结算费 N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJSF"] = dbfRow;

		//32 JGSXF 手续费 N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGSXF"] = dbfRow;

		//33 JGQSYJ 结算参与人佣金 N 12, 2 结算参与人系统自用
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGQSYJ"] = dbfRow;

		//34 JGQTFY 其它费用 N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGQTFY"] = dbfRow;

		//35 JGZJJE 资金金额 N 17, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGZJJE"] = dbfRow;

		//36 JGSFJE 收付净额 N 18, 2
		dbfRow.iType = 3;
		dbfRow.dValue = dValue;
		out_mapDBFValue["JGSFJE"] = dbfRow;

		//37 JGJSBZ 交收标志 C 1
		// JGJSBZ(交收标志)，‘Y’: 交收成功； ‘N’: 交收失败
		if (bIsSzEtfTrade)
		{
			// ETF交易
			if (bIsSzEtf_securityid)
			{
				// 是ETF代码
				dbfRow.iType = 0;
				dbfRow.strValue = "Y";
				out_mapDBFValue["JGJSBZ"] = dbfRow;
			}
			else
			{
			}
		}
		else
		{
			// 非ETF交易
		}

		//38 JGZYDH 摘要代号 C 4

		//39 JGCJRQ 成交日期 C 8 CCYYMMDD
		boost::gregorian::date CurrentDate;
		citRow = in_mapRowData.find("oper_time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);

			CurrentDate = boost::gregorian::from_undelimited_string(dbfRow.strValue);

			out_mapDBFValue["JGCJRQ"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//40 JGQSRQ 清算日期 C 8 CCYYMMDD
		out_mapDBFValue["JGQSRQ"] = dbfRow;

		//41 JGJSRQ 交收日期 C 8 CCYYMMDD
		dbfRow.iType = 0;
		boost::gregorian::date_duration dra(1);
		CurrentDate += dra;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		CurrentDate -= dra;

		out_mapDBFValue["JGJSRQ"] = dbfRow;

		//42 JGFSRQ 发送日期 C 8 CCYYMMDD
		dbfRow.iType = 0;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		out_mapDBFValue["JGFSRQ"] = dbfRow;

		//43 JGQTRQ 其它日期 C 8 CCYYMMDD

		//44 JGSCDM 市场代码 C 2

		//45 JGJYFS 交易方式 C 2
		/*
		MXJYFS（交易方式），表示业务委托的或渠道。‘01’ ： 普通交易 (经深交所 )，‘02’：综合金融服务平台交易 (经深交所 )，‘‘03’ ：经深交所申报的非交易业务，
		‘04’ ：经中国结算深圳分公司D-COM系统申报的非交易业务， ‘05’ ：经中国结算总公司基金业务平台申报的，
		‘06’ ：经中国结算总公司TA系统申报的业务， ‘07’ ：经其它渠道申报的业务，‘08’ ：中国结算深圳分公司前台业务， ‘09’ ：中国结算深圳分公司特殊处理， ‘ ’ ：其它。
		*/
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF交易
			dbfRow.strValue = "03";
		}
		else
		{
			// 非ETF交易
			dbfRow.strValue = "01";
		}
		out_mapDBFValue["JGJYFS"] = dbfRow;

		//46 JGZQDM2 证券代码 2 C 8

		//47 JGTGDY2 托管单元 2 C 6

		//48 JGDDBH2 订单编号 2 C 16 保留字段
		//49 JGFJSM 附加说明 C 100
		//50 JGBYBZ 备用标志 C 1 结算参与人系统自用
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}


/*
深圳市场SJSTJ结算
股份结算对帐库 SJSDZ.DBF
Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::Gen_SJSDZ(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const string& strFilePath)
{
	static const string ftag("SZSettle::Gen_SJSDZ() ");

	string strA_DbfPrefix(strFilePath);

	string strA_FileName(strA_DbfPrefix);
	strA_FileName += "/SJSDZ_";

	std::string strA_FileNameNextDay(strA_FileName);

	strA_FileName += m_strNow;
	strA_FileName += ".dbf";

	strA_FileNameNextDay += m_strNextDay;
	strA_FileNameNextDay += ".dbf";

	string strQuer("SELECT *,DATE_FORMAT(`oper_time`,'%Y%m%d%H%i%s.000') AS time "
		"FROM `stock_asset` WHERE trade_market=102 "
		"UNION "
		"SELECT *, DATE_FORMAT(`oper_time`,'%Y%m%d%H%i%s.000') AS time "
		"FROM `stock_etf_asset` WHERE trade_market=102 "
		"ORDER BY oper_time ASC;");

	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	vector<map<string, struct MySqlCnnC602_DF::DataInRow> > vecAMapRowData;

	// 查询出所有的深圳市场成交记录
	int iRes = in_mysqlConn->Query(strQuer, &pResultSet, ulAffectedRows);
	if (1 == iRes)
	{
		// select
		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		while ((0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData)))
		{
			// A股
			vecAMapRowData.push_back(mapRowData);
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "执行" << strQuer << "错误res=" << iRes;

		return -1;
	}

	// 释放
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;

	if (0 != vecAMapRowData.size())
	{
		int iReturn = Gen_DBF_SJSDZ(vecAMapRowData, strA_FileName, false);
		if (0 != iReturn)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Gen_DBF_SJSDZ failed file=" << strA_FileName;
			return -1;
		}

		iReturn = Gen_DBF_SJSDZ(vecAMapRowData, strA_FileNameNextDay, true);
		if (0 != iReturn)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Gen_DBF_SJSDZ failed file=" << strA_FileNameNextDay;
			return -1;
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成SJSDZ.dbf";
	}

	return 0;
}

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
int SZSettle::Gen_DBF_SJSDZ(vector<map<string, struct MySqlCnnC602_DF::DataInRow> >& in_vecMapRowData,
	const std::string& strFilePath, bool bIsNextDay)
{
	static const string ftag("SZSettle::Gen_DBF_SJSDZ() ");

	int iReturn = 0;

	// 再生成清算文件
	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	// 不同批次的结构相同
	DBF_SJSDZ(vecSetting);

	TgwDBFOperHelper adbfWriter;

	if (!boost::filesystem::exists(strFilePath))
	{
		// 不存在则创建
		iReturn = adbfWriter.Create(strFilePath, vecSetting);
		if (0 != iReturn)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "adbfWriter.Create failed file=" << strFilePath;
			return -1;
		}
	}

	iReturn = adbfWriter.Open(strFilePath);
	if (0 != iReturn)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "adbfWriter.Open failed file=" << strFilePath;
		return -1;
	}

	map<string, struct TgwDBFOperHelper_DF::DataInRow> mapDBFValue;
	for (size_t st = 0; st < in_vecMapRowData.size(); ++st)
	{
		iReturn = DBF_Value_SJSDZ(in_vecMapRowData[st], mapDBFValue, bIsNextDay);
		if (0 != iReturn)
		{
			adbfWriter.Close();

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "DBF_Value_SJSDZ failed file=" << strFilePath;
			return -1;
		}

		iReturn = adbfWriter.Append(mapDBFValue);
		if (0 != iReturn)
		{
			adbfWriter.Close();

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "adbfWriter.Append failed file=" << strFilePath;
			return -1;
		}
	}

	adbfWriter.Close();

	return 0;
}

/*
深圳市场SJSDZ结算dbf格式
股份结算对帐库 SJSDZ.DBF
Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::DBF_SJSDZ(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SZSettle::DBF_SJSDZ() ");

	try
	{
		struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

		//序号 字段名 字段描述 类型 长度 备注
		//1 DZTGDY 托管单元 C 6
		Column.strName = "DZTGDY";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//2 DZZQDM 证券代码 C 8
		Column.strName = "DZZQDM";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//3 DZZQZH 证券账户 C 20
		Column.strName = "DZZQZH";
		Column.cType = 'C';
		Column.uWidth = 20;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//4 DZGFXZ 股份性质 C 2
		Column.strName = "DZGFXZ";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//5 DZLTLX 流通类型 C 1
		Column.strName = "DZLTLX";
		Column.cType = 'C';
		Column.uWidth = 1;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//6 DZZYGS 总拥股数 N 17,2
		Column.strName = "DZZYGS";
		Column.cType = 'N';
		Column.uWidth = 17;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//7 DZFSRQ 发送日期 C 8 CCYYMMDD
		Column.strName = "DZFSRQ";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//8 DZBYBZ 备用标志 C 1 结算参与人系统自用
		Column.strName = "DZBYBZ";
		Column.cType = 'C';
		Column.uWidth = 1;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);
	}
	catch (exception &e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}

	return 0;
}

/*
深圳市场SJSTJ结算dbf取值
股份结算对帐库 SJSDZ.DBF
Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::DBF_Value_SJSDZ(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue,
	bool bIsNextDay)
{
	static const string ftag("SZSettle::DBF_Value_SJSDZ() ");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.end();

		//序号 字段名 字段描述 类型 长度 备注
		//1 DZTGDY 托管单元 C 6
		dbfRow.iType = 0;
		dbfRow.strValue = "stgw01";
		out_mapDBFValue["DZTGDY"] = dbfRow;

		//2 DZZQDM 证券代码 C 8
		citRow = in_mapRowData.find("stock_id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["DZZQDM"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//3 DZZQZH 证券账户 C 20
		citRow = in_mapRowData.find("account_id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["DZZQZH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//4 DZGFXZ 股份性质 C 2
		// ‘00 ’：无限售流通股；
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["DZGFXZ"] = dbfRow;

		//5 DZLTLX 流通类型 C 1
		// ‘0’：无特殊限制条件，是否售由其“股份性质”决定；
		dbfRow.iType = 0;
		dbfRow.strValue = "0";
		out_mapDBFValue["DZLTLX"] = dbfRow;

		//6 DZZYGS 总拥股数 N 17,2
		citRow = in_mapRowData.find("stock_available");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			Tgw_StringUtil::String2Double_atof(citRow->second.strValue, dbfRow.dValue);

			out_mapDBFValue["DZZYGS"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		if (bIsNextDay)
		{
			double dTemp = 0;
			citRow = in_mapRowData.find("stock_auction_purchase_balance");
			if (citRow != in_mapRowData.end())
			{
				Tgw_StringUtil::String2Double_atof(citRow->second.strValue, dTemp);
				out_mapDBFValue["DZZYGS"].dValue += dTemp;
			}
			citRow = in_mapRowData.find("stock_staple_purchase_balance");
			if (citRow != in_mapRowData.end())
			{
				Tgw_StringUtil::String2Double_atof(citRow->second.strValue, dTemp);
				out_mapDBFValue["DZZYGS"].dValue += dTemp;
			}
			citRow = in_mapRowData.find("stock_etf_redemption_balance");
			if (citRow != in_mapRowData.end())
			{
				Tgw_StringUtil::String2Double_atof(citRow->second.strValue, dTemp);
				out_mapDBFValue["DZZYGS"].dValue += dTemp;
			}
		}

		//7 DZFSRQ 发送日期 C 8 CCYYMMDD
		citRow = in_mapRowData.find("time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);

			out_mapDBFValue["DZFSRQ"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//8 DZBYBZ 备用标志 C 1 结算参与人系统自用

	}
	catch (exception &e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}

	return 0;
}

/*
深圳市场SJSTJ结算dbf格式
Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::SZ_DBF_SJSTJ(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SZSettle::SZ_DBF_SJSTJ() ");

	struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;
	//序号 字段名 字段描述 类型 长度 备注
	//	1 JGJSZH 结算账号 C 6
	Column.strName = "JGJSZH";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//2 JGBFZH 备付金账户 C 25
	Column.strName = "JGBFZH";
	Column.cType = 'C';
	Column.uWidth = 25;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//3 JGSJLX 数据类型 C 2
	Column.strName = "JGSJLX";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//4 JGYWLB 业务类别 C 4
	Column.strName = "JGYWLB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//5 JGZQDM 证券代码 C 8
	Column.strName = "JGZQDM";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//6 JGJYDY 交易单元 C 6
	Column.strName = "JGJYDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//7 JGTGDY 托管单元 C 6
	Column.strName = "JGTGDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//8 JGZQZH 证券账户号码 C 20
	Column.strName = "JGZQZH";
	Column.cType = 'C';
	Column.uWidth = 20;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//9 JGDDBH 客户订单编号 C 24
	Column.strName = "JGDDBH";
	Column.cType = 'C';
	Column.uWidth = 24;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//10 JGYYB 营业部代码 C 4
	Column.strName = "JGYYB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//11 JGZXBH 执行编号 C 16
	Column.strName = "JGZXBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//12 JGYWLSH 业务流水号 C 16
	Column.strName = "JGYWLSH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//13 JGCJSL 成交数量 N 15,2
	Column.strName = "JGCJSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//14 JGQSSL 清算数量 N 15,2
	Column.strName = "JGQSSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//15 JGJSSL 交收数量 N 15,2
	Column.strName = "JGJSSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 4;
	io_vecSetting.push_back(Column);

	//16 JGCJJG 成交价格 N 13,4
	Column.strName = "JGCJJG";
	Column.cType = 'N';
	Column.uWidth = 13;
	Column.uDecWidth = 4;
	io_vecSetting.push_back(Column);

	//17 JGQSJG 清算价格 N 18,9
	Column.strName = "JGQSJG";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 9;
	io_vecSetting.push_back(Column);

	//18 JGXYJY 信用交易标识 C 1
	Column.strName = "JGXYJY";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//19 JGPCBS 平仓标识 C 1
	Column.strName = "JGPCBS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//20 JGZQLB 证券类别 C 2
	Column.strName = "JGZQLB";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//21 JGZQZL 证券子类 C 2
	Column.strName = "JGZQZL";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//22 JGGFXZ 股份性质 C 2
	Column.strName = "JGGFXZ";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//23 JGJSFS 交收方式 C 1
	Column.strName = "JGJSFS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//24 JGHBDH 货币代号 C 3
	Column.strName = "JGHBDH";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//25 JGQSBJ 清算本金 N 17,2
	Column.strName = "JGQSBJ";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//26 JGYHS 印花税 N 12,2
	Column.strName = "JGYHS";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//27 JGJYJSF 交易经手费 N 12,2
	Column.strName = "JGJYJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//28 JGJGGF 监管规费 N 12,2
	Column.strName = "JGJGGF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//29 JGGHF 过户费 N 12,2
	Column.strName = "JGGHF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//30 JGJSF 结算费 N 12,2
	Column.strName = "JGJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//31 JGSXF 手续费 N 12,2
	Column.strName = "JGSXF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//32 JGQSYJ 结算参与人佣金 N 12,2 结算参与人系统自用
	Column.strName = "JGQSYJ";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//33 JGQTFY 其它费用 N 12,2
	Column.strName = "JGQTFY";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//34 JGZJJE 资金金额 N 17,2
	Column.strName = "JGZJJE";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//35 JGSFJE 收付净额 N 18,2
	Column.strName = "JGSFJE";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//36 JGJSBZ 交收标志 C 1
	Column.strName = "JGJSBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//37 JGZYDH 摘要代号 C 4
	Column.strName = "JGZYDH";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//38 JGCJRQ 成交日期 C 8 CCYYMMDD
	Column.strName = "JGCJRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//39 JGQSRQ 清算日期 C 8 CCYYMMDD
	Column.strName = "JGQSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//40 JGJSRQ 交收日期 C 8 CCYYMMDD
	Column.strName = "JGJSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//41 JGFSRQ 发送日期 C 8 CCYYMMDD
	Column.strName = "JGFSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//42 JGQTRQ 其它日期 C 8 CCYYMMDD
	Column.strName = "JGQTRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//43 JGSCDM 市场代码 C 2
	Column.strName = "JGSCDM";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//44 JGJYFS 交易方式 C 2
	Column.strName = "JGJYFS";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//45 JGZQDM2 证券代码 2 C 8
	Column.strName = "JGZQDM2";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//46 JGTGDY2 托管单元 2 C 6
	Column.strName = "JGTGDY2";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//47 JGDDBH2 订单编号 2 C 16 保留字段
	Column.strName = "JGDDBH2";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//48 JGFJSM 附加说明 C 100
	Column.strName = "JGFJSM";
	Column.cType = 'C';
	Column.uWidth = 30;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//49 JGBYBZ 备用标志 C 1
	Column.strName = "JGBYBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	return 0;
}

/*
深圳市场SJSTJ结算dbf取值
Return :
0 -- 成功
非0 -- 失败
*/
int SZSettle::SZ_DBF_Value_SJSTJ(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SZSettle::SZ_DBF_Value_SJSTJ() ");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;

		//序号 字段名 字段描述 类型 长度 备注
		//	1 JGJSZH 结算账号 C 6

		//2 JGBFZH 备付金账户 C 25

		//3 JGSJLX 数据类型 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["JGSJLX"] = dbfRow;

		//4 JGYWLB 业务类别 C 4
		dbfRow.iType = 0;
		dbfRow.strValue = "JY00";
		out_mapDBFValue["JGYWLB"] = dbfRow;

		//5 JGZQDM 证券代码 C 8
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGZQDM"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//6 JGJYDY 交易单元 C 6
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGJYDY"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//7 JGTGDY 托管单元 C 6
		out_mapDBFValue["JGTGDY"] = dbfRow;

		//8 JGZQZH 证券账户号码 C 20
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGZQZH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//9 JGDDBH 客户订单编号 C 24
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGDDBH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//10 JGYYB 营业部代码 C 4
		citRow = in_mapRowData.find("market_branchid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGYYB"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//11 JGZXBH 执行编号 C 16
		citRow = in_mapRowData.find("execid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGZXBH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//12 JGYWLSH 业务流水号 C 16
		citRow = in_mapRowData.find("orderid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGYWLSH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//13 JGCJSL 成交数量 N 15,2
		citRow = in_mapRowData.find("match_qty");
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	如果是卖，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::String2Double_atof(strValue, dbfRow.dValue);

			out_mapDBFValue["JGCJSL"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//14 JGQSSL 清算数量 N 15,2
		out_mapDBFValue["JGQSSL"] = dbfRow;

		//15 JGJSSL 交收数量 N 15,2
		out_mapDBFValue["JGJSSL"] = dbfRow;

		//16 JGCJJG 成交价格 N 13,4
		citRow = in_mapRowData.find("match_price");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;

			Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 4);

			out_mapDBFValue["JGCJJG"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//17 JGQSJG 清算价格 N 18,9
		out_mapDBFValue["JGQSJG"] = dbfRow;

		//18 JGXYJY 信用交易标识 C 1
		citRow = in_mapRowData.find("trade_type");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;

			std::string strTradeType(citRow->second.strValue);
			int iTradeType = 0;
			Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);
			if (simutgw::margin_cash == iTradeType)
			{  // 融资交易--融资买入 买券还券
				dbfRow.strValue = "1";
			}
			else if (simutgw::margin_stock == iTradeType)
			{  // 融券交易--融券卖出 卖券还款
				dbfRow.strValue = "2";
			}
			else
			{
				//
			}

			out_mapDBFValue["JGXYJY"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//19 JGPCBS 平仓标识 C 1

		//20 JGZQLB 证券类别 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["JGZQLB"] = dbfRow;

		//21 JGZQZL 证券子类 C 2

		//22 JGGFXZ 股份性质 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["JGGFXZ"] = dbfRow;

		//23 JGJSFS 交收方式 C 1
		dbfRow.iType = 0;
		dbfRow.strValue = "N";
		out_mapDBFValue["JGJSFS"] = dbfRow;

		//24 JGHBDH 货币代号 C 3
		dbfRow.iType = 0;
		dbfRow.strValue = "RMB";
		out_mapDBFValue["JGHBDH"] = dbfRow;

		//25 JGQSBJ 清算本金 N 17,2
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1))
			{
				//	如果是买，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dbfRow.dValue, 2);

			out_mapDBFValue["JGQSBJ"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}


		//26 JGYHS 印花税 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGYHS"] = dbfRow;

		//27 JGJYJSF 交易经手费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJYJSF"] = dbfRow;

		//28 JGJGGF 监管规费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJGGF"] = dbfRow;

		//29 JGGHF 过户费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGGHF"] = dbfRow;

		//30 JGJSF 结算费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJSF"] = dbfRow;

		//31 JGSXF 手续费 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGSXF"] = dbfRow;

		//32 JGQSYJ 结算参与人佣金 N 12,2 结算参与人系统自用
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGQSYJ"] = dbfRow;

		//33 JGQTFY 其它费用 N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGQTFY"] = dbfRow;

		//34 JGZJJE 资金金额 N 17,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGZJJE"] = dbfRow;

		//35 JGSFJE 收付净额 N 18,2
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1))
			{
				//	如果是买，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dbfRow.dValue, 2);

			out_mapDBFValue["JGSFJE"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		// 36 JGJSBZ 交收标志 C 1

		//37 JGZYDH 摘要代号 C 4

		//38 JGCJRQ 成交日期 C 8 CCYYMMDD
		boost::gregorian::date CurrentDate;
		citRow = in_mapRowData.find("oper_time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);

			CurrentDate = boost::gregorian::from_undelimited_string(dbfRow.strValue);

			out_mapDBFValue["JGCJRQ"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//39 JGQSRQ 清算日期 C 8 CCYYMMDD
		out_mapDBFValue["JGQSRQ"] = dbfRow;

		//40 JGJSRQ 交收日期 C 8 CCYYMMDD
		dbfRow.iType = 0;
		boost::gregorian::date_duration dra(1);
		CurrentDate += dra;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		CurrentDate -= dra;

		out_mapDBFValue["JGJSRQ"] = dbfRow;

		//41 JGFSRQ 发送日期 C 8 CCYYMMDD
		dbfRow.iType = 0;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		out_mapDBFValue["JGFSRQ"] = dbfRow;

		//42 JGQTRQ 其它日期 C 8 CCYYMMDD

		//43 JGSCDM 市场代码 C 2

		//44 JGJYFS 交易方式 C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["JGJYFS"] = dbfRow;

		//45 JGZQDM2 证券代码 2 C 8

		//46 JGTGDY2 托管单元 2 C 6

		//47 JGDDBH2 订单编号 2 C 16 保留字段

		//48 JGFJSM 附加说明 C 100

		//49 JGBYBZ 备用标志
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

