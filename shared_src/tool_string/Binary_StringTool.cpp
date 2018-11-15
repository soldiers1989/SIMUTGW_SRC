#include "Binary_StringTool.h"

/*
binary 用的 string tool
*/
namespace Binary_StringTool
{
	/*
	往char array中写入数据，不足的位填空格
	*/
	int CopyStringToNetchararray(char** dst, const std::string& src, const size_t maxLen)
	{
		char black = ' ';

		char* local = *dst;		
		if ( maxLen >= src.length() )
		{
			memcpy(local, src.c_str(), src.length());

			int i = 0;
			for ( i = 0; i < ( maxLen - src.length() ); ++i )
			{
				memcpy(local + src.length() + i, &black, 1);
			}
		}
		else
		{
			memcpy(local, src.c_str(), maxLen);
		}

		return 0;
	}

};