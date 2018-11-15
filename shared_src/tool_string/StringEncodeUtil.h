#ifndef __STRING_ENCODE_UTIL_H__
#define __STRING_ENCODE_UTIL_H__

#include <time.h>
#include <mutex>
#include "boost/date_time.hpp"

/*
String����ת��������
*/
namespace StringEncodeUtil
{
	using namespace std;
	//
	// Members
	//

	//
	// Functions
	//	
	/*
	��utf8������ַ���תΪgbk
	*/
	string& UtfToGbk( const string& in_strSrc, string& out_strDest );

	/*
	��gbk������ַ���תΪutf8
	*/
	string& GbkToUtf( const string& in_strSrc, string& out_strDest );
}

#endif