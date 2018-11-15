#ifndef __BINARY_STRING_TOOL_H__
#define __BINARY_STRING_TOOL_H__

#include <memory>
#include <string>
#include <stdint.h>

#include "util/EzLog.h"

#include <iostream>

/*
binary �õ� string tool
*/
namespace Binary_StringTool
{
	/*
	��char array��д�����ݣ������λ��ո�
	*/
	int CopyStringToNetchararray(char** dst, const std::string& src, const size_t maxLen);

};

#endif