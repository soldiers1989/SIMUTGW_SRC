#ifndef __BINARY_STRING_TOOL_H__
#define __BINARY_STRING_TOOL_H__

#include <memory>
#include <string>
#include <stdint.h>

#include "util/EzLog.h"

#include <iostream>

/*
binary 用的 string tool
*/
namespace Binary_StringTool
{
	/*
	往char array中写入数据，不足的位填空格
	*/
	int CopyStringToNetchararray(char** dst, const std::string& src, const size_t maxLen);

};

#endif