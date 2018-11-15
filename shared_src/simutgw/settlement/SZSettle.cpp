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
��ʼ����Ա����
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
������ϸ
ÿ�������ID����һ��

Return :
0 -- �ɹ�
��0 -- ʧ��
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
�������
��һ��

Return :
0 -- �ɹ�
��0 -- ʧ��
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
�����г�SJSMXn����
������ϸ�� SJSMXn.DBF
����SJSMX1.DBF(����A��)��SJSMX2.DBF(����B��)

Param:
in_strTradeType: 0 -- A�ɣ� 1 -- B��

Return :
0 -- �ɹ�
��0 -- ʧ��
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

	vector<map<string, struct MySqlCnnC602_DF::DataInRow> > vecAMapRowData;
	vector<map<string, struct MySqlCnnC602_DF::DataInRow> > vecBMapRowData;

	// ��ѯ�����е������г��ɽ���¼
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

			// ���ݹ�Ʊ�������ͻ��ֵ���ͬ���ε������ļ�
			if (simutgw::TADE_TYPE::a_trade == iTradeType
				|| simutgw::TADE_TYPE::margin_cash == iTradeType
				|| simutgw::TADE_TYPE::margin_stock == iTradeType
				|| simutgw::TADE_TYPE::etf_buy == iTradeType
				|| simutgw::TADE_TYPE::etf_sell == iTradeType
				|| simutgw::TADE_TYPE::etf_crt == iTradeType
				|| simutgw::TADE_TYPE::etf_rdp == iTradeType)
			{
				// A��
				vecAMapRowData.push_back(mapRowData);
			}
			else if (simutgw::b_trade == iTradeType)
			{
				// B��
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
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ִ��" << strQuer << "����res=" << iRes;

		return -1;
	}

	// �ͷ�
	in_mysqlConn->FreeResult(&pResultSet);
	pResultSet = NULL;

	string strA_FileName(in_strFilePath);
	strA_FileName += "/";
	if (!in_strSettleGroupName.empty())
	{
		// ������ر���	
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
		// ������ر���	
		strB_FileName += in_strSettleGroupName;
		strB_FileName += "_";
	}
	strB_FileName += "SJSMX2_";
	strB_FileName += m_strNow;
	strB_FileName += ".dbf";

	// �����������ļ�
	TgwDBFOperHelper BdbfWriter;

	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	// ��ͬ���εĽṹ��ͬ
	DBF_SJSMXn(vecSetting);

	// ��һ����
	if (0 != vecAMapRowData.size())
	{
		TgwDBFOperHelper adbfWriter;

		if (!boost::filesystem::exists(strA_FileName))
		{
			// �������򴴽�
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
			// ������ر���
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� SJSMX1.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� " << in_strSettleGroupName << "_SJSMX1.dbf";
		}
	}

	// �ڶ�����
	if (0 != vecBMapRowData.size())
	{
		TgwDBFOperHelper bdbfWriter;

		if (!boost::filesystem::exists(strB_FileName))
		{
			// �������򴴽�
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
			// ������ر���
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� SJSMX2.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� " << in_strSettleGroupName << "_SJSMX2.dbf";
		}
	}

	return iReturn;
}

/*
�����г�SJSMXn����dbf��ʽ
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SZSettle::DBF_SJSMXn(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SZSettle::DBF_SJSMXn() ");

	struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;
	//��� �ֶ��� �ֶ����� ���� ���� ��ע
	//	1 MXJSZH �����˺� C 6
	Column.strName = "MXJSZH";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//2 MXBFZH �������˻� C 25
	Column.strName = "MXBFZH";
	Column.cType = 'C';
	Column.uWidth = 25;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//3 MXSJLX �������� C 2
	Column.strName = "MXSJLX";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//4 MXYWLB ҵ����� C 4
	Column.strName = "MXYWLB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//5 MXZQDM ֤ȯ���� C 8
	Column.strName = "MXZQDM";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//6 MXJYDY ���׵�Ԫ C 6
	Column.strName = "MXJYDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//7 MXTGDY �йܵ�Ԫ C 6
	Column.strName = "MXTGDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//8 MXZQZH ֤ȯ�˻����� C 20
	Column.strName = "MXZQZH";
	Column.cType = 'C';
	Column.uWidth = 20;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//9		MXDDBH �ͻ��������/���뵥�� C 24
	Column.strName = "MXDDBH";
	Column.cType = 'C';
	Column.uWidth = 24;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//10 MXYYB Ӫҵ������ C 4
	Column.strName = "MXYYB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//11 MXZXBH ִ�б�� C 16
	Column.strName = "MXZXBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//12 MXYWLSH ҵ����ˮ�� C 16
	Column.strName = "MXYWLSH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//13 MXCJSL �ɽ����� N 15,2
	Column.strName = "MXCJSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//14 MXQSSL �������� N 15,2
	Column.strName = "MXQSSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//15 MXCJJG �ɽ��۸� N 13,4
	Column.strName = "MXCJJG";
	Column.cType = 'N';
	Column.uWidth = 13;
	Column.uDecWidth = 4;
	io_vecSetting.push_back(Column);

	//16 MXQSJG ����۸� N 18,9
	Column.strName = "MXQSJG";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 9;
	io_vecSetting.push_back(Column);

	//17 MXXYJY ���ý��ױ�ʶ C 1
	Column.strName = "MXXYJY";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//18 MXPCBS ƽ�ֱ�ʶ C 1
	Column.strName = "MXPCBS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//19 MXZQLB ֤ȯ��� C 2
	Column.strName = "MXZQLB";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//20 MXZQZL ֤ȯ����� C 2
	Column.strName = "MXZQZL";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//21 MXGFXZ �ɷ����� C 2
	Column.strName = "MXGFXZ";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//22 MXJSFS ���շ�ʽ C 1
	Column.strName = "MXJSFS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//23 MXHBDH ���Ҵ��� C 3
	Column.strName = "MXHBDH";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);
	//24 MXQSBJ ���㱾�� N 17,2
	Column.strName = "MXQSBJ";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//25 MXYHS ӡ��˰ N 12,2
	Column.strName = "MXYHS";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//26 MXJYJSF ���׾��ַ� N 12,2
	Column.strName = "MXJYJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//27 MXJGGF ��ܹ�� N 12,2
	Column.strName = "MXJGGF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//28 MXGHF ������ N 12,2
	Column.strName = "MXGHF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//29 MXJSF ����� N 12,2
	Column.strName = "MXJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//30 MXSXF ������ N 12,2
	Column.strName = "MXSXF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//31 MXQSYJ ���������Ӷ�� N 12,2 �� �� ���� �� ϵͳ����
	Column.strName = "MXQSYJ";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//32 MXQTFY �������� N 12,2
	Column.strName = "MXQTFY";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//33 MXZJJE �ʽ��� N 17,2
	Column.strName = "MXZJJE";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//34 MXSFJE �ո����� N 18,2
	Column.strName = "MXSFJE";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//35 MXCJRQ �ɽ����� C 8 CCYYMMDD
	Column.strName = "MXCJRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//36 MXQSRQ �������� C 8 CCYYMMDD
	Column.strName = "MXQSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//37 MXJSRQ �������� C 8 CCYYMMDD
	Column.strName = "MXJSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//38 MXFSRQ �������� C 8 CCYYMMDD
	Column.strName = "MXFSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//39 MXQTRQ �������� C 8 CCYYMMDD
	Column.strName = "MXQTRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//40 MXSCDM �г����� C 2 �����ֶ�
	Column.strName = "MXSCDM";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//41 MXJYFS ���׷�ʽ C 2
	Column.strName = "MXJYFS";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//42 MXZQDM2 ֤ȯ����2 C 8
	Column.strName = "MXZQDM2";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//43 MXTGDY2 �йܵ�Ԫ2 C 6
	Column.strName = "MXTGDY2";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//44 MXDDBH2 �������2 C 16 �����ֶ�
	Column.strName = "MXDDBH2";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//45 MXCWDH ������� C 4
	Column.strName = "MXCWDH";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//46 MXPPHM ƥ����� C 10 ����
	Column.strName = "MXPPHM";
	Column.cType = 'C';
	Column.uWidth = 10;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//47 MXFJSM ����˵�� C 30
	Column.strName = "MXFJSM";
	Column.cType = 'C';
	Column.uWidth = 30;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//48 MXBYBZ ���ñ�־C 1 �� �� ���� �� ϵͳ����
	Column.strName = "MXBYBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	return 0;
}

/*
�����г�SJSMX1����dbfȡֵ
������ϸ�� SJSMXn.DBF
Return :
0 -- �ɹ�
��0 -- ʧ��
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
		// ���ȯ����
		string strSecurity_id2;
		// �Ƿ���Sz ETF����
		bool bIsSzEtfTrade = false;
		// �Ƿ���Sz ETF����
		bool bIsSzEtf_securityid = false;
		double dValue = 0.0;

		//
		//

		//��� �ֶ��� �ֶ����� ���� ���� ��ע
		//	1 MXJSZH �����˺� C 6
		dbfRow.iType = 0;
		dbfRow.strValue = "stgw01";
		out_mapDBFValue["MXJSZH"] = dbfRow;

		//2 MXBFZH �������˻� C 25
		dbfRow.iType = 0;
		dbfRow.strValue = "bfzh_12345";
		out_mapDBFValue["MXBFZH"] = dbfRow;

		//3 MXSJLX �������� C 2
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
			// δ�ҵ���ֵ
			strSecurityid = "";
		}

		citRow = in_mapRowData.find("security_id2");
		if (citRow != in_mapRowData.end())
		{
			strSecurity_id2 = citRow->second.strValue;
		}
		else
		{
			// δ�ҵ���ֵ
			strSecurity_id2 = "";
		}

		// �ж��Ƿ������ȯ
		if (strSecurity_id2.empty())
		{
			// �������ȯ
			bIsSzEtf_securityid = true;
		}
		else
		{
			// �����ȯ
			bIsSzEtf_securityid = false;
		}

		//4 MXYWLB ҵ����� C 4
		dbfRow.iType = 0;
		citRow = in_mapRowData.find("trade_type");
		if (citRow != in_mapRowData.end())
		{
			int iTradeType = 0;
			strTradeType = citRow->second.strValue;
			Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);

			// ���ݹ�Ʊ�������ͻ��ֵ���ͬ���ε������ļ�
			if (simutgw::TADE_TYPE::a_trade == iTradeType
				|| simutgw::TADE_TYPE::margin_cash == iTradeType
				|| simutgw::TADE_TYPE::margin_stock == iTradeType
				|| simutgw::TADE_TYPE::etf_buy == iTradeType
				|| simutgw::TADE_TYPE::etf_sell == iTradeType)
			{
				// JY00 ���о���ƽ̨���ף� �ۺϽ��ڷ���ƽ̨���ף���������ģʽ��
				dbfRow.strValue = "JY00";
			}
			else if (simutgw::TADE_TYPE::etf_crt == iTradeType)
			{
				// ��ETF����
				bIsSzEtfTrade = true;
				if (bIsSzEtf_securityid)
				{
					// ��ETF����
					// SGSF �깺�ݶ�������г�ETF
					dbfRow.strValue = "SGSF";
				}
				else
				{
					// �����֤ȯ����
					// SGSQ �깺���ȯ���������г�ETF
					dbfRow.strValue = "SGSQ";
				}
			}
			else if (simutgw::TADE_TYPE::etf_rdp == iTradeType)
			{
				// ��ETF����
				bIsSzEtfTrade = true;
				if (bIsSzEtf_securityid)
				{
					// ��ETF����
					// SHSF ��طݶ�������г�ETF
					dbfRow.strValue = "SHSF";
				}
				else
				{
					// �����֤ȯ����
					// SHSQ ������ȯ���������г�ETF
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
			// δ�ҵ���ֵ

		}
		out_mapDBFValue["MXYWLB"] = dbfRow;

		//5 MXZQDM ֤ȯ���� C 8
		dbfRow.iType = 0;
		dbfRow.strValue = strSecurityid;
		out_mapDBFValue["MXZQDM"] = dbfRow;

		//6 MXJYDY ���׵�Ԫ C 6
		citRow = in_mapRowData.find("security_seat");
		if (in_mapRowData.end() != citRow)
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXJYDY"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ

		}

		//7 MXTGDY �йܵ�Ԫ C 6 ͬ��
		out_mapDBFValue["MXTGDY"] = dbfRow;

		//8 MXZQZH ֤ȯ�˻����� C 20
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXZQZH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//9		MXDDBH �ͻ��������/���뵥�� C 24
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXDDBH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//10 MXYYB Ӫҵ������ C 4
		citRow = in_mapRowData.find("market_branchid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXYYB"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//11 MXZXBH ִ�б�� C 16
		citRow = in_mapRowData.find("execid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXZXBH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//12 MXYWLSH ҵ����ˮ�� C 16
		citRow = in_mapRowData.find("orderid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXYWLSH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//13 MXCJSL �ɽ����� N 15,2
		citRow = in_mapRowData.find("match_qty");
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	�����������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::String2Double_atof(strValue, dbfRow.dValue);

			out_mapDBFValue["MXCJSL"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//14 MXQSSL �������� N 15,2
		if (bIsSzEtfTrade)
		{
			// ETF����
		}
		else
		{
			// ��ETF����
			out_mapDBFValue["MXQSSL"] = dbfRow;
		}


		//15 MXCJJG �ɽ��۸� N 13,4
		if (bIsSzEtfTrade)
		{
			// ETF����
		}
		else
		{
			// ��ETF����
			citRow = in_mapRowData.find("match_price");
			if (citRow != in_mapRowData.end())
			{
				dbfRow.iType = 3;

				Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 4);

				out_mapDBFValue["MXCJJG"] = dbfRow;
			}
			else
			{
				// δ�ҵ���ֵ
			}
		}

		//16 MXQSJG ����۸� N 18,9
		if (bIsSzEtfTrade)
		{
			// ETF����
		}
		else
		{
			// ��ETF����
			out_mapDBFValue["MXQSJG"] = dbfRow;
		}

		//17 MXXYJY ���ý��ױ�ʶ C 1
		if (bIsSzEtfTrade)
		{
			// ETF����
		}
		else
		{
			// ��ETF����
			citRow = in_mapRowData.find("trade_type");
			if (citRow != in_mapRowData.end())
			{
				dbfRow.iType = 0;

				std::string strTradeType(citRow->second.strValue);
				int iTradeType = 0;
				Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);
				if (simutgw::margin_cash == iTradeType)
				{  // ���ʽ���--�������� ��ȯ��ȯ
					dbfRow.strValue = "1";
				}
				else if (simutgw::margin_stock == iTradeType)
				{  // ��ȯ����--��ȯ���� ��ȯ����
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
				// δ�ҵ���ֵ
			}
		}

		//18 MXPCBS ƽ�ֱ�ʶ C 1

		//19 MXZQLB ֤ȯ��� C 2
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF����
			dbfRow.strValue = "EF";
		}
		else
		{
			// ��ETF����			
			dbfRow.strValue = "00";
		}
		out_mapDBFValue["MXZQLB"] = dbfRow;

		//20 MXZQZL ֤ȯ����� C 2		
		if (bIsSzEtfTrade)
		{
			// ETF����
			dbfRow.iType = 0;
			dbfRow.strValue = "0";
			// EF 0 ���г� ETF
			out_mapDBFValue["MXZQZL"] = dbfRow;
		}
		else
		{
			// ��ETF����			
		}

		//21 MXGFXZ �ɷ����� C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["MXGFXZ"] = dbfRow;

		//22 MXJSFS ���շ�ʽ C 1
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF����
			if (bIsSzEtf_securityid)
			{
				// ��ETF����
				dbfRow.strValue = "Y";
			}
			else
			{
				dbfRow.strValue = "A";
			}
		}
		else
		{
			// ��ETF����		
			dbfRow.strValue = "A";
		}
		out_mapDBFValue["MXJSFS"] = dbfRow;

		//23 MXHBDH ���Ҵ��� C 3
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
				//	���������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dValue, 2);
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//24 MXQSBJ ���㱾�� N 17,2	
		if (bIsSzEtfTrade)
		{
			// ETF����
			if (bIsSzEtf_securityid)
			{
				// ��ETF����		

			}
			else
			{
			}
		}
		else
		{
			// ��ETF����		
			dbfRow.iType = 3;
			dbfRow.dValue = dValue;
			out_mapDBFValue["MXQSBJ"] = dbfRow;
		}

		//25 MXYHS ӡ��˰ N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXYHS"] = dbfRow;

		//26 MXJYJSF ���׾��ַ� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJYJSF"] = dbfRow;

		//27 MXJGGF ��ܹ�� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJGGF"] = dbfRow;

		//28 MXGHF ������ N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXGHF"] = dbfRow;

		//29 MXJSF ����� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJSF"] = dbfRow;

		//30 MXSXF ������ N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXSXF"] = dbfRow;

		//31 MXQSYJ ���������Ӷ�� N 12,2 �� �� ���� �� ϵͳ����
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXQSYJ"] = dbfRow;

		//32 MXQTFY �������� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXQTFY"] = dbfRow;

		//33 MXZJJE �ʽ��� N 17,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXZJJE"] = dbfRow;

		//34 MXSFJE �ո����� N 18,2
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			string strValue;
			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1) 
				|| 0 == citRowSide->second.strValue.compare("D"))
			{
				//	���������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dValue, 2);


		}
		else
		{
			// δ�ҵ���ֵ
		}

		if (bIsSzEtfTrade)
		{
			// ETF����
			if (bIsSzEtf_securityid)
			{
				// ��ETF����		

			}
			else
			{
			}
		}
		else
		{
			// ��ETF����
			dbfRow.iType = 3;
			dbfRow.dValue = dValue;
			out_mapDBFValue["MXSFJE"] = dbfRow;
		}

		//35 MXCJRQ �ɽ����� C 8 CCYYMMDD T��
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
			// δ�ҵ���ֵ
		}

		//36 MXQSRQ �������� C 8 CCYYMMDD T��
		out_mapDBFValue["MXQSRQ"] = dbfRow;

		//37 MXJSRQ �������� C 8 CCYYMMDD T+1��
		dbfRow.iType = 0;
		boost::gregorian::date_duration dra(1);
		CurrentDate += dra;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		CurrentDate -= dra;

		out_mapDBFValue["MXJSRQ"] = dbfRow;

		//38 MXFSRQ �������� C 8 CCYYMMDD T��
		dbfRow.iType = 0;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		out_mapDBFValue["MXFSRQ"] = dbfRow;

		//39 MXQTRQ �������� C 8 CCYYMMDD

		//40 MXSCDM �г����� C 2 �����ֶ�

		//41 MXJYFS ���׷�ʽ C 2
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF����
			dbfRow.strValue = "03";
		}
		else
		{
			// ��ETF����
			dbfRow.strValue = "01";
		}
		out_mapDBFValue["MXJYFS"] = dbfRow;

		//42 MXZQDM2 ֤ȯ����2 C 8
		if (bIsSzEtfTrade)
		{
			// ETF����
			if (bIsSzEtf_securityid)
			{
				// ��ETF����
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
			// ��ETF����
		}

		//43 MXTGDY2 �йܵ�Ԫ2 C 6

		//44 MXDDBH2 �������2 C 16 �����ֶ�

		//45 MXCWDH ������� C 4

		//46 MXPPHM ƥ����� C 10 ����

		//47 MXFJSM ����˵�� C 30

		//48 MXBYBZ ���ñ�־C 1 �� �� ���� �� ϵͳ����
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

/*
�����г�SJSMX2����dbfȡֵ
������ϸ�� SJSMXn.DBF
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SZSettle::DBF_Value_SJSMX2(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, struct TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SZSettle::DBF_Value_SJSMX2() ");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;

		//��� �ֶ��� �ֶ����� ���� ���� ��ע
		//	1 MXJSZH �����˺� C 6

		//2 MXBFZH �������˻� C 25

		//3 MXSJLX �������� C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "02";
		out_mapDBFValue["MXSJLX"] = dbfRow;

		//4 MXYWLB ҵ����� C 4
		dbfRow.iType = 0;
		dbfRow.strValue = "BG1C";
		out_mapDBFValue["MXYWLB"] = dbfRow;

		//5 MXZQDM ֤ȯ���� C 8
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXZQDM"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//6 MXJYDY ���׵�Ԫ C 6
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXJYDY"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//7 MXTGDY �йܵ�Ԫ C 6 ͬ��
		out_mapDBFValue["MXTGDY"] = dbfRow;

		//8 MXZQZH ֤ȯ�˻����� C 20
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXZQZH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//9		MXDDBH �ͻ��������/���뵥�� C 24
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXDDBH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//10 MXYYB Ӫҵ������ C 4
		citRow = in_mapRowData.find("market_branchid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXYYB"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//11 MXZXBH ִ�б�� C 16
		citRow = in_mapRowData.find("execid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXZXBH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//12 MXYWLSH ҵ����ˮ�� C 16
		citRow = in_mapRowData.find("orderid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["MXYWLSH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//13 MXCJSL �ɽ����� N 15,2
		citRow = in_mapRowData.find("match_qty");
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	�����������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::String2Double_atof(strValue, dbfRow.dValue);

			out_mapDBFValue["MXCJSL"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//14 MXQSSL �������� N 15,2
		out_mapDBFValue["MXQSSL"] = dbfRow;

		//15 MXCJJG �ɽ��۸� N 13,4
		citRow = in_mapRowData.find("match_price");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;

			Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 4);

			out_mapDBFValue["MXCJJG"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//16 MXQSJG ����۸� N 18,9
		out_mapDBFValue["MXQSJG"] = dbfRow;

		//17 MXXYJY ���ý��ױ�ʶ C 1

		//18 MXPCBS ƽ�ֱ�ʶ C 1

		//19 MXZQLB ֤ȯ��� C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "20";
		out_mapDBFValue["MXZQLB"] = dbfRow;

		//20 MXZQZL ֤ȯ����� C 2

		//21 MXGFXZ �ɷ����� C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["MXZQLB"] = dbfRow;

		//22 MXJSFS ���շ�ʽ C 1
		dbfRow.iType = 0;
		dbfRow.strValue = "A";
		out_mapDBFValue["MXJSFS"] = dbfRow;

		//23 MXHBDH ���Ҵ��� C 3
		dbfRow.iType = 0;
		dbfRow.strValue = "HKD";
		out_mapDBFValue["MXHBDH"] = dbfRow;

		//24 MXQSBJ ���㱾�� N 17,2
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1))
			{
				//	���������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dbfRow.dValue, 2);

			out_mapDBFValue["MXQSBJ"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}


		//25 MXYHS ӡ��˰ N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXYHS"] = dbfRow;

		//26 MXJYJSF ���׾��ַ� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJYJSF"] = dbfRow;

		//27 MXJGGF ��ܹ�� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJGGF"] = dbfRow;

		//28 MXGHF ������ N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXGHF"] = dbfRow;

		//29 MXJSF ����� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXJSF"] = dbfRow;

		//30 MXSXF ������ N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXSXF"] = dbfRow;

		//31 MXQSYJ ���������Ӷ�� N 12,2 �� �� ���� �� ϵͳ����
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXQSYJ"] = dbfRow;

		//32 MXQTFY �������� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXQTFY"] = dbfRow;

		//33 MXZJJE �ʽ��� N 17,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["MXZJJE"] = dbfRow;

		//34 MXSFJE �ո����� N 18,2
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1))
			{
				//	���������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dbfRow.dValue, 2);

			out_mapDBFValue["MXSFJE"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//35 MXCJRQ �ɽ����� C 8 CCYYMMDD T��
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
			// δ�ҵ���ֵ
		}

		//36 MXQSRQ �������� C 8 CCYYMMDD T��
		dbfRow.iType = 0;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);

		out_mapDBFValue["MXQSRQ"] = dbfRow;

		//37 MXJSRQ �������� C 8 CCYYMMDD T+3��
		dbfRow.iType = 0;
		boost::gregorian::date_duration dra(3);
		CurrentDate += dra;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		CurrentDate -= dra;

		out_mapDBFValue["MXJSRQ"] = dbfRow;

		//38 MXFSRQ �������� C 8 CCYYMMDD T��
		dbfRow.iType = 0;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);

		out_mapDBFValue["MXFSRQ"] = dbfRow;

		//39 MXQTRQ �������� C 8 CCYYMMDD T��
		out_mapDBFValue["MXQTRQ"] = dbfRow;

		//40 MXSCDM �г����� C 2 �����ֶ�

		//41 MXJYFS ���׷�ʽ C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["MXJYFS"] = dbfRow;

		//42 MXZQDM2 ֤ȯ����2 C 8

		//43 MXTGDY2 �йܵ�Ԫ2 C 6
		dbfRow.iType = 0;
		dbfRow.strValue = out_mapDBFValue["MXTGDY"].strValue;
		out_mapDBFValue["MXTGDY2"] = dbfRow;

		//44 MXDDBH2 �������2 C 16 �����ֶ�

		//45 MXCWDH ������� C 4

		//46 MXPPHM ƥ����� C 10 ����

		//47 MXFJSM ����˵�� C 30

		//48 MXBYBZ ���ñ�־C 1 �� �� ���� �� ϵͳ����
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

/*
�����г�SJSJG����
Return :
0 -- �ɹ�
��0 -- ʧ��
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

	// ��ѯ�����е������г��ɽ���¼
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

			/* ���˳�����ҵ�� ???
			*SJSJG.DBF �ӿ��ļ����������Σ�����ʾ��ϸ����������ݰ��� SJSMXn.DBF
			*�зǵ�����������ϸ����ʽ��ս����������ҵ����ΥԼ�����ý���Լ��Ǽ�
			*�����ɷݺ��ʽ���ϸ��
			*/
			if (iTradeType >= simutgw::TADE_TYPE::etf_crt)
			{
				vecMapRowData.push_back(mapRowData);
			}
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

	string strFileName(in_strFilePath);
	strFileName += "/";
	if (!in_strSettleGroupName.empty())
	{
		// ������ر���	
		strFileName += in_strSettleGroupName;
		strFileName += "_";
	}
	strFileName += "SJSJG_";
	strFileName += m_strNow;
	strFileName += ".dbf";

	// �����������ļ�
	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	DBF_SJSJG(vecSetting);

	if (0 != vecMapRowData.size())
	{
		TgwDBFOperHelper dbfWriter;

		if (!boost::filesystem::exists(strFileName))
		{
			// �������򴴽�
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
			// ������ر���
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� SJSJG.dbf";
		}
		else
		{
			BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼�������� " << in_strSettleGroupName << "_SJSJG.dbf";
		}
	}

	return iReturn;
}


/*
�����г�SJSJG����dbf��ʽ
��ϸ����� SJSJG.DBF
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SZSettle::DBF_SJSJG(std::vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SZSettle::DBF_SJSJG() ");

	struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;
	//��� �ֶ��� �ֶ����� ���� ���� ��ע
	//	1 JGJSZH �����˺� C 6
	Column.strName = "JGJSZH";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//2 JGBFZH �������˻� C 25
	Column.strName = "JGBFZH";
	Column.cType = 'C';
	Column.uWidth = 25;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//3 JGSJLX �������� C 2
	Column.strName = "JGSJLX";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//4 JGYWLB ҵ����� C 4
	Column.strName = "JGYWLB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//5 JGZQDM ֤ȯ���� C 8
	Column.strName = "JGZQDM";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//6 JGJYDY ���׵�Ԫ C 6
	Column.strName = "JGJYDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//7 JGTGDY �йܵ�Ԫ C 6
	Column.strName = "JGTGDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//8 JGZQZH ֤ȯ�˻����� C 20
	Column.strName = "JGZQZH";
	Column.cType = 'C';
	Column.uWidth = 20;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//9 JGDDBH �ͻ�������� C 24
	Column.strName = "JGDDBH";
	Column.cType = 'C';
	Column.uWidth = 24;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//10 JGYYB Ӫҵ������ C 4
	Column.strName = "JGYYB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//11 JGZXBH ִ�б�� C 16
	Column.strName = "JGZXBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//12 JGYWLSH ҵ����ˮ�� C 16
	Column.strName = "JGYWLSH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//13 JGCJSL �ɽ����� N 15,2
	Column.strName = "JGCJSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//14 JGQSSL �������� N 15,2
	Column.strName = "JGQSSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//15 JGJSSL �������� N 15,2
	Column.strName = "JGJSSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//16 JGCJJG �ɽ��۸� N 13,4
	Column.strName = "JGCJJG";
	Column.cType = 'N';
	Column.uWidth = 13;
	Column.uDecWidth = 4;
	io_vecSetting.push_back(Column);

	//17 JGQSJG ����۸� N 18,9
	Column.strName = "JGQSJG";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 9;
	io_vecSetting.push_back(Column);

	//18 JGXYJY ���ý��ױ�ʶ C 1
	Column.strName = "JGXYJY";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//19 JGPCBS ƽ�ֱ�ʶ C 1
	Column.strName = "JGPCBS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//20 JGZQLB ֤ȯ��� C 2
	Column.strName = "JGZQLB";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//21 JGZQZL ֤ȯ���� C 2
	Column.strName = "JGZQZL";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//22 JGGFXZ �ɷ����� C 2
	Column.strName = "JGGFXZ";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//23 JGLTLX ��ͨ���� C 1
	Column.strName = "JGLTLX";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//24 JGJSFS ���շ�ʽ C 1
	Column.strName = "JGJSFS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//25 JGHBDH ���Ҵ��� C 3
	Column.strName = "JGHBDH";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//26 JGQSBJ ���㱾�� N 17,2
	Column.strName = "JGQSBJ";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//27 JGYHS ӡ��˰ N 12,2
	Column.strName = "JGYHS";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//28 JGJYJSF ���׾��ַ� N 12,2
	Column.strName = "JGJYJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//29 JGJGGF ��ܹ�� N 12,2
	Column.strName = "JGJGGF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//30 JGGHF ������ N 12,2
	Column.strName = "JGGHF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//31 JGJSF ����� N 12,2
	Column.strName = "JGJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//32 JGSXF ������ N 12,2
	Column.strName = "JGSXF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//33 JGQSYJ ���������Ӷ�� N 12,2 ���������ϵͳ����
	Column.strName = "JGQSYJ";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//34 JGQTFY �������� N 12,2
	Column.strName = "JGQTFY";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//35 JGZJJE �ʽ��� N 17,2
	Column.strName = "JGZJJE";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//36 JGSFJE �ո����� N 18,2
	Column.strName = "JGSFJE";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//37 JGJSBZ ���ձ�־ C 1
	Column.strName = "JGJSBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//38 JGZYDH ժҪ���� C 4
	Column.strName = "JGZYDH";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//39 JGCJRQ �ɽ����� C 8 CCYYMMDD
	Column.strName = "JGCJRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//40 JGQSRQ �������� C 8 CCYYMMDD
	Column.strName = "JGQSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//41 JGJSRQ �������� C 8 CCYYMMDD
	Column.strName = "JGJSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//42 JGFSRQ �������� C 8 CCYYMMDD
	Column.strName = "JGFSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//43 JGQTRQ �������� C 8 CCYYMMDD
	Column.strName = "JGQTRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//44 JGSCDM �г����� C 2
	Column.strName = "JGSCDM";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//45 JGJYFS ���׷�ʽ C 2
	Column.strName = "JGJYFS";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//46 JGZQDM2 ֤ȯ���� 2 C 8
	Column.strName = "JGZQDM2";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//47 JGTGDY2 �йܵ�Ԫ 2 C 6
	Column.strName = "JGTGDY2";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//48 JGDDBH2 ������� 2 C 16 �����ֶ�
	Column.strName = "JGDDBH2";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//49 JGFJSM ����˵�� C 100
	Column.strName = "JGFJSM";
	Column.cType = 'C';
	Column.uWidth = 100;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//50 JGBYBZ ���ñ�־ C 1 ���������ϵͳ����
	Column.strName = "JGBYBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	return 0;
}

/*
�����г�SJSJG����dbfȡֵ
��ϸ����� SJSJG.DBF

Return :
0 -- �ɹ�
1 -- ������������д�룬����������һ��
��0 -- ʧ��
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
		// ���ȯ����
		string strSecurity_id2;
		// �Ƿ���Sz ETF����
		bool bIsSzEtfTrade = false;
		// �Ƿ���Sz ETF����
		bool bIsSzEtf_securityid = false;
		double dValue = 0.0;

		//��� �ֶ��� �ֶ����� ���� ���� ��ע
		//	1 JGJSZH �����˺� C 6
		dbfRow.iType = 0;
		dbfRow.strValue = "stgw01";
		out_mapDBFValue["JGJSZH"] = dbfRow;

		//2 JGBFZH �������˻� C 25
		dbfRow.iType = 0;
		dbfRow.strValue = "bfzh_12345";
		out_mapDBFValue["JGBFZH"] = dbfRow;

		//3 JGSJLX �������� C 2
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
			// δ�ҵ���ֵ
			strSecurityid = "";
		}

		citRow = in_mapRowData.find("security_id2");
		if (citRow != in_mapRowData.end())
		{
			strSecurity_id2 = citRow->second.strValue;
		}
		else
		{
			// δ�ҵ���ֵ
			strSecurity_id2 = "";
		}

		// �ж��Ƿ������ȯ
		if (strSecurity_id2.empty())
		{
			// �������ȯ
			bIsSzEtf_securityid = true;
		}
		else
		{
			// �����ȯ
			bIsSzEtf_securityid = false;
		}

		//4 JGYWLB ҵ����� C 4
		dbfRow.iType = 0;
		citRow = in_mapRowData.find("trade_type");
		if (citRow != in_mapRowData.end())
		{
			int iTradeType = 0;
			
			strTradeType = citRow->second.strValue;
			Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);

			// ���ݹ�Ʊ�������ͻ��ֵ���ͬ���ε������ļ�
			if (simutgw::TADE_TYPE::a_trade == iTradeType
				|| simutgw::TADE_TYPE::margin_cash == iTradeType
				|| simutgw::TADE_TYPE::margin_stock == iTradeType
				|| simutgw::TADE_TYPE::etf_buy == iTradeType
				|| simutgw::TADE_TYPE::etf_sell == iTradeType)
			{
				// JY00	���н��׻��ۺ�Э�齻�ף��������գ�����ΥԼ�Ĵ�����֤ȯ�ر�
				dbfRow.strValue = "JY00";
			}
			else if (simutgw::TADE_TYPE::etf_crt == iTradeType)
			{
				// ��ETF����
				bIsSzEtfTrade = true;
				if (bIsSzEtf_securityid)
				{
					// ��ETF����
					// EFXC ETF�깺����ֽ���
					dbfRow.strValue = "EFXC";
				}
				else
				{
					// �����֤ȯ����
					return 1;
				}
			}
			else if (simutgw::TADE_TYPE::etf_rdp == iTradeType)
			{
				// ��ETF����
				bIsSzEtfTrade = true;
				if (bIsSzEtf_securityid)
				{
					// ��ETF����
					// EFXC ETF�깺����ֽ���
					dbfRow.strValue = "EFXC";
				}
				else
				{
					// �����֤ȯ����
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
			// δ�ҵ���ֵ

		}
		out_mapDBFValue["JGYWLB"] = dbfRow;

		//5 JGZQDM ֤ȯ���� C 8
		dbfRow.iType = 0;
		dbfRow.strValue = strSecurityid;
		out_mapDBFValue["JGZQDM"] = dbfRow;

		//6 JGJYDY ���׵�Ԫ C 6
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGJYDY"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//7 JGTGDY �йܵ�Ԫ C 6
		out_mapDBFValue["JGTGDY"] = dbfRow;

		//8 JGZQZH ֤ȯ�˻����� C 20
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGZQZH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//9 JGDDBH �ͻ�������� C 24
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGDDBH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//10 JGYYB Ӫҵ������ C 4
		citRow = in_mapRowData.find("market_branchid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGYYB"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//11 JGZXBH ִ�б�� C 16
		citRow = in_mapRowData.find("execid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGZXBH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//12 JGYWLSH ҵ����ˮ�� C 16
		citRow = in_mapRowData.find("orderid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGYWLSH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//13 JGCJSL �ɽ����� N 15,2
		citRow = in_mapRowData.find("match_qty");
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	�����������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::String2Double_atof(strValue, dbfRow.dValue);

			out_mapDBFValue["JGCJSL"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//14 JGQSSL �������� N 15,2
		out_mapDBFValue["JGQSSL"] = dbfRow;

		//15 JGJSSL �������� N 15, 2
		out_mapDBFValue["JGJSSL"] = dbfRow;

		//16 JGCJJG �ɽ��۸� N 13,4
		citRow = in_mapRowData.find("match_price");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;

			Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 4);

			out_mapDBFValue["JGCJJG"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//17 JGQSJG ����۸� N 18,9
		out_mapDBFValue["JGQSJG"] = dbfRow;

		//18 JGXYJY ���ý��ױ�ʶ C 1
		citRow = in_mapRowData.find("trade_type");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;

			std::string strTradeType(citRow->second.strValue);
			int iTradeType = 0;
			Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);
			if (simutgw::margin_cash == iTradeType)
			{  // ���ʽ���--�������� ��ȯ��ȯ
				dbfRow.strValue = "1";
			}
			else if (simutgw::margin_stock == iTradeType)
			{  // ��ȯ����--��ȯ���� ��ȯ����
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
			// δ�ҵ���ֵ
		}

		//19 JGPCBS ƽ�ֱ�ʶ C 1

		//20 JGZQLB ֤ȯ��� C 2
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF����
			dbfRow.strValue = "EF";
		}
		else
		{
			// ��ETF����			
			dbfRow.strValue = "00";
		}
		out_mapDBFValue["JGZQLB"] = dbfRow;

		//21 JGZQZL ֤ȯ���� C 2
		if (bIsSzEtfTrade)
		{
			// ETF����
			dbfRow.iType = 0;
			dbfRow.strValue = "0";
			// EF 0 ���г� ETF
			out_mapDBFValue["JGZQZL"] = dbfRow;
		}
		else
		{
			// ��ETF����			
		}

		//22 JGGFXZ �ɷ����� C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["JGGFXZ"] = dbfRow;

		//23 JGLTLX ��ͨ���� C 1

		//24 JGJSFS ���շ�ʽ C 1
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF����
			if (bIsSzEtf_securityid)
			{
				// ��ETF����
				dbfRow.strValue = "Z";
			}
			else
			{
			}
		}
		else
		{
			// ��ETF����		
			dbfRow.strValue = "A";
		}
		out_mapDBFValue["JGJSFS"] = dbfRow;

		//25 JGHBDH ���Ҵ��� C 3
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
				//	���������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dValue, 2);


		}
		else
		{
			// δ�ҵ���ֵ
		}

		//26 JGQSBJ ���㱾�� N 17, 2
		dbfRow.iType = 3;
		dbfRow.dValue = dValue;
		out_mapDBFValue["JGQSBJ"] = dbfRow;

		//27 JGYHS ӡ��˰ N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGYHS"] = dbfRow;

		//28 JGJYJSF ���׾��ַ� N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJYJSF"] = dbfRow;

		//29 JGJGGF ��ܹ�� N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJGGF"] = dbfRow;

		//30 JGGHF ������ N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGGHF"] = dbfRow;

		//31 JGJSF ����� N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJSF"] = dbfRow;

		//32 JGSXF ������ N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGSXF"] = dbfRow;

		//33 JGQSYJ ���������Ӷ�� N 12, 2 ���������ϵͳ����
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGQSYJ"] = dbfRow;

		//34 JGQTFY �������� N 12, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGQTFY"] = dbfRow;

		//35 JGZJJE �ʽ��� N 17, 2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGZJJE"] = dbfRow;

		//36 JGSFJE �ո����� N 18, 2
		dbfRow.iType = 3;
		dbfRow.dValue = dValue;
		out_mapDBFValue["JGSFJE"] = dbfRow;

		//37 JGJSBZ ���ձ�־ C 1
		// JGJSBZ(���ձ�־)����Y��: ���ճɹ��� ��N��: ����ʧ��
		if (bIsSzEtfTrade)
		{
			// ETF����
			if (bIsSzEtf_securityid)
			{
				// ��ETF����
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
			// ��ETF����
		}

		//38 JGZYDH ժҪ���� C 4

		//39 JGCJRQ �ɽ����� C 8 CCYYMMDD
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
			// δ�ҵ���ֵ
		}

		//40 JGQSRQ �������� C 8 CCYYMMDD
		out_mapDBFValue["JGQSRQ"] = dbfRow;

		//41 JGJSRQ �������� C 8 CCYYMMDD
		dbfRow.iType = 0;
		boost::gregorian::date_duration dra(1);
		CurrentDate += dra;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		CurrentDate -= dra;

		out_mapDBFValue["JGJSRQ"] = dbfRow;

		//42 JGFSRQ �������� C 8 CCYYMMDD
		dbfRow.iType = 0;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		out_mapDBFValue["JGFSRQ"] = dbfRow;

		//43 JGQTRQ �������� C 8 CCYYMMDD

		//44 JGSCDM �г����� C 2

		//45 JGJYFS ���׷�ʽ C 2
		/*
		MXJYFS�����׷�ʽ������ʾҵ��ί�еĻ���������01�� �� ��ͨ���� (����� )����02�����ۺϽ��ڷ���ƽ̨���� (����� )������03�� ��������걨�ķǽ���ҵ��
		��04�� �����й��������ڷֹ�˾D-COMϵͳ�걨�ķǽ���ҵ�� ��05�� �����й������ܹ�˾����ҵ��ƽ̨�걨�ģ�
		��06�� �����й������ܹ�˾TAϵͳ�걨��ҵ�� ��07�� �������������걨��ҵ�񣬡�08�� ���й��������ڷֹ�˾ǰ̨ҵ�� ��09�� ���й��������ڷֹ�˾���⴦�� �� �� ��������
		*/
		dbfRow.iType = 0;
		if (bIsSzEtfTrade)
		{
			// ETF����
			dbfRow.strValue = "03";
		}
		else
		{
			// ��ETF����
			dbfRow.strValue = "01";
		}
		out_mapDBFValue["JGJYFS"] = dbfRow;

		//46 JGZQDM2 ֤ȯ���� 2 C 8

		//47 JGTGDY2 �йܵ�Ԫ 2 C 6

		//48 JGDDBH2 ������� 2 C 16 �����ֶ�
		//49 JGFJSM ����˵�� C 100
		//50 JGBYBZ ���ñ�־ C 1 ���������ϵͳ����
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}


/*
�����г�SJSTJ����
�ɷݽ�����ʿ� SJSDZ.DBF
Return :
0 -- �ɹ�
��0 -- ʧ��
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

	// ��ѯ�����е������г��ɽ���¼
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
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "����û�гɽ���¼��������SJSDZ.dbf";
	}

	return 0;
}

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
int SZSettle::Gen_DBF_SJSDZ(vector<map<string, struct MySqlCnnC602_DF::DataInRow> >& in_vecMapRowData,
	const std::string& strFilePath, bool bIsNextDay)
{
	static const string ftag("SZSettle::Gen_DBF_SJSDZ() ");

	int iReturn = 0;

	// �����������ļ�
	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;

	// ��ͬ���εĽṹ��ͬ
	DBF_SJSDZ(vecSetting);

	TgwDBFOperHelper adbfWriter;

	if (!boost::filesystem::exists(strFilePath))
	{
		// �������򴴽�
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
�����г�SJSDZ����dbf��ʽ
�ɷݽ�����ʿ� SJSDZ.DBF
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SZSettle::DBF_SJSDZ(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SZSettle::DBF_SJSDZ() ");

	try
	{
		struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;

		//��� �ֶ��� �ֶ����� ���� ���� ��ע
		//1 DZTGDY �йܵ�Ԫ C 6
		Column.strName = "DZTGDY";
		Column.cType = 'C';
		Column.uWidth = 6;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//2 DZZQDM ֤ȯ���� C 8
		Column.strName = "DZZQDM";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//3 DZZQZH ֤ȯ�˻� C 20
		Column.strName = "DZZQZH";
		Column.cType = 'C';
		Column.uWidth = 20;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//4 DZGFXZ �ɷ����� C 2
		Column.strName = "DZGFXZ";
		Column.cType = 'C';
		Column.uWidth = 2;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//5 DZLTLX ��ͨ���� C 1
		Column.strName = "DZLTLX";
		Column.cType = 'C';
		Column.uWidth = 1;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//6 DZZYGS ��ӵ���� N 17,2
		Column.strName = "DZZYGS";
		Column.cType = 'N';
		Column.uWidth = 17;
		Column.uDecWidth = 2;
		io_vecSetting.push_back(Column);

		//7 DZFSRQ �������� C 8 CCYYMMDD
		Column.strName = "DZFSRQ";
		Column.cType = 'C';
		Column.uWidth = 8;
		Column.uDecWidth = 0;
		io_vecSetting.push_back(Column);

		//8 DZBYBZ ���ñ�־ C 1 ���������ϵͳ����
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
�����г�SJSTJ����dbfȡֵ
�ɷݽ�����ʿ� SJSDZ.DBF
Return :
0 -- �ɹ�
��0 -- ʧ��
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

		//��� �ֶ��� �ֶ����� ���� ���� ��ע
		//1 DZTGDY �йܵ�Ԫ C 6
		dbfRow.iType = 0;
		dbfRow.strValue = "stgw01";
		out_mapDBFValue["DZTGDY"] = dbfRow;

		//2 DZZQDM ֤ȯ���� C 8
		citRow = in_mapRowData.find("stock_id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["DZZQDM"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//3 DZZQZH ֤ȯ�˻� C 20
		citRow = in_mapRowData.find("account_id");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["DZZQZH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//4 DZGFXZ �ɷ����� C 2
		// ��00 ������������ͨ�ɣ�
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["DZGFXZ"] = dbfRow;

		//5 DZLTLX ��ͨ���� C 1
		// ��0���������������������Ƿ������䡰�ɷ����ʡ�������
		dbfRow.iType = 0;
		dbfRow.strValue = "0";
		out_mapDBFValue["DZLTLX"] = dbfRow;

		//6 DZZYGS ��ӵ���� N 17,2
		citRow = in_mapRowData.find("stock_available");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			Tgw_StringUtil::String2Double_atof(citRow->second.strValue, dbfRow.dValue);

			out_mapDBFValue["DZZYGS"] = dbfRow;
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

		//7 DZFSRQ �������� C 8 CCYYMMDD
		citRow = in_mapRowData.find("time");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue.substr(0, 8);

			out_mapDBFValue["DZFSRQ"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//8 DZBYBZ ���ñ�־ C 1 ���������ϵͳ����

	}
	catch (exception &e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}

	return 0;
}

/*
�����г�SJSTJ����dbf��ʽ
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SZSettle::SZ_DBF_SJSTJ(vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> &io_vecSetting)
{
	static const string ftag("SZSettle::SZ_DBF_SJSTJ() ");

	struct TgwDBFOperHelper_DF::ColumnSettingInDBF Column;
	//��� �ֶ��� �ֶ����� ���� ���� ��ע
	//	1 JGJSZH �����˺� C 6
	Column.strName = "JGJSZH";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//2 JGBFZH �������˻� C 25
	Column.strName = "JGBFZH";
	Column.cType = 'C';
	Column.uWidth = 25;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//3 JGSJLX �������� C 2
	Column.strName = "JGSJLX";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//4 JGYWLB ҵ����� C 4
	Column.strName = "JGYWLB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//5 JGZQDM ֤ȯ���� C 8
	Column.strName = "JGZQDM";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//6 JGJYDY ���׵�Ԫ C 6
	Column.strName = "JGJYDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//7 JGTGDY �йܵ�Ԫ C 6
	Column.strName = "JGTGDY";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//8 JGZQZH ֤ȯ�˻����� C 20
	Column.strName = "JGZQZH";
	Column.cType = 'C';
	Column.uWidth = 20;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//9 JGDDBH �ͻ�������� C 24
	Column.strName = "JGDDBH";
	Column.cType = 'C';
	Column.uWidth = 24;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//10 JGYYB Ӫҵ������ C 4
	Column.strName = "JGYYB";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//11 JGZXBH ִ�б�� C 16
	Column.strName = "JGZXBH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//12 JGYWLSH ҵ����ˮ�� C 16
	Column.strName = "JGYWLSH";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//13 JGCJSL �ɽ����� N 15,2
	Column.strName = "JGCJSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//14 JGQSSL �������� N 15,2
	Column.strName = "JGQSSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//15 JGJSSL �������� N 15,2
	Column.strName = "JGJSSL";
	Column.cType = 'N';
	Column.uWidth = 15;
	Column.uDecWidth = 4;
	io_vecSetting.push_back(Column);

	//16 JGCJJG �ɽ��۸� N 13,4
	Column.strName = "JGCJJG";
	Column.cType = 'N';
	Column.uWidth = 13;
	Column.uDecWidth = 4;
	io_vecSetting.push_back(Column);

	//17 JGQSJG ����۸� N 18,9
	Column.strName = "JGQSJG";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 9;
	io_vecSetting.push_back(Column);

	//18 JGXYJY ���ý��ױ�ʶ C 1
	Column.strName = "JGXYJY";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//19 JGPCBS ƽ�ֱ�ʶ C 1
	Column.strName = "JGPCBS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//20 JGZQLB ֤ȯ��� C 2
	Column.strName = "JGZQLB";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//21 JGZQZL ֤ȯ���� C 2
	Column.strName = "JGZQZL";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//22 JGGFXZ �ɷ����� C 2
	Column.strName = "JGGFXZ";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//23 JGJSFS ���շ�ʽ C 1
	Column.strName = "JGJSFS";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//24 JGHBDH ���Ҵ��� C 3
	Column.strName = "JGHBDH";
	Column.cType = 'C';
	Column.uWidth = 3;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//25 JGQSBJ ���㱾�� N 17,2
	Column.strName = "JGQSBJ";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//26 JGYHS ӡ��˰ N 12,2
	Column.strName = "JGYHS";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//27 JGJYJSF ���׾��ַ� N 12,2
	Column.strName = "JGJYJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//28 JGJGGF ��ܹ�� N 12,2
	Column.strName = "JGJGGF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//29 JGGHF ������ N 12,2
	Column.strName = "JGGHF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//30 JGJSF ����� N 12,2
	Column.strName = "JGJSF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//31 JGSXF ������ N 12,2
	Column.strName = "JGSXF";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//32 JGQSYJ ���������Ӷ�� N 12,2 ���������ϵͳ����
	Column.strName = "JGQSYJ";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//33 JGQTFY �������� N 12,2
	Column.strName = "JGQTFY";
	Column.cType = 'N';
	Column.uWidth = 12;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//34 JGZJJE �ʽ��� N 17,2
	Column.strName = "JGZJJE";
	Column.cType = 'N';
	Column.uWidth = 17;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//35 JGSFJE �ո����� N 18,2
	Column.strName = "JGSFJE";
	Column.cType = 'N';
	Column.uWidth = 18;
	Column.uDecWidth = 2;
	io_vecSetting.push_back(Column);

	//36 JGJSBZ ���ձ�־ C 1
	Column.strName = "JGJSBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//37 JGZYDH ժҪ���� C 4
	Column.strName = "JGZYDH";
	Column.cType = 'C';
	Column.uWidth = 4;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//38 JGCJRQ �ɽ����� C 8 CCYYMMDD
	Column.strName = "JGCJRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//39 JGQSRQ �������� C 8 CCYYMMDD
	Column.strName = "JGQSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//40 JGJSRQ �������� C 8 CCYYMMDD
	Column.strName = "JGJSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//41 JGFSRQ �������� C 8 CCYYMMDD
	Column.strName = "JGFSRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//42 JGQTRQ �������� C 8 CCYYMMDD
	Column.strName = "JGQTRQ";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//43 JGSCDM �г����� C 2
	Column.strName = "JGSCDM";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//44 JGJYFS ���׷�ʽ C 2
	Column.strName = "JGJYFS";
	Column.cType = 'C';
	Column.uWidth = 2;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//45 JGZQDM2 ֤ȯ���� 2 C 8
	Column.strName = "JGZQDM2";
	Column.cType = 'C';
	Column.uWidth = 8;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//46 JGTGDY2 �йܵ�Ԫ 2 C 6
	Column.strName = "JGTGDY2";
	Column.cType = 'C';
	Column.uWidth = 6;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//47 JGDDBH2 ������� 2 C 16 �����ֶ�
	Column.strName = "JGDDBH2";
	Column.cType = 'C';
	Column.uWidth = 16;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//48 JGFJSM ����˵�� C 100
	Column.strName = "JGFJSM";
	Column.cType = 'C';
	Column.uWidth = 30;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	//49 JGBYBZ ���ñ�־ C 1
	Column.strName = "JGBYBZ";
	Column.cType = 'C';
	Column.uWidth = 1;
	Column.uDecWidth = 0;
	io_vecSetting.push_back(Column);

	return 0;
}

/*
�����г�SJSTJ����dbfȡֵ
Return :
0 -- �ɹ�
��0 -- ʧ��
*/
int SZSettle::SZ_DBF_Value_SJSTJ(const map<string, struct MySqlCnnC602_DF::DataInRow> &in_mapRowData,
	map<string, TgwDBFOperHelper_DF::DataInRow> &out_mapDBFValue)
{
	static const string ftag("SZSettle::SZ_DBF_Value_SJSTJ() ");

	try
	{
		out_mapDBFValue.clear();

		struct TgwDBFOperHelper_DF::DataInRow dbfRow;

		//��� �ֶ��� �ֶ����� ���� ���� ��ע
		//	1 JGJSZH �����˺� C 6

		//2 JGBFZH �������˻� C 25

		//3 JGSJLX �������� C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["JGSJLX"] = dbfRow;

		//4 JGYWLB ҵ����� C 4
		dbfRow.iType = 0;
		dbfRow.strValue = "JY00";
		out_mapDBFValue["JGYWLB"] = dbfRow;

		//5 JGZQDM ֤ȯ���� C 8
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRow = in_mapRowData.find("securityid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGZQDM"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//6 JGJYDY ���׵�Ԫ C 6
		citRow = in_mapRowData.find("security_seat");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGJYDY"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//7 JGTGDY �йܵ�Ԫ C 6
		out_mapDBFValue["JGTGDY"] = dbfRow;

		//8 JGZQZH ֤ȯ�˻����� C 20
		citRow = in_mapRowData.find("security_account");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGZQZH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//9 JGDDBH �ͻ�������� C 24
		citRow = in_mapRowData.find("clordid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGDDBH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//10 JGYYB Ӫҵ������ C 4
		citRow = in_mapRowData.find("market_branchid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGYYB"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//11 JGZXBH ִ�б�� C 16
		citRow = in_mapRowData.find("execid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGZXBH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//12 JGYWLSH ҵ����ˮ�� C 16
		citRow = in_mapRowData.find("orderid");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;
			dbfRow.strValue = citRow->second.strValue;

			out_mapDBFValue["JGYWLSH"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//13 JGCJSL �ɽ����� N 15,2
		citRow = in_mapRowData.find("match_qty");
		map<string, struct MySqlCnnC602_DF::DataInRow>::const_iterator citRowSide = in_mapRowData.find("side");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare("2"))
			{
				//	�����������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::String2Double_atof(strValue, dbfRow.dValue);

			out_mapDBFValue["JGCJSL"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//14 JGQSSL �������� N 15,2
		out_mapDBFValue["JGQSSL"] = dbfRow;

		//15 JGJSSL �������� N 15,2
		out_mapDBFValue["JGJSSL"] = dbfRow;

		//16 JGCJJG �ɽ��۸� N 13,4
		citRow = in_mapRowData.find("match_price");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 3;

			Tgw_StringUtil::StringLiToDouble(citRow->second.strValue, dbfRow.dValue, 4);

			out_mapDBFValue["JGCJJG"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		//17 JGQSJG ����۸� N 18,9
		out_mapDBFValue["JGQSJG"] = dbfRow;

		//18 JGXYJY ���ý��ױ�ʶ C 1
		citRow = in_mapRowData.find("trade_type");
		if (citRow != in_mapRowData.end())
		{
			dbfRow.iType = 0;

			std::string strTradeType(citRow->second.strValue);
			int iTradeType = 0;
			Tgw_StringUtil::String2Int_atoi(strTradeType, iTradeType);
			if (simutgw::margin_cash == iTradeType)
			{  // ���ʽ���--�������� ��ȯ��ȯ
				dbfRow.strValue = "1";
			}
			else if (simutgw::margin_stock == iTradeType)
			{  // ��ȯ����--��ȯ���� ��ȯ����
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
			// δ�ҵ���ֵ
		}

		//19 JGPCBS ƽ�ֱ�ʶ C 1

		//20 JGZQLB ֤ȯ��� C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["JGZQLB"] = dbfRow;

		//21 JGZQZL ֤ȯ���� C 2

		//22 JGGFXZ �ɷ����� C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "00";
		out_mapDBFValue["JGGFXZ"] = dbfRow;

		//23 JGJSFS ���շ�ʽ C 1
		dbfRow.iType = 0;
		dbfRow.strValue = "N";
		out_mapDBFValue["JGJSFS"] = dbfRow;

		//24 JGHBDH ���Ҵ��� C 3
		dbfRow.iType = 0;
		dbfRow.strValue = "RMB";
		out_mapDBFValue["JGHBDH"] = dbfRow;

		//25 JGQSBJ ���㱾�� N 17,2
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1))
			{
				//	���������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dbfRow.dValue, 2);

			out_mapDBFValue["JGQSBJ"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}


		//26 JGYHS ӡ��˰ N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGYHS"] = dbfRow;

		//27 JGJYJSF ���׾��ַ� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJYJSF"] = dbfRow;

		//28 JGJGGF ��ܹ�� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJGGF"] = dbfRow;

		//29 JGGHF ������ N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGGHF"] = dbfRow;

		//30 JGJSF ����� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGJSF"] = dbfRow;

		//31 JGSXF ������ N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGSXF"] = dbfRow;

		//32 JGQSYJ ���������Ӷ�� N 12,2 ���������ϵͳ����
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGQSYJ"] = dbfRow;

		//33 JGQTFY �������� N 12,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGQTFY"] = dbfRow;

		//34 JGZJJE �ʽ��� N 17,2
		dbfRow.iType = 3;
		dbfRow.dValue = 0;
		out_mapDBFValue["JGZJJE"] = dbfRow;

		//35 JGSFJE �ո����� N 18,2
		citRow = in_mapRowData.find("match_amount");
		if (citRow != in_mapRowData.end() && citRowSide != in_mapRowData.end())
		{
			dbfRow.iType = 3;
			string strValue;

			if (0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_B)
				|| 0 == citRowSide->second.strValue.compare(simutgw::STEPMSG_SIDE_BUY_1))
			{
				//	���������ӷ��Ÿ�
				strValue = "-";
			}
			strValue += citRow->second.strValue;

			Tgw_StringUtil::StringLiToDouble(strValue, dbfRow.dValue, 2);

			out_mapDBFValue["JGSFJE"] = dbfRow;
		}
		else
		{
			// δ�ҵ���ֵ
		}

		// 36 JGJSBZ ���ձ�־ C 1

		//37 JGZYDH ժҪ���� C 4

		//38 JGCJRQ �ɽ����� C 8 CCYYMMDD
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
			// δ�ҵ���ֵ
		}

		//39 JGQSRQ �������� C 8 CCYYMMDD
		out_mapDBFValue["JGQSRQ"] = dbfRow;

		//40 JGJSRQ �������� C 8 CCYYMMDD
		dbfRow.iType = 0;
		boost::gregorian::date_duration dra(1);
		CurrentDate += dra;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		CurrentDate -= dra;

		out_mapDBFValue["JGJSRQ"] = dbfRow;

		//41 JGFSRQ �������� C 8 CCYYMMDD
		dbfRow.iType = 0;
		dbfRow.strValue = boost::gregorian::to_iso_string(CurrentDate);
		out_mapDBFValue["JGFSRQ"] = dbfRow;

		//42 JGQTRQ �������� C 8 CCYYMMDD

		//43 JGSCDM �г����� C 2

		//44 JGJYFS ���׷�ʽ C 2
		dbfRow.iType = 0;
		dbfRow.strValue = "01";
		out_mapDBFValue["JGJYFS"] = dbfRow;

		//45 JGZQDM2 ֤ȯ���� 2 C 8

		//46 JGTGDY2 �йܵ�Ԫ 2 C 6

		//47 JGDDBH2 ������� 2 C 16 �����ֶ�

		//48 JGFJSM ����˵�� C 100

		//49 JGBYBZ ���ñ�־
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

