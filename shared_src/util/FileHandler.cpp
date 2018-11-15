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

		// �ж��ļ��Ƿ����
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

		// ȡ�ļ���С
		uint64_t uiSize = boost::filesystem::file_size(in_path);

		// �ж��ļ��Ƿ����
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
	��ȡ�ļ�ͷ��
	ANSI��ASCII���ڵ�1�ֽ��ַ�128������char�͵�����������2�ֽڣ���һ���ֽ���0X80���ϣ���char�͸�����һ�ֽڡ�
	�ļ���ͷû�б�־��ֱ��������
	Unicode��
	USC2-LE��USC-2BE���̶���2�ֽڱ�ʾ�ַ�������Ӣ���ַ�Ҳ��2�ֽڡ���0xFFEF��С�ˣ���0xEFFF����ˣ���ͷΪ��־
	UTF8��utf8�Ǳ䳤�ı��룬Ӣ���ַ�����1�ֽڣ����ֺ����������ַ���2�ֽڻ���3�ֽڡ���Ϊ��BOM�Ͳ���BOM�ģ�BOM(Byte Order Mark)�����ļ���ͷ�ı�־��
	��BOM��UTF-8�ļ��ǣ���ͷ���ֽڣ�EE BB EF
	����BOM��UTF-8����ͷΪ������ֱ�������ݣ���ɺ�ANSI��һ����
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
		//utf8-bom �ļ���ͷ��FF BB BF,����bom�Ժ���
		type = TextUTF8;
	}
	else if (0xFF == szHeadBuf[0] && 0xFE == szHeadBuf[1])
	{
		//С��Unicode  �ļ���ͷ��FF FE  intel x86�ܹ�������С�˴洢����ֱ�Ӷ�ȡ
		type = TextUNICODE;
	}
	else if (0xFE == szHeadBuf[0] && 0xFF == szHeadBuf[1])
	{
		//���Unicode  �ļ���ͷ��FE FF
		type = TextUNICODE_BIG;
	}
	else
	{
		//ansi����unf8 ��bom
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

	// ���ļ�ͷ�ж�ƫ��λ��
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
		//utf8-bom �ļ���ͷ��FF BB BF,�Ժ���
		iRes = std::fseek(fp, 3, SEEK_SET);
		{
			string strDebug("ReadUtf8File with utf8-bom");
			EzLog::t(ftag, strDebug);
		}
	}
	else
	{
		// ����bom
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