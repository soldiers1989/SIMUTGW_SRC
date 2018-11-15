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
网络数据包组包、拆包工具
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
	Binary计算校验和
	*/
	static uint32_t GenerateCheckSum(char *buf, uint32_t bufLen)
	{
		size_t idx = 0;
		uint32_t cks = 0;
		for ( idx = 0L, cks = 0; idx < bufLen; cks += (uint32_t) buf[idx++] );
		return ( cks % 256 );
	}

	/*
	Binary计算校验和

	@param const std::vector<uint8_t>& vctData : 数据段
	@param size_t iBeginPos : 开始计算位置
	@param uint32_t bufLen : 尾部长度
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
	检验包的校验和是否正确

	@return true : 正确
	@return false : 错误
	*/
	static bool IsPackageCheckSumValid(std::shared_ptr<simutgw::BinaryNetPack>& pBinaryNetPack);

	/*
	组二进制 Logon 消息的响应包

	@return 0 : 成功
	@return -1 : 失败
	*/
	static int LocalPackToNetBuffer_Logon(struct simutgw::binary::BINARY_LOGON& msgobj,
		std::vector<uint8_t>& out_vctData);

	/*
	组二进制 平台状态消息 Platform State Info 消息的响应包

	@return 0 : 成功
	@return -1 : 失败
	*/
	static int LocalPackToNetBuffer_PlatformStateInfo(struct simutgw::binary::PLATFORM_STATE_INFO& msgobj,
		std::vector<uint8_t>& out_vctData);

	/*
	组二进制 心跳 Heartbeat 消息的响应包

	@return 0 : 成功
	@return -1 : 失败
	*/
	static int LocalPackToNetBuffer_Heartbeat(std::vector<uint8_t>& out_vctData);

	/*
	组二进制 平台状态同步 Report Synchronization 消息的响应包

	@return 0 : 成功
	@return -1 : 失败
	*/
	static int LocalPackToNetBuffer_ReportSynch(struct simutgw::binary::REPORT_SYNCHRONIZATION& execReportSynchObj,
		std::vector<uint8_t>& out_vctData);

	/*
	组二进制 NewOrder_100101 消息的响应包

	@return 0 : 成功
	@return -1 : 失败
	*/
	static int LocalPackToNetBuffer_NewOrder_100101(
	struct simutgw::binary::NEW_ORDER& newOrderObj,
	struct simutgw::binary::NEW_ORDER_EXT_100101& newOrder_extObj,
		std::vector<uint8_t>& out_vctData);

	/*
	组二进制 ExecReport_200102 消息的响应包

	@return 0 : 成功
	@return -1 : 失败
	*/
	static int LocalPackToNetBuffer_ExecReport_200102(struct simutgw::binary::EXECUTION_REPORT& execRepotObj,
	struct simutgw::binary::EXECUTION_REPORT_EXT_200102& execRepot_extObj, std::vector<uint8_t>& out_vctData);

	/*
	组二进制 ExecReport_200115 消息的响应包

	@return 0 : 成功
	@return -1 : 失败
	*/
	static int LocalPackToNetBuffer_ExecReport_Matched_200115(
	struct simutgw::binary::EXECUTION_REPORT_MATCHED& execRepotObj,
	struct simutgw::binary::EXECUTION_REPORT_MATCHED_EXT_200115& execRepot_extObj,
		std::vector<uint8_t>& out_vctData);
};
#endif