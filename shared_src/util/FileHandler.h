#ifndef __FILE_HANDLER_H__
#define __FILE_HANDLER_H__

#include <string>
#include <stdio.h>
#include <cstdio>

typedef enum tagTextCodeType
{

}TextCodeType;


class FileHandler
{
	//
	// Members
	//
public:
	/*
	文本文件的不同编码
	*/
	enum TextCodeType
	{
		TextUnkown = -1,
		TextANSI = 0,
		TextUTF8,
		TextUNICODE,
		TextUNICODE_BIG
	};

protected:
	FILE* m_pfile;

	//
	// Functions
	//
public:
	FileHandler();
	virtual ~FileHandler();

	int ReadFileContent( const std::string& in_path, std::string& out_filecontent, TextCodeType* pKnownType );

protected:
	void CleanUp( void );

	int OpenFile( const std::string& in_path );

	int CloseFile( FILE** pfp );

	int ReadFile( std::string& out_filecontent, TextCodeType* pKnownType );

	int GetFileEncodeType( FILE* fp, TextCodeType& out_fileEncodeType );

	int ReadAnsiFile( FILE* fp, std::string& out_fileContent );

	int ReadUtf8File( FILE* fp, std::string& out_fileContent );
};

#endif