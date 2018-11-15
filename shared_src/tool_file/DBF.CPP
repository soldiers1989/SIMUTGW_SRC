// DBF.cpp: implementation of the CDBF class.
//
//////////////////////////////////////////////////////////////////////

#include "tool_file/DBF.H"

#ifndef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
//#include "platform.h"
#endif


// XinChenK@21cn.com add (2013/04/12)
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif
// end
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDBF::CDBF()
{
	m_nCurrOffset = 0;
	m_nRecBeginOffset = 0;
	m_nStructSize = 0;
	m_nCurrRecNo = 0;
	_open = false;
	recInfo.rec_num = 0;
	recInfo.rec_size = 0;
	recInfo.fld_num = 0;
	_open = false;
	m_CurrLockType = _AutoLock;
	m_FieldNum = 0;
}

CDBF::~CDBF()
{
	ClearFieldInfo();
	if (_open)
		close(_dbfhandle);
}

int CDBF::Space(char *p)
{
	char *s = p;
	while (*s) {
		if (*s == ' ')
			s++;
		else
			return 0;
	}
	return 1;
}

void CDBF::Ltrim(char *s)
{
	int i = 0;
	int len;
	char *tmp;
	len = strlen(s);
	if (Space(s)){
		*s = '\0';
		return;
	}
	tmp = s;
	for (i = 0; *tmp && *tmp == ' '; tmp++, i++);
	memmove(s, tmp, len);
}

void CDBF::Rtrim(char *s)
{
	int len;
	len = strlen(s);
	char *p = s + len - 1;

	if (Space(s)){
		*s = '\0';
		return;
	}
	while (*p == ' ') p--;
	*(++p) = '\0';
}

void CDBF::Alltrim(char *s)
{
	if (s == NULL) return;
	Ltrim(s);
	Rtrim(s);
}


int CDBF::Open(const char *filename, int opentype, int lockType)
{
	unsigned char tmpstr[33];
	int i, offset;
	long tmpoffset;

	if (_open)
	{
		strcpy(errMessage, "opened.");
		return DBF_FAIL;
	}

	m_CurrLockType = lockType;

	m_nCurrOffset = 0;
	m_nRecBeginOffset = 0;
	m_nStructSize = 0;
	m_nCurrRecNo = 0;
	_open = false;
	recInfo.rec_num = 0;
	recInfo.rec_size = 0;
	recInfo.fld_num = 0;

#ifdef WIN32
	if (opentype == O_RDONLY)
		_dbfhandle = _sopen(filename, O_RDONLY | O_BINARY, SH_DENYNO, S_IREAD);
	else
		_dbfhandle = _sopen(filename, O_RDWR | O_BINARY, SH_DENYNO, S_IREAD | S_IWRITE);
#else
	if (opentype == O_RDONLY)
		_dbfhandle = open(filename, O_RDONLY, S_IREAD );
	else
		_dbfhandle = open(filename, O_RDWR, S_IREAD | S_IWRITE );
#endif


	if (_dbfhandle == -1)
	{
		strcpy(errMessage, "open dbf failed.");
		return DBF_FAIL;
	}

	if (LockHead() == DBF_FAIL)
		return DBF_FAIL;
	//取记录数
	memset(tmpstr, 0, sizeof(tmpstr));
	lseek(_dbfhandle, 4L, SEEK_SET);
	read(_dbfhandle, tmpstr, 4L);
	recInfo.rec_num = *(unsigned long *)&tmpstr;

	//取记录长度
	memset(tmpstr, 0, sizeof(tmpstr));
	lseek(_dbfhandle, 10L, SEEK_SET);
	read(_dbfhandle, tmpstr, 2L);
	recInfo.rec_size = *(unsigned short *)&tmpstr;

	//取文件结构长度
	memset(tmpstr, 0, sizeof(tmpstr));
	lseek(_dbfhandle, 8L, SEEK_SET);
	read(_dbfhandle, tmpstr, 2L);
	m_nStructSize = *(unsigned short *)&tmpstr;

	//取字段信息
	i = 0;
	offset = 0;
	tmpoffset = 32;
	while (i < MAX_FLD_NUM)
	{
		lseek(_dbfhandle, tmpoffset, SEEK_SET);
		//取字段名
		read(_dbfhandle, tmpstr, 32);
		recInfo.fld_num++;
		memcpy(fld_info[i].name, tmpstr, 10);
		fld_info[i].name[10] = 0;
		Alltrim(fld_info[i].name);
		//取字段类型
		fld_info[i].type = tmpstr[11];
		//取字段长度
		fld_info[i].width = tmpstr[16];
		//取字段偏移量
		fld_info[i].offset = offset;
		//取小数位数
		fld_info[i].dec_d = tmpstr[17];

		offset += fld_info[i].width;
		tmpoffset += 32;
		if (tmpoffset >= (long)m_nStructSize - 1)
			break;
		i++;
	}
	UnlockHead();

	m_nRecBeginOffset = m_nStructSize;

	m_nCurrOffset = m_nRecBeginOffset;

	_open = true;

	strcpy(m_FileName, filename);
	m_OpenType = opentype;
	m_LockType = lockType;

	return DBF_SUCCESS;
}

void CDBF::Close()
{
	if (_open)
		close(_dbfhandle);

	m_nCurrOffset = 0;
	m_nRecBeginOffset = 0;
	m_nStructSize = 0;
	m_nCurrRecNo = 0;
	recInfo.rec_num = 0;
	recInfo.rec_size = 0;
	recInfo.fld_num = 0;
	_open = false;
}

int CDBF::LockRecord()
{
#ifdef WIN32
	long offset;
	time_t t1, t2;
	int RetCode;

	time(&t1);

	offset = BASE_LOCK_OFFSET + m_nCurrOffset;

	lseek(_dbfhandle, offset, SEEK_SET);

	do  {
		RetCode = _locking(_dbfhandle, LK_NBLCK, recInfo.rec_size);
		time(&t2);
	} while (difftime(t2, t1) <= 5 && RetCode != 0);

	if (RetCode == -1)
		return DBF_FAIL;

#endif
	return DBF_SUCCESS;
}

int CDBF::UnlockRecord()
{
#ifdef WIN32
	long offset;

	offset = BASE_LOCK_OFFSET + m_nCurrOffset;

	lseek(_dbfhandle, offset, SEEK_SET);
	_locking(_dbfhandle, LK_UNLCK, recInfo.rec_size);
#endif
	return DBF_SUCCESS;
}

int CDBF::LockHead()
{
#ifdef WIN32
	long offset;

	offset = BASE_LOCK_OFFSET + 1L;

	lseek(_dbfhandle, offset, SEEK_SET);
	if (_locking(_dbfhandle, LK_NBLCK, 32L) == -1)
	{
		strcpy(errMessage, "locking failed.");
		return DBF_FAIL;
	}
#endif
	return DBF_SUCCESS;
}

int CDBF::UnlockHead()
{
#ifdef WIN32
	long offset;

	offset = BASE_LOCK_OFFSET + 1L;

	lseek(_dbfhandle, offset, SEEK_SET);
	if (_locking(_dbfhandle, LK_UNLCK, 32L) == -1)
	{
		strcpy(errMessage, "Unlocking failed.");
		return DBF_FAIL;
	}
#endif
	return DBF_SUCCESS;
}

int CDBF::GoTo(long recNo)
{
	m_nCurrOffset = m_nRecBeginOffset + (recNo - 1) * recInfo.rec_size;
	if (IsEof() || IsBof())
	{
		strcpy(errMessage, "is eof or bof.");
		return DBF_FAIL;
	}
	m_nCurrRecNo = recNo;
	return DBF_SUCCESS;
}

//hsn add by 2001.11.10
int CDBF::GoToRec(long recNo)
{
	GetRecordNum();
	if (recNo > (long)recInfo.rec_num)
		return DBF_FAIL;
	m_nCurrRecNo = recNo;
	m_nCurrOffset = m_nRecBeginOffset + (recNo - 1)*recInfo.rec_size;
	lseek(_dbfhandle, m_nCurrOffset, SEEK_SET);
	read(_dbfhandle, recInfo.rec_data, recInfo.rec_size);
	return DBF_SUCCESS;
}
//hsn add by 2001.11.10


int CDBF::ReadRecord(long recNo)
{
	char tmpstr[MAX_RECORD_SIZE + 1];
	int nRc;
	tmpstr[0] = 0;

	if (recNo < 1L || recNo > GetRecordNum())
		return DBF_FAIL;

	if ((nRc = GoTo(recNo)) == DBF_FAIL)
		return nRc;

	if (m_CurrLockType == _AutoLock)
	{
		if ((nRc = LockRecord()) == DBF_FAIL)
		{
			return nRc;
		}
	}

	lseek(_dbfhandle, m_nCurrOffset, SEEK_SET);
	if (read(_dbfhandle, tmpstr, recInfo.rec_size) != recInfo.rec_size)
		nRc = DBF_FAIL;

	if (m_CurrLockType == _AutoLock)
	{
		if ((nRc = UnlockRecord()) == DBF_FAIL)
		{
			return nRc;
		}
	}

	m_nCurrRecNo = recNo;
	memcpy(recInfo.rec_data, tmpstr, recInfo.rec_size);
	return DBF_SUCCESS;
}

bool CDBF::IsDelete()
{
	if (recInfo.rec_data[0] == RECORD_DEL_FLAG)
		return true;
	return false;
}

int CDBF::FindFld(const char *fldName)
{
	int i;
	for (i = 0; i < recInfo.fld_num; i++)
	{
#ifdef _MSC_VER
		if (stricmp(fld_info[i].name, fldName) == 0)
		{
			return i;
		}
#else
		if (strcasecmp(fld_info[i].name,fldName)==0)
		{
			return i;
		}
#endif

	}
	return -1;
}

int CDBF::GetFldValue(const char *fldName, char *fldValue)
{
	long i;

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	memcpy(fldValue, recInfo.rec_data + fld_info[i].offset + 1, fld_info[i].width);
	fldValue[fld_info[i].width] = '\0';
	Alltrim(fldValue);
	return DBF_SUCCESS;
}

int CDBF::GetFldValue(const char *fldName, char &fldValue)
{
	long i;

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	fldValue = recInfo.rec_data[fld_info[i].offset + 1];
	return DBF_SUCCESS;
}

int CDBF::GetFldValue(const char *fldName, double &fldValue)
{
	long i;
	char tmpstr[30];

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;
	memcpy(tmpstr, recInfo.rec_data + fld_info[i].offset + 1, fld_info[i].width);
	tmpstr[fld_info[i].width] = '\0';
	Alltrim(tmpstr);
	if (memcmp(tmpstr, "-.", 2) == 0)
		strcpy(tmpstr, "0");
	fldValue = atof(tmpstr);
	return DBF_SUCCESS;
}

int CDBF::GetFldValue(const char *fldName, int &fldValue)
{
	long i;
	char tmpstr[30];

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;
	memcpy(tmpstr, recInfo.rec_data + fld_info[i].offset + 1, fld_info[i].width);
	tmpstr[fld_info[i].width] = '\0';
	Alltrim(tmpstr);

#ifdef _MSC_VER
	if (stricmp(tmpstr, "-") == 0)
		strcpy(tmpstr, "0");
#else
	if (strcasecmp(tmpstr, "-") == 0)
		strcpy(tmpstr, "0");
#endif

	fldValue = atoi(tmpstr);
	return DBF_SUCCESS;
}

int CDBF::GetFldValue(const char *fldName, long &fldValue)
{
	long i;
	char tmpstr[30];

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;
	memcpy(tmpstr, recInfo.rec_data + fld_info[i].offset + 1, fld_info[i].width);
	tmpstr[fld_info[i].width] = '\0';
	Alltrim(tmpstr);

#ifdef _MSC_VER
	if (stricmp(tmpstr, "-") == 0)
		strcpy(tmpstr, "0");
#else
	if (strcasecmp(tmpstr, "-") == 0)
		strcpy(tmpstr, "0");
#endif

	fldValue = atol(tmpstr);
	return DBF_SUCCESS;
}

int CDBF::GetFldValue(size_t nFldIndex, char * chrFldValue)
{
	if (nFldIndex + 1 > recInfo.fld_num)
	{
		return DBF_FAIL;
	}
	memcpy(chrFldValue, recInfo.rec_data + fld_info[nFldIndex].offset + 1, fld_info[nFldIndex].width);
	chrFldValue[fld_info[nFldIndex].width] = '\0';
	Alltrim(chrFldValue);
	return DBF_SUCCESS;
}

int CDBF::GetFldName(size_t nFldIndex, const char *& chrRetName) const
{
	if (nFldIndex + 1 > recInfo.fld_num)
	{
		return DBF_FAIL;
	}

	chrRetName = fld_info[nFldIndex].name;
	return 0;
}

bool CDBF::IsEof()
{
	if (m_nCurrOffset >= m_nRecBeginOffset + recInfo.rec_num*recInfo.rec_size)
	{
		return true;
	}
	return false;

}

bool CDBF::IsBof()
{
	if (m_nCurrOffset < m_nRecBeginOffset)
	{
		return true;
	}
	return false;
}

int CDBF::ModifyFileDate()
{
#ifdef WIN32

	int nRc;
	CTime t = CTime::GetCurrentTime();
	char tmp[3];
	CString s = t.Format("%y");
	strcpy(tmp, s);
	f_date[0] = (unsigned char)atoi(tmp);
	s = t.Format("%m");
	strcpy(tmp, s);
	f_date[1] = (unsigned char)atoi(tmp);
	s = t.Format("%d");
	strcpy(tmp, s);
	f_date[2] = (unsigned char)atoi(tmp);


	if (m_HeadLockType == 'H')
	{
		if ((nRc = LockHead()) == DBF_FAIL)
		{
			return nRc;
		}
	}
	lseek(_dbfhandle, 1L, SEEK_SET);
	write(_dbfhandle, f_date, 3);
	if (m_HeadLockType == 'H')
	{
		if ((nRc = UnlockHead()) == DBF_FAIL)
		{
			return nRc;
		}
	}
#endif
	return DBF_SUCCESS;
}

int CDBF::GetRecordNum()
{
	char tmpstr[10];
	//取记录数
	if (LockHead() == DBF_FAIL)
		return DBF_FAIL;
	memset(tmpstr, 0, sizeof(tmpstr));
	lseek(_dbfhandle, 4L, SEEK_SET);
	read(_dbfhandle, tmpstr, 4L);
	recInfo.rec_num = *(unsigned long *)&tmpstr;

	UnlockHead();
	return recInfo.rec_num;
}

/*int CDBF::EditBegin()
{
int nRc;
if (!_open)
{
strcpy(errMessage,"not open");
return DBF_FAIL;
}
nRc=LockRecord();
if (nRc==DBF_FAIL)
return nRc;
return DBF_FAIL;
}

int CDBF::EditEnd()
{
int nRc;
if (!_open)
{
strcpy(errMessage,"not open");
return DBF_FAIL;
}
nRc=UnlockRecord();
if (nRc==DBF_FAIL)
return nRc;
return DBF_FAIL;
}*/


long CDBF::AddNewRecord()
{
	unsigned long tmp_rec_num = 0;
	int nRc;

	memset(recInfo.rec_data, ' ', sizeof(recInfo.rec_data));

	if (LockFile() == DBF_FAIL)
	{
		return ((long)DBF_FAIL);
	}

	//  if (LockFile() == DBF_FAIL)
	//  {
	//int iNum = 0;
	//while(1)
	//{
	//	if (LockFile() == DBF_FAIL)
	//	{
	//		if(iNum < 20)
	//		{
	//			Sleep(100);
	//			iNum++;
	//			continue;
	//		}
	//		else
	//			return ((long)DBF_FAIL);
	//	}
	//	else
	//		break;
	//
	//}
	//      //return ((long)DBF_FAIL);
	//  }

	GetRecordNum();

	m_HeadLockType = 'F';

	m_nCurrOffset = m_nRecBeginOffset + recInfo.rec_num * recInfo.rec_size;
	//lseek(_dbfhandle, m_nCurrOffset + recInfo.rec_size, SEEK_SET);
	lseek(_dbfhandle, m_nCurrOffset, SEEK_SET);
	nRc = write(_dbfhandle, recInfo.rec_data, recInfo.rec_size);
	if (nRc != recInfo.rec_size)
	{
		UnlockFile();
		return ((long)DBF_FAIL);
	}

	tmp_rec_num = recInfo.rec_num + 1L;

	//写记录数
	lseek(_dbfhandle, 4L, SEEK_SET);
	nRc = write(_dbfhandle, &tmp_rec_num, 4);

	// 	char tmpchr[2];
	// 	lseek(_dbfhandle, 0, SEEK_END);
	// 	tmpchr[0] = FILE_EOF_FLAG;
	// 	nRc = write(_dbfhandle, tmpchr, 1);

	recInfo.rec_num++;
	return recInfo.rec_num;
}

int CDBF::UpdateRecord()
{
	int nRc;
	char tmpchr[2];

	lseek(_dbfhandle, m_nCurrOffset, SEEK_SET);
	nRc = write(_dbfhandle, recInfo.rec_data, recInfo.rec_size);
	if (nRc != recInfo.rec_size)
	{
		UnlockFile();
		return ((int)DBF_FAIL);
	}

	//写文件结束符
	// 	lseek(_dbfhandle, m_nCurrOffset + recInfo.rec_size, SEEK_SET);
	// 	tmpchr[0] = FILE_EOF_FLAG;
	// 	nRc = write(_dbfhandle, tmpchr, 1);
	if (m_nCurrRecNo == recInfo.rec_num)
	{
		long lOff = m_nRecBeginOffset + (recInfo.rec_num) * recInfo.rec_size;
		lseek(_dbfhandle, lOff, SEEK_SET);
		tmpchr[0] = FILE_EOF_FLAG;
		nRc = write(_dbfhandle, tmpchr, 1);
	}

	ModifyFileDate();
	UnlockFile();
	return DBF_SUCCESS;
}

int CDBF::FillFldValue(const char *fldName, const char *fldValue)
{
	char tmpstr[MAX_FLD_DATALEN];
	unsigned char fmt[30];
	long i;

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	sprintf((char *)fmt, "%%-%ds", (int)fld_info[i].width);
	sprintf(tmpstr, (const char *)fmt, fldValue);
	memcpy(recInfo.rec_data + 1 + fld_info[i].offset, tmpstr, fld_info[i].width);

	return DBF_SUCCESS;
}

// XeKernel@gmail.com add (2013/04/12)
int CDBF::FillFldValue(size_t nFldIndex, const char * chrFldValue)
{
	char tmpstr[MAX_FLD_DATALEN];
	unsigned char fmt[30];

	if (nFldIndex + 1 > recInfo.fld_num) // no nFldIndex < 0 because of size_t
		return DBF_FAIL;

	sprintf((char *)fmt, "%%-%ds", (int)fld_info[nFldIndex].width);
	sprintf(tmpstr, (const char *)fmt, chrFldValue);
	memcpy(recInfo.rec_data + 1 + fld_info[nFldIndex].offset, tmpstr, fld_info[nFldIndex].width);

	return DBF_SUCCESS;
}
// end

// v1.02 add for des
int CDBF::FillFldValue(const char *fldName, const char *fldValue, int more)
{
	long i;

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	memcpy(recInfo.rec_data + 1 + fld_info[i].offset, fldValue, fld_info[i].width);

	return DBF_SUCCESS;
}
// end

int CDBF::FillFldValue(const char *fldName, int fldValue)
{
	char tmpstr[MAX_FLD_DATALEN];
	unsigned char fmt[30];
	long i;

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	sprintf((char *)fmt, "%%%dd", fld_info[i].width);
	sprintf(tmpstr, (const char *)fmt, fldValue);
	memcpy(recInfo.rec_data + 1 + fld_info[i].offset, tmpstr, fld_info[i].width);
	return DBF_SUCCESS;
}

int CDBF::FillFldValue(const char *fldName, char fldValue)
{
	long i;

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	recInfo.rec_data[fld_info[i].offset + 1] = fldValue;
	return DBF_SUCCESS;
}

int CDBF::FillFldValue(const char *fldName, unsigned int fldValue)
{
	char tmpstr[MAX_FLD_DATALEN + 1];
	unsigned char fmt[30];
	long i;

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	sprintf((char *)fmt, "%%%dd", fld_info[i].width);
	sprintf(tmpstr, (const char *)fmt, (int)fldValue);
	memcpy(recInfo.rec_data + 1 + fld_info[i].offset, tmpstr, fld_info[i].width);
	return DBF_SUCCESS;
}

int CDBF::FillFldValue(const char *fldName, long fldValue)
{
	char tmpstr[MAX_FLD_DATALEN + 1];
	unsigned char fmt[30];
	long i;

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	sprintf((char *)fmt, "%%%dld", fld_info[i].width);
	sprintf(tmpstr, (const char *)fmt, fldValue);
	memcpy(recInfo.rec_data + 1 + fld_info[i].offset, tmpstr, fld_info[i].width);

	return DBF_SUCCESS;
}

int CDBF::FillFldValue(const char *fldName, unsigned long fldValue)
{
	char tmpstr[MAX_FLD_DATALEN + 1];
	unsigned char fmt[30];
	long i;

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	sprintf((char *)fmt, "%%%dld", (int)fld_info[i].width);
	sprintf(tmpstr, (const char *)fmt, fldValue);
	memcpy(recInfo.rec_data + 1 + fld_info[i].offset, tmpstr, fld_info[i].width);

	return DBF_SUCCESS;
}

int CDBF::FillFldValue(const char *fldName, double fldValue)
{
	char tmpstr[MAX_FLD_DATALEN + 1];
	unsigned char fmt[30];
	long i;

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	sprintf((char *)fmt, "%%%d.%df", fld_info[i].width, fld_info[i].dec_d);
	sprintf(tmpstr, (const char *)fmt, fldValue);
	memcpy(recInfo.rec_data + 1 + fld_info[i].offset, tmpstr, fld_info[i].width);

	return DBF_SUCCESS;
}

int CDBF::FillFldValue(size_t nFldIndex, int iValue)
{
	char tmpstr[MAX_FLD_DATALEN];
	unsigned char fmt[30];

	if (nFldIndex + 1 > recInfo.fld_num) // no nFldIndex < 0 because of size_t
		return DBF_FAIL;


	sprintf((char *)fmt, "%%%dd", fld_info[nFldIndex].width);
	sprintf(tmpstr, (const char *)fmt, iValue);
	memcpy(recInfo.rec_data + 1 + fld_info[nFldIndex].offset, tmpstr, fld_info[nFldIndex].width);
	return DBF_SUCCESS;
}



int CDBF::MoveLast()
{
	GetRecordNum();
	if (recInfo.rec_num == 0)
	{
		strcpy(errMessage, "not exists record");
		return DBF_FAIL;
	}
	m_nCurrOffset = m_nRecBeginOffset + (recInfo.rec_num - 1)*recInfo.rec_size;
	m_nCurrRecNo = recInfo.rec_num;
	strcpy(errMessage, "is last record");
	return DBF_SUCCESS;
}

int CDBF::MoveFirst()
{
	m_nCurrOffset = m_nRecBeginOffset;
	m_nCurrRecNo = 1;
	return DBF_SUCCESS;
}

int CDBF::FindRecord(const char *fldName, const char *fldValue)
{
	long fldOffset = 0, j, tmpoffset, tmprecordno = 1;

	fldOffset = FindFld(fldName);
	if (fldOffset < 0)
		return DBF_FAIL;

	tmpoffset = m_nRecBeginOffset;//记录开始位置
	GetRecordNum();
	for (j = 0; j < (int)recInfo.rec_num; j++)
	{
		lseek(_dbfhandle, tmpoffset, SEEK_SET);
		/*if ((nRc=LockRecord())==DBF_FAIL)
		{
		return nRc;
		}*/
		read(_dbfhandle, recInfo.rec_data, recInfo.rec_size);
		//UnlockRecord();
		if (memcmp(recInfo.rec_data + fld_info[fldOffset].offset + 1, fldValue, fld_info[fldOffset].width) == 0)
		{
			m_nCurrRecNo = tmprecordno;
			m_nCurrOffset = tmpoffset;
			strcpy(errMessage, "fount record");
			return DBF_SUCCESS;
		}
		tmprecordno++;
		tmpoffset += recInfo.rec_size;
	}

	strcpy(errMessage, "don't found record");
	return DBF_FAIL;
}

//2005.05.30. huangsn add 统计符合条件的记录数
long CDBF::CountRecord(const char *fldName, const char *fldValue)
{
	long fldOffset = 0, j, tmpoffset, count = 0;

	fldOffset = FindFld(fldName);
	if (fldOffset < 0)
		return DBF_FAIL;

	tmpoffset = m_nRecBeginOffset;//记录开始位置
	GetRecordNum();
	for (j = 0; j < (int)recInfo.rec_num; j++)
	{
		lseek(_dbfhandle, tmpoffset, SEEK_SET);
		read(_dbfhandle, recInfo.rec_data, recInfo.rec_size);
		if (memcmp(recInfo.rec_data + fld_info[fldOffset].offset + 1, fldValue, fld_info[fldOffset].width) == 0)
			count++;
		tmpoffset += recInfo.rec_size;
	}
	return count;
}
//2005.05.30. huangsn add

//2005.05.30. huangsn add 统计符合条件的记录数,多条件
long CDBF::CountRecord_M(const char *fldName, const char *fldValue, const char *fldName1, const char *fldValue1)
{
	long fldOffset = 0, fldOffset1 = 0, j, tmpoffset, count = 0;

	fldOffset = FindFld(fldName);
	fldOffset1 = FindFld(fldName1);
	if (fldOffset < 0)
		return DBF_FAIL;
	if (fldOffset1 < 0)
		return DBF_FAIL;

	tmpoffset = m_nRecBeginOffset;//记录开始位置
	GetRecordNum();
	for (j = 0; j < (int)recInfo.rec_num; j++)
	{
		lseek(_dbfhandle, tmpoffset, SEEK_SET);
		read(_dbfhandle, recInfo.rec_data, recInfo.rec_size);
		if (memcmp(recInfo.rec_data + fld_info[fldOffset].offset + 1, fldValue, fld_info[fldOffset].width) == 0 &&
			memcmp(recInfo.rec_data + fld_info[fldOffset1].offset + 1, fldValue1, fld_info[fldOffset1].width) == 0)
			count++;
		tmpoffset += recInfo.rec_size;
	}
	return count;
}
//2005.05.30. huangsn add

int CDBF::FindRecord_m(const char *fldName, const char *fldValue, const char *fldName1, const char *fldValue1)
{
	long fldOffset = 0, fldOffset1 = 0, j, tmpoffset, tmprecordno = 1;

	fldOffset = FindFld(fldName);
	fldOffset1 = FindFld(fldName1);
	if (fldOffset < 0)
		return DBF_FAIL;
	if (fldOffset1 < 0)
		return DBF_FAIL;

	tmpoffset = m_nRecBeginOffset;//记录开始位置
	GetRecordNum();
	for (j = 0; j < (int)recInfo.rec_num; j++)
	{
		lseek(_dbfhandle, tmpoffset, SEEK_SET);
		read(_dbfhandle, recInfo.rec_data, recInfo.rec_size);
		if (memcmp(recInfo.rec_data + fld_info[fldOffset].offset + 1, fldValue, fld_info[fldOffset].width) == 0 &&
			memcmp(recInfo.rec_data + fld_info[fldOffset1].offset + 1, fldValue1, fld_info[fldOffset1].width) == 0)
		{
			m_nCurrRecNo = tmprecordno;
			m_nCurrOffset = tmpoffset;
			strcpy(errMessage, "fount record");
			return DBF_SUCCESS;
		}
		tmprecordno++;
		tmpoffset += recInfo.rec_size;
	}

	strcpy(errMessage, "don't found record");
	return DBF_FAIL;
}

int CDBF::FindRecord_m3(const char *fldName, const char *fldValue, const char *fldName1, const char *fldValue1, const char *fldName2, const char *fldValue2)
{
	long fldOffset = 0, fldOffset1 = 0, fldOffset2 = 0, j, tmpoffset, tmprecordno = 1;

	fldOffset = FindFld(fldName);
	fldOffset1 = FindFld(fldName1);
	fldOffset2 = FindFld(fldName2);
	if (fldOffset < 0)
		return DBF_FAIL;
	if (fldOffset1 < 0)
		return DBF_FAIL;
	if (fldOffset2 < 0)
		return DBF_FAIL;

	tmpoffset = m_nRecBeginOffset;//记录开始位置
	GetRecordNum();
	for (j = 0; j < (int)recInfo.rec_num; j++)
	{
		lseek(_dbfhandle, tmpoffset, SEEK_SET);
		read(_dbfhandle, recInfo.rec_data, recInfo.rec_size);
		if (memcmp(recInfo.rec_data + fld_info[fldOffset].offset + 1, fldValue, fld_info[fldOffset].width) == 0 &&
			memcmp(recInfo.rec_data + fld_info[fldOffset1].offset + 1, fldValue1, fld_info[fldOffset1].width) == 0 &&
			memcmp(recInfo.rec_data + fld_info[fldOffset2].offset + 1, fldValue2, fld_info[fldOffset2].width) == 0)
		{
			m_nCurrRecNo = tmprecordno;
			m_nCurrOffset = tmpoffset;
			strcpy(errMessage, "fount record");
			return DBF_SUCCESS;
		}
		tmprecordno++;
		tmpoffset += recInfo.rec_size;
	}

	strcpy(errMessage, "don't found record");
	return DBF_FAIL;
}

char *CDBF::GetErrorMsg()
{
	return errMessage;
}

int CDBF::GetColNum()
{
	return (recInfo.fld_num);
}

const FIELD_INFO * CDBF::GetColInfo(int colNo)
{
	if (recInfo.fld_num <= 0 || colNo >= recInfo.fld_num)
		return (const FIELD_INFO *)NULL;
	return &fld_info[colNo];
}

int CDBF::Zap()
{
#ifdef WIN32
	//	int fp;
	unsigned long tmp_rec_num;

	if (_open == false)
		return DBF_FAIL;

	//写记录数
	lseek(_dbfhandle, 4L, SEEK_SET);
	tmp_rec_num = 0L;
	write(_dbfhandle, &tmp_rec_num, 4);

	chsize(_dbfhandle, m_nStructSize);

	if (errno != 0)
		return DBF_FAIL;
	Close();
	ReOpen();
#endif

	return DBF_SUCCESS;
}

char * CDBF::GetFileName()
{
	return m_FileName;
}

int CDBF::ReOpen()
{
	return(Open(m_FileName, m_OpenType, m_LockType));
}

int CDBF::DeleteRecord(long recNo)
{
	if (recNo < 1L || recNo > GetRecordNum())
		return DBF_FAIL;
	ReadRecord(recNo);
	recInfo.rec_data[0] = '*';
	return(ReplaceRecord(recNo));
}

int CDBF::DeleteRecord()
{
	recInfo.rec_data[0] = RECORD_DEL_FLAG;
	return DBF_SUCCESS;
}

int CDBF::UnDeleteRecord(long recNo)
{
	if (recNo < 1L || recNo > GetRecordNum())
		return DBF_FAIL;
	ReadRecord(recNo);
	recInfo.rec_data[0] = RECORD_BEGIN_FLAG;
	return(ReplaceRecord(recNo));
}

int CDBF::UnDeleteRecord()
{
	recInfo.rec_data[0] = RECORD_BEGIN_FLAG;
	return DBF_SUCCESS;
}

int CDBF::ReplaceRecord(long recNo)
{
	//调用该函数之前必须先调用ReadRecord();
	int nRc = DBF_SUCCESS;

	if (recNo < 1L || recNo > GetRecordNum())
		return DBF_FAIL;

	//	if ((nRc=GoTo(recNo))==DBF_FAIL)
	//		return nRc;

	if (m_CurrLockType == _AutoLock)
	{
		if ((nRc = LockRecord()) == DBF_FAIL)
		{
			return nRc;
		}
	}

	lseek(_dbfhandle, m_nCurrOffset, SEEK_SET);
	if (write(_dbfhandle, recInfo.rec_data, recInfo.rec_size) != recInfo.rec_size)
		nRc = DBF_FAIL;

	if (m_CurrLockType == _AutoLock)
	{
		if (UnlockRecord() == DBF_FAIL)
		{
			return DBF_FAIL;
		}
	}

	if (nRc == DBF_FAIL)
		return nRc;

	m_HeadLockType = 'H';

	return (ModifyFileDate());

}
int CDBF::UpdateFldValue(const char *fldName, const char *fldValue)
{
	char tmpstr[MAX_FLD_DATALEN];
	unsigned char fmt[30];
	long i;

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	sprintf((char *)fmt, "%%-%ds", (int)fld_info[i].width);
	sprintf(tmpstr, (const char *)fmt, fldValue);
	memcpy(recInfo.rec_data + fld_info[i].offset + 1, tmpstr, fld_info[i].width);

	return DBF_SUCCESS;
}

int CDBF::UpdateFldValue(const char *fldName, int fldValue)
{
	long i;
	char tmpstr[MAX_FLD_DATALEN + 1];
	char fmtStr[20];

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	sprintf(fmtStr, "%%%dd", fld_info[i].width);
	sprintf(tmpstr, fmtStr, fldValue);

	memcpy(recInfo.rec_data + fld_info[i].offset + 1, tmpstr, fld_info[i].width);

	return DBF_SUCCESS;
}

int CDBF::UpdateFldValue(const char *fldName, unsigned int fldValue)
{
	long i;
	char tmpstr[MAX_FLD_DATALEN + 1];
	char fmtStr[20];

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	sprintf(fmtStr, "%%%dd", fld_info[i].width);
	sprintf(tmpstr, fmtStr, fldValue);

	memcpy(recInfo.rec_data + fld_info[i].offset + 1, tmpstr, fld_info[i].width);

	return DBF_SUCCESS;
}

int CDBF::UpdateFldValue(const char *fldName, long fldValue)
{
	long i;
	char tmpstr[MAX_FLD_DATALEN + 1];
	char fmtStr[20];

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	sprintf(fmtStr, "%%%dld", fld_info[i].width);
	sprintf(tmpstr, fmtStr, fldValue);

	memcpy(recInfo.rec_data + fld_info[i].offset + 1, tmpstr, fld_info[i].width);

	return DBF_SUCCESS;
}

int CDBF::UpdateFldValue(const char *fldName, unsigned long fldValue)
{
	long i;
	char tmpstr[MAX_FLD_DATALEN + 1];
	char fmtStr[20];

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	sprintf(fmtStr, "%%%dld", fld_info[i].width);
	sprintf(tmpstr, fmtStr, fldValue);

	memcpy(recInfo.rec_data + fld_info[i].offset + 1, tmpstr, fld_info[i].width);

	return DBF_SUCCESS;
}

int CDBF::UpdateFldValue(const char *fldName, double fldValue)
{
	long i;
	char tmpstr[MAX_FLD_DATALEN + 1];
	char fmtStr[20];

	i = FindFld(fldName);
	if (i < 0)
		return DBF_FAIL;

	if (fld_info[i].type == 'N' || fld_info[i].type == 'F')
		sprintf(fmtStr, "%%%d.%df", fld_info[i].width, fld_info[i].dec_d);
	else
		sprintf(fmtStr, "%%%d.2f", fld_info[i].width);
	sprintf(tmpstr, fmtStr, fldValue);

	memcpy(recInfo.rec_data + fld_info[i].offset + 1, tmpstr, fld_info[i].width);

	return DBF_SUCCESS;
}

int CDBF::LockFile()
{
#ifdef WIN32
	time_t t1, t2;
	int RetCode;

	time(&t1);

	lseek(_dbfhandle, BASE_LOCK_OFFSET, SEEK_SET);
	//lseek(_dbfhandle, 0, SEEK_SET);
	do  {
		RetCode = _locking(_dbfhandle, LK_NBLCK, LOCK_FILE_LEN);
		time(&t2);
	} while (difftime(t2, t1) <= 5 && RetCode != 0);

	if (RetCode == 0)
	{
		return DBF_SUCCESS;
	}
	else
	{
		return DBF_FAIL;
	}

#else
	return DBF_SUCCESS;

#endif
}

int CDBF::UnlockFile()
{
#ifdef WIN32
	int RetCode = 0;
	lseek(_dbfhandle, BASE_LOCK_OFFSET, SEEK_SET);
	RetCode = _locking(_dbfhandle, LK_UNLCK, LOCK_FILE_LEN);
	if (RetCode == 0)
	{
		return DBF_SUCCESS;
	}
	else
	{
		return DBF_FAIL;
	}
#else
	return DBF_SUCCESS;
#endif
}

int CDBF::AddField(const char *fieldName, char type, unsigned short width, unsigned short decWidth)
{
	//     char tmpstr[12];

	if (m_FieldNum >= 255)
		return DBF_FAIL;

	if (type != 'C' && type != 'N' && type != 'D')
		return DBF_FAIL;


	DBF_FIELD_INFO * pNewField = new DBF_FIELD_INFO;

	if (!pNewField)
	{
		ClearFieldInfo();
		return DBF_FAIL;
	}
	m_pField[m_FieldNum] = pNewField;
	// 	memset(pNewField, 0x00, sizeof(DBF_FIELD_INFO));

	//     sprintf(tmpstr, "%-10s", fieldName);
	// 	int iSize = min(sizeof(pNewField->fieldName), strlen(fieldName));

	strncpy(pNewField->fieldName, fieldName, sizeof(pNewField->fieldName));
	pNewField->standby = 0;
	pNewField->type = type;
	pNewField->width = (char)width;
	pNewField->decWidth = (char)decWidth;
	memset(pNewField->other, 0, sizeof(pNewField->other));

	unsigned long beginOffiset = 1;
	if (m_FieldNum != 0)
	{
		DBF_FIELD_INFO & stPrevInfo = *(m_pField[m_FieldNum - 1]);
		beginOffiset = stPrevInfo.beginOffset + stPrevInfo.width;
	}
	pNewField->beginOffset = beginOffiset;

	m_FieldNum++;

	return DBF_SUCCESS;
}

int CDBF::Create(const char *fileName)
{
	char tmpstr[20];
	FILE * fp;
	int i;

	if (fileName[0] == 0)
	{
		strcpy(errMessage, "fileName is Null.");
		ClearFieldInfo();
		return DBF_FAIL;
	}

	if (m_FieldNum <= 0 || m_FieldNum > 255)
		return DBF_FAIL;

	if ((fp = fopen(fileName, "wb+")) == NULL)
	{
		strcpy(errMessage, "fopen is failed.");
		ClearFieldInfo();
		return DBF_FAIL;
	}

	time_t t;
	struct tm tmlocal;

	t = ::time(&t);
	tmlocal = *(::localtime(&t));

	DBF_HEAD _head;
	_head.fileFlag = 0x03;
	_head.lastDate[0] = (unsigned char)(tmlocal.tm_year % 100);
	_head.lastDate[1] = (unsigned char)(tmlocal.tm_mon + 1);
	_head.lastDate[2] = (unsigned char)tmlocal.tm_mday;
	_head.recNum = 0;
	_head.structSize = (unsigned short)(FILE_HEAD_LEN + 32 * m_FieldNum + 1);
	_head.recSize = 0;

	for (i = 0; i < m_FieldNum; i++)
	{
		_head.recSize += m_pField[i]->width;
	}
	_head.recSize++;

	memset(_head.other, 0, sizeof(_head.other));

	if (fwrite(&_head, sizeof(DBF_HEAD), 1, fp) != 1)
	{
		strcpy(errMessage, "write file head failed.");
		fclose(fp);
		ClearFieldInfo();
		return DBF_FAIL;
	}

	for (i = 0; i < m_FieldNum; i++)
	{
		if (fwrite(m_pField[i], sizeof(DBF_FIELD_INFO), 1, fp) != 1)
		{
			sprintf(errMessage, "write filed [%d] failed", i);
			fclose(fp);
			ClearFieldInfo();
			return DBF_FAIL;
		}
	}

	strcpy(tmpstr, "\x0d");
	fwrite(tmpstr, 1, 1, fp);
	fclose(fp);

	ClearFieldInfo();

	return DBF_SUCCESS;
}

void CDBF::ClearFieldInfo()
{
	int i;

	for (i = 0; i < m_FieldNum; i++)
		delete m_pField[i];
	m_FieldNum = 0;
}

unsigned long CDBF::GetCurrRecNum()
{
	return m_nCurrRecNo;
}


