#include "ETF_Conf_Reader.h"

#include "boost/filesystem.hpp"

#include "ETFHelper.h"
#include "conf_etf_info.h"

#include "simutgw/stgw_config/g_values_biz.h"

#include "util/EzLog.h"

ETFHelper::ETFHelper()
{
}


ETFHelper::~ETFHelper()
{
}

/*
����ָ��Ŀ¼��etf�ļ�
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ETFHelper::LoadETF(const std::string& in_strDir)
{
	static const std::string strTag("ETFHelper::LoadETF() ");

	if (in_strDir.empty() || 0 == in_strDir.length())
	{
		EzLog::e(strTag, "Dir is empty");
		return -1;
	}

	//std::tr2::sys::path current;
	//std::tr2::sys::current_path(current);
	//EzLog::i("Current path", current.string());

	boost::filesystem::path pDir(in_strDir);
	if (!boost::filesystem::is_directory(pDir))
	{
		EzLog::e(strTag, in_strDir + " is not a Dir");
		return -1;
	}

	EzLog::i(strTag, "Start loading ETF info");

	std::string strSzPath = pDir.string() + "/sz";
	LoadSZPcf(strSzPath);

	std::string strShPath = pDir.string() + "/sh";
	LoadSHETF(strShPath);

	return 0;
}

/*
����ָ��Ŀ¼��pcf�ļ�
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ETFHelper::LoadSZPcf(const std::string& in_strDir)
{
	static const std::string strTag("ETFHelper::LoadSZPcf() ");

	if (in_strDir.empty() || 0 == in_strDir.length())
	{
		EzLog::e(strTag, "Dir is empty");
		return -1;
	}

	if (!boost::filesystem::is_directory(boost::filesystem::path(in_strDir)))
	{
		EzLog::e(strTag, in_strDir + " is not a Dir");
		return -1;
	}

	std::vector<std::string> vecFileName;
	int iRes = GetPCFFileNameInDir(in_strDir, vecFileName);
	if (0 != iRes)
	{
		return iRes;
	}

	for (std::string pcfFile : vecFileName)
	{
		ETF_Conf_Reader::Read_SZ_ETF_Pcf_Conf(pcfFile);
	}

	return 0;
}

/*
����ָ��Ŀ¼���Ϻ�etf�����ļ�
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ETFHelper::LoadSHETF(const std::string& in_strDir)
{
	static const std::string strTag("ETFHelper::LoadSHETF() ");

	if (in_strDir.empty() || 0 == in_strDir.length())
	{
		EzLog::e(strTag, "Dir is empty");
		return -1;
	}

	if (!boost::filesystem::is_directory(boost::filesystem::path(in_strDir)))
	{
		EzLog::e(strTag, in_strDir + " is not a Dir");
		return -1;
	}

	std::vector<std::string> vecFileName;
	int iRes = Get_SHETF_FileNameInDir(in_strDir, vecFileName);
	if (0 != iRes)
	{
		return iRes;
	}

	for (std::string SHetfFile : vecFileName)
	{
		ETF_Conf_Reader::Read_SH_ETF_Conf(SHetfFile);
	}

	return 0;
}

/*
��ѯĳ֧etf����Ϣ
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ETFHelper::Query(const std::string& in_strSecurityID,
	std::shared_ptr<struct simutgw::SzETF>& ptrEtf)
{
	// static const std::string ftag("ETFHelper::GetPCFFileNameInDir() ");

	simutgw::etfReadLock Locker(simutgw::g_mtxMapSZEtf);

	if (simutgw::g_mapSZETF.end() == simutgw::g_mapSZETF.find(in_strSecurityID))
	{
		return -1;
	}

	ptrEtf = simutgw::g_mapSZETF[in_strSecurityID];

	return 0;
}

/*
��ӡ��ǰ�洢��ETF��Ϣ��֤ȯ����
*/
int ETFHelper::Overview()
{
	static const std::string strTag("ETFHelper::Overview() ");

	simutgw::etfReadLock Locker(simutgw::g_mtxMapSZEtf);

	size_t stNum = simutgw::g_mapSZETF.size();
	std::string strOutput("��ǰ����"), strTrans;
	sof_string::itostr(stNum, strTrans);
	strOutput += strTrans;
	strOutput += "֧ETF";
	if (0 == stNum)
	{
		EzLog::i(strTag, strOutput);
		return 0;
	}

	strOutput += "��֤ȯ����ֱ�Ϊ[";
	std::map<string, std::shared_ptr<simutgw::SzETF>>::const_iterator cit =
		simutgw::g_mapSZETF.begin();

	for (size_t i = 1; i <= stNum; ++i, ++cit)
	{
		strOutput += cit->first;
		if (i == stNum)
		{
			strOutput += "].";
		}
		else
		{
			strOutput += "],[";
		}
	}

	EzLog::i(strTag, strOutput);

	return 0;
}

/*
��ȡָ��Ŀ¼���µ�pcf�ļ�����
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ETFHelper::GetPCFFileNameInDir(const std::string& in_strDir, std::vector<std::string>& out_vecFileName)
{
	// static const std::string ftag("ETFHelper::GetPCFFileNameInDir() ");

	std::string strFileNameWithPath;
	std::string strFileName;
	boost::filesystem::path path(in_strDir);
	if (!boost::filesystem::exists(path))
	{
		return -1;
	}

	boost::filesystem::directory_iterator end_iter;
	for (boost::filesystem::directory_iterator iter(path); iter != end_iter; ++iter)
	{
		if (boost::filesystem::is_regular_file(iter->status()))
		{
			strFileNameWithPath = iter->path().string();

			strFileName = iter->path().filename().string();
			if (Check_SZETF_FileName(strFileName))
			{
				out_vecFileName.push_back(strFileNameWithPath);
			}
		}
	}

	return 0;
}

/*
��ȡָ��Ŀ¼���µ��Ϻ�ETF�ļ�����
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ETFHelper::Get_SHETF_FileNameInDir(const std::string& in_strDir, std::vector<std::string>& out_vecFileName)
{
	// static const std::string ftag("ETFHelper::Get_SHETF_FileNameInDir() ");
	
	std::string strFileNameWithPath;
	std::string strFileName;
	boost::filesystem::path path(in_strDir);
	if (!boost::filesystem::exists(path))
	{
		return -1;
	}

	boost::filesystem::directory_iterator end_iter;
	for (boost::filesystem::directory_iterator iter(path); iter != end_iter; ++iter)
	{
		if (boost::filesystem::is_regular_file(iter->status()))
		{
			strFileNameWithPath = iter->path().string();

			strFileName = iter->path().filename().string();
			if (Check_SHETF_FileName(strFileName))
			{
				out_vecFileName.push_back(strFileNameWithPath);
			}
		}
	}

	/*
	std::vector<std::string> vectFiles;
	FileOperHelper::ListFilesInPath(strSettlePath, vectFiles);
	*/
	/*
	for (boost::filesystem::directory_iterator end, dir(in_strDir); end != dir; ++dir)
	{
	const boost::filesystem::path &path = dir->path();
	if (path.extension() == ".ETF")
	{
	//strFileName = path.string();
	const boost::filesystem::path::filename bfile = path.filename;


	if ((strFileName))
	{
	out_vecFileName.push_back(strFileName);
	}
	}
	}
	*/

	return 0;
}

/*
���ShenZhen pcf�ļ����ƵĺϷ���

�ļ���������Ϊ�� pcf_<ETF ����>_YYYYMMDD.xml ������ YYYYMMDD Ϊ T�����ڡ� ʾ�����£�
pcf_159901_20150115.xml
*/
bool ETFHelper::Check_SZETF_FileName(const std::string& in_strFileName)
{
	if (in_strFileName.empty() || 23 > in_strFileName.length())
	{
		return false;
	}

	string strFileName = in_strFileName;

	if (23 != strFileName.length())
	{
		return false;
	}

	if ("pcf" != strFileName.substr(0, 3))
	{
		return false;
	}

	if (!('_' == strFileName[3] && '_' == strFileName[10] && '.' == strFileName[19]))
	{
		return false;
	}

	if ("xml" != strFileName.substr(20, 3))
	{
		return false;
	}

	return true;
}

/*
����Ϻ�ETF�ļ����ƵĺϷ���


*/
bool ETFHelper::Check_SHETF_FileName(const std::string& in_strFileName)
{
	return true;
}
