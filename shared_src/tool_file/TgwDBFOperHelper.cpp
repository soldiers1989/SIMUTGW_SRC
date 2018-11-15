#include "tool_file/TgwDBFOperHelper.h"

#include "boost/filesystem.hpp"

#include "util/EzLog.h"
#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"

TgwDBFOperHelper::TgwDBFOperHelper(void) :m_lRecNoNow(-1), m_lTotalNum(0)
{
	m_pDBF = std::shared_ptr<CDBF>(new CDBF());
}

TgwDBFOperHelper::~TgwDBFOperHelper(void)
{
	Close();
}

/*
	创建dbf
	*/
int TgwDBFOperHelper::Create(const std::string &strFileName, const std::vector<TgwDBFOperHelper_DF::ColumnSettingInDBF>& vecSet)
{
	static const std::string strTag("Create()");

	if (strFileName.empty())
	{
		std::string strError("创建dbf文件失败，文件名为空");

		EzLog::e(strTag, strError);
		return -1;
	}

	try
	{
		int iRes = -1;

		for (size_t st = 0; st < vecSet.size(); ++st)
		{
			//	添加字段

			iRes = m_pDBF->AddField(vecSet[st].strName.c_str(), vecSet[st].cType,
				vecSet[st].uWidth, vecSet[st].uDecWidth);
			if (iRes == DBF_FAIL)
			{
				std::string strError("添加字段name[");
				strError += vecSet[st].strName;
				strError += "]失败";

				EzLog::e(strTag, strError);
				return -1;
			}
		}

		//	创建dbf文件
		iRes = m_pDBF->Create((char *)strFileName.c_str());
		if (iRes != DBF_SUCCESS)
		{
			std::string strError("创建dbf文件[");
			strError += strFileName;
			strError += "]失败";

			EzLog::e(strTag, strError);
			return -1;
		}

		m_strFilePath = strFileName;
	}
	catch (std::exception &e)
	{
		EzLog::ex(strTag, e);

		return -1;
	}

	return 0;
}

/*
	打开dbf
	*/
int TgwDBFOperHelper::Open(const std::string &strFileName)
{
	static const std::string strTag("Open()");

	if (strFileName.empty())
	{
		std::string strError("打开dbf文件失败，文件名为空");

		EzLog::e(strTag, strError);
		return -1;
	}
	else if (!boost::filesystem::exists(strFileName))
	{
		std::string strError("打开dbf文件失败，文件[");
		strError += strFileName;
		strError += "]不存在";

		EzLog::e(strTag, strError);
		return -1;
	}
	else
	{
		try
		{
			int iRes = m_pDBF->Open(strFileName.c_str(), CDBF::_ReadWrite, CDBF::_AutoLock);

			if (iRes != DBF_SUCCESS)
			{
				std::string strError("打开dbf文件[");
				strError += strFileName;
				strError += "]失败";

				EzLog::e(strTag, strError);
				return -1;
			}

			//	open成功，记录下dbf文件名
			m_strFilePath = strFileName;
		}
		catch (std::exception &e)
		{
			EzLog::ex(strTag, e);

			return -1;
		}
	}

	return 0;
}

/*
	关闭dbf
	*/
int TgwDBFOperHelper::Close()
{
	// static const std::string strTag("Close()");

	m_pDBF->Close();

	return 0;
}

/*
	在dbf末尾添加一行记录
	*/
int TgwDBFOperHelper::Append(const std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& mapData)
{
	static const std::string strTag("Append()");

	if (NULL == m_pDBF)
	{
		std::string strError("dbf文件句柄为空");

		EzLog::e(strTag, strError);
		return -1;
	}

	try
	{
		//	要添加的记录行号
		m_lRecNoNow = m_pDBF->AddNewRecord();
		if (m_lRecNoNow == DBF_FAIL)
		{
			std::string strError("添加新记录失败");

			EzLog::e(strTag, strError);
			return -1;
		}

		int iRes = m_pDBF->GoToRec(m_lRecNoNow);
		if (iRes == DBF_FAIL)
		{
			std::string strTran;
			std::string strError("到新记录失败[");
			strError += m_pDBF->GetErrorMsg();
			strError += ",error[";

#ifdef _MSC_VER
			strError += sof_string::itostr((int)GetLastError(), strTran);
#else
			int iErr = errno;
			strError += sof_string::itostr(iErr, strTran);
			strError += ", strerror=";
			strError += strerror(iErr);
#endif			

			EzLog::e(strTag, strError);
			return -1;
		}

		std::map<std::string, TgwDBFOperHelper_DF::DataInRow>::const_iterator itMap = mapData.begin();

		while (itMap != mapData.end())
		{
			if (itMap->second.iType == 0 || itMap->second.iType == 4)
			{
				// value为string
				string strValue = itMap->second.strValue;
				iRes = m_pDBF->FillFldValue(itMap->first.c_str(), itMap->second.strValue.c_str());
			}
			else if (itMap->second.iType == 1)
			{
				// value为long
				iRes = m_pDBF->FillFldValue(itMap->first.c_str(), itMap->second.lValue);
			}
			else if (itMap->second.iType == 2)
			{
				// value为unsigned long
				iRes = m_pDBF->FillFldValue(itMap->first.c_str(), itMap->second.ulValue);
			}
			else if (itMap->second.iType == 3)
			{
				// value为double
				iRes = m_pDBF->FillFldValue(itMap->first.c_str(), itMap->second.dValue);
			}
			else
			{
				std::string strValue;
				std::string strError("未知的iType[");
				strError += sof_string::itostr(itMap->second.iType, strValue);

				EzLog::e(strTag, strError);
				return -1;
			}

			if (iRes == DBF_FAIL)
			{
				std::string strError("Fill新记录字段失败[");
				strError += itMap->first.c_str();
				strError += "],";
				strError += m_pDBF->GetErrorMsg();

				std::string strTran;
				strError += ",error[";
#ifdef _MSC_VER
				strError += sof_string::itostr((int)GetLastError(), strTran);
#else
				int iErr = errno;
				strError += sof_string::itostr(iErr, strTran);
				strError += ", strerror=";
				strError += strerror(iErr);
#endif					

				EzLog::e(strTag, strError);
				return -1;
			}

			++itMap;
		}

		iRes = m_pDBF->UpdateRecord();
		if (iRes == DBF_FAIL)
		{
			std::string strError("更新新记录失败[");
			strError += m_pDBF->GetErrorMsg();

			EzLog::e(strTag, strError);
			return -1;
		}

		m_lTotalNum++;
	}
	catch (std::exception &e)
	{
		EzLog::ex(strTag, e);

		return -1;
	}

	return 0;

}

/*
在dbf末尾添加一行记录
*/
int TgwDBFOperHelper::Append()
{
	static const std::string strTag("Append()");

	if (NULL == m_pDBF)
	{
		std::string strError("dbf文件句柄为空");

		EzLog::e(strTag, strError);
		return -1;
	}

	try
	{
		//	要添加的记录行号
		m_lRecNoNow = m_pDBF->AddNewRecord();
		if (m_lRecNoNow == DBF_FAIL)
		{
			std::string strError("添加新记录失败");

			EzLog::e(strTag, strError);
			return -1;
		}

		int iRes = m_pDBF->GoToRec(m_lRecNoNow);
		if (iRes == DBF_FAIL)
		{
			std::string strError("到新记录失败[");
			strError += m_pDBF->GetErrorMsg();

			EzLog::e(strTag, strError);
			return -1;
		}

		iRes = m_pDBF->FillFldValue("SCDM", "01");
		if (iRes == DBF_FAIL)
		{
			std::string strError("FillFldValue[SCDM]失败[");
			strError += m_pDBF->GetErrorMsg();

			EzLog::e(strTag, strError);
			return -1;
		}

		iRes = m_pDBF->FillFldValue("JLLX", "001");
		if (iRes == DBF_FAIL)
		{
			std::string strError("FillFldValue[JLLX]失败[");
			strError += m_pDBF->GetErrorMsg();

			EzLog::e(strTag, strError);
			return -1;
		}

		iRes = m_pDBF->FillFldValue("JYFS", "001");
		if (iRes == DBF_FAIL)
		{
			std::string strError("FillFldValue[JYFS]失败[");
			strError += m_pDBF->GetErrorMsg();

			EzLog::e(strTag, strError);
			return -1;
		}
		//m_pDBF->FillFldValue("JSFS", "001");
		//m_pDBF->FillFldValue("YWLX", "001");

		iRes = m_pDBF->UpdateRecord();
		if (iRes == DBF_FAIL)
		{
			std::string strError("更新新记录失败[");
			strError += m_pDBF->GetErrorMsg();

			EzLog::e(strTag, strError);
			return -1;
		}
	}
	catch (std::exception &e)
	{
		EzLog::ex(strTag, e);

		return -1;
	}

	return 0;

}

/*
读取一行dbf文件内容
Return:
0 -- 成功
1 -- 无内容
-1 -- 失败
*/
int TgwDBFOperHelper::Read(const std::vector<TgwDBFOperHelper_DF::ColumnSettingInDBF>& in_vecSet,
	std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& out_mapData)
{
	static const std::string strTag("TgwDBFOperHelper::Read()");

	if (NULL == m_pDBF)
	{
		std::string strError("dbf文件句柄为空");

		EzLog::e(strTag, strError);
		return -1;
	}

	try
	{
		if (0 == m_lTotalNum)
		{
			m_lTotalNum = m_pDBF->GetRecordNum();
			m_lRecNoNow = 0;
		}

		++m_lRecNoNow;
		if (m_lRecNoNow > m_lTotalNum)
		{
			return 1;
		}

		//	要添加的记录行号
		int iRes = m_pDBF->ReadRecord(m_lRecNoNow);

		uint64_t ui64Tmp = 0;
		TgwDBFOperHelper_DF::DataInRow rowData;
		for (size_t st = 0; st < in_vecSet.size(); ++st)
		{
			const TgwDBFOperHelper_DF::ColumnSettingInDBF &set = in_vecSet[st];
			//EzLog::i(strTag, set.strName+" "+set.cType);
			switch (set.cType)
			{
			case 'C':
			{
				char *cValue = new char[set.uWidth + 1];
				cValue[0] = 0;
				rowData.iType = 0;
				iRes = m_pDBF->GetFldValue(set.strName.c_str(), cValue);
				if (iRes == DBF_FAIL)
				{
					std::string strError("读取字段值失败[");
					strError += m_pDBF->GetErrorMsg();

					EzLog::e(strTag, strError);
					return -1;
				}
				rowData.strValue.clear();
				//rowData.strValue.append(cValue, set.uWidth);
				rowData.strValue.append(cValue);

				//EzLog::i(strTag, rowData.strValue);

				delete[]cValue;
				break;
			}

			case 'N':
				rowData.iType = 3;
				iRes = m_pDBF->GetFldValue(set.strName.c_str(), rowData.dValue);
				if (iRes == DBF_FAIL)
				{
					std::string strError("读取字段值失败[");
					strError += m_pDBF->GetErrorMsg();

					EzLog::e(strTag, strError);
					return -1;
				}

				//EzLog::i(strTag, sof_string::itostr((int)rowData.dValue, rowData.strValue));
				break;

			default:
				EzLog::e(strTag, "不支持的dbf字段类型[" + set.cType);
				rowData.iType = 0;
				rowData.lValue = 0;
				rowData.dValue = 0;
				rowData.ulValue = 0;
				rowData.strValue.clear();
				break;
			}

			out_mapData.insert(make_pair(set.strName, rowData));
		}
	}
	catch (std::exception &e)
	{
		EzLog::ex(strTag, e);

		return -1;
	}

	return 0;
}