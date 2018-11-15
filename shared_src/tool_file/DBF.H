// DBF.h: interface for the CDBF class.
// writer: zhang kai jun
// last modify date:1999.05.26

// Modifier           : XeKernel@gmail.com
// Modified Type      : Add
// Last Modified Date : 2013.04.15
//////////////////////////////////////////////////////////////////////
#ifndef _DBF_H_
#define _DBF_H_

#define MAX_FLD_NUM		  128
#define MAX_RECORD_SIZE   2048
#define MAX_FLD_DATALEN	  255
#define FILE_EOF_FLAG	  0x1A
#define RECORD_BEGIN_FLAG 0x20
#define RECORD_DEL_FLAG	  0x2A
#define STRUCT_EOF_FLAG   0x0D
#define BASE_LOCK_OFFSET  0x40000000L
#define LOCK_FILE_LEN  0x3FFFFFFFL
#define FILE_HEAD_LEN 32

#define DBF_SUCCESS  0
#define DBF_FAIL     -1

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
//#include "platform.h"

#ifdef _WIN32 
#include <WTypes.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/locking.h>
#include <share.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <atltime.h>
#endif

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

struct FIELD_INFO
{
	char name[20];
	char type;
	int  offset;
	unsigned char width;
	unsigned char dec_d;
};

struct RECORD_INFO
{
	unsigned long  rec_num;
	unsigned short  rec_size;
	unsigned short fld_num;
	char   rec_data[MAX_RECORD_SIZE];
};

struct DBF_HEAD
{
    char fileFlag;
    char lastDate[3];
    unsigned long recNum;
    unsigned short structSize;
    unsigned short recSize;
    char other[20];
};

struct DBF_FIELD_INFO
{
    char fieldName[10];
    char standby;
    char type;
    unsigned long beginOffset;
    char width;
    char decWidth;
    char other[14];
};

class CDBF  
{
protected:
	FIELD_INFO fld_info[MAX_FLD_NUM];
	RECORD_INFO recInfo;

	unsigned long m_nRecBeginOffset;
	unsigned short m_nStructSize;

	unsigned char f_date[4];

	int _dbfhandle;
	bool  _open;

    unsigned long m_nCurrOffset;
	unsigned long m_nCurrRecNo;
	char errMessage[100];

    int m_CurrLockType;

    char m_FileName[256];
    int m_OpenType;
    int m_LockType;

public:

	enum OpenType
    {
		_ReadOnly	= O_RDONLY,
		_ReadWrite	= O_RDWR
	};

    enum LockType
    {
        _AutoLock = 0,
        _NoLock   = 1
    };

public:
	int LockRecord();
	int UnlockRecord();
	int LockHead();
	int UnlockHead();
    int LockFile();
    int UnlockFile();
	int ModifyFileDate();
    int Space(char *p);
    void Ltrim(char *s);
    void Rtrim(char *s);
    void Alltrim(char *s);
    int  ReOpen();
	int FindFld(const char *fldName);
	int GoTo(long recNo);    
    char m_HeadLockType;

    //create dbf use
    DBF_FIELD_INFO *m_pField[255];
    int m_FieldNum;
    void ClearFieldInfo();

public:
	CDBF();
	virtual ~CDBF();
	int Open(const char *filename, int opentype, int lockType);
    char *GetErrorMsg();
    char *GetFileName();
	void Close();
	bool IsDelete();
    int  Zap();
	bool IsEof();
	bool IsBof();
	int  GetRecordNum();
	int GoToRec(long recNo);
	unsigned long GetCurrRecNum();

	int ReadRecord(long recNo);
	int GetFldValue(const char *fldName,char *fldValue);
	int GetFldValue(const char *fldName,char &fldValue);
	int GetFldValue(const char *fldName,double &fldValue);
	int GetFldValue(const char *fldName,int &fldValue);
	int GetFldValue(const char *fldName,long &fldValue);
	int GetFldValue(size_t nFldIndex, char * chrFldValue); // XeKernel@gmail.com add (2013/04/15) be careful of chrFldValue's length
	int GetFldName(size_t nFldIndex, const char *& chrRetName) const; // XeKernel@gmail.com add (2013/04/15)

	int UpdateFldValue(const char *, const char *);
	int UpdateFldValue(const char *, int);
	int UpdateFldValue(const char *, unsigned int);
	int UpdateFldValue(const char *, long);
	int UpdateFldValue(const char *, unsigned long);
	int UpdateFldValue(const char *, double);
    int ReplaceRecord(long recNo);

	long AddNewRecord();
	int FillFldValue(const char *, const char *, int); // v1.02 add
	int FillFldValue(const char *, const char *);
	int FillFldValue(const char *, int);
	int FillFldValue(const char *, char);
	int FillFldValue(const char *, unsigned int);
	int FillFldValue(const char *, long);
	int FillFldValue(const char *, unsigned long);
	int FillFldValue(const char *, double);
	int FillFldValue(size_t nFldIndex, const char * chrFldValue); // XeKernel@gmail.com add (2013/04/12)
	int FillFldValue(size_t nFldIndex, int iValue); // XeKernel@gmail.com add (2013/04/12)
    int UpdateRecord();

    int AddField(const char *fieldName, char type, unsigned short width, unsigned short decWidth);
    int Create(const char *fileName);

	int MoveLast();
	int MoveFirst();
	int FindRecord(const char *fldName, const char *fldValue);
	//2005.05.30. huangsn add
	long CountRecord(const char *fldName, const char *fldValue);
	long CountRecord_M(const char *fldName, const char *fldValue, const char *fldName1, const char *fldValue1);
	int FindRecord_m(const char *fldName, const char *fldValue, const char *fldName1, const char *fldValue1);
	int FindRecord_m3(const char *fldName, const char *fldValue, const char *fldName1, const char *fldValue1,const char *fldName2, const char *fldValue2);
    int DeleteRecord(long recNo);
    int UnDeleteRecord(long recNo);

	int DeleteRecord();
	int UnDeleteRecord();

    int GetColNum();
    const FIELD_INFO * GetColInfo(int colNo);
};

#endif // !defined(AFX_DBF_H__6715E1B4_DB79_11D2_9193_00C0DFA002E9__INCLUDED_)
