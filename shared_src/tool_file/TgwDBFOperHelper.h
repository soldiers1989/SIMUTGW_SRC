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
			'C' --	字符
			'N' --  浮点数
			'D' --  整数??
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
	DBF操作类
	提供写dbf文件方法
	暂时未考虑多线程操作
*/
class TgwDBFOperHelper
{
	/*
		member
	*/
private:
	//	DBF句柄
	std::shared_ptr<CDBF> m_pDBF;
	
	//	
	std::string m_strKeyName;
	
	std::string m_strFilePath;

	std::map<std::string, long> m_mapKeyRecno;

	// 当前行号
	long m_lRecNoNow;

	// 总行数
	long m_lTotalNum;
	/*
		function
	*/
public:
	TgwDBFOperHelper(void);
	virtual ~TgwDBFOperHelper(void);
	
	/*
		创建dbf
	*/
	int Create(const std::string &strFileName, const std::vector<TgwDBFOperHelper_DF::ColumnSettingInDBF>& vecSet);

	/*
		打开dbf
	*/
	int Open( const std::string &strFileName );
	
	/*
		关闭dbf
	*/
	int Close();
	
	/*
		在dbf末尾添加一行记录
	*/
	int Append(const std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& mapData );

	/*
	在dbf末尾添加一行记录
	*/
	int Append();

	/*
		update操作
	*/
	int Update();

	/*
		insert操作
	*/
	int Insert();

	/*
		delete操作
	*/
	int Delete();

	/*
	读取一行dbf文件内容
	Return:
	0 -- 成功
	1 -- 无内容
	-1 -- 失败
	*/
	int Read(const std::vector<TgwDBFOperHelper_DF::ColumnSettingInDBF>& in_vecSet,
		std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& out_mapData);
};

#endif