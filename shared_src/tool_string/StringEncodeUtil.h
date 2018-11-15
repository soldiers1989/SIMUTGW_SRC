#ifndef __STRING_ENCODE_UTIL_H__
#define __STRING_ENCODE_UTIL_H__

#include <time.h>
#include <mutex>
#include "boost/date_time.hpp"

/*
String编码转换工具类
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
	将utf8编码的字符串转为gbk
	*/
	string& UtfToGbk( const string& in_strSrc, string& out_strDest );

	/*
	将gbk编码的字符串转为utf8
	*/
	string& GbkToUtf( const string& in_strSrc, string& out_strDest );
}

#endif