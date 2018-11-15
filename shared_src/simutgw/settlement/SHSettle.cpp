#include "SHSettle.h"

#include "boost/filesystem.hpp"
#include "boost/date_time.hpp"

#include "config/conf_msg.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "order/define_order_msg.h"

#include "tool_file/TgwDBFOperHelper.h"
#include "tool_file/FileOperHelper.h"

#include "tool_string/Tgw_StringUtil.h"

#include "tool_mysql/MysqlConnectionPool.h"

SHSettle::SHSettle()
	:m_scl(keywords::channel = "SHSettle")
{
	Init();
}

SHSettle::~SHSettle()
{
}

/*
初始化成员变量
*/
int SHSettle::Init()
{
	boost::gregorian::date CurrentDate = boost::gregorian::day_clock::local_day();
	boost::gregorian::date_duration dra(1);

	m_strNow = boost::gregorian::to_iso_string(CurrentDate);
	m_strNextDay = boost::gregorian::to_iso_string(CurrentDate + dra);

	return 0;
}

/*
结算明细
每个清算池ID都有一份

Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::MakeSettlementDetails(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strSettleGroupName, const std::string& in_strFilePath)
{
	static const string ftag("SHSettle::MakeSettlementDetails() ");

	int iRes = Gen_gh(in_mysqlConn, in_strSettleGroupName, in_strFilePath);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Gen_gh failed";
		return -1;
	}

	iRes = Gen_bc1(in_mysqlConn, in_strSettleGroupName, in_strFilePath);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Gen_bc1 failed";

		return -1;
	}

	iRes = Gen_zqbd(in_mysqlConn, in_strSettleGroupName, in_strFilePath);
	if (0 != iRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Gen_zqbd failed";

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
int SHSettle::MakeSettlementSummary(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strFilePath)
{
	static const string ftag("SHSettle::MakeSettlementSummary() ");

	int iRes = Gen_zqye_inTwoDays(in_mysqlConn, in_strFilePath);
	if (0 != iRes)
	{
		return -1;
	}
	return 0;
}

/*
上海市场gh结算
过户数据接口 gh.dbf

Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::Gen_gh(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::string& in_strSettleGroupName,
	const string& in_strFilePath)
{
	static const string ftag("SHSettle::Gen_gh() ");

	string strDbfPrefix(in_strFilePath);

	//	以seat席位为key，存下对应的value
	map<string, vector< map<string, struct MySqlCnnC602_DF::DataInRow> > >mapSeatDbfValue;

	int iReturn = 0;

	std::string strTrans;
	string strQuer("SELECT *,DATE_FORMAT(`trade_time`,'%Y%m%d%H%i%s') AS trade_time_trans, "
		"DATE_FORMAT(`order_time`,'%Y%m%d%H%i%s') AS order_time_trans "
		"FROM `order_match` WHERE trade_market=101 AND ");

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

	strQuer += "AND (trade_type=";
	strQuer += sof_string::itostr(simutgw::a_trade, strTrans);
	strQuer += " OR trade_type=";
	strQuer += sof_string::itostr(simutgw::margin_cash, strTrans);
	strQuer += " OR trade_type=";
	strQuer += sof_string::itostr(simutgw::margin_stock, strTrans);

	strQuer += ") ORDER BY trade_time ASC";
	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	// 查询
	int iRes = in_mysqlConn->Query(strQuer, &pResultSet, ulAffectedRows);
	if (1 == iRes)
	{
		// select
		string strKey;
		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		while ((0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData)))
		{
			map<string, struct MySqlCnnC602_DF::DataInRow>::iterator mapIt = mapRowData.find("security_seat");
			if (mapRowData.end() != mapIt)
			{
				strKey = mapIt->second.strValue;
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "没有席位，id[" << mapRowData["id"].strValue << "]";

				continue;
			}

			mapSeatDbfValue[strKey].push_back(mapRowData);
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "执行" << strQuer << "错误res=" << iRes;

		iReturn = -1;
	}

	// 释放
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;

	if (0 != iReturn)
	{
		return iReturn;
	}

	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	iRes = DBF_gh(vecSetting);

	// 根据查询到的结果生成DBF文件
	map<string, vector< map<string, struct MySqlCnnC602_DF::DataInRow> > >::iterator mapIterSeat = mapSeatDbfValue.begin();
	if (mapSeatDbfValue.end() == mapIterSeat)
	{
		if (in_strSettleGroupName.empty())
		{
			// 无清算池别名
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 gh.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 " << in_strSettleGroupName << "_gh.dbf";
		}

		return 0;
	}

	// write dbf file
	map<string, struct TgwDBFOperHelper_DF::DataInRow> mapDBFValue;
	TgwDBFOperHelper dbfWriter;

	string strFileName(strDbfPrefix);
	strFileName += "/";
	if (!in_strSettleGroupName.empty())
	{
		// 有清算池别名	
		strFileName += in_strSettleGroupName;
		strFileName += "_";
	}
	strFileName += "gh_";
	strFileName += m_strNow;
	strFileName += ".dbf";

	if (!boost::filesystem::exists(strFileName))
	{
		// 如果该dbf文件不存在
		iRes = dbfWriter.Create(strFileName, vecSetting);
		if (0 != iRes)
		{
			return -1;
		}

	}

	iRes = dbfWriter.Open(strFileName);
	if (0 != iRes)
	{
		return -1;
	}

	while (mapSeatDbfValue.end() != mapIterSeat)
	{
		for (size_t st = 0; st < mapIterSeat->second.size(); ++st)
		{
			//遍历每一个seat对应的记录
			iRes = DBF_Value_gh((mapIterSeat->second)[st], mapDBFValue);
			if (0 != iRes)
			{
				return -1;
			}

			iRes = dbfWriter.Append(mapDBFValue);
			if (0 != iRes)
			{
				return -1;
			}
		}
		dbfWriter.Close();

		++mapIterSeat;
	}

	return iReturn;
}


/*
上海市场gh结算dbf格式
过户数据接口 gh.dbf
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::DBF_gh(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SHSettle::DBF_gh() ");

	try
	{
		struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

		//序号	字段名	字段描述	类型
		//1	Gddm	证券账户	C10
		Column.strName = "Gddm";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//2	Gdxm	股东姓名，新交易系统切换后，上交所会将该字段置为空格。请以中国证券登记结算有限责任公司发布的数据为准。	C8
		Column.strName = "Gdxm";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//3	Bcrq	成交日期，格式为YYYYMMDD	C8
		Column.strName = "Bcrq";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//4	Cjbh	成交编号	N8
		Column.strName = "Cjbh";
		Column.cType = 'N';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//5	Gsdm	席位号	C5
		Column.strName = "Gsdm";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//6	Cjsl	成交数量	N10
		Column.strName = "Cjsl";
		Column.cType = 'N';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//7	Bcye	本次余额，新交易系统切换后，该字段被置为0。	N10
		Column.strName = "Bcye";
		Column.cType = 'N';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//8	zqdm	证券代码	C6
		Column.strName = "zqdm";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//9	sbsj	申报时间，格式为HHMMSS	C6
		Column.strName = "sbsj";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//10	cjsj	成交时间，格式为HHMMSS	C6
		Column.strName = "cjsj";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);


		//11	cjjg	成交价格	N8(3)
		Column.strName = "cjjg";
		Column.cType = 'N';
		Column.uWidth = 8;
		Column.uDecWidth = 3;
		io_vecSetting.push_back(Column);

		//12	cjje	成交金额，溢出处理参见实时成交回报接口成交金额字段	N12(2)
		Column.strName = "cjje";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//13	sqbh	会员内部订单号，同申报接口中的reff（字段4：会员内部订单号）。	C10
		Column.strName = "sqbh";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//14	bs	B 普通订单，买卖方向：买入S 普通订单，买卖方向：卖出	C1
		Column.strName = "bs";
		Column.cType = 'C';
		Column.uWidth = 1;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//15	mjbh	操作员代码	C5
		Column.strName = "mjbh";
		Column.cType = 'C';
		Column.uWidth = 5;
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
上海市场gh结算dbf取值
过户数据接口 gh.dbf
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::DBF_Value_gh(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SHSettle::DBF_Value_gh() ");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;

		//序号	字段名	字段描述	类型
		//1	Gddm	证券账户	C10
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["Gddm"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//2	Gdxm	股东姓名，新交易系统切换后，上交所会将该字段置为空格。请以中国证券登记结算有限责任公司发布的数据为准。	C8
		dbfRow.iType = 0;
		dbfRow.strValue = "        ";

		out_mapDBFValue["Gdxm"] = dbfRow;

		//3	Bcrq	成交日期，格式为YYYYMMDD	C8
		citRow = in_mapRowData.find("trade_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);

			out_mapDBFValue["Bcrq"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//4	Cjbh	成交编号	N8
		citRow = in_mapRowData.find("id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 1;
			Tgw_StringUtil::String2Long_atol(citRow->second.strValue, dbfRow.lValue);

			out_mapDBFValue["Cjbh"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//5	Gsdm	席位号	C5
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["Gsdm"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//6	Cjsl	成交数量	N10
		citRow = in_mapRowData.find("match_qty");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 1;
			Tgw_StringUtil::String2Long_atol(citRow->second.strValue, dbfRow.lValue);

			out_mapDBFValue["Cjsl"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//7	Bcye	本次余额，新交易系统切换后，该字段被置为0。	N10
		dbfRow.iType = 1;
		dbfRow.lValue = 0;

		out_mapDBFValue["Bcye"] = dbfRow;

		//8	zqdm	证券代码	C6
		citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["zqdm"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//9	sbsj	申报时间，格式为HHMMSS	C6
		citRow = in_mapRowData.find("order_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(8, 6);

			out_mapDBFValue["sbsj"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//10	cjsj	成交时间，格式为HHMMSS	C6
		citRow = in_mapRowData.find("trade_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(8, 6);

			out_mapDBFValue["cjsj"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//11	cjjg	成交价格	N8(3)
		citRow = in_mapRowData.find("match_price");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 3);

			out_mapDBFValue["cjjg"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//12	cjje	成交金额，溢出处理参见实时成交回报接口成交金额字段	N12(2)
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 2);

			out_mapDBFValue["cjje"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//13	sqbh	会员内部订单号，同申报接口中的reff（字段4：会员内部订单号）。	C10
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["sqbh"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowTradeTye = in_mapRowData.find("trade_type");

		//14	bs	
		/*
		B 普通订单，买卖方向：买入S 普通订单，买卖方向：卖出	C1
		1 担保品买入订单，券入投资者信用帐户，申报接口中(owflag, BS)字段取值为(“_XY”,“B”)；
		2 担保品卖出订单，券从投资者信用帐户出，申报接口中(owflag, BS)字段取值为(“_XY”,“S”)；
		3 融资买入订单，券入投资者信用帐户，申报接口中(owflag, BS)字段取值为(“_RZ”,“B”)；
		4 卖券还款订单，券从投资者信用帐户出，申报接口中(owflag, BS)字段取值为(“_RZ”,“S”)；
		5 买券还券订单，券入证券公司融券专用帐户，申报接口中(owflag, BS)字段取值为(“_RQ”,“B”)；
		6 融券卖出订单，券从证券公司融券专用帐户出，申报接口中(owflag, BS)字段取值为(“_RQ”,“S”)；
		7 平仓买入订单，券入证券公司融券专用帐户，申报接口中(owflag, BS)字段取值为(“_PC”,“B”)；
		8 平仓卖出订单，券从投资者信用帐户出，申报接口中(owflag, BS)字段取值为(“_PC”,“S”)；

		*/
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			int iTradeType = 0;
			Tgw_StringUtil::String2Int_atoi(citRowTradeTye->second.strValue, iTradeType);

			dbfRow.strValue = citRowSide->second.strValue;

			/*
			if (simutgw::a_trade == iTradeType ||
				simutgw::b_trade == iTradeType)
			{ // 普通订单
				if (0 == citRowSide->second.strValue.compare("1"))
				{
					// 买
					dbfRow.strValue = "B";
				}
				else if (0 == citRowSide->second.strValue.compare("2"))
				{
					// 卖
					dbfRow.strValue = "S";
				}
			}
			else if (simutgw::margin_cash == iTradeType)
			{ // 融资
				if (0 == citRowSide->second.strValue.compare("1"))
				{
					// 买
					dbfRow.strValue = "3";
				}
				else if (0 == citRowSide->second.strValue.compare("2"))
				{
					// 卖
					dbfRow.strValue = "4";
				}
			}
			else if (simutgw::margin_stock == iTradeType)
			{ // 融券
				if (0 == citRowSide->second.strValue.compare("1"))
				{
					// 买
					dbfRow.strValue = "5";
				}
				else if (0 == citRowSide->second.strValue.compare("2"))
				{
					// 卖
					dbfRow.strValue = "6";
				}
			}
			*/

			out_mapDBFValue["bs"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//15	mjbh	操作员代码	C5
		dbfRow.iType = 0;
		dbfRow.strValue = "simu ";
		out_mapDBFValue["mjbh"] = dbfRow;

	}
	catch (exception &e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}

	return 0;
}

/*
上海市场bc1结算
过户数据接口 bc1.dbf

Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::Gen_bc1(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::string& in_strSettleGroupName,
	const string& in_strFilePath)
{
	static const string ftag("SHSettle::Gen_bc1() ");

	string strDbfPrefix(in_strFilePath);

	//	以账号为key，存下对应的value
	map<string, vector< map<string, struct MySqlCnnC602_DF::DataInRow> > >mapSeatDbfValue;

	int iReturn = 0;

	std::string strTrans;
	string strQuer("SELECT *,DATE_FORMAT(`trade_time`,'%Y%m%d%H%i%s') AS trade_time_trans, "
		"DATE_FORMAT(`order_time`,'%Y%m%d%H%i%s') AS order_time_trans "
		"FROM `order_match` WHERE trade_market=101 AND ");

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

	strQuer += "AND trade_type=";
	strQuer += sof_string::itostr(simutgw::TADE_TYPE::b_trade, strTrans);
	strQuer += " ORDER BY trade_time ASC";
	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	// 查询
	int iRes = in_mysqlConn->Query(strQuer, &pResultSet, ulAffectedRows);
	if (1 == iRes)
	{
		// select
		string strKey;
		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		while ((0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData)))
		{
			map<string, struct MySqlCnnC602_DF::DataInRow>::iterator mapIt = mapRowData.find("security_account");
			if (mapRowData.end() != mapIt)
			{
				strKey = mapIt->second.strValue;
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "没有账号，id[" << mapRowData["id"].strValue;

				continue;
			}

			mapSeatDbfValue[strKey].push_back(mapRowData);
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "执行" << strQuer << "错误res=" << iRes;

		iReturn = -1;
	}

	// 释放
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;

	if (0 != iReturn)
	{
		return iReturn;
	}

	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	iRes = DBF_bc1(vecSetting);

	// 根据查询到的结果生成DBF文件
	map<string, vector< map<string, struct MySqlCnnC602_DF::DataInRow> > >::iterator mapIterSeat = mapSeatDbfValue.begin();
	if (mapSeatDbfValue.end() == mapIterSeat)
	{
		if (in_strSettleGroupName.empty())
		{
			// 无清算池别名
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 bc1.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 " << in_strSettleGroupName << "_bc1.dbf";
		}

		return 0;
	}

	map<string, struct TgwDBFOperHelper_DF::DataInRow> mapDBFValue;

	TgwDBFOperHelper dbfWriter;

	string strFileName(strDbfPrefix);
	strFileName += "/";
	if (!in_strSettleGroupName.empty())
	{
		// 有清算池别名
		strFileName += in_strSettleGroupName;
		strFileName += "_";
	}
	strFileName += "bc1_";
	strFileName += m_strNow;
	strFileName += ".dbf";

	if (!boost::filesystem::exists(strFileName))
	{
		// 如果该dbf文件不存在
		iRes = dbfWriter.Create(strFileName, vecSetting);
		if (0 != iRes)
		{
			return -1;
		}

	}

	iRes = dbfWriter.Open(strFileName);
	if (0 != iRes)
	{
		return -1;
	}

	while (mapSeatDbfValue.end() != mapIterSeat)
	{
		for (size_t st = 0; st < mapIterSeat->second.size(); ++st)
		{
			//遍历每一个seat对应的记录
			iRes = DBF_Value_bc1((mapIterSeat->second)[st], mapDBFValue);
			if (0 != iRes)
			{
				return -1;
			}

			iRes = dbfWriter.Append(mapDBFValue);
			if (0 != iRes)
			{
				return -1;
			}
		}
		dbfWriter.Close();

		++mapIterSeat;
	}

	return iReturn;
}

/*
上海市场bc1结算dbf格式
过户数据接口 bc1.dbf
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::DBF_bc1(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SHSettle::DBF_bc1() ");

	try
	{
		struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

		//序号	字段名	字段描述	类型
		//1 SSCCRC_ID Character 15 交收编号
		Column.strName = "SSCCRC_ID";
		Column.cType = 'C';
		Column.uWidth = 15;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//2 SSE_ORDER Character 16 委托编号
		Column.strName = "SSE_ORDER";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//3 SSE_REF Character 16 成交编号
		Column.strName = "SSE_REF";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//4 INV_CODE Character 10 证券帐户代码
		Column.strName = "INV_CODE";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//5 SEC_CODE Character 6 证券代码
		Column.strName = "SEC_CODE";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//6 TRADE_DATE Character 8 交易日期
		Column.strName = "TRADE_DATE";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//7 BS Character 1 买卖标志
		Column.strName = "BS";
		Column.cType = 'C';
		Column.uWidth = 1;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//8 PRICE Numeric 8,3 成交价格
		Column.strName = "PRICE";
		Column.cType = 'N';
		Column.uWidth = 8;
		Column.uDecWidth = 3;
		io_vecSetting.push_back(Column);

		//9 VOLUME Numeric 10,0 成交数量
		Column.strName = "VOLUME";
		Column.cType = 'N';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//10 TRADE_VAL Numeric 13,2 交易金额
		Column.strName = "TRADE_VAL";
		Column.cType = 'N';
		Column.uWidth = 13;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//11 ORDER_TIME Character 6 委托时间
		Column.strName = "ORDER_TIME";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//12 TRADE_TIME Character 6 成交时间
		Column.strName = "TRADE_TIME";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//13 SEAT_ID Character 5 交易单元
		Column.strName = "SEAT_ID";
		Column.cType = 'C';
		Column.uWidth = 5;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//14 TRADE_CP Character 5 交易单元所属结算参与人
		Column.strName = "TRADE_CP";
		Column.cType = 'C';
		Column.uWidth = 5;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//15 SETTLE_CP Character 5 交收结算参与人
		Column.strName = "SETTLE_CP";
		Column.cType = 'C';
		Column.uWidth = 5;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//16 CP_ORDER Character 16 申请编号
		Column.strName = "CP_ORDER";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//17 SETTLEDATE Character 8 交收日期
		Column.strName = "SETTLEDATE";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//18 STAMP_DUTY Numeric 12,2 印花税
		Column.strName = "STAMP_DUTY";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//19 SSE_CHARGE Numeric 12,2 经手费
		Column.strName = "SSE_CHARGE";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//20 TRANSF_FEE Numeric 12,2 过户费
		Column.strName = "TRANSF_FEE";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//21 CLEAR_FEE Numeric 12,2 结算费
		Column.strName = "CLEAR_FEE";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//22 OTHER_FEE Numeric 12,2 监管费
		Column.strName = "OTHER_FEE";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//23 TOTAL_FEE Numeric 12,2 费用合计
		Column.strName = "TOTAL_FEE";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//24 SETTLE_AMT Numeric 13,2 交收金额
		Column.strName = "SETTLE_AMT";
		Column.cType = 'N';
		Column.uWidth = 13;
		Column.uDecWidth = 2;
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
上海市场bc1结算dbf取值
过户数据接口 bc1.dbf
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::DBF_Value_bc1(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SHSettle::DBF_Value_bc1()");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;

		//序号	字段名	字段描述	类型
		//1 SSCCRC_ID Character 15 交收编号
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SSCCRC_ID"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//2 SSE_ORDER Character 16 委托编号
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SSE_ORDER"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//3 SSE_REF Character 16 成交编号
		citRow = in_mapRowData.find("orderid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SSE_REF"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//4 INV_CODE Character 10 证券帐户代码
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["INV_CODE"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//5 SEC_CODE Character 6 证券代码
		citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SEC_CODE"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//6 TRADE_DATE Character 8 交易日期
		citRow = in_mapRowData.find("trade_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);

			out_mapDBFValue["TRADE_DATE"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//7 BS Character 1 买卖标志
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRowSide->second.strValue;

			/*
			if (0 == citRowSide->second.strValue.compare("1"))
			{
				dbfRow.strValue = "B";
			}
			else if (0 == citRowSide->second.strValue.compare("2"))
			{
				dbfRow.strValue = "S";
			}
			*/

			out_mapDBFValue["BS"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//8 PRICE Numeric 8,3 成交价格
		citRow = in_mapRowData.find("match_price");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue = citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dbfRow.dValue, 3);

			out_mapDBFValue["PRICE"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//9 VOLUME Numeric 10,0 成交数量
		citRow = in_mapRowData.find("match_qty");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;

			string strValue;
			if (0 == citRowSide->second.strValue.compare("2"))
			{
				// 卖为负
				strValue = "-";
			}
			strValue += citRow->second.strValue;
			Tgw_StringUtil::String2Double_atof(strValue, dbfRow.dValue);

			out_mapDBFValue["VOLUME"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//10 TRADE_VAL Numeric 13,2 交易金额
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;

			string strValue;
			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_SELL_S)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_SELL_2))
			{
				// 买为负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dbfRow.dValue, 2);

			out_mapDBFValue["TRADE_VAL"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//11 ORDER_TIME Character 6 委托时间
		citRow = in_mapRowData.find("order_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(8, 6);

			out_mapDBFValue["ORDER_TIME"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//12 TRADE_TIME Character 6 成交时间
		citRow = in_mapRowData.find("trade_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(8, 6);

			out_mapDBFValue["TRADE_TIME"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}


		//13 SEAT_ID Character 5 交易单元
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SEAT_ID"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}


		//14 TRADE_CP Character 5 交易单元所属结算参与人


		//15 SETTLE_CP Character 5 交收结算参与人


		//16 CP_ORDER Character 16 申请编号
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["CP_ORDER"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}


		//17 SETTLEDATE Character 8 交收日期
		string strDate;
		boost::gregorian::date CurrentDate;
		citRow = in_mapRowData.find("order_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			strDate = citRow->second.strValue.substr(0, 8);
			CurrentDate = boost::gregorian::from_undelimited_string(strDate);

			boost::gregorian::date_duration dra(3);
			CurrentDate += dra;
			dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
			CurrentDate -= dra;

			out_mapDBFValue["SETTLEDATE"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//18 STAMP_DUTY Numeric 12,2 印花税
		dbfRow.iType = 3;
		dbfRow.dValue = 0;

		out_mapDBFValue["STAMP_DUTY"] = dbfRow;

		//19 SSE_CHARGE Numeric 12,2 经手费
		out_mapDBFValue["SSE_CHARGE"] = dbfRow;

		//20 TRANSF_FEE Numeric 12,2 过户费
		out_mapDBFValue["TRANSF_FEE"] = dbfRow;

		//21 CLEAR_FEE Numeric 12,2 结算费
		out_mapDBFValue["CLEAR_FEE"] = dbfRow;

		//22 OTHER_FEE Numeric 12,2 监管费
		out_mapDBFValue["OTHER_FEE"] = dbfRow;

		//23 TOTAL_FEE Numeric 12,2 费用合计
		out_mapDBFValue["TOTAL_FEE"] = dbfRow;

		//24 SETTLE_AMT Numeric 13,2 交收金额
		out_mapDBFValue["SETTLE_AMT"] = out_mapDBFValue["TRADE_VAL"];

	}
	catch (exception &e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}

	return 0;
}


/*
上海市场zqbd结算
zqbd(证券变动文件)
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::Gen_zqbd(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strSettleGroupName, const string& in_strFilePath)
{
	static const string ftag("SHSettle::Gen_zqbd() ");

	string strQuer("SELECT *,DATE_FORMAT(`trade_time`,'%Y%m%d%H%i%s.000') AS oper_time "
		"FROM `order_match` WHERE trade_market=101 AND ");
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

	// 查询出所有的上海市场成交记录
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

			vecMapRowData.push_back(mapRowData);
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "执行" << strQuer << "错误res=" << iRes;

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
	strFileName += "zqbd_";
	strFileName += m_strNow;
	strFileName += ".dbf";

	// 再生成清算文件
	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	DBF_zqbd(vecSetting);

	if (0 == vecMapRowData.size())
	{

		if (in_strSettleGroupName.empty())
		{
			// 无清算池别名
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 zqbd.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成 " << in_strSettleGroupName << "_zqbd.dbf";
		}
	}
	else
	{
		TgwDBFOperHelper dbfWriter;

		int iReturn = 0;
		
		if (!boost::filesystem::exists(strFileName))
		{
			// 不存在则创建
			iReturn = dbfWriter.Create(strFileName, vecSetting);
			if (0 != iReturn)
			{
				return -1;
			}
		}
		iReturn = dbfWriter.Open(strFileName);
		if (0 != iReturn)
		{
			return -1;
		}

		map<string, struct TgwDBFOperHelper_DF::DataInRow> mapDBFValue;
		for (size_t st = 0; st < vecMapRowData.size(); ++st)
		{
			iReturn = DBF_Value_zqbd(vecMapRowData[st], mapDBFValue);
			if (0 != iReturn)
			{
				dbfWriter.Close();
				return -1;
			}

			iReturn = dbfWriter.Append(mapDBFValue);
			if (0 != iReturn)
			{
				dbfWriter.Close();
				return -1;
			}
		}

		dbfWriter.Close();
	}

	return 0;
}

/*
上海市场zqbd结算dbf格式
zqbd(证券变动文件)
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::DBF_zqbd(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SHSettle::DBF_zqbd() ");

	try
	{
		struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

		//序号	字段名	字段描述	类型
		//1 SCDM Character 2 市场代码
		Column.strName = "SCDM";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//2 QSBH Character 8托 管 会 员 的 清 算 编号，对于 QFII 账户，填写托管银行的清算编号
		Column.strName = "QSBH";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//3 ZQZH Character 10 证券账户
		Column.strName = "ZQZH";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//4 XWH Character 5 交易单元
		Column.strName = "XWH";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//5 ZQDM Character 6 证券代码
		Column.strName = "ZQDM";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//6 ZQLB Character 2 证券类别
		Column.strName = "ZQLB";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//7 LTLX Character 1 流通类型
		Column.strName = "LTLX";
		Column.cType = 'C';
		Column.uWidth = 1;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//8 QYLB Character 2 权益类别
		Column.strName = "QYLB";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//9 GPNF Character 4 挂牌年份
		Column.strName = "GPNF";
		Column.cType = 'C';
		Column.uWidth = 4;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//10 BDSL Character 16 999999999999999 变动数量
		Column.strName = "BDSL";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//11 BDLX Character 3 变动类型
		Column.strName = "BDLX";
		Column.cType = 'C';
		Column.uWidth = 3;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//12 BDRQ Character 8 变动日期
		Column.strName = "BDRQ";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//13 SL Character 16 999999999999999 数量(保留字段)
		Column.strName = "SL";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//14 BH Character 20 编号(保留字段)
		Column.strName = "BH";
		Column.cType = 'C';
		Column.uWidth = 20;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//15 BY Character 20 备用
		Column.strName = "BY";
		Column.cType = 'C';
		Column.uWidth = 20;
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
上海市场zqbd结算
zqbd(证券变动文件)
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::DBF_Value_zqbd(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	out_mapDBFValue.clear();

	struct TgwDBFOperHelper_DF::DataInRow dbfRow;

	//NO 字段名 类型 长 度 格式定义 说明
	//1 SCDM Character 2 市场代码
	dbfRow.iType = 0;
	dbfRow.strValue = "01";

	//2 QSBH Character 8 托管会员的清算编号，对于QFII账户，填写托管银行的清算编号

	//3 ZQZH Character 10 证券账户
	map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("security_account");
	if (in_mapRowData.end() != citRow)
	{
		dbfRow.iType = 0;
		dbfRow.strValue = citRow->second.strValue;
	}

	//4 XWH Character 5 交易单元
	citRow = in_mapRowData.find("security_seat");
	if (in_mapRowData.end() != citRow)
	{
		dbfRow.iType = 0;
		dbfRow.strValue = citRow->second.strValue;
	}

	//5 ZQDM Character 6 证券代码
	citRow = in_mapRowData.find("securityid");
	if (in_mapRowData.end() != citRow)
	{
		dbfRow.iType = 0;
		dbfRow.strValue = citRow->second.strValue;
	}

	//6 ZQLB Character 2 证券类别
	dbfRow.iType = 0;
	dbfRow.strValue = "PT";

	//7 LTLX Character 1 流通类型
	dbfRow.iType = 0;
	dbfRow.strValue = "0";

	//8 QYLB Character 2 权益类别


	//9 GPNF Character 4 挂牌年份


	//10 BDSL Character 16 999999999999999 变动数量 有符号
	citRow = in_mapRowData.find("match_qty");
	if (citRow != in_mapRowData.end())
	{
		dbfRow.iType = 1;
		Tgw_StringUtil::String2Long_atol(citRow->second.strValue, dbfRow.lValue);

		out_mapDBFValue["BDSL"] = dbfRow;
	}
	else
	{
		// 未找到该值
	}

	//11 BDLX Character 3 变动类型
	//???

	//12 BDRQ Character 8 变动日期
	citRow = in_mapRowData.find("trade_time_trans");
	if (citRow != in_mapRowData.end())
	{
		dbfRow.iType = 0;
		dbfRow.strValue = citRow->second.strValue.substr(0, 8);

		out_mapDBFValue["BDRQ"] = dbfRow;
	}
	else
	{
		// 未找到该值
	}

	//13 SL Character 16 999999999999999 数量(保留字段)

	//14 BH Character 20 编号(保留字段)

	//15 BY Character 20 备用

	return 0;
}

/*
上海市场zqye结算 含T、T+1日

zqye(证券余额对账文件)
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::Gen_zqye_inTwoDays(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const string& strFilePath)
{
	static const string ftag("SHSettle::Gen_zqye()");

	string strA_DbfPrefix(strFilePath);

	string strA_FileName(strA_DbfPrefix);
	strA_FileName += "/zqye_";

	std::string strA_FileNameNextDay(strA_FileName);

	strA_FileName += m_strNow;
	strA_FileName += ".dbf";

	strA_FileNameNextDay += m_strNextDay;
	strA_FileNameNextDay += ".dbf";

	string strQuer("SELECT *,DATE_FORMAT(`oper_time`,'%Y%m%d%H%i%s.000') AS time "
		"FROM `stock_asset` WHERE trade_market=101 "
		"UNION "
		"SELECT *, DATE_FORMAT(`oper_time`,'%Y%m%d%H%i%s.000') AS time "
		"FROM `stock_etf_asset` WHERE trade_market = 101 "
		"ORDER BY oper_time ASC");

	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	vector<map<string, struct MySqlCnnC602_DF::DataInRow> > vecAMapRowData;

	// 查询出所有的上海市场成交记录
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


	// 再生成清算文件
	if (0 != vecAMapRowData.size())
	{
		int iReturn = 0;
		iReturn = Gen_DBF_zqye(vecAMapRowData, strA_FileName, false);
		if( 0 != iReturn)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Gen_DBF_zqye failed" << strA_FileName;
			return -1;
		}
		
		iReturn = Gen_DBF_zqye(vecAMapRowData, strA_FileNameNextDay, true);
		if( 0 != iReturn)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Gen_DBF_zqye failed" << strA_FileNameNextDay;
			return -1;
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成zqye.dbf";
	}

	return 0;
}

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
int SHSettle::Gen_DBF_zqye(vector<map<string, struct MySqlCnnC602_DF::DataInRow> >& in_vecMapRowData,
	const std::string& strFilePath, bool bIsNextDay)
{
	static const string ftag("SHSettle::Gen_DBF_zqye()");

	int iReturn = 0;

	// 再生成清算文件
	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;
	DBF_zqye(vecSetting);

	TgwDBFOperHelper AdbfWriter;

	if (!boost::filesystem::exists(strFilePath))
	{
		// 不存在则创建
		iReturn = AdbfWriter.Create(strFilePath, vecSetting);
		if (0 != iReturn)
		{
			return -1;
		}
	}
	iReturn = AdbfWriter.Open(strFilePath);
	if (0 != iReturn)
	{
		return -1;
	}

	map<string, struct TgwDBFOperHelper_DF::DataInRow> mapDBFValue;
	for (size_t st = 0; st < in_vecMapRowData.size(); ++st)
	{
		iReturn = DBF_Value_zqye(in_vecMapRowData[st], mapDBFValue, bIsNextDay);
		if (0 != iReturn)
		{
			AdbfWriter.Close();
			return -1;
		}

		iReturn = AdbfWriter.Append(mapDBFValue);
		if (0 != iReturn)
		{
			AdbfWriter.Close();
			return -1;
		}
	}

	AdbfWriter.Close();

	return 0;
}

/*
上海市场zqye00001结算dbf格式
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::DBF_zqye(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SHSettle::DBF_zqye()");

	try
	{
		struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

		//NO 字段名 类型 长度 格式定义 说明
		//1 SCDM Character 2 市场代码
		Column.strName = "SCDM";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//2 QSBH Character 8 托管会员的清算编号，对于 QFII 账户，填写托管银行的清算编号
		Column.strName = "QSBH";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//3 ZQZH Character 10 证券账户
		Column.strName = "ZQZH";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//4 XWH Character 5 指定交易单元
		Column.strName = "XWH";
		Column.cType = 'C';
		Column.uWidth = 5;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//5 ZQDM Character 6 证券代码
		Column.strName = "ZQDM";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//6 ZQLB Character 2 证券类别
		Column.strName = "ZQLB";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//7 LTLX Character 1 流通类型
		Column.strName = "LTLX";
		Column.cType = 'C';
		Column.uWidth = 1;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//8 QYLB Character 2 权益类别
		Column.strName = "QYLB";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//9 GPNF Character 4 挂牌年份
		Column.strName = "GPNF";
		Column.cType = 'C';
		Column.uWidth = 4;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//10 YE1 Character 16 999999999999999 余额1（ 全部余额）
		Column.strName = "YE1";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//11 YE2 Character 16 999999999999999余额2 （冻结证券数量, 含权利受限登记部分）
		Column.strName = "YE2";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//12 BY Character 12 备用
		Column.strName = "BY";
		Column.cType = 'C';
		Column.uWidth = 12;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//13 JZRQ Character 8 截止日期
		Column.strName = "JZRQ";
		Column.cType = 'C';
		Column.uWidth = 8;
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
上海市场zqye00001结算dbf取值
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::DBF_Value_zqye(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue,
	bool bIsNextDay)
{
	static const string ftag("SHSettle::DBF_Value_zqye() ");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;

		//NO 字段名 类型 长度 格式定义 说明
		//1 SCDM Character 2 市场代码
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["SCDM"] = dbfRow;

		//2 QSBH Character 8 托管会员的清算编号，对于 QFII 账户，填写托管银行的清算编号
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["QSBH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//3 ZQZH Character 10 证券账户
		citRow = in_mapRowData.find("account_id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["ZQZH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//4 XWH Character 5 指定交易单元 ???委托消息中没有

		//5 ZQDM Character 6 证券代码
		citRow = in_mapRowData.find("stock_id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["ZQDM"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//6 ZQLB Character 2 证券类别
		dbfRow.iType = 0;
		dbfRow.strValue = "PT";
		out_mapDBFValue["ZQLB"] = dbfRow;

		//7 LTLX Character 1 流通类型
		dbfRow.iType = 0;
		dbfRow.strValue = "N";
		out_mapDBFValue["LTLX"] = dbfRow;

		//8 QYLB Character 2 权益类别

		//9 GPNF Character 4 挂牌年份
		dbfRow.iType = 0;
		dbfRow.strValue = "0";
		out_mapDBFValue["GPNF"] = dbfRow;

		//10 YE1 Character 16 999999999999999 余额1（ 全部余额）
		citRow = in_mapRowData.find("stock_balance");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["YE1"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//11 YE2 Character 16 999999999999999余额2 （冻结证券数量, 含权利受限登记部分）
		dbfRow.iType = 0;
		dbfRow.strValue = "0";

		out_mapDBFValue["YE2"] = dbfRow;

		//12 BY Character 12 备用

		//13 JZRQ Character 8 截止日期
		citRow = in_mapRowData.find("time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);


			out_mapDBFValue["JZRQ"] = dbfRow;
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
				out_mapDBFValue["YE1"].dValue += dTemp;
			}
			citRow = in_mapRowData.find("stock_staple_purchase_balance");
			if (citRow != in_mapRowData.end())
			{
				Tgw_StringUtil::String2Double_atof(citRow->second.strValue, dTemp);
				out_mapDBFValue["YE1"].dValue += dTemp;
			}
			citRow = in_mapRowData.find("stock_etf_redemption_balance");
			if (citRow != in_mapRowData.end())
			{
				Tgw_StringUtil::String2Double_atof(citRow->second.strValue, dTemp);
				out_mapDBFValue["YE1"].dValue += dTemp;
			}
			citRow = in_mapRowData.find("stock_available");
			if (citRow != in_mapRowData.end())
			{
				Tgw_StringUtil::String2Double_atof(citRow->second.strValue, dTemp);
				out_mapDBFValue["YE1"].dValue += dTemp;
			}
		}

	}
	catch (exception &e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}

	return 0;
}

/*
上海市场结算dbf格式
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::SH_DBF_jsmx00001(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SHSettle::SH_DBF_jsmx00001()");
	struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

	//NO 字段名 类型 长	度 格式定义 说明
	//1 SCDM Character 2 市场代码
	Column.strName = "SCDM";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//2 JLLX Character 3 记录类型
	Column.strName = "JLLX";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//3 JYFS Character 3 交易方式
	Column.strName = "JYFS";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//4 JSFS Character 3 交收方式
	Column.strName = "JSFS";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//5 YWLX Character 3 业务类型
	Column.strName = "YWLX";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//6 QSBZ Character 3 清算标志
	Column.strName = "QSBZ";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//7 GHLX Character 3 过户类型
	Column.strName = "GHLX";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//8 JSBH Character 16 交收编号
	Column.strName = "JSBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//9 CJBH Character 16 999999999999999 成交编号
	Column.strName = "CJBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//10 SQBH Character 16 申请编号
	Column.strName = "SQBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//11 WTBH Character 16 委托编号
	Column.strName = "WTBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//	12 JYRQ Character 8 交易日期
	Column.strName = "JYRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//13 QSRQ Character 8 清算日期
	Column.strName = "QSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//	14 JSRQ Character 8 交收日期
	Column.strName = "JSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//15 QTRQ Character 8 其它日期
	Column.strName = "QTRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//16 WTSJ Character 6 委托时间
	Column.strName = "WTSJ";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//17 CJSJ Character 6 成交时间
	Column.strName = "CJSJ";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//18 XWH1 Character 5 业务单元 1
	Column.strName = "XWH1";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//19 XWH2 Character 5 业务单元 2
	Column.strName = "XWH2";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//20 XWHY Character 8业务单元 1 所属结算参与人的清算编号
	Column.strName = "XWHY";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//21 JSHY Character 8结算参与人的清算编号
	Column.strName = "JSHY";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//22 TGHY Character 8 托管银行的清算编号
	Column.strName = "TGHY";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//23 ZQZH Character 10 证券账号
	Column.strName = "ZQZH";
	Column.cType = 'C';
	Column.uWidth = 10;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//24 ZQDM1 Character 6 证券代码 1
	Column.strName = "ZQDM1";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//25 ZQDM2 Character 6 证券代码 2
	Column.strName = "ZQDM2";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//26 ZQLB Character 2 证券类别
	Column.strName = "ZQLB";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//27 LTLX Character 1 流通类型
	Column.strName = "LTLX";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//28 QYLB Character 2 权益类别
	Column.strName = "QYLB";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//29 GPNF Character 4 挂牌年份
	Column.strName = "GPNF";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//30 MMBZ Character 1 买卖标志
	Column.strName = "MMBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//31 SL Character 16 999999999999999 交收数量
	Column.strName = "SL";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//32 CJSL Character 16 999999999999999 成交数量
	Column.strName = "CJSL";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//33 ZJZH Character 25 资金账号
	Column.strName = "ZJZH";
	Column.cType = 'C';
	Column.uWidth = 25;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//34 BZ Character 3 币种
	Column.strName = "BZ";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//35 JG1 Character 17 999999.999999999 价格 1
	Column.strName = "JG1";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//36 JG2 Character 17 999999.999999999 价格 2
	Column.strName = "JG2";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//37 QSJE Character 19 999999999999999.99 清算金额
	Column.strName = "QSJE";
	Column.cType = 'C';
	Column.uWidth = 19;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//38 YHS Character 17 9999999999999.99 印花税
	Column.strName = "YHS";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//39 JSF Character 17 9999999999999.99 经手费
	Column.strName = "JSF";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//40 GHF Character 17 9999999999999.99 过户费
	Column.strName = "GHF";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//41 ZGF Character 17 9999999999999.99 证管费
	Column.strName = "ZGF";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//42 SXF Character 17 9999999999999.99 手续费
	Column.strName = "SXF";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//43 QTJE1 Character 19 999999999999999.99 其它金额 1
	Column.strName = "QTJE1";
	Column.cType = 'C';
	Column.uWidth = 19;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//44 QTJE2 Character 19 999999999999999.99 其它金额 2
	Column.strName = "QTJE2";
	Column.cType = 'C';
	Column.uWidth = 19;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//45 QTJE3 Character 19 999999999999999.99 其它金额 3
	Column.strName = "QTJE3";
	Column.cType = 'C';
	Column.uWidth = 19;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//46 SJSF Character 19 999999999999999.99 实际收付
	Column.strName = "SJSF";
	Column.cType = 'C';
	Column.uWidth = 19;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//47 JGDM Character 4 结果代码
	Column.strName = "JGDM";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//48 FJSM Character 40 附加说明
	Column.strName = "FJSM";
	Column.cType = 'C';
	Column.uWidth = 40;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	return 0;
}

/*
上海市场结算dbf取值
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::SH_DBF_Value_jsmx00001(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SHSettle::SH_DBF_Value_jsmx0001()");

	try
	{
		struct TgwDBFOperHelper_DF::DataInRow dbfRow;

		//NO 字段名 类型 长	度 格式定义 说明
		//1 SCDM Character 2 市场代码
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["SCDM"] = dbfRow;

		//2 JLLX Character 3 记录类型
		dbfRow.iType = 0;
		dbfRow.strValue = "001";
		out_mapDBFValue["JLLX"] = dbfRow;

		//3 JYFS Character 3 交易方式
		dbfRow.iType = 0;
		dbfRow.strValue = "001";
		out_mapDBFValue["JYFS"] = dbfRow;

		//4 JSFS Character 3 交收方式
		dbfRow.iType = 0;
		dbfRow.strValue = "001";
		out_mapDBFValue["JSFS"] = dbfRow;

		//5 YWLX Character 3 业务类型
		dbfRow.iType = 0;
		dbfRow.strValue = "001";
		out_mapDBFValue["YWLX"] = dbfRow;

		//6 QSBZ Character 3 清算标志
		dbfRow.iType = 0;
		dbfRow.strValue = "060";
		out_mapDBFValue["QSBZ"] = dbfRow;

		//7 GHLX Character 3 过户类型
		dbfRow.iType = 0;
		dbfRow.strValue = "00A";
		out_mapDBFValue["GHLX"] = dbfRow;

		//8 JSBH Character 16 交收编号
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("execid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JSBH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//9 CJBH Character 16 999999999999999 成交编号
		out_mapDBFValue["CJBH"] = dbfRow;

		//10 SQBH Character 16 申请编号
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SQBH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//11 WTBH Character 16 委托编号
		out_mapDBFValue["WTBH"] = dbfRow;

		//	12 JYRQ Character 8 交易日期
		citRow = in_mapRowData.find("oper_time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);


			out_mapDBFValue["JYRQ"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//13 QSRQ Character 8 清算日期
		out_mapDBFValue["QSRQ"] = dbfRow;

		//	14 JSRQ Character 8 交收日期
		out_mapDBFValue["JSRQ"] = dbfRow;

		//15 QTRQ Character 8 其它日期

		//16 WTSJ Character 6 委托时间
		citRow = in_mapRowData.find("oper_time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(7, 6);


			out_mapDBFValue["WTSJ"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//17 CJSJ Character 6 成交时间
		out_mapDBFValue["CJSJ"] = dbfRow;

		//18 XWH1 Character 5 业务单元 1
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["XWH1"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//19 XWH2 Character 5 业务单元 2
		out_mapDBFValue["XWH2"] = dbfRow;

		//20 XWHY Character 8业务单元1所属结算参与人的清算编号

		//21 JSHY Character 8结算参与人的清算编号

		//22 TGHY Character 8 托管银行的清算编号

		//23 ZQZH Character 10 证券账号
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["ZQZH"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//24 ZQDM1 Character 6 证券代码1
		citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["ZQDM1"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//25 ZQDM2 Character 6 证券代码 2

		//26 ZQLB Character 2 证券类别
		dbfRow.iType = 0;
		dbfRow.strValue = "PT";
		out_mapDBFValue["ZQLB"] = dbfRow;

		//27 LTLX Character 1 流通类型
		dbfRow.iType = 0;
		dbfRow.strValue = "0";
		out_mapDBFValue["LTLX"] = dbfRow;

		//28 QYLB Character 2 权益类别

		//29 GPNF Character 4 挂牌年份

		//30 MMBZ Character 1 买卖标志
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRowSide->second.strValue;

			/*
			if (0 == citRowSide->second.strValue.compare("1"))
			{
				//	如果是买，B
				dbfRow.strValue = "B";
			}
			else if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	如果是卖，S
				dbfRow.strValue = "S";
			}
			*/

			out_mapDBFValue["MMBZ"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//31 SL Character 16 999999999999999 交收数量
		citRow = in_mapRowData.find("match_qty");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			string strValue;

			if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	如果是卖，则加符号负
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			dbfRow.strValue = strValue;

			out_mapDBFValue["SL"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//32 CJSL Character 16 999999999999999 成交数量
		out_mapDBFValue["CJSL"] = dbfRow;

		//33 ZJZH Character 25 资金账号

		//34 BZ Character 3 币种
		dbfRow.iType = 0;
		dbfRow.strValue = "RMB";
		out_mapDBFValue["BZ"] = dbfRow;

		//35 JG1 Character 17 999999.999999999 价格 1
		citRow = in_mapRowData.find("match_price");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;

			Tgw_StringUtil::StringLiToStringYuan(citRow->second.strValue, dbfRow.strValue, 9);

			out_mapDBFValue["JG1"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//36 JG2 Character 17 999999.999999999 价格 2
		out_mapDBFValue["JG2"] = dbfRow;

		//37 QSJE Character 19 999999999999999.99 清算金额
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 0;

			Tgw_StringUtil::StringLiToStringYuan(citRow->second.strValue, dbfRow.strValue, 2);

			out_mapDBFValue["QSJE"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//38 YHS Character 17 9999999999999.99 印花税
		dbfRow.iType = 0;
		dbfRow.strValue = "0.00";
		out_mapDBFValue["YHS"] = dbfRow;

		//39 JSF Character 17 9999999999999.99 经手费
		dbfRow.iType = 0;
		dbfRow.strValue = "0.00";
		out_mapDBFValue["JSF"] = dbfRow;

		//40 GHF Character 17 9999999999999.99 过户费
		dbfRow.iType = 0;
		dbfRow.strValue = "0.00";
		out_mapDBFValue["GHF"] = dbfRow;

		//41 ZGF Character 17 9999999999999.99 证管费
		dbfRow.iType = 0;
		dbfRow.strValue = "0.00";
		out_mapDBFValue["ZGF"] = dbfRow;

		//42 SXF Character 17 9999999999999.99 手续费
		dbfRow.iType = 0;
		dbfRow.strValue = "0.00";
		out_mapDBFValue["SXF"] = dbfRow;


		//43 QTJE1 Character 19 999999999999999.99 其它金额 1

		//44 QTJE2 Character 19 999999999999999.99 其它金额 2

		//45 QTJE3 Character 19 999999999999999.99 其它金额 3

		//46 SJSF Character 19 999999999999999.99 实际收付
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 0;

			Tgw_StringUtil::StringLiToStringYuan(citRow->second.strValue, dbfRow.strValue, 2);

			out_mapDBFValue["SJSF"] = dbfRow;
		}
		else
		{
			// 未找到该值
		}

		//47 JGDM Character 4 结果代码
		dbfRow.iType = 0;
		dbfRow.strValue = "0000";
		out_mapDBFValue["JGDM"] = dbfRow;


		//48 FJSM Character 40 附加说明


	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}

	return 0;
}

/*
上海市场jsmx00001结算
Return :
0 -- 成功
非0 -- 失败
*/
int SHSettle::SH_Gen_jsmx00001(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const string& strFilePath)
{
	static const string ftag("SHSettle::SH_Gen_jsmx00001()");

	string strDbfPrefix(strFilePath);
	strDbfPrefix += "\\A";

	const string strDbfPostfix(".dbf");

	//	以seat席位为key，存下对应的value
	map<string, vector< map<string, struct MySqlCnnC602_DF::DataInRow> > >mapSeatDbfValue;

	int iReturn = 0;

	std::string strTrans;
	string strQuer("SELECT *,DATE_FORMAT(`trade_time`,'%Y%m%d%H%i%s') AS trade_time_trans, "
		"DATE_FORMAT(`order_time`,'%Y%m%d%H%i%s') AS order_time_trans "
		"FROM `order_match` WHERE trade_market=101 AND trade_type=");
	strQuer += sof_string::itostr(simutgw::TADE_TYPE::a_trade, strTrans);
	strQuer += " ORDER BY trade_time ASC";
	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	// 查询
	int iRes = in_mysqlConn->Query(strQuer, &pResultSet, ulAffectedRows);
	if (1 == iRes)
	{
		// select
		string strKey;
		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		while ((0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData)))
		{
			map<string, struct MySqlCnnC602_DF::DataInRow>::iterator mapIt = mapRowData.find("security_seat");
			if (mapRowData.end() != mapIt)
			{
				strKey = mapIt->second.strValue;
			}
			else
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "没有席位，id[" << mapRowData["id"].strValue;

				continue;
			}

			mapSeatDbfValue[strKey].push_back(mapRowData);
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "执行" << strQuer << "错误res=" << iRes;

		iReturn = -1;
	}

	// 释放
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;

	if (0 != iReturn)
	{
		return iReturn;
	}

	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	iRes = SH_DBF_jsmx00001(vecSetting);

	// 根据查询到的结果生成DBF文件
	map<string, vector< map<string, struct MySqlCnnC602_DF::DataInRow> > >::iterator mapIterSeat = mapSeatDbfValue.begin();
	if (mapSeatDbfValue.end() == mapIterSeat)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "由于没有成交记录，不生成jsmx.dbf";

		return 0;
	}
	while (mapSeatDbfValue.end() != mapIterSeat)
	{
		map<string, struct TgwDBFOperHelper_DF::DataInRow> mapDBFValue;

		TgwDBFOperHelper dbfWriter;

		string strFileName(strDbfPrefix);
		strFileName += "\\jsmx";
		strFileName += strDbfPostfix;

		if (!boost::filesystem::exists(strFileName))
		{
			// 如果该dbf文件不存在
			iRes = dbfWriter.Create(strFileName, vecSetting);
			if (0 != iRes)
			{
				return -1;
			}

		}

		iRes = dbfWriter.Open(strFileName);
		if (0 != iRes)
		{
			return -1;
		}

		for (size_t st = 0; st < mapIterSeat->second.size(); ++st)
		{
			//遍历每一个seat对应的记录
			iRes = SH_DBF_Value_jsmx00001((mapIterSeat->second)[st], mapDBFValue);
			if (0 != iRes)
			{
				return -1;
			}

			iRes = dbfWriter.Append(mapDBFValue);
			if (0 != iRes)
			{
				return -1;
			}
		}
		dbfWriter.Close();

		++mapIterSeat;
	}

	return iReturn;
}
