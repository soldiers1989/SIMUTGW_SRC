#ifndef __NET_PACKAGE_H__
#define __NET_PACKAGE_H__

#include <string>

/*
�������ݰ�
*/
namespace simutgw
{

	// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)

	// �������ݰ�ͷ
	struct NET_PACKAGE_HEAD
	{
		// ��ͷ��ʼ�ֶ� BeginString  4�ֽ� 0-3
		char beginstring[4];
		// ����汾�� 4�ֽ� 4-7
		char version[4];
		// ��ʱ��� 10�ֽ� 8-17
		char timestamp[10];
		// ���������� 4�ֽ� 18-21
		char type[4];
		// �����ݵ����ݳ��ȣ��������˰��ṹ�Ͱ�ͷβ 4�ֽ� 22-25
		char datalen[4];
		// ������У��ֵ md5���������� 16�ֽ� 26-41
		char check[16];
		// ��ͷchecksum��6�ֽ� 42-47
		char headcksum[6];
	};

	// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

	// ���ػ����������ݰ�ͷ
	struct NET_PACKAGE_HEAD_LOCAL
	{
		// ��ͷ��ʼ�ֶ� BeginString  4�ֽ� 0-3
		std::string beginstring;
		// ����汾�� 4�ֽ� 4-7
		int iVersion;
		// ��ʱ��� 10�ֽ� 8-17
		int iTimeStamp;
		// ���������� 4�ֽ� 18-21
		int iType;
		// �����ݵ����ݳ��ȣ��������˰��ṹ�Ͱ�ͷβ 4�ֽ� 22-25
		int iDatalen;
		// ������У��ֵ md5���������� 16�ֽ� 26-41
		std::string check;
		// ��ͷchecksum 6�ֽ� 42-47
		int iHeadChecksum;
	};

	//��ͷ���ȣ�Ϊ�̶���С48�ֽ�
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