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
��ʼ����Ա����
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
������ϸ
ÿ�������ID����һ��

Return :
0 -- �ɹ�
��0 -- ʧ��
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
�������
��һ��

Return :
0 -- �ɹ�
��0 -- ʧ��
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
�Ϻ��г�gh����
�������ݽӿ� gh.dbf

Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::Gen_gh(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::string& in_strSettleGroupName,
	const string& in_strFilePath)
{
	static const string ftag("SHSettle::Gen_gh() ");

	string strDbfPrefix(in_strFilePath);

	//	��seatϯλΪkey�����¶�Ӧ��value
	map<string, vector< map<string, struct MySqlCnnC602_DF::DataInRow> > >mapSeatDbfValue;

	int iReturn = 0;

	std::string strTrans;
	string strQuer("SELECT *,DATE_FORMAT(`trade_time`,'%Y%m%d%H%i%s') AS trade_time_trans, "
		"DATE_FORMAT(`order_time`,'%Y%m%d%H%i%s') AS order_time_trans "
		"FROM `order_match` WHERE trade_market=101 AND ");

	if (in_strSettleGroupName.empty())
	{
		// ������ر���
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

	// ��ѯ
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
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "û��ϯλ��id[" << mapRowData["id"].strValue << "]";

				continue;
			}

			mapSeatDbfValue[strKey].push_back(mapRowData);
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ִ��" << strQuer << "����res=" << iRes;

		iReturn = -1;
	}

	// �ͷ�
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;

	if (0 != iReturn)
	{
		return iReturn;
	}

	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	iRes = DBF_gh(vecSetting);

	// ���ݲ�ѯ���Ľ������DBF�ļ�
	map<string, vector< map<string, struct MySqlCnnC602_DF::DataInRow> > >::iterator mapIterSeat = mapSeatDbfValue.begin();
	if (mapSeatDbfValue.end() == mapIterSeat)
	{
		if (in_strSettleGroupName.empty())
		{
			// ������ر���
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� gh.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� " << in_strSettleGroupName << "_gh.dbf";
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
		// ������ر���	
		strFileName += in_strSettleGroupName;
		strFileName += "_";
	}
	strFileName += "gh_";
	strFileName += m_strNow;
	strFileName += ".dbf";

	if (!boost::filesystem::exists(strFileName))
	{
		// �����dbf�ļ�������
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
			//����ÿһ��seat��Ӧ�ļ�¼
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
�Ϻ��г�gh����dbf��ʽ
�������ݽӿ� gh.dbf
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::DBF_gh(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SHSettle::DBF_gh() ");

	try
	{
		struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

		//���	�ֶ���	�ֶ�����	����
		//1	Gddm	֤ȯ�˻�	C10
		Column.strName = "Gddm";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//2	Gdxm	�ɶ��������½���ϵͳ�л����Ͻ����Ὣ���ֶ���Ϊ�ո������й�֤ȯ�Ǽǽ����������ι�˾����������Ϊ׼��	C8
		Column.strName = "Gdxm";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//3	Bcrq	�ɽ����ڣ���ʽΪYYYYMMDD	C8
		Column.strName = "Bcrq";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//4	Cjbh	�ɽ����	N8
		Column.strName = "Cjbh";
		Column.cType = 'N';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//5	Gsdm	ϯλ��	C5
		Column.strName = "Gsdm";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//6	Cjsl	�ɽ�����	N10
		Column.strName = "Cjsl";
		Column.cType = 'N';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//7	Bcye	�������½���ϵͳ�л��󣬸��ֶα���Ϊ0��	N10
		Column.strName = "Bcye";
		Column.cType = 'N';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//8	zqdm	֤ȯ����	C6
		Column.strName = "zqdm";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//9	sbsj	�걨ʱ�䣬��ʽΪHHMMSS	C6
		Column.strName = "sbsj";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//10	cjsj	�ɽ�ʱ�䣬��ʽΪHHMMSS	C6
		Column.strName = "cjsj";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);


		//11	cjjg	�ɽ��۸�	N8(3)
		Column.strName = "cjjg";
		Column.cType = 'N';
		Column.uWidth = 8;
		Column.uDecWidth = 3;
		io_vecSetting.push_back(Column);

		//12	cjje	�ɽ����������μ�ʵʱ�ɽ��ر��ӿڳɽ�����ֶ�	N12(2)
		Column.strName = "cjje";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//13	sqbh	��Ա�ڲ������ţ�ͬ�걨�ӿ��е�reff���ֶ�4����Ա�ڲ������ţ���	C10
		Column.strName = "sqbh";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//14	bs	B ��ͨ������������������S ��ͨ������������������	C1
		Column.strName = "bs";
		Column.cType = 'C';
		Column.uWidth = 1;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//15	mjbh	����Ա����	C5
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
�Ϻ��г�gh����dbfȡֵ
�������ݽӿ� gh.dbf
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::DBF_Value_gh(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SHSettle::DBF_Value_gh() ");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;

		//���	�ֶ���	�ֶ�����	����
		//1	Gddm	֤ȯ�˻�	C10
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["Gddm"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//2	Gdxm	�ɶ��������½���ϵͳ�л����Ͻ����Ὣ���ֶ���Ϊ�ո������й�֤ȯ�Ǽǽ����������ι�˾����������Ϊ׼��	C8
		dbfRow.iType = 0;
		dbfRow.strValue = "        ";

		out_mapDBFValue["Gdxm"] = dbfRow;

		//3	Bcrq	�ɽ����ڣ���ʽΪYYYYMMDD	C8
		citRow = in_mapRowData.find("trade_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);

			out_mapDBFValue["Bcrq"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//4	Cjbh	�ɽ����	N8
		citRow = in_mapRowData.find("id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 1;
			Tgw_StringUtil::String2Long_atol(citRow->second.strValue, dbfRow.lValue);

			out_mapDBFValue["Cjbh"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//5	Gsdm	ϯλ��	C5
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["Gsdm"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//6	Cjsl	�ɽ�����	N10
		citRow = in_mapRowData.find("match_qty");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 1;
			Tgw_StringUtil::String2Long_atol(citRow->second.strValue, dbfRow.lValue);

			out_mapDBFValue["Cjsl"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//7	Bcye	�������½���ϵͳ�л��󣬸��ֶα���Ϊ0��	N10
		dbfRow.iType = 1;
		dbfRow.lValue = 0;

		out_mapDBFValue["Bcye"] = dbfRow;

		//8	zqdm	֤ȯ����	C6
		citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["zqdm"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//9	sbsj	�걨ʱ�䣬��ʽΪHHMMSS	C6
		citRow = in_mapRowData.find("order_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(8, 6);

			out_mapDBFValue["sbsj"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//10	cjsj	�ɽ�ʱ�䣬��ʽΪHHMMSS	C6
		citRow = in_mapRowData.find("trade_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(8, 6);

			out_mapDBFValue["cjsj"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//11	cjjg	�ɽ��۸�	N8(3)
		citRow = in_mapRowData.find("match_price");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 3);

			out_mapDBFValue["cjjg"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//12	cjje	�ɽ����������μ�ʵʱ�ɽ��ر��ӿڳɽ�����ֶ�	N12(2)
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 2);

			out_mapDBFValue["cjje"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//13	sqbh	��Ա�ڲ������ţ�ͬ�걨�ӿ��е�reff���ֶ�4����Ա�ڲ������ţ���	C10
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["sqbh"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowTradeTye = in_mapRowData.find("trade_type");

		//14	bs	
		/*
		B ��ͨ������������������S ��ͨ������������������	C1
		1 ����Ʒ���붩����ȯ��Ͷ���������ʻ����걨�ӿ���(owflag, BS)�ֶ�ȡֵΪ(��_XY��,��B��)��
		2 ����Ʒ����������ȯ��Ͷ���������ʻ������걨�ӿ���(owflag, BS)�ֶ�ȡֵΪ(��_XY��,��S��)��
		3 �������붩����ȯ��Ͷ���������ʻ����걨�ӿ���(owflag, BS)�ֶ�ȡֵΪ(��_RZ��,��B��)��
		4 ��ȯ�������ȯ��Ͷ���������ʻ������걨�ӿ���(owflag, BS)�ֶ�ȡֵΪ(��_RZ��,��S��)��
		5 ��ȯ��ȯ������ȯ��֤ȯ��˾��ȯר���ʻ����걨�ӿ���(owflag, BS)�ֶ�ȡֵΪ(��_RQ��,��B��)��
		6 ��ȯ����������ȯ��֤ȯ��˾��ȯר���ʻ������걨�ӿ���(owflag, BS)�ֶ�ȡֵΪ(��_RQ��,��S��)��
		7 ƽ�����붩����ȯ��֤ȯ��˾��ȯר���ʻ����걨�ӿ���(owflag, BS)�ֶ�ȡֵΪ(��_PC��,��B��)��
		8 ƽ������������ȯ��Ͷ���������ʻ������걨�ӿ���(owflag, BS)�ֶ�ȡֵΪ(��_PC��,��S��)��

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
			{ // ��ͨ����
				if (0 == citRowSide->second.strValue.compare("1"))
				{
					// ��
					dbfRow.strValue = "B";
				}
				else if (0 == citRowSide->second.strValue.compare("2"))
				{
					// ��
					dbfRow.strValue = "S";
				}
			}
			else if (simutgw::margin_cash == iTradeType)
			{ // ����
				if (0 == citRowSide->second.strValue.compare("1"))
				{
					// ��
					dbfRow.strValue = "3";
				}
				else if (0 == citRowSide->second.strValue.compare("2"))
				{
					// ��
					dbfRow.strValue = "4";
				}
			}
			else if (simutgw::margin_stock == iTradeType)
			{ // ��ȯ
				if (0 == citRowSide->second.strValue.compare("1"))
				{
					// ��
					dbfRow.strValue = "5";
				}
				else if (0 == citRowSide->second.strValue.compare("2"))
				{
					// ��
					dbfRow.strValue = "6";
				}
			}
			*/

			out_mapDBFValue["bs"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//15	mjbh	����Ա����	C5
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
�Ϻ��г�bc1����
�������ݽӿ� bc1.dbf

Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::Gen_bc1(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
	const std::string& in_strSettleGroupName,
	const string& in_strFilePath)
{
	static const string ftag("SHSettle::Gen_bc1() ");

	string strDbfPrefix(in_strFilePath);

	//	���˺�Ϊkey�����¶�Ӧ��value
	map<string, vector< map<string, struct MySqlCnnC602_DF::DataInRow> > >mapSeatDbfValue;

	int iReturn = 0;

	std::string strTrans;
	string strQuer("SELECT *,DATE_FORMAT(`trade_time`,'%Y%m%d%H%i%s') AS trade_time_trans, "
		"DATE_FORMAT(`order_time`,'%Y%m%d%H%i%s') AS order_time_trans "
		"FROM `order_match` WHERE trade_market=101 AND ");

	if (in_strSettleGroupName.empty())
	{
		// ������ر���
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

	// ��ѯ
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
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "û���˺ţ�id[" << mapRowData["id"].strValue;

				continue;
			}

			mapSeatDbfValue[strKey].push_back(mapRowData);
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ִ��" << strQuer << "����res=" << iRes;

		iReturn = -1;
	}

	// �ͷ�
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;

	if (0 != iReturn)
	{
		return iReturn;
	}

	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	iRes = DBF_bc1(vecSetting);

	// ���ݲ�ѯ���Ľ������DBF�ļ�
	map<string, vector< map<string, struct MySqlCnnC602_DF::DataInRow> > >::iterator mapIterSeat = mapSeatDbfValue.begin();
	if (mapSeatDbfValue.end() == mapIterSeat)
	{
		if (in_strSettleGroupName.empty())
		{
			// ������ر���
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� bc1.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� " << in_strSettleGroupName << "_bc1.dbf";
		}

		return 0;
	}

	map<string, struct TgwDBFOperHelper_DF::DataInRow> mapDBFValue;

	TgwDBFOperHelper dbfWriter;

	string strFileName(strDbfPrefix);
	strFileName += "/";
	if (!in_strSettleGroupName.empty())
	{
		// ������ر���
		strFileName += in_strSettleGroupName;
		strFileName += "_";
	}
	strFileName += "bc1_";
	strFileName += m_strNow;
	strFileName += ".dbf";

	if (!boost::filesystem::exists(strFileName))
	{
		// �����dbf�ļ�������
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
			//����ÿһ��seat��Ӧ�ļ�¼
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
�Ϻ��г�bc1����dbf��ʽ
�������ݽӿ� bc1.dbf
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::DBF_bc1(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SHSettle::DBF_bc1() ");

	try
	{
		struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

		//���	�ֶ���	�ֶ�����	����
		//1 SSCCRC_ID Character 15 ���ձ��
		Column.strName = "SSCCRC_ID";
		Column.cType = 'C';
		Column.uWidth = 15;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//2 SSE_ORDER Character 16 ί�б��
		Column.strName = "SSE_ORDER";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//3 SSE_REF Character 16 �ɽ����
		Column.strName = "SSE_REF";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//4 INV_CODE Character 10 ֤ȯ�ʻ�����
		Column.strName = "INV_CODE";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//5 SEC_CODE Character 6 ֤ȯ����
		Column.strName = "SEC_CODE";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//6 TRADE_DATE Character 8 ��������
		Column.strName = "TRADE_DATE";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//7 BS Character 1 ������־
		Column.strName = "BS";
		Column.cType = 'C';
		Column.uWidth = 1;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//8 PRICE Numeric 8,3 �ɽ��۸�
		Column.strName = "PRICE";
		Column.cType = 'N';
		Column.uWidth = 8;
		Column.uDecWidth = 3;
		io_vecSetting.push_back(Column);

		//9 VOLUME Numeric 10,0 �ɽ�����
		Column.strName = "VOLUME";
		Column.cType = 'N';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//10 TRADE_VAL Numeric 13,2 ���׽��
		Column.strName = "TRADE_VAL";
		Column.cType = 'N';
		Column.uWidth = 13;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//11 ORDER_TIME Character 6 ί��ʱ��
		Column.strName = "ORDER_TIME";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//12 TRADE_TIME Character 6 �ɽ�ʱ��
		Column.strName = "TRADE_TIME";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//13 SEAT_ID Character 5 ���׵�Ԫ
		Column.strName = "SEAT_ID";
		Column.cType = 'C';
		Column.uWidth = 5;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//14 TRADE_CP Character 5 ���׵�Ԫ�������������
		Column.strName = "TRADE_CP";
		Column.cType = 'C';
		Column.uWidth = 5;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//15 SETTLE_CP Character 5 ���ս��������
		Column.strName = "SETTLE_CP";
		Column.cType = 'C';
		Column.uWidth = 5;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//16 CP_ORDER Character 16 ������
		Column.strName = "CP_ORDER";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//17 SETTLEDATE Character 8 ��������
		Column.strName = "SETTLEDATE";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//18 STAMP_DUTY Numeric 12,2 ӡ��˰
		Column.strName = "STAMP_DUTY";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//19 SSE_CHARGE Numeric 12,2 ���ַ�
		Column.strName = "SSE_CHARGE";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//20 TRANSF_FEE Numeric 12,2 ������
		Column.strName = "TRANSF_FEE";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//21 CLEAR_FEE Numeric 12,2 �����
		Column.strName = "CLEAR_FEE";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//22 OTHER_FEE Numeric 12,2 ��ܷ�
		Column.strName = "OTHER_FEE";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//23 TOTAL_FEE Numeric 12,2 ���úϼ�
		Column.strName = "TOTAL_FEE";
		Column.cType = 'N';
		Column.uWidth = 12;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//24 SETTLE_AMT Numeric 13,2 ���ս��
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
�Ϻ��г�bc1����dbfȡֵ
�������ݽӿ� bc1.dbf
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::DBF_Value_bc1(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SHSettle::DBF_Value_bc1()");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;

		//���	�ֶ���	�ֶ�����	����
		//1 SSCCRC_ID Character 15 ���ձ��
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SSCCRC_ID"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//2 SSE_ORDER Character 16 ί�б��
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SSE_ORDER"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//3 SSE_REF Character 16 �ɽ����
		citRow = in_mapRowData.find("orderid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SSE_REF"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//4 INV_CODE Character 10 ֤ȯ�ʻ�����
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["INV_CODE"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//5 SEC_CODE Character 6 ֤ȯ����
		citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SEC_CODE"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//6 TRADE_DATE Character 8 ��������
		citRow = in_mapRowData.find("trade_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);

			out_mapDBFValue["TRADE_DATE"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//7 BS Character 1 ������־
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
			// δ�ҵ���ֵ
		}

		//8 PRICE Numeric 8,3 �ɽ��۸�
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
			// δ�ҵ���ֵ
		}

		//9 VOLUME Numeric 10,0 �ɽ�����
		citRow = in_mapRowData.find("match_qty");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;

			string strValue;
			if (0 == citRowSide->second.strValue.compare("2"))
			{
				// ��Ϊ��
				strValue = "-";
			}
			strValue += citRow->second.strValue;
			Tgw_StringUtil::String2Double_atof(strValue, dbfRow.dValue);

			out_mapDBFValue["VOLUME"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//10 TRADE_VAL Numeric 13,2 ���׽��
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;

			string strValue;
			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_SELL_S)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_SELL_2))
			{
				// ��Ϊ��
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dbfRow.dValue, 2);

			out_mapDBFValue["TRADE_VAL"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//11 ORDER_TIME Character 6 ί��ʱ��
		citRow = in_mapRowData.find("order_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(8, 6);

			out_mapDBFValue["ORDER_TIME"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//12 TRADE_TIME Character 6 �ɽ�ʱ��
		citRow = in_mapRowData.find("trade_time_trans");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(8, 6);

			out_mapDBFValue["TRADE_TIME"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}


		//13 SEAT_ID Character 5 ���׵�Ԫ
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SEAT_ID"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}


		//14 TRADE_CP Character 5 ���׵�Ԫ�������������


		//15 SETTLE_CP Character 5 ���ս��������


		//16 CP_ORDER Character 16 ������
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["CP_ORDER"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}


		//17 SETTLEDATE Character 8 ��������
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
			// δ�ҵ���ֵ
		}

		//18 STAMP_DUTY Numeric 12,2 ӡ��˰
		dbfRow.iType = 3;
		dbfRow.dValue = 0;

		out_mapDBFValue["STAMP_DUTY"] = dbfRow;

		//19 SSE_CHARGE Numeric 12,2 ���ַ�
		out_mapDBFValue["SSE_CHARGE"] = dbfRow;

		//20 TRANSF_FEE Numeric 12,2 ������
		out_mapDBFValue["TRANSF_FEE"] = dbfRow;

		//21 CLEAR_FEE Numeric 12,2 �����
		out_mapDBFValue["CLEAR_FEE"] = dbfRow;

		//22 OTHER_FEE Numeric 12,2 ��ܷ�
		out_mapDBFValue["OTHER_FEE"] = dbfRow;

		//23 TOTAL_FEE Numeric 12,2 ���úϼ�
		out_mapDBFValue["TOTAL_FEE"] = dbfRow;

		//24 SETTLE_AMT Numeric 13,2 ���ս��
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
�Ϻ��г�zqbd����
zqbd(֤ȯ�䶯�ļ�)
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::Gen_zqbd(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const std::string& in_strSettleGroupName, const string& in_strFilePath)
{
	static const string ftag("SHSettle::Gen_zqbd() ");

	string strQuer("SELECT *,DATE_FORMAT(`trade_time`,'%Y%m%d%H%i%s.000') AS oper_time "
		"FROM `order_match` WHERE trade_market=101 AND ");
	if (in_strSettleGroupName.empty())
	{
		// ������ر���
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

	// ��ѯ�����е��Ϻ��г��ɽ���¼
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
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "ִ��" << strQuer << "����res=" << iRes;

		return -1;
	}

	// �ͷ�
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;

	string strFileName(in_strFilePath);
	strFileName += "/";
	if (!in_strSettleGroupName.empty())
	{
		// ������ر���
		strFileName += in_strSettleGroupName;
		strFileName += "_";
	}
	strFileName += "zqbd_";
	strFileName += m_strNow;
	strFileName += ".dbf";

	// �����������ļ�
	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	DBF_zqbd(vecSetting);

	if (0 == vecMapRowData.size())
	{

		if (in_strSettleGroupName.empty())
		{
			// ������ر���
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� zqbd.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� " << in_strSettleGroupName << "_zqbd.dbf";
		}
	}
	else
	{
		TgwDBFOperHelper dbfWriter;

		int iReturn = 0;
		
		if (!boost::filesystem::exists(strFileName))
		{
			// �������򴴽�
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
�Ϻ��г�zqbd����dbf��ʽ
zqbd(֤ȯ�䶯�ļ�)
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::DBF_zqbd(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SHSettle::DBF_zqbd() ");

	try
	{
		struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

		//���	�ֶ���	�ֶ�����	����
		//1 SCDM Character 2 �г�����
		Column.strName = "SCDM";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//2 QSBH Character 8�� �� �� Ա �� �� �� ��ţ����� QFII �˻�����д�й����е�������
		Column.strName = "QSBH";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//3 ZQZH Character 10 ֤ȯ�˻�
		Column.strName = "ZQZH";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//4 XWH Character 5 ���׵�Ԫ
		Column.strName = "XWH";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//5 ZQDM Character 6 ֤ȯ����
		Column.strName = "ZQDM";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//6 ZQLB Character 2 ֤ȯ���
		Column.strName = "ZQLB";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//7 LTLX Character 1 ��ͨ����
		Column.strName = "LTLX";
		Column.cType = 'C';
		Column.uWidth = 1;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//8 QYLB Character 2 Ȩ�����
		Column.strName = "QYLB";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//9 GPNF Character 4 �������
		Column.strName = "GPNF";
		Column.cType = 'C';
		Column.uWidth = 4;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//10 BDSL Character 16 999999999999999 �䶯����
		Column.strName = "BDSL";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//11 BDLX Character 3 �䶯����
		Column.strName = "BDLX";
		Column.cType = 'C';
		Column.uWidth = 3;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//12 BDRQ Character 8 �䶯����
		Column.strName = "BDRQ";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//13 SL Character 16 999999999999999 ����(�����ֶ�)
		Column.strName = "SL";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//14 BH Character 20 ���(�����ֶ�)
		Column.strName = "BH";
		Column.cType = 'C';
		Column.uWidth = 20;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//15 BY Character 20 ����
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
�Ϻ��г�zqbd����
zqbd(֤ȯ�䶯�ļ�)
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::DBF_Value_zqbd(const std::map<std::string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	std::map<std::string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	out_mapDBFValue.clear();

	struct TgwDBFOperHelper_DF::DataInRow dbfRow;

	//NO �ֶ��� ���� �� �� ��ʽ���� ˵��
	//1 SCDM Character 2 �г�����
	dbfRow.iType = 0;
	dbfRow.strValue = "01";

	//2 QSBH Character 8 �йܻ�Ա�������ţ�����QFII�˻�����д�й����е�������

	//3 ZQZH Character 10 ֤ȯ�˻�
	map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("security_account");
	if (in_mapRowData.end() != citRow)
	{
		dbfRow.iType = 0;
		dbfRow.strValue = citRow->second.strValue;
	}

	//4 XWH Character 5 ���׵�Ԫ
	citRow = in_mapRowData.find("security_seat");
	if (in_mapRowData.end() != citRow)
	{
		dbfRow.iType = 0;
		dbfRow.strValue = citRow->second.strValue;
	}

	//5 ZQDM Character 6 ֤ȯ����
	citRow = in_mapRowData.find("securityid");
	if (in_mapRowData.end() != citRow)
	{
		dbfRow.iType = 0;
		dbfRow.strValue = citRow->second.strValue;
	}

	//6 ZQLB Character 2 ֤ȯ���
	dbfRow.iType = 0;
	dbfRow.strValue = "PT";

	//7 LTLX Character 1 ��ͨ����
	dbfRow.iType = 0;
	dbfRow.strValue = "0";

	//8 QYLB Character 2 Ȩ�����


	//9 GPNF Character 4 �������


	//10 BDSL Character 16 999999999999999 �䶯���� �з���
	citRow = in_mapRowData.find("match_qty");
	if (citRow != in_mapRowData.end())
	{
		dbfRow.iType = 1;
		Tgw_StringUtil::String2Long_atol(citRow->second.strValue, dbfRow.lValue);

		out_mapDBFValue["BDSL"] = dbfRow;
	}
	else
	{
		// δ�ҵ���ֵ
	}

	//11 BDLX Character 3 �䶯����
	//???

	//12 BDRQ Character 8 �䶯����
	citRow = in_mapRowData.find("trade_time_trans");
	if (citRow != in_mapRowData.end())
	{
		dbfRow.iType = 0;
		dbfRow.strValue = citRow->second.strValue.substr(0, 8);

		out_mapDBFValue["BDRQ"] = dbfRow;
	}
	else
	{
		// δ�ҵ���ֵ
	}

	//13 SL Character 16 999999999999999 ����(�����ֶ�)

	//14 BH Character 20 ���(�����ֶ�)

	//15 BY Character 20 ����

	return 0;
}

/*
�Ϻ��г�zqye���� ��T��T+1��

zqye(֤ȯ�������ļ�)
Return :
0 -- �ɹ�
��0 -- ʧ��
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

	// ��ѯ�����е��Ϻ��г��ɽ���¼
	int iRes = in_mysqlConn->Query(strQuer, &pResultSet, ulAffectedRows);
	if (1 == iRes)
	{
		// select
		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		while ((0 != in_mysqlConn->FetchNextRow(&pResultSet, mapRowData)))
		{
			// A��
			vecAMapRowData.push_back(mapRowData);
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ִ��" << strQuer << "����res=" << iRes;

		return -1;
	}

	// �ͷ�
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;


	// �����������ļ�
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
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼��������zqye.dbf";
	}

	return 0;
}

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
int SHSettle::Gen_DBF_zqye(vector<map<string, struct MySqlCnnC602_DF::DataInRow> >& in_vecMapRowData,
	const std::string& strFilePath, bool bIsNextDay)
{
	static const string ftag("SHSettle::Gen_DBF_zqye()");

	int iReturn = 0;

	// �����������ļ�
	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;
	DBF_zqye(vecSetting);

	TgwDBFOperHelper AdbfWriter;

	if (!boost::filesystem::exists(strFilePath))
	{
		// �������򴴽�
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
�Ϻ��г�zqye00001����dbf��ʽ
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::DBF_zqye(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SHSettle::DBF_zqye()");

	try
	{
		struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

		//NO �ֶ��� ���� ���� ��ʽ���� ˵��
		//1 SCDM Character 2 �г�����
		Column.strName = "SCDM";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//2 QSBH Character 8 �йܻ�Ա�������ţ����� QFII �˻�����д�й����е�������
		Column.strName = "QSBH";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//3 ZQZH Character 10 ֤ȯ�˻�
		Column.strName = "ZQZH";
		Column.cType = 'C';
		Column.uWidth = 10;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//4 XWH Character 5 ָ�����׵�Ԫ
		Column.strName = "XWH";
		Column.cType = 'C';
		Column.uWidth = 5;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//5 ZQDM Character 6 ֤ȯ����
		Column.strName = "ZQDM";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//6 ZQLB Character 2 ֤ȯ���
		Column.strName = "ZQLB";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//7 LTLX Character 1 ��ͨ����
		Column.strName = "LTLX";
		Column.cType = 'C';
		Column.uWidth = 1;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//8 QYLB Character 2 Ȩ�����
		Column.strName = "QYLB";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//9 GPNF Character 4 �������
		Column.strName = "GPNF";
		Column.cType = 'C';
		Column.uWidth = 4;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//10 YE1 Character 16 999999999999999 ���1�� ȫ����
		Column.strName = "YE1";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//11 YE2 Character 16 999999999999999���2 ������֤ȯ����, ��Ȩ�����޵Ǽǲ��֣�
		Column.strName = "YE2";
		Column.cType = 'C';
		Column.uWidth = 16;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//12 BY Character 12 ����
		Column.strName = "BY";
		Column.cType = 'C';
		Column.uWidth = 12;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//13 JZRQ Character 8 ��ֹ����
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
�Ϻ��г�zqye00001����dbfȡֵ
Return :
0 -- �ɹ�
��0 -- ʧ��
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

		//NO �ֶ��� ���� ���� ��ʽ���� ˵��
		//1 SCDM Character 2 �г�����
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["SCDM"] = dbfRow;

		//2 QSBH Character 8 �йܻ�Ա�������ţ����� QFII �˻�����д�й����е�������
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["QSBH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//3 ZQZH Character 10 ֤ȯ�˻�
		citRow = in_mapRowData.find("account_id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["ZQZH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//4 XWH Character 5 ָ�����׵�Ԫ ???ί����Ϣ��û��

		//5 ZQDM Character 6 ֤ȯ����
		citRow = in_mapRowData.find("stock_id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["ZQDM"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//6 ZQLB Character 2 ֤ȯ���
		dbfRow.iType = 0;
		dbfRow.strValue = "PT";
		out_mapDBFValue["ZQLB"] = dbfRow;

		//7 LTLX Character 1 ��ͨ����
		dbfRow.iType = 0;
		dbfRow.strValue = "N";
		out_mapDBFValue["LTLX"] = dbfRow;

		//8 QYLB Character 2 Ȩ�����

		//9 GPNF Character 4 �������
		dbfRow.iType = 0;
		dbfRow.strValue = "0";
		out_mapDBFValue["GPNF"] = dbfRow;

		//10 YE1 Character 16 999999999999999 ���1�� ȫ����
		citRow = in_mapRowData.find("stock_balance");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["YE1"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//11 YE2 Character 16 999999999999999���2 ������֤ȯ����, ��Ȩ�����޵Ǽǲ��֣�
		dbfRow.iType = 0;
		dbfRow.strValue = "0";

		out_mapDBFValue["YE2"] = dbfRow;

		//12 BY Character 12 ����

		//13 JZRQ Character 8 ��ֹ����
		citRow = in_mapRowData.find("time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);


			out_mapDBFValue["JZRQ"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
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
�Ϻ��г�����dbf��ʽ
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::SH_DBF_jsmx00001(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SHSettle::SH_DBF_jsmx00001()");
	struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

	//NO �ֶ��� ���� ��	�� ��ʽ���� ˵��
	//1 SCDM Character 2 �г�����
	Column.strName = "SCDM";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//2 JLLX Character 3 ��¼����
	Column.strName = "JLLX";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//3 JYFS Character 3 ���׷�ʽ
	Column.strName = "JYFS";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//4 JSFS Character 3 ���շ�ʽ
	Column.strName = "JSFS";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//5 YWLX Character 3 ҵ������
	Column.strName = "YWLX";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//6 QSBZ Character 3 �����־
	Column.strName = "QSBZ";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//7 GHLX Character 3 ��������
	Column.strName = "GHLX";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//8 JSBH Character 16 ���ձ��
	Column.strName = "JSBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//9 CJBH Character 16 999999999999999 �ɽ����
	Column.strName = "CJBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//10 SQBH Character 16 ������
	Column.strName = "SQBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//11 WTBH Character 16 ί�б��
	Column.strName = "WTBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//	12 JYRQ Character 8 ��������
	Column.strName = "JYRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//13 QSRQ Character 8 ��������
	Column.strName = "QSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//	14 JSRQ Character 8 ��������
	Column.strName = "JSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//15 QTRQ Character 8 ��������
	Column.strName = "QTRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//16 WTSJ Character 6 ί��ʱ��
	Column.strName = "WTSJ";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//17 CJSJ Character 6 �ɽ�ʱ��
	Column.strName = "CJSJ";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//18 XWH1 Character 5 ҵ��Ԫ 1
	Column.strName = "XWH1";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//19 XWH2 Character 5 ҵ��Ԫ 2
	Column.strName = "XWH2";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//20 XWHY Character 8ҵ��Ԫ 1 ������������˵�������
	Column.strName = "XWHY";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//21 JSHY Character 8��������˵�������
	Column.strName = "JSHY";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//22 TGHY Character 8 �й����е�������
	Column.strName = "TGHY";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//23 ZQZH Character 10 ֤ȯ�˺�
	Column.strName = "ZQZH";
	Column.cType = 'C';
	Column.uWidth = 10;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//24 ZQDM1 Character 6 ֤ȯ���� 1
	Column.strName = "ZQDM1";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//25 ZQDM2 Character 6 ֤ȯ���� 2
	Column.strName = "ZQDM2";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//26 ZQLB Character 2 ֤ȯ���
	Column.strName = "ZQLB";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//27 LTLX Character 1 ��ͨ����
	Column.strName = "LTLX";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//28 QYLB Character 2 Ȩ�����
	Column.strName = "QYLB";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//29 GPNF Character 4 �������
	Column.strName = "GPNF";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//30 MMBZ Character 1 ������־
	Column.strName = "MMBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//31 SL Character 16 999999999999999 ��������
	Column.strName = "SL";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//32 CJSL Character 16 999999999999999 �ɽ�����
	Column.strName = "CJSL";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//33 ZJZH Character 25 �ʽ��˺�
	Column.strName = "ZJZH";
	Column.cType = 'C';
	Column.uWidth = 25;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//34 BZ Character 3 ����
	Column.strName = "BZ";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//35 JG1 Character 17 999999.999999999 �۸� 1
	Column.strName = "JG1";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//36 JG2 Character 17 999999.999999999 �۸� 2
	Column.strName = "JG2";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//37 QSJE Character 19 999999999999999.99 ������
	Column.strName = "QSJE";
	Column.cType = 'C';
	Column.uWidth = 19;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//38 YHS Character 17 9999999999999.99 ӡ��˰
	Column.strName = "YHS";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//39 JSF Character 17 9999999999999.99 ���ַ�
	Column.strName = "JSF";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//40 GHF Character 17 9999999999999.99 ������
	Column.strName = "GHF";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//41 ZGF Character 17 9999999999999.99 ֤�ܷ�
	Column.strName = "ZGF";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//42 SXF Character 17 9999999999999.99 ������
	Column.strName = "SXF";
	Column.cType = 'C';
	Column.uWidth = 17;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//43 QTJE1 Character 19 999999999999999.99 ������� 1
	Column.strName = "QTJE1";
	Column.cType = 'C';
	Column.uWidth = 19;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//44 QTJE2 Character 19 999999999999999.99 ������� 2
	Column.strName = "QTJE2";
	Column.cType = 'C';
	Column.uWidth = 19;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//45 QTJE3 Character 19 999999999999999.99 ������� 3
	Column.strName = "QTJE3";
	Column.cType = 'C';
	Column.uWidth = 19;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//46 SJSF Character 19 999999999999999.99 ʵ���ո�
	Column.strName = "SJSF";
	Column.cType = 'C';
	Column.uWidth = 19;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//47 JGDM Character 4 �������
	Column.strName = "JGDM";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//48 FJSM Character 40 ����˵��
	Column.strName = "FJSM";
	Column.cType = 'C';
	Column.uWidth = 40;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	return 0;
}

/*
�Ϻ��г�����dbfȡֵ
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::SH_DBF_Value_jsmx00001(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SHSettle::SH_DBF_Value_jsmx0001()");

	try
	{
		struct TgwDBFOperHelper_DF::DataInRow dbfRow;

		//NO �ֶ��� ���� ��	�� ��ʽ���� ˵��
		//1 SCDM Character 2 �г�����
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["SCDM"] = dbfRow;

		//2 JLLX Character 3 ��¼����
		dbfRow.iType = 0;
		dbfRow.strValue = "001";
		out_mapDBFValue["JLLX"] = dbfRow;

		//3 JYFS Character 3 ���׷�ʽ
		dbfRow.iType = 0;
		dbfRow.strValue = "001";
		out_mapDBFValue["JYFS"] = dbfRow;

		//4 JSFS Character 3 ���շ�ʽ
		dbfRow.iType = 0;
		dbfRow.strValue = "001";
		out_mapDBFValue["JSFS"] = dbfRow;

		//5 YWLX Character 3 ҵ������
		dbfRow.iType = 0;
		dbfRow.strValue = "001";
		out_mapDBFValue["YWLX"] = dbfRow;

		//6 QSBZ Character 3 �����־
		dbfRow.iType = 0;
		dbfRow.strValue = "060";
		out_mapDBFValue["QSBZ"] = dbfRow;

		//7 GHLX Character 3 ��������
		dbfRow.iType = 0;
		dbfRow.strValue = "00A";
		out_mapDBFValue["GHLX"] = dbfRow;

		//8 JSBH Character 16 ���ձ��
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("execid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JSBH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//9 CJBH Character 16 999999999999999 �ɽ����
		out_mapDBFValue["CJBH"] = dbfRow;

		//10 SQBH Character 16 ������
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["SQBH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//11 WTBH Character 16 ί�б��
		out_mapDBFValue["WTBH"] = dbfRow;

		//	12 JYRQ Character 8 ��������
		citRow = in_mapRowData.find("oper_time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);


			out_mapDBFValue["JYRQ"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//13 QSRQ Character 8 ��������
		out_mapDBFValue["QSRQ"] = dbfRow;

		//	14 JSRQ Character 8 ��������
		out_mapDBFValue["JSRQ"] = dbfRow;

		//15 QTRQ Character 8 ��������

		//16 WTSJ Character 6 ί��ʱ��
		citRow = in_mapRowData.find("oper_time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(7, 6);


			out_mapDBFValue["WTSJ"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//17 CJSJ Character 6 �ɽ�ʱ��
		out_mapDBFValue["CJSJ"] = dbfRow;

		//18 XWH1 Character 5 ҵ��Ԫ 1
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["XWH1"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//19 XWH2 Character 5 ҵ��Ԫ 2
		out_mapDBFValue["XWH2"] = dbfRow;

		//20 XWHY Character 8ҵ��Ԫ1������������˵�������

		//21 JSHY Character 8��������˵�������

		//22 TGHY Character 8 �й����е�������

		//23 ZQZH Character 10 ֤ȯ�˺�
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["ZQZH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//24 ZQDM1 Character 6 ֤ȯ����1
		citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["ZQDM1"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//25 ZQDM2 Character 6 ֤ȯ���� 2

		//26 ZQLB Character 2 ֤ȯ���
		dbfRow.iType = 0;
		dbfRow.strValue = "PT";
		out_mapDBFValue["ZQLB"] = dbfRow;

		//27 LTLX Character 1 ��ͨ����
		dbfRow.iType = 0;
		dbfRow.strValue = "0";
		out_mapDBFValue["LTLX"] = dbfRow;

		//28 QYLB Character 2 Ȩ�����

		//29 GPNF Character 4 �������

		//30 MMBZ Character 1 ������־
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRowSide->second.strValue;

			/*
			if (0 == citRowSide->second.strValue.compare("1"))
			{
				//	�������B
				dbfRow.strValue = "B";
			}
			else if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	���������S
				dbfRow.strValue = "S";
			}
			*/

			out_mapDBFValue["MMBZ"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//31 SL Character 16 999999999999999 ��������
		citRow = in_mapRowData.find("match_qty");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			string strValue;

			if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	�����������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			dbfRow.strValue = strValue;

			out_mapDBFValue["SL"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//32 CJSL Character 16 999999999999999 �ɽ�����
		out_mapDBFValue["CJSL"] = dbfRow;

		//33 ZJZH Character 25 �ʽ��˺�

		//34 BZ Character 3 ����
		dbfRow.iType = 0;
		dbfRow.strValue = "RMB";
		out_mapDBFValue["BZ"] = dbfRow;

		//35 JG1 Character 17 999999.999999999 �۸� 1
		citRow = in_mapRowData.find("match_price");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;

			Tgw_StringUtil::StringLiToStringYuan(citRow->second.strValue, dbfRow.strValue, 9);

			out_mapDBFValue["JG1"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//36 JG2 Character 17 999999.999999999 �۸� 2
		out_mapDBFValue["JG2"] = dbfRow;

		//37 QSJE Character 19 999999999999999.99 ������
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 0;

			Tgw_StringUtil::StringLiToStringYuan(citRow->second.strValue, dbfRow.strValue, 2);

			out_mapDBFValue["QSJE"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//38 YHS Character 17 9999999999999.99 ӡ��˰
		dbfRow.iType = 0;
		dbfRow.strValue = "0.00";
		out_mapDBFValue["YHS"] = dbfRow;

		//39 JSF Character 17 9999999999999.99 ���ַ�
		dbfRow.iType = 0;
		dbfRow.strValue = "0.00";
		out_mapDBFValue["JSF"] = dbfRow;

		//40 GHF Character 17 9999999999999.99 ������
		dbfRow.iType = 0;
		dbfRow.strValue = "0.00";
		out_mapDBFValue["GHF"] = dbfRow;

		//41 ZGF Character 17 9999999999999.99 ֤�ܷ�
		dbfRow.iType = 0;
		dbfRow.strValue = "0.00";
		out_mapDBFValue["ZGF"] = dbfRow;

		//42 SXF Character 17 9999999999999.99 ������
		dbfRow.iType = 0;
		dbfRow.strValue = "0.00";
		out_mapDBFValue["SXF"] = dbfRow;


		//43 QTJE1 Character 19 999999999999999.99 ������� 1

		//44 QTJE2 Character 19 999999999999999.99 ������� 2

		//45 QTJE3 Character 19 999999999999999.99 ������� 3

		//46 SJSF Character 19 999999999999999.99 ʵ���ո�
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 0;

			Tgw_StringUtil::StringLiToStringYuan(citRow->second.strValue, dbfRow.strValue, 2);

			out_mapDBFValue["SJSF"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//47 JGDM Character 4 �������
		dbfRow.iType = 0;
		dbfRow.strValue = "0000";
		out_mapDBFValue["JGDM"] = dbfRow;


		//48 FJSM Character 40 ����˵��


	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}

	return 0;
}

/*
�Ϻ��г�jsmx00001����
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SHSettle::SH_Gen_jsmx00001(std::shared_ptr<MySqlCnnC602> &in_mysqlConn, const string& strFilePath)
{
	static const string ftag("SHSettle::SH_Gen_jsmx00001()");

	string strDbfPrefix(strFilePath);
	strDbfPrefix += "\\A";

	const string strDbfPostfix(".dbf");

	//	��seatϯλΪkey�����¶�Ӧ��value
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

	// ��ѯ
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
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "û��ϯλ��id[" << mapRowData["id"].strValue;

				continue;
			}

			mapSeatDbfValue[strKey].push_back(mapRowData);
		}
	}
	else
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ִ��" << strQuer << "����res=" << iRes;

		iReturn = -1;
	}

	// �ͷ�
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;

	if (0 != iReturn)
	{
		return iReturn;
	}

	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	iRes = SH_DBF_jsmx00001(vecSetting);

	// ���ݲ�ѯ���Ľ������DBF�ļ�
	map<string, vector< map<string, struct MySqlCnnC602_DF::DataInRow> > >::iterator mapIterSeat = mapSeatDbfValue.begin();
	if (mapSeatDbfValue.end() == mapIterSeat)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼��������jsmx.dbf";

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
			// �����dbf�ļ�������
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
			//����ÿһ��seat��Ӧ�ļ�¼
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
