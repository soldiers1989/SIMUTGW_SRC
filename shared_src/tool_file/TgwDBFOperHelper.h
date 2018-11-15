#ifndef __TGW_DBF_OPER_HELPER_H__
#define __TGW_DBF_OPER_HELPER_H__

#include <map>
#include <vector>
#include <memory>

#include "tool_file/DBF.H"

namespace TgwDBFOperHelper_DF
{
	struct ColumnSettingInDBF
	{
		/*
			'C' --	�ַ�
			'N' --  ������
			'D' --  ����??
		*/
		char cType;
		unsigned short uWidth;
		unsigned short uDecWidth;
		std::string strName;
	};

	struct DataInRow
	{
		/*
			0 -- string
			1 -- long
			2 -- unsigned long
			3 -- double
		*/
		int iType;

		long lValue;
		unsigned long ulValue;
		double dValue;
		std::string strValue;
	};
}

/*
	DBF������
	�ṩдdbf�ļ�����
	��ʱδ���Ƕ��̲߳���
*/
class TgwDBFOperHelper
{
	/*
		member
	*/
private:
	//	DBF���
	std::shared_ptr<CDBF> m_pDBF;
	
	//	
	std::string m_strKeyName;
	
	std::string m_strFilePath;

	std::map<std::string, long> m_mapKeyRecno;

	// ��ǰ�к�
	long m_lRecNoNow;

	// ������
	long m_lTotalNum;
	/*
		function
	*/
public:
	TgwDBFOperHelper(void);
	virtual ~TgwDBFOperHelper(void);
	
	/*
		����dbf
	*/
	int Create(const std::string &strFileName, const std::vector<TgwDBFOperHelper_DF::ColumnSettingInDBF>& vecSet);

	/*
		��dbf
	*/
	int Open( const std::string &strFileName );
	
	/*
		�ر�dbf
	*/
	int Close();
	
	/*
		��dbfĩβ���һ�м�¼
	*/
	int Append(const std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& mapData );

	/*
	��dbfĩβ���һ�м�¼
	*/
	int Append();

	/*
		update����
	*/
	int Update();

	/*
		insert����
	*/
	int Insert();

	/*
		delete����
	*/
	int Delete();

	/*
	��ȡһ��dbf�ļ�����
	Return:
	0 -- �ɹ�
	1 -- ������
	-1 -- ʧ��
	*/
	int Read(const std::vector<TgwDBFOperHelper_DF::ColumnSettingInDBF>& in_vecSet,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& out_mapData);
};

#endif