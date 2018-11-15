#ifndef __PACK_ASSEMBLER_H__
#define __PACK_ASSEMBLER_H__

#include "NetPackage.h"

#include <stdint.h>
#include <vector>
#include <time.h>
#include <memory>

/*
�������ݰ�������������
*/
class PacketAssembler
{
	//
	// Members
	//
protected:


	//
	// Functions
	//
public:
	PacketAssembler(void);
	~PacketAssembler(void);

	static std::string& DebugOut(std::shared_ptr<simutgw::NET_PACKAGE>& ptrNetPack,
		std::string& out_content);

	static std::string& DebugOut(simutgw::NET_PACKAGE_HEAD& netPackHead,
		std::string& out_content);

	/*
	������Ϣ������ɱ��ذ�

	@param bool bTrimWasteData : �Ƿ�ɾ����Ч����
	true -- ɾ��
	false -- ��ɾ��
	@param bool bCheckTimestamp : �Ƿ���ʱ���
	true -- ���
	false -- �����
	*/
	static int RecvPackage(std::vector<uint8_t>& vctBuffer,
		std::vector<std::shared_ptr<simutgw::NET_PACKAGE>>& recvedDatas,
		bool bTrimWasteData, bool bCheckTimestamp);

	/*
	���������ݰ�ͷת��Ϊ���ظ�ʽ

	@param bool bCheckTimestamp : �Ƿ���ʱ���
	true -- ���
	false -- �����
	*/
	static int NetPackageHeadToLocal(simutgw::NET_PACKAGE_HEAD& stNetPack,
		std::shared_ptr<simutgw::NET_PACKAGE>& ptrNetPack, bool bCheckTimestamp);

	static int LocalPackageToNetBuffer(const int in_iMsgType, const std::string &in_data,
		const int* in_piTimeStamp, std::vector<uint8_t>& out_vctData);

	static int GenerateNetPackCRC(const std::string& in_strNetPackData,
		std::string& out_checkSum);

	static bool CheckNetPackCRC(const std::shared_ptr<simutgw::NET_PACKAGE>& ptrNetPack);

	static bool PackageCompare(std::shared_ptr<simutgw::NET_PACKAGE>& ptrNetPackDest,
		std::shared_ptr<simutgw::NET_PACKAGE>& ptrNetPackSrc);

	/*
	�������еİ�ͷ����checksum
	*/
	static int GenerateChecksum(const std::vector<uint8_t>& vctData, size_t iBeginPos);

	static int GenerateHeadChecksum(const simutgw::NET_PACKAGE_HEAD& netPackHead);

	static int GenerateHeadChecksum(const std::shared_ptr<simutgw::NET_PACKAGE>& prtNetPack);

	/*
	�õ���ǰ��ʱ���
	*/
	static int GetTimeStamp()
	{
		time_t t = time(NULL);

		int j = static_cast<int>(t);
		return j;
	}

	/*
	�鿴��ǰʱ����Ƿ��ǵ���

	@param const int iStamp : timestamp
	@param bool bCheck : �Ƿ���ʱ���
	true -- ���
	false -- �����

	@return :
	-1 -- ��
	0 -- ��
	*/
	static int ValidateTimeStamp(const int iStamp, bool bCheck);
};

#endif