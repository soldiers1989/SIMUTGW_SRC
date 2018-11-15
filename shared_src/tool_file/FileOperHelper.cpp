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
根据相对路径，创建当前目录的父目录下的目录
@param const std::string& in_strChildDirPath : 在当前目录的父目录下创建的目录名
@param std::string& out_strAbsolutePath : 返回的绝对路径

Return:
0--成功
非0--失败
*/
int FileOperHelper::MakeDir_InParentPath(const std::string& in_strDirPath, std::string& out_strAbsolutePath)
{
	// static const std::string ftag("FileOperHelper::MakeDir_InParentPath()");

	//取得当前程序所在目录
	boost::filesystem::path old_cpath = boost::filesystem::current_path();

	// 取得父目录
	old_cpath = old_cpath.parent_path();

	// 取得绝对路径
	boost::filesystem::path file_path = old_cpath / in_strDirPath;

	out_strAbsolutePath = file_path.string();

	if (boost::filesystem::exists(file_path))
	{
		//	如果目录存在
		return 0;
	}
	else
	{
		//目录不存在, 创建
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
根据绝对路径，创建目录
Param:
statement
Return:
0--成功
非0--失败
*/
int FileOperHelper::MakeDir(const std::string& in_strAbsolutePath)
{
	// static const std::string ftag("FileOperHelper::MakeDir()");

	if (boost::filesystem::exists(in_strAbsolutePath))
	{
		//	如果目录存在
		return 0;
	}
	else
	{
		//目录不存在, 创建
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
在当前目录的父目录下，创建目录，自动添加日期目录
@param const std::string& in_strDirPath : 在当前目录的父目录下创建的目录名
@param std::string& out_strDay : 当前日期字符串
@param std::string& out_strAbsolutePath : 返回的绝对路径

Return:
0--成功
非0--失败

创建 ../statement/20170608 目录
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
0--成功
非0--失败
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
列出文件夹下所有文件，不嵌套下钻

@param const std::string& in_strFolder : 文件夹
@param std::vector<std::string>& out_vectFiles : 文件列表

Return:
0--成功
非0--失败
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