#include "FileOperHelper.h"

#include "boost/date_time/gregorian/gregorian.hpp"

#include "boost/algorithm/string.hpp" 
#include "boost/filesystem.hpp"
#include "boost/filesystem/path.hpp"  
#include "boost/filesystem/operations.hpp"  
#include "boost/format.hpp"

#include <exception>

#include "util/EzLog.h"

FileOperHelper::~FileOperHelper(void)
{
}

/*
�������·����������ǰĿ¼�ĸ�Ŀ¼�µ�Ŀ¼
@param const std::string& in_strChildDirPath : �ڵ�ǰĿ¼�ĸ�Ŀ¼�´�����Ŀ¼��
@param std::string& out_strAbsolutePath : ���صľ���·��

Return:
0--�ɹ�
��0--ʧ��
*/
int FileOperHelper::MakeDir_InParentPath(const std::string& in_strDirPath, std::string& out_strAbsolutePath)
{
	// static const std::string ftag("FileOperHelper::MakeDir_InParentPath()");

	//ȡ�õ�ǰ��������Ŀ¼
	boost::filesystem::path old_cpath = boost::filesystem::current_path();

	// ȡ�ø�Ŀ¼
	old_cpath = old_cpath.parent_path();

	// ȡ�þ���·��
	boost::filesystem::path file_path = old_cpath / in_strDirPath;

	out_strAbsolutePath = file_path.string();

	if (boost::filesystem::exists(file_path))
	{
		//	���Ŀ¼����
		return 0;
	}
	else
	{
		//Ŀ¼������, ����
		bool bRes = boost::filesystem::create_directories(file_path);
		if (bRes)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}

	return 0;
}

/*
���ݾ���·��������Ŀ¼
Param:
statement
Return:
0--�ɹ�
��0--ʧ��
*/
int FileOperHelper::MakeDir(const std::string& in_strAbsolutePath)
{
	// static const std::string ftag("FileOperHelper::MakeDir()");

	if (boost::filesystem::exists(in_strAbsolutePath))
	{
		//	���Ŀ¼����
		return 0;
	}
	else
	{
		//Ŀ¼������, ����
		bool bRes = boost::filesystem::create_directories(in_strAbsolutePath);
		if (bRes)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}

	return 0;
}

/*
�ڵ�ǰĿ¼�ĸ�Ŀ¼�£�����Ŀ¼���Զ��������Ŀ¼
@param const std::string& in_strDirPath : �ڵ�ǰĿ¼�ĸ�Ŀ¼�´�����Ŀ¼��
@param std::string& out_strDay : ��ǰ�����ַ���
@param std::string& out_strAbsolutePath : ���صľ���·��

Return:
0--�ɹ�
��0--ʧ��

���� ../statement/20170608 Ŀ¼
*/
int FileOperHelper::MakeDirDate_InParentPath(const std::string& in_strDirPath, std::string& out_strDay, std::string& out_strAbsolutePath)
{
	// static const std::string ftag("FileOperHelper::MakeDirDate_InParentPath()");

	out_strDay = boost::gregorian::to_iso_string(boost::gregorian::day_clock::local_day());

	std::string strCurrentDirPath = in_strDirPath;
	strCurrentDirPath += "/";
	strCurrentDirPath += out_strDay;

	int iRes = MakeDir_InParentPath(strCurrentDirPath, out_strAbsolutePath);
	if (0 == iRes)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

/*
Retrieves the fully qualified path for the file that contains the specified module.

Return:
0--�ɹ�
��0--ʧ��
*/
int FileOperHelper::GetProgramDir(std::string& out_ProgramPath)
{
	static const std::string ftag("FileOperHelper::GetProgramDir() ");

	try
	{
		/*
		// Full path
		TCHAR exeFullPath[MAX_PATH];
		GetModuleFileName( NULL, exeFullPath, MAX_PATH );
		string strPath = __TEXT( "" );

		// Get full path of the file
		strPath = exeFullPath;
		size_t pos = strPath.find_last_of(L'\\', strPath.length());

		// the directory without the file name
		out_ProgramPath = strPath.substr( 0, pos );
		*/
		out_ProgramPath = boost::filesystem::initial_path<boost::filesystem::path>().string();
		return 0;
	}
	catch (std::exception& e)
	{
		EzLog::e(ftag, e.what());
		EzLog::ex(ftag, e);
		return -1;
	}
}

/*
�г��ļ����������ļ�����Ƕ������

@param const std::string& in_strFolder : �ļ���
@param std::vector<std::string>& out_vectFiles : �ļ��б�

Return:
0--�ɹ�
��0--ʧ��
*/
int FileOperHelper::ListFilesInPath(const std::string& in_strFolder, std::vector<std::string>& out_vectFiles)
{
	// static const std::string ftag("FileOperHelper::GetProgramDir() ");

	boost::filesystem::path path(in_strFolder);
	if (!boost::filesystem::exists(path))
	{
		return -1;
	}

	boost::filesystem::directory_iterator end_iter;
	for (boost::filesystem::directory_iterator iter(path); iter != end_iter; ++iter)
	{
		if (boost::filesystem::is_regular_file(iter->status()))
		{
			string strPath = iter->path().string();
			std::cout << strPath << std::endl;
			out_vectFiles.push_back(strPath);
		}
	}

	return 0;
}