#ifndef __NET_PACKAGE_H__
#define __NET_PACKAGE_H__

#include <string>

/*
网络数据包
*/
namespace simutgw
{

	// 将字节对齐方式设为1
#pragma pack(push,1)

	// 网络数据包头
	struct NET_PACKAGE_HEAD
	{
		// 包头起始字段 BeginString  4字节 0-3
		char beginstring[4];
		// 封包版本号 4字节 4-7
		char version[4];
		// 包时间戳 10字节 8-17
		char timestamp[10];
		// 包数据类型 4字节 18-21
		char type[4];
		// 包数据的内容长度，不包含此包结构和包头尾 4字节 22-25
		char datalen[4];
		// 包数据校验值 md5二进制数据 16字节 26-41
		char check[16];
		// 包头checksum，6字节 42-47
		char headcksum[6];
	};

	// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

	// 本地化的网络数据包头
	struct NET_PACKAGE_HEAD_LOCAL
	{
		// 包头起始字段 BeginString  4字节 0-3
		std::string beginstring;
		// 封包版本号 4字节 4-7
		int iVersion;
		// 包时间戳 10字节 8-17
		int iTimeStamp;
		// 包数据类型 4字节 18-21
		int iType;
		// 包数据的内容长度，不包含此包结构和包头尾 4字节 22-25
		int iDatalen;
		// 包数据校验值 md5二进制数据 16字节 26-41
		std::string check;
		// 包头checksum 6字节 42-47
		int iHeadChecksum;
	};

	//包头长度，为固定大小48字节
	static unsigned int NET_PACKAGE_HEADLEN = sizeof(NET_PACKAGE_HEAD);

	static const char NETPACK_BEGINSTRING[4]={'N','E','T',':'};
	
	static const unsigned int NETPACK_BEGINSTR_LEN = 4;

	static const int NETPACK_VERSION = 1;

	static const unsigned int NETPACK_VERSION_LEN = 4;

	static const unsigned int NETPACK_TIMESTAMP_LEN = 10;

	static const unsigned int NETPACK_TYPE_LEN = 4;

	static const unsigned int NETPACK_DATALEN_LEN = 4;

	static const unsigned int NETPACK_CHECK_LEN = 16;
	
	static const unsigned int NETPACK_HEADCKSUM_LEN = 6;

	struct NET_PACKAGE
	{
		struct NET_PACKAGE_HEAD_LOCAL head;
		std::string data;
	};

};


#endif