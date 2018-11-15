#ifndef __BINARY_PACKET_ASSEMBLER_H__
#define __BINARY_PACKET_ASSEMBLER_H__

#include <vector>
#include <stdint.h>
#include <vector>
#include <time.h>
#include <memory>
#include <stdint.h>

#include "util/EzLog.h"

#include "BinaryNetPack.h"
#include "BinaryMessageStructDefine.h"

/*
�������ݰ�������������
*/
class BinaryPacketAssembler
{
	//
	// Members
	//
public:
	static const uint32_t HEAD_SIZE = sizeof(simutgw::binary::BINARY_HEAD);
	static const uint32_t TAIL_SIZE = sizeof(simutgw::binary::BINARY_TAIL);

protected:


	//
	// Functions
	//
public:
	BinaryPacketAssembler(void);
	~BinaryPacketAssembler(void);

	static int RecvPackage(std::vector<uint8_t>& vctBuffer,
		std::vector<std::shared_ptr<struct simutgw::BinaryNetPack>>& recvedDatas,
		bool bTrimWasteData);

	/*
	Binary����У���
	*/
	static uint32_t GenerateCheckSum(char *buf, uint32_t bufLen)
	{
		size_t idx = 0;
		uint32_t cks = 0;
		for ( idx = 0L, cks = 0; idx < bufLen; cks += (uint32_t) buf[idx++] );
		return ( cks % 256 );
	}

	/*
	Binary����У���

	@param const std::vector<uint8_t>& vctData : ���ݶ�
	@param size_t iBeginPos : ��ʼ����λ��
	@param uint32_t bufLen : β������
	*/
	static uint32_t GenerateCheckSum(const std::vector<uint8_t>& vctData, size_t iBeginPos, uint32_t bufLen)
	{
		size_t idx = iBeginPos;
		uint32_t cks = 0;
		while ( idx < vctData.size() &&
			idx < ( iBeginPos + bufLen ) )
		{
			cks += static_cast<uint32_t>( vctData[idx] );
			++idx;
		}
		uint32_t cksRes = cks % 256;

		return cksRes;
	}

	/*
	�������У����Ƿ���ȷ

	@return true : ��ȷ
	@return false : ����
	*/
	static bool IsPackageCheckSumValid(std::shared_ptr<simutgw::BinaryNetPack>& pBinaryNetPack);

	/*
	������� Logon ��Ϣ����Ӧ��

	@return 0 : �ɹ�
	@return -1 : ʧ��
	*/
	static int LocalPackToNetBuffer_Logon(struct simutgw::binary::BINARY_LOGON& msgobj,
		std::vector<uint8_t>& out_vctData);

	/*
	������� ƽ̨״̬��Ϣ Platform State Info ��Ϣ����Ӧ��

	@return 0 : �ɹ�
	@return -1 : ʧ��
	*/
	static int LocalPackToNetBuffer_PlatformStateInfo(struct simutgw::binary::PLATFORM_STATE_INFO& msgobj,
		std::vector<uint8_t>& out_vctData);

	/*
	������� ���� Heartbeat ��Ϣ����Ӧ��

	@return 0 : �ɹ�
	@return -1 : ʧ��
	*/
	static int LocalPackToNetBuffer_Heartbeat(std::vector<uint8_t>& out_vctData);

	/*
	������� ƽ̨״̬ͬ�� Report Synchronization ��Ϣ����Ӧ��

	@return 0 : �ɹ�
	@return -1 : ʧ��
	*/
	static int LocalPackToNetBuffer_ReportSynch(struct simutgw::binary::REPORT_SYNCHRONIZATION& execReportSynchObj,
		std::vector<uint8_t>& out_vctData);

	/*
	������� NewOrder_100101 ��Ϣ����Ӧ��

	@return 0 : �ɹ�
	@return -1 : ʧ��
	*/
	static int LocalPackToNetBuffer_NewOrder_100101(
	struct simutgw::binary::NEW_ORDER& newOrderObj,
	struct simutgw::binary::NEW_ORDER_EXT_100101& newOrder_extObj,
		std::vector<uint8_t>& out_vctData);

	/*
	������� ExecReport_200102 ��Ϣ����Ӧ��

	@return 0 : �ɹ�
	@return -1 : ʧ��
	*/
	static int LocalPackToNetBuffer_ExecReport_200102(struct simutgw::binary::EXECUTION_REPORT& execRepotObj,
	struct simutgw::binary::EXECUTION_REPORT_EXT_200102& execRepot_extObj, std::vector<uint8_t>& out_vctData);

	/*
	������� ExecReport_200115 ��Ϣ����Ӧ��

	@return 0 : �ɹ�
	@return -1 : ʧ��
	*/
	static int LocalPackToNetBuffer_ExecReport_Matched_200115(
	struct simutgw::binary::EXECUTION_REPORT_MATCHED& execRepotObj,
	struct simutgw::binary::EXECUTION_REPORT_MATCHED_EXT_200115& execRepot_extObj,
		std::vector<uint8_t>& out_vctData);
};
#endif