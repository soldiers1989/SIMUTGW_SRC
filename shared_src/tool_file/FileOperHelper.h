#ifndef __FILE_OPER_HELPER_H__
#define __FILE_OPER_HELPER_H__

#include <string>
#include <vector>

class FileOperHelper
{
public:
	virtual ~FileOperHelper(void);

	/*
	根据相对路径，创建当前目录的父目录下的目录
	@param const std::string& in_strChildDirPath : 在当前目录的父目录下创建的目录名
	@param std::string& out_strAbsolutePath : 返回的绝对路径

	Return:
	0--成功
	非0--失败
	*/
	static int MakeDir_InParentPath(const std::string& in_strDirPath, std::string& out_strAbsolutePath);

	/*
		根据绝对路径，创建目录
		Param:
		statement
		Return:
		0--成功
		非0--失败
		*/
	static int MakeDir(const std::string& in_strAbsolutePath);

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
	static int MakeDirDate_InParentPath(const std::string& in_strDirPath, std::string& out_strDay, std::string& out_strAbsolutePath);

	/*
	Retrieves the fully qualified path for the file that contains the specified module.

	Return:
	0--成功
	非0--失败
	*/
	static int GetProgramDir(std::string& out_ProgramPath);

	/*
	列出文件夹下所有文件，不嵌套下钻

	@param const std::string& in_strFolder : 文件夹
	@param std::vector<std::string>& out_vectFiles : 文件列表

	Return:
	0--成功
	非0--失败
	*/
	static int ListFilesInPath(const std::string& in_strFolder, std::vector<std::string>& out_vectFiles);

private:
	// 禁止默认构造函数
	FileOperHelper(void);
};

#endif