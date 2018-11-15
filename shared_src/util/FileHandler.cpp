#include "FileHandler.h"

#include <iostream>
#include <fstream>

#ifdef _MSC_VER
#include <codecvt>
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <locale>
#include <stdio.h>
#include <string>
#include <stdint.h>


#include <boost/filesystem.hpp>

#include "util/EzLog.h"
#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/StringEncodeUtil.h"


using namespace std;

FileHandler::FileHandler()
{
	m_pfile = nullptr;
}


FileHandler::~FileHandler()
{
	CleanUp();
}

void FileHandler::CleanUp(void)
{
	CloseFile(&m_pfile);
}

int FileHandler::OpenFile(const string& in_path)
{
	static const string ftag("FileHandler::OpenFile() ");

	try
	{
		const int ciPathLen = 260;
		char szCurrDir[ciPathLen] = { 0 };
		char* pRes = NULL;

#ifdef _MSC_VER		
		//::GetCurrentDirectory(MAX_PATH, szCurrDir);

		//SetCurrentDirectory(szCurrDir);

		pRes = _getcwd(szCurrDir, ciPathLen);
		if (NULL == pRes)
		{
			string strTran;
			string strDebug("getcwd errcode=");
			DWORD dErr = GetLastError();
			strDebug += sof_string::itostr((uint64_t)dErr, strTran);
			EzLog::e(ftag, strDebug);

			return -1;
		}
#else
		pRes = getcwd(szCurrDir, ciPathLen);
		if (NULL == pRes)
		{
			int err = errno;
			if (ERANGE == err)
			{
				string strDebug("getcwd errno ERANGE");
				EzLog::e(ftag, strDebug);
			}
			else
			{
				string strTran;
				string strDebug("getcwd errno ");
				strDebug += sof_string::itostr(err, strTran);
				EzLog::e(ftag, strDebug);
			}

			return -1;
		}
#endif

		// 判断文件是否存在
		bool bRes = boost::filesystem::exists(in_path);
		if (!bRes)
		{
			string strDebug("file not exists:[");
			strDebug += in_path;
			strDebug += "], current directory=[";
			strDebug += szCurrDir;
			strDebug += "]";
			EzLog::e(ftag, strDebug);

			return -1;
		}

		bRes = boost::filesystem::is_directory(in_path);
		if (bRes)
		{
			string strDebug("path is a directory:[");
			strDebug += in_path;
			strDebug += "]";
			EzLog::e(ftag, strDebug);

			return -1;
		}

		// 取文件大小
		uint64_t uiSize = boost::filesystem::file_size(in_path);

		// 判断文件是否过大
		if (1024 * 1024 < uiSize)
		{
			string strDebug("file not exists:[");
			strDebug += in_path;
			strDebug += "]";
			EzLog::e(ftag, strDebug);

			return -1;
		}

#ifdef _MSC_VER		
		errno_t err = fopen_s(&m_pfile, in_path.c_str(), "r");
		if (0 != err) {

			string strTran;
			string strDebug("File opening failed:[");
			strDebug += in_path;
			strDebug += "] errcode=";
			strDebug += sof_string::itostr(err, strTran);
			EzLog::e(ftag, strDebug);

			return -1;
		}
#else
		m_pfile = fopen(in_path.c_str(), "r");
		if (nullptr == m_pfile)
		{
			int err = errno;

			string strTran;
			string strDebug("File opening failed, errno=");
			strDebug += sof_string::itostr(err, strTran);
			EzLog::e(ftag, strDebug);

			return -1;
		}
#endif

		return 0;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}
	catch (...)
	{
		string strTran;
		string strDebug("unkown exception,errcode");

#ifdef _MSC_VER		
		DWORD dErr = GetLastError();
		strDebug += sof_string::itostr((uint64_t)dErr, strTran);
#else
		int err = errno;
		strDebug += sof_string::itostr(err, strTran);
#endif		
		EzLog::e(ftag, strDebug);
		return -1;
	}

	return 0;
}

int FileHandler::CloseFile(FILE** pfp)
{
	static const string ftag("FileHandler::CloseFile() ");

	if (nullptr == pfp || nullptr == *pfp)
	{
		return 0;
	}

	try
	{
		std::fclose(*pfp);
		*pfp = nullptr;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}
	catch (...)
	{
		string strTran;
		string strDebug("unkown exception,errcode");

#ifdef _MSC_VER		
		DWORD dErr = GetLastError();
		strDebug += sof_string::itostr((uint64_t)dErr, strTran);
#else
		int err = errno;
		strDebug += sof_string::itostr(err, strTran);
#endif		
		EzLog::e(ftag, strDebug);
		return -1;
	}

	return 0;
}

int FileHandler::ReadFileContent(const string& in_path, string& out_filecontent, TextCodeType* pKnownType)
{
	// static const string ftag("FileHandler::ReadFileContent() ");

	if (in_path.empty())
	{
		return -1;
	}

	int iRes = OpenFile(in_path);
	if (0 != iRes)
	{
		return -1;
	}

	string strContent;
	iRes = ReadFile(strContent, pKnownType);
	if (0 != iRes)
	{
		return -1;
	}

	out_filecontent = strContent;

	return 0;
}

int FileHandler::ReadFile(string& out_filecontent, TextCodeType* pKnownType)
{
	static const string ftag("FileHandler::ReadFile() ");

	if (nullptr == m_pfile)
	{
		return -1;
	}

	try
	{
		int iRes = 0;
		TextCodeType emFileType = TextCodeType::TextUnkown;

		if (nullptr == pKnownType)
		{
			iRes = GetFileEncodeType(m_pfile, emFileType);
			if (0 != iRes)
			{
				EzLog::e(ftag, "GetFileEncodeType error");
				return -1;
			}
		}
		else
		{
			emFileType = *pKnownType;
		}

		string strFileContent;

		switch (emFileType)
		{
		case FileHandler::TextANSI:
			iRes = ReadAnsiFile(m_pfile, strFileContent);

			break;

		case FileHandler::TextUTF8:
			iRes = ReadUtf8File(m_pfile, strFileContent);

			break;

		case FileHandler::TextUNICODE:
		case FileHandler::TextUNICODE_BIG:

		default:
			string strTran;
			string strDebug("unsupported text type=");
			strDebug += sof_string::itostr((int)emFileType, strTran);
			EzLog::e(ftag, strDebug);

			return -1;

			break;
		}

		if (0 == iRes)
		{
			out_filecontent = strFileContent;
			return 0;
		}
		else
		{
			return -1;
		}

	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}
	catch (...)
	{
		string strTran;
		string strDebug("unkown exception,errcode");

#ifdef _MSC_VER		
		DWORD dErr = GetLastError();
		strDebug += sof_string::itostr((uint64_t)dErr, strTran);
#else
		int err = errno;
		strDebug += sof_string::itostr(err, strTran);
#endif		
		EzLog::e(ftag, strDebug);
		return -1;
	}

	return 0;
}

int FileHandler::GetFileEncodeType(FILE* fp, TextCodeType& out_fileEncodeType)
{
	static const string ftag("FileHandler::GetFileEncodeType() ");

	if (nullptr == fp)
	{
		return -1;
	}

	/*
	读取文件头，
	ANSI：ASCII在内的1字节字符128个，即char型的正数，汉字2字节，第一个字节是0X80以上，即char型负数第一字节。
	文件开头没有标志，直接是内容
	Unicode：
	USC2-LE和USC-2BE：固定的2字节表示字符，包括英文字符也是2字节。以0xFFEF（小端）和0xEFFF（大端）开头为标志
	UTF8：utf8是变长的编码，英文字符还有1字节，汉字和其他各国字符用2字节或者3字节。分为带BOM和不带BOM的，BOM(Byte Order Mark)就是文件开头的标志。
	带BOM的UTF-8文件是：开头三字节，EE BB EF
	不带BOM的UTF-8，开头为特征，直接是内容，造成和ANSI的一样。
	*/

	unsigned char szHeadBuf[3] = { 0 };
	TextCodeType type = TextCodeType::TextUnkown;

	int iRes = std::fseek(fp, 0, SEEK_SET);
#ifdef _MSC_VER		
	if (0 != iRes)
	{
		string strTran;
		string strDebug("fseek failed errcode=");
		DWORD dErr = GetLastError();
		strDebug += sof_string::itostr((uint64_t)dErr, strTran);
		EzLog::e(ftag, strDebug);
		return -1;
	}
#else
	if (0 != iRes)
	{	
		int err = errno;

		string strTran;
		string strDebug("fseek failed errcode=");
		strDebug += sof_string::itostr(err, strTran);
		EzLog::e(ftag, strDebug);

		return -1;
	}
#endif

	std::fread(szHeadBuf, 1, 3, m_pfile);

	if (0xEF == szHeadBuf[0] && 0xBB == szHeadBuf[1] && 0xBF == szHeadBuf[2])
	{
		//utf8-bom 文件开头：FF BB BF,不带bom稍后解决
		type = TextUTF8;
	}
	else if (0xFF == szHeadBuf[0] && 0xFE == szHeadBuf[1])
	{
		//小端Unicode  文件开头：FF FE  intel x86架构自身是小端存储，可直接读取
		type = TextUNICODE;
	}
	else if (0xFE == szHeadBuf[0] && 0xFF == szHeadBuf[1])
	{
		//大端Unicode  文件开头：FE FF
		type = TextUNICODE_BIG;
	}
	else
	{
		//ansi或者unf8 无bom
		type = TextANSI;
	}

	out_fileEncodeType = type;

	return 0;
}

int FileHandler::ReadAnsiFile(FILE* fp, string& out_fileContent)
{
	static const string ftag("FileHandler::ReadAnsiFile() ");

	//
	{
		string strDebug("ReadAnsiFile");
		EzLog::t(ftag, strDebug);
	}

	string strContent;

	int iRes = std::fseek(fp, 0, SEEK_SET);
#ifdef _MSC_VER		
	if (0 != iRes)
	{
		string strTran;
		string strDebug("fseek failed errcode=");
		DWORD dErr = GetLastError();
		strDebug += sof_string::itostr((uint64_t)dErr, strTran);
		EzLog::e(ftag, strDebug);
		return -1;
	}
#else
	if (0 != iRes)
	{
		int err = errno;

		string strTran;
		string strDebug("fseek failed errcode=");
		strDebug += sof_string::itostr(err, strTran);
		EzLog::e(ftag, strDebug);

		return -1;
	}
#endif

	int c; // note: int, not char, required to handle EOF
	while (EOF != (c = std::fgetc(fp)))
	{
		// standard C I/O file reading loop
		strContent += static_cast<char>(c);
	}

	out_fileContent = strContent;

	if (std::ferror(fp))
	{
#ifdef _MSC_VER		
		string strTran;
		string strDebug("I/O error when reading errcode=");
		DWORD dErr = GetLastError();
		strDebug += sof_string::itostr((uint64_t)dErr, strTran);
		EzLog::e(ftag, strDebug);
#else
		int err = errno;

		string strTran;
		string strDebug("I/O error when reading errcode=");
		strDebug += sof_string::itostr(err, strTran);
		EzLog::e(ftag, strDebug);
#endif
		return -1;
	}
	else if (std::feof(fp))
	{
		//string strDebug("End of file reached successfully");
		//EzLog::t(ftag, strDebug);

		return 0;
	}

	return 0;
}

int FileHandler::ReadUtf8File(FILE* fp, string& out_fileContent)
{
	static const string ftag("FileHandler::ReadUtf8File() ");

	//

	string strContent;

	// 读文件头判断偏移位置
	unsigned char szHeadBuf[3] = { 0 };

	int iRes = std::fseek(fp, 0, SEEK_SET);
#ifdef _MSC_VER		
	if (0 != iRes)
	{
		string strTran;
		string strDebug("fseek failed errcode=");
		DWORD dErr = GetLastError();
		strDebug += sof_string::itostr((uint64_t)dErr, strTran);
		EzLog::e(ftag, strDebug);
		return -1;
	}
#else
	if (0 != iRes)
	{
		int err = errno;

		string strTran;
		string strDebug("fseek failed errcode=");
		strDebug += sof_string::itostr(err, strTran);
		EzLog::e(ftag, strDebug);

		return -1;
	}
#endif

	std::fread(szHeadBuf, 1, 3, fp);

	if (0xEF == szHeadBuf[0] && 0xBB == szHeadBuf[1] && 0xBF == szHeadBuf[2])
	{
		//utf8-bom 文件开头：FF BB BF,稍后解决
		iRes = std::fseek(fp, 3, SEEK_SET);
		{
			string strDebug("ReadUtf8File with utf8-bom");
			EzLog::t(ftag, strDebug);
		}
	}
	else
	{
		// 不带bom
		iRes = std::fseek(fp, 0, SEEK_SET);
		{
			string strDebug("ReadUtf8File with no bom");
			EzLog::t(ftag, strDebug);
		}
	}

#ifdef _MSC_VER		
	if (0 != iRes)
	{
		string strTran;
		string strDebug("fseek failed errcode=");
		DWORD dErr = GetLastError();
		strDebug += sof_string::itostr((uint64_t)dErr, strTran);
		EzLog::e(ftag, strDebug);
		return -1;
		}
#else
	if (0 != iRes)
	{
		int err = errno;

		string strTran;
		string strDebug("fseek failed errcode=");
		strDebug += sof_string::itostr(err, strTran);
		EzLog::e(ftag, strDebug);

		return -1;
	}
#endif

	int c; // note: int, not char, required to handle EOF
	while (EOF != (c = std::fgetc(fp)))
	{
		// standard C I/O file reading loop
		strContent += static_cast<char>(c);
	}

	// UTF-8 to GBK
	string strGbkContent;
	StringEncodeUtil::UtfToGbk(strContent, strGbkContent);

	out_fileContent = strGbkContent;

	if (std::ferror(fp))
	{
#ifdef _MSC_VER		
		string strTran;
		string strDebug("I/O error when reading errcode=");
		DWORD dErr = GetLastError();
		strDebug += sof_string::itostr((uint64_t)dErr, strTran);
		EzLog::e(ftag, strDebug);
#else
		int err = errno;

		string strTran;
		string strDebug("I/O error when reading errcode=");
		strDebug += sof_string::itostr(err, strTran);
		EzLog::e(ftag, strDebug);
#endif
		return -1;
	}
	else if (std::feof(fp))
	{
		//string strDebug("End of file reached successfully");
		//EzLog::t(ftag, strDebug);

		return 0;
	}

	return 0;
	}