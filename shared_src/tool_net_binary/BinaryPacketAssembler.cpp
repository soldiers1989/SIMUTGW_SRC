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
		// 剩余的数据端不够包头数据长度，还需继续缓存
		return 1;
	}

	// net package head pos
	std::vector<uint8_t>::iterator itHeadPos = vctBuffer.end();

	// 数据buffer是否搜索完毕
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

		// 从头开始解析
		// 消息头
		// 域名    字段描述
		// MsgType  消息类型
		// BodyLength 消息体长度
		simutgw::binary::BINARY_HEAD msgHead;
		memcpy(&msgHead, &vctBuffer[0], HEAD_SIZE);

		uint32_t uiMsgType = ntohl(msgHead.MsgType);
		uint32_t uiBodyLength = ntohl(msgHead.BodyLength);
		
		uint32_t uiNetPackLen = HEAD_SIZE + uiBodyLength + TAIL_SIZE;
		// check if vector is bigger enough
		if ( uiNetPackLen > vctBuffer.size() )
		{
			// 剩余的数据端不够包头+包身+包尾数据长度，还需继续缓存
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
		// 添加数据包入缓存
		recvedDatas.push_back(ptrNetPack);

		// 清除已接受的数据包
		vctBuffer.erase(itHeadPos, itHeadPos + uiNetPackLen);

		// vector内容已变动，iterator状态需重置

		// 继续下一轮搜索
		bIsBuffNeedProcessed = true;

	} while ( bIsBuffNeedProcessed );

	return 0;
}

/*
检验包的校验和是否正确

@return true : 正确
@return false : 错误
*/
bool BinaryPacketAssembler::IsPackageCheckSumValid(std::shared_ptr<simutgw::BinaryNetPack>& pBinaryNetPack)
{
	// static const std::string ftag("BinaryPacketAssembler::IsPackageValid() ");

	uint32_t uiPointPos = 0;

	if ( nullptr == pBinaryNetPack )
	{
		return false;
	}

	// 看长度是否为0
	if ( 0 == pBinaryNetPack->ui32BodyLength )
	{
		// 和标称的是否一致
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
		// 仅有4位，不合法
		return false;
	}
	else
	{
		uiPointPos = HEAD_SIZE + TAIL_SIZE + pBinaryNetPack->ui32BodyLength;
		if ( pBinaryNetPack->vctBuffer.size() != uiPointPos )
		{
			// 没有留下校验和的位置
			return false;
		}
	}

	return true;

	/*
	// 计算消息体校验和是否正确
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
组二进制 Logon 消息的响应包

@return 0 : 成功
@return -1 : 失败
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_Logon(
struct simutgw::binary::BINARY_LOGON& msgobj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_Logon() ");

	// BodyLength 消息体长度
	uint32_t uiBodyLength = sizeof(simutgw::binary::BINARY_LOGON);

	out_vctData.resize(uiBodyLength + HEAD_SIZE);

	simutgw::binary::BINARY_HEAD head;
	head.MsgType = htonl(simutgw::BINARY::MsgType_Logon1);
	head.BodyLength = htonl(uiBodyLength);

	memcpy(&( out_vctData[0] ), &head, HEAD_SIZE);

	size_t uiPos = HEAD_SIZE;
	memcpy(&( out_vctData[uiPos] ), &msgobj, sizeof(simutgw::binary::BINARY_LOGON));

	// 计算消息体校验和
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
组二进制 平台状态消息 Platform State Info 消息的响应包

@return 0 : 成功
@return -1 : 失败
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_PlatformStateInfo(
struct simutgw::binary::PLATFORM_STATE_INFO& msgobj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_PlatformStateInfo() ");

	// BodyLength 消息体长度
	uint32_t uiBodyLength = sizeof(simutgw::binary::PLATFORM_STATE_INFO);

	out_vctData.resize(uiBodyLength + HEAD_SIZE);

	simutgw::binary::BINARY_HEAD head;
	head.MsgType = htonl(simutgw::BINARY::MsgType_PlatformStateInfo6);
	head.BodyLength = htonl(uiBodyLength);

	memcpy(&( out_vctData[0] ), &head, HEAD_SIZE);

	size_t uiPos = HEAD_SIZE;
	memcpy(&( out_vctData[uiPos] ), &msgobj, sizeof(simutgw::binary::PLATFORM_STATE_INFO));

	// 计算消息体校验和
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
组二进制 心跳 Heartbeat 消息的响应包

@return 0 : 成功
@return -1 : 失败
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_Heartbeat(std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_Heartbeat() ");

	// BodyLength 消息体长度
	uint32_t uiBodyLength = 0;

	out_vctData.resize(uiBodyLength + HEAD_SIZE);

	simutgw::binary::BINARY_HEAD head;
	head.MsgType = htonl(simutgw::BINARY::MsgType_HeartBeat3);
	head.BodyLength = htonl(0);

	memcpy(&( out_vctData[0] ), &head, HEAD_SIZE);

	// 计算消息体校验和
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
组二进制 平台状态同步 Report Synchronization 消息的响应包

@return 0 : 成功
@return -1 : 失败
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_ReportSynch(struct simutgw::binary::REPORT_SYNCHRONIZATION& execReportSynchObj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_ReportSynch() ");

	// BodyLength 消息体长度
	uint32_t uiBodyLength = sizeof(simutgw::binary::REPORT_SYNCHRONIZATION);

	out_vctData.resize(uiBodyLength + HEAD_SIZE);

	simutgw::binary::BINARY_HEAD head;
	head.MsgType = htonl(simutgw::BINARY::MsgType_ReportSynchronization5);
	head.BodyLength = htonl(uiBodyLength);

	memcpy(&( out_vctData[0] ), &head, HEAD_SIZE);

	size_t uiPos = HEAD_SIZE;
	memcpy(&( out_vctData[uiPos] ), &execReportSynchObj, sizeof(simutgw::binary::REPORT_SYNCHRONIZATION));
	
	// 计算消息体校验和
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
组二进制 NewOrder_100101 消息的响应包

@return 0 : 成功
@return -1 : 失败
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_NewOrder_100101(
struct simutgw::binary::NEW_ORDER& newOrderObj,
struct simutgw::binary::NEW_ORDER_EXT_100101& newOrder_extObj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_NewOrder_100101() ");

	// BodyLength 消息体长度
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

	// 计算消息体校验和
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
组二进制 ExecReport_200102 消息的响应包

@return 0 : 成功
@return -1 : 失败
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_ExecReport_200102(
struct simutgw::binary::EXECUTION_REPORT& execRepotObj,
struct simutgw::binary::EXECUTION_REPORT_EXT_200102& execRepot_extObj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_ExecReport_200102() ");

	// BodyLength 消息体长度
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

	// 计算消息体校验和
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
组二进制 ExecReport_200115 消息的响应包

@return 0 : 成功
@return -1 : 失败
*/
int BinaryPacketAssembler::LocalPackToNetBuffer_ExecReport_Matched_200115(
struct simutgw::binary::EXECUTION_REPORT_MATCHED& execRepotObj,
struct simutgw::binary::EXECUTION_REPORT_MATCHED_EXT_200115& execRepot_extObj,
	std::vector<uint8_t>& out_vctData)
{
	// static const std::string ftag("BinaryPacketAssembler::LocalPackToNetBuffer_ExecReport_Matched_200115() ");

	// BodyLength 消息体长度
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

	// 计算消息体校验和
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