#include "BinaryPacketAssembler.h"

#include <stdint.h>

#ifdef _MSC_VER
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "BinaryMsgTypeDefine.h"
#include "BinaryMessageStructDefine.h"


int BinaryPacketAssembler::RecvPackage(std::vector<uint8_t>& vctBuffer,
	std::vector<std::shared_ptr<struct simutgw::BinaryNetPack>>& recvedDatas,
	bool bTrimWasteData)
{
	// static const std::string ftag("BinaryPacketAssembler::RecvPackage() ");

	size_t uiBufferSize = vctBuffer.size();
	if ( uiBufferSize < HEAD_SIZE )
	{
		// ʣ������ݶ˲�����ͷ���ݳ��ȣ������������
		return 1;
	}

	// net package head pos
	std::vector<uint8_t>::iterator itHeadPos = vctBuffer.end();

	// ����buffer�Ƿ��������
	bool bIsBuffNeedProcessed = false;

	do
	{
		bIsBuffNeedProcessed = false;

		uiBufferSize = vctBuffer.size();
		if ( 0 == uiBufferSize )
		{
			return 0;
		}
		else if ( uiBufferSize < HEAD_SIZE )
		{
			return 1;
		}

		//uiNetCompare_BeginStringPos = 0;
		itHeadPos = vctBuffer.begin();

		// ��ͷ��ʼ����
		// ��Ϣͷ
		// ����    �ֶ�����
		// MsgType  ��Ϣ����
		// BodyLength ��Ϣ�峤��
		simutgw::binary::BINARY_HEAD msgHead;
		memcpy(&msgHead, &vctBuffer[0], HEAD_SIZE);

		uint32_t uiMsgType = ntohl(msgHead.MsgType);
		uint32_t uiBodyLength = ntohl(msgHead.BodyLength);
		
		uint32_t uiNetPackLen = HEAD_SIZE + uiBodyLength + TAIL_SIZE;
		// check if vector is bigger enough
		if ( uiNetPackLen > vctBuffer.size() )
		{
			// ʣ������ݶ˲�����ͷ+����+��β���ݳ��ȣ������������
			return 1;
		}

		// copy net pack to return buffer.
		std::shared_ptr<struct simutgw::BinaryNetPack> ptrNetPack =
			std::shared_ptr<struct simutgw::BinaryNetPack>(new simutgw::BinaryNetPack());

		ptrNetPack->ui32MsgType = uiMsgType;
		ptrNetPack->ui32BodyLength = uiBodyLength;
		if ( 0 < uiBodyLength )
		{
			ptrNetPack->vctBuffer.assign(itHeadPos, itHeadPos + uiNetPackLen);
		}
		// ������ݰ��뻺��
		recvedDatas.push_back(ptrNetPack);

		// ����ѽ��ܵ����ݰ�
		vctBuffer.erase(itHeadPos, itHeadPos + uiNetPackLen);

		// vector�����ѱ䶯��iterator״̬������

		// ������һ������
		bIsBuffNeedProcessed = true;

	} while ( bIsBuffNeedProcessed );

	return 0;
}

/*
�������У����Ƿ���ȷ

@return true : ��ȷ
@return false : ����
*/
bool BinaryPacketAssembler::IsPackageCheckSumValid(std::shared_ptr<simutgw::BinaryNetPack>& pBinaryNetPack)
{
	// static const std::string ftag("BinaryPacketAssembler::IsPackageValid() ");

	uint32_t uiPointPos = 0;

	if ( nullptr == pBinaryNetPack )
	{
		return false;
	}

	// �������Ƿ�Ϊ0
	if ( 0 == pBinaryNetPack->ui32BodyLength )
	{
		// �ͱ�Ƶ��Ƿ�һ��
		if ( 0 == pBinaryNetPack->vctBuffer.size() )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if ( 4 >= pBinaryNetPack->vctBuffer.size() )
	{
		// ����4λ�����Ϸ�
		return false;
	}
	else
	{
		uiPointPos = HEAD_SIZE + TAIL_SIZE + pBinaryNetPack->ui32BodyLength;
		if ( pBinaryNetPack->vctBuffer.size() != uiPointPos )
		{
			// û������У��͵�λ��
			return false;
		}
	}

	return true;

	/*
	// ������Ϣ��У����Ƿ���ȷ
	uint32_t uiCalcedCheckSum = GenerateCheckSum(pBinaryNetPack->vctBuffer, 0, pBinaryNetPack->ui32BodyLength + HEAD_SIZE);

	uiPointPos = HEAD_SIZE + pBinaryNetPack->ui32BodyLength;
	simutgw::binary::BINARY_TAIL msgTail;
	memcpy(&msgTail, &( pBinaryNetPack->vctBuffer )[uiPointPos], TAIL_SIZE);

	uint32_t uiPackageCheckSum = ntohl(msgTail.Checksum);

	if ( uiCalcedCheckSum != uiPackageCheckSum )
	{
		return false;
	}

	return true;
	*/
}

/*
������� Logon ��Ϣ����Ӧ��

@return 0 : �ɹ�
@return -1 : ʧ��
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_Logon(
struct simutgw::binary::BINARY_LOGON& msgobj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_Logon() ");

	// BodyLength ��Ϣ�峤��
	uint32_t uiBodyLength = sizeof(simutgw::binary::BINARY_LOGON);

	out_vctData.resize(uiBodyLength + HEAD_SIZE);

	simutgw::binary::BINARY_HEAD head;
	head.MsgType = htonl(simutgw::BINARY::MsgType_Logon1);
	head.BodyLength = htonl(uiBodyLength);

	memcpy(&( out_vctData[0] ), &head, HEAD_SIZE);

	size_t uiPos = HEAD_SIZE;
	memcpy(&( out_vctData[uiPos] ), &msgobj, sizeof(simutgw::binary::BINARY_LOGON));

	// ������Ϣ��У���
	uint32_t uiCalcedCheckSum = GenerateCheckSum(out_vctData, 0, uiBodyLength + HEAD_SIZE);
	uint32_t uiNetCheckSum = htonl(uiCalcedCheckSum);
	char szCheck[sizeof(uint32_t)] = { 0 };
	memcpy(szCheck, &uiNetCheckSum, sizeof(uint32_t));

	for ( int i = 0; i < 4; ++i )
	{
		out_vctData.push_back(szCheck[i]);
	}

	return 0;
}

/*
������� ƽ̨״̬��Ϣ Platform State Info ��Ϣ����Ӧ��

@return 0 : �ɹ�
@return -1 : ʧ��
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_PlatformStateInfo(
struct simutgw::binary::PLATFORM_STATE_INFO& msgobj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_PlatformStateInfo() ");

	// BodyLength ��Ϣ�峤��
	uint32_t uiBodyLength = sizeof(simutgw::binary::PLATFORM_STATE_INFO);

	out_vctData.resize(uiBodyLength + HEAD_SIZE);

	simutgw::binary::BINARY_HEAD head;
	head.MsgType = htonl(simutgw::BINARY::MsgType_PlatformStateInfo6);
	head.BodyLength = htonl(uiBodyLength);

	memcpy(&( out_vctData[0] ), &head, HEAD_SIZE);

	size_t uiPos = HEAD_SIZE;
	memcpy(&( out_vctData[uiPos] ), &msgobj, sizeof(simutgw::binary::PLATFORM_STATE_INFO));

	// ������Ϣ��У���
	uint32_t uiCalcedCheckSum = GenerateCheckSum(out_vctData, 0, uiBodyLength + HEAD_SIZE);
	uint32_t uiNetCheckSum = htonl(uiCalcedCheckSum);
	char szCheck[sizeof(uint32_t)] = { 0 };
	memcpy(szCheck, &uiNetCheckSum, sizeof(uint32_t));

	for ( int i = 0; i < sizeof(uint32_t); ++i )
	{
		out_vctData.push_back(szCheck[i]);
	}

	return 0;
}

/*
������� ���� Heartbeat ��Ϣ����Ӧ��

@return 0 : �ɹ�
@return -1 : ʧ��
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_Heartbeat(std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_Heartbeat() ");

	// BodyLength ��Ϣ�峤��
	uint32_t uiBodyLength = 0;

	out_vctData.resize(uiBodyLength + HEAD_SIZE);

	simutgw::binary::BINARY_HEAD head;
	head.MsgType = htonl(simutgw::BINARY::MsgType_HeartBeat3);
	head.BodyLength = htonl(0);

	memcpy(&( out_vctData[0] ), &head, HEAD_SIZE);

	// ������Ϣ��У���
	uint32_t uiCalcedCheckSum = GenerateCheckSum(out_vctData, 0, uiBodyLength + HEAD_SIZE);
	uint32_t uiNetCheckSum = htonl(uiCalcedCheckSum);
	char szCheck[sizeof(uint32_t)] = { 0 };
	memcpy(szCheck, &uiNetCheckSum, sizeof(uint32_t));

	for ( int i = 0; i < sizeof(uint32_t); ++i )
	{
		out_vctData.push_back(szCheck[i]);
	}

	return 0;
}

/*
������� ƽ̨״̬ͬ�� Report Synchronization ��Ϣ����Ӧ��

@return 0 : �ɹ�
@return -1 : ʧ��
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_ReportSynch(struct simutgw::binary::REPORT_SYNCHRONIZATION& execReportSynchObj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_ReportSynch() ");

	// BodyLength ��Ϣ�峤��
	uint32_t uiBodyLength = sizeof(simutgw::binary::REPORT_SYNCHRONIZATION);

	out_vctData.resize(uiBodyLength + HEAD_SIZE);

	simutgw::binary::BINARY_HEAD head;
	head.MsgType = htonl(simutgw::BINARY::MsgType_ReportSynchronization5);
	head.BodyLength = htonl(uiBodyLength);

	memcpy(&( out_vctData[0] ), &head, HEAD_SIZE);

	size_t uiPos = HEAD_SIZE;
	memcpy(&( out_vctData[uiPos] ), &execReportSynchObj, sizeof(simutgw::binary::REPORT_SYNCHRONIZATION));
	
	// ������Ϣ��У���
	uint32_t uiCalcedCheckSum = GenerateCheckSum(out_vctData, 0, uiBodyLength + HEAD_SIZE);
	uint32_t uiNetCheckSum = htonl(uiCalcedCheckSum);
	char szCheck[sizeof(uint32_t)] = { 0 };
	memcpy(szCheck, &uiNetCheckSum, sizeof(uint32_t));

	for ( int i = 0; i < sizeof(uint32_t); ++i )
	{
		out_vctData.push_back(szCheck[i]);
	}
	return 0;
}

/*
������� NewOrder_100101 ��Ϣ����Ӧ��

@return 0 : �ɹ�
@return -1 : ʧ��
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_NewOrder_100101(
struct simutgw::binary::NEW_ORDER& newOrderObj,
struct simutgw::binary::NEW_ORDER_EXT_100101& newOrder_extObj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_NewOrder_100101() ");

	// BodyLength ��Ϣ�峤��
	uint32_t uiBodyLength = sizeof(simutgw::binary::NEW_ORDER) + sizeof(simutgw::binary::NEW_ORDER_EXT_100101);

	out_vctData.resize(uiBodyLength + HEAD_SIZE);

	simutgw::binary::BINARY_HEAD head;
	head.MsgType = htonl(simutgw::BINARY::MsgType_NewOrder100101);
	head.BodyLength = htonl(uiBodyLength);

	memcpy(&( out_vctData[0] ), &head, HEAD_SIZE);

	size_t uiPos = HEAD_SIZE;
	memcpy(&( out_vctData[uiPos] ), &newOrderObj, sizeof(simutgw::binary::NEW_ORDER));

	uiPos += sizeof(simutgw::binary::NEW_ORDER);
	memcpy(&( out_vctData[uiPos] ), &newOrder_extObj, sizeof(simutgw::binary::NEW_ORDER_EXT_100101));

	// ������Ϣ��У���
	uint32_t uiCalcedCheckSum = GenerateCheckSum(out_vctData, 0, uiBodyLength + HEAD_SIZE);
	uint32_t uiNetCheckSum = htonl(uiCalcedCheckSum);
	char szCheck[sizeof(uint32_t)] = { 0 };
	memcpy(szCheck, &uiNetCheckSum, sizeof(uint32_t));

	for ( int i = 0; i < sizeof(uint32_t); ++i )
	{
		out_vctData.push_back(szCheck[i]);
	}
	return 0;
}


/*
������� ExecReport_200102 ��Ϣ����Ӧ��

@return 0 : �ɹ�
@return -1 : ʧ��
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_ExecReport_200102(
struct simutgw::binary::EXECUTION_REPORT& execRepotObj,
struct simutgw::binary::EXECUTION_REPORT_EXT_200102& execRepot_extObj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_ExecReport_200102() ");

	// BodyLength ��Ϣ�峤��
	uint32_t uiBodyLength = sizeof(simutgw::binary::EXECUTION_REPORT) + sizeof(simutgw::binary::EXECUTION_REPORT_EXT_200102);

	out_vctData.resize(uiBodyLength + HEAD_SIZE);

	simutgw::binary::BINARY_HEAD head;
	head.MsgType = htonl(simutgw::BINARY::MsgType_ExecutionReport200102);
	head.BodyLength = htonl(uiBodyLength);

	memcpy(&( out_vctData[0] ), &head, HEAD_SIZE);

	size_t uiPos = HEAD_SIZE;
	memcpy(&( out_vctData[uiPos] ), &execRepotObj, sizeof(simutgw::binary::EXECUTION_REPORT));

	uiPos += sizeof(simutgw::binary::EXECUTION_REPORT);
	memcpy(&( out_vctData[uiPos] ), &execRepot_extObj, sizeof(simutgw::binary::EXECUTION_REPORT_EXT_200102));

	// ������Ϣ��У���
	uint32_t uiCalcedCheckSum = GenerateCheckSum(out_vctData, 0, uiBodyLength + HEAD_SIZE);
	uint32_t uiNetCheckSum = htonl(uiCalcedCheckSum);
	char szCheck[sizeof(uint32_t)] = { 0 };
	memcpy(szCheck, &uiNetCheckSum, sizeof(uint32_t));

	for ( int i = 0; i < sizeof(uint32_t); ++i )
	{
		out_vctData.push_back(szCheck[i]);
	}
	return 0;
}

/*
������� ExecReport_200115 ��Ϣ����Ӧ��

@return 0 : �ɹ�
@return -1 : ʧ��
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_ExecReport_Matched_200115(
struct simutgw::binary::EXECUTION_REPORT_MATCHED& execRepotObj,
struct simutgw::binary::EXECUTION_REPORT_MATCHED_EXT_200115& execRepot_extObj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_ExecReport_Matched_200115() ");

	// BodyLength ��Ϣ�峤��
	uint32_t uiBodyLength = sizeof(simutgw::binary::EXECUTION_REPORT_MATCHED) + sizeof(simutgw::binary::EXECUTION_REPORT_MATCHED_EXT_200115);

	out_vctData.resize(uiBodyLength + HEAD_SIZE);

	simutgw::binary::BINARY_HEAD head;
	head.MsgType = htonl(simutgw::BINARY::MsgType_MatchedExecutionReport200115);
	head.BodyLength = htonl(uiBodyLength);

	memcpy(&( out_vctData[0] ), &head, HEAD_SIZE);

	size_t uiPos = HEAD_SIZE;
	memcpy(&( out_vctData[uiPos] ), &execRepotObj, sizeof(simutgw::binary::EXECUTION_REPORT_MATCHED));

	uiPos += sizeof(simutgw::binary::EXECUTION_REPORT_MATCHED);
	memcpy(&( out_vctData[uiPos] ), &execRepot_extObj, sizeof(simutgw::binary::EXECUTION_REPORT_MATCHED_EXT_200115));

	// ������Ϣ��У���
	uint32_t uiCalcedCheckSum = GenerateCheckSum(out_vctData, 0, uiBodyLength + HEAD_SIZE);
	uint32_t uiNetCheckSum = htonl(uiCalcedCheckSum);
	char szCheck[sizeof(uint32_t)] = { 0 };
	memcpy(szCheck, &uiNetCheckSum, sizeof(uint32_t));

	for ( int i = 0; i < sizeof(uint32_t); ++i )
	{
		out_vctData.push_back(szCheck[i]);
	}
	return 0;
}