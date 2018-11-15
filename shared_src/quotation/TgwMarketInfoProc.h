#ifndef __TGW_MARKET_INFO_PROC_H__
#define __TGW_MARKET_INFO_PROC_H__

#include "simutgw_flowwork/FlowWorkBase.h"

#include "config/conf_msg.h"

#include "util/EzLog.h"

/*
撮合交易模拟平台的行情处理类
*/
class TgwMarketInfoProc : public FlowWorkBase
{
	//
	// Members
	//

	// 市场交易容量的Key是否已创立
	// true -- 已创立
	// false -- 未创立
public:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	static bool m_bIsMarketTradeVolumnCreated;

	//
	// Functions
	//
public:
	TgwMarketInfoProc(void);
	virtual ~TgwMarketInfoProc(void);

	//从高速行情网关写入的Redis读取行情
	int ReadRealMarketInfo();

protected:

	/* 
	计算和上次行情之间的交易差
	Param :
	bool& out_bIsRecordBefore :
	true -- 该行情之前在Redis中存在
	false -- 该行情之前在Redis中不存在

	Return:
	0 -- 计算
	非0 -- 计算失败
	*/
	int CalcQuotationGap(const struct AStockQuot& struAstockQuot, 
		uint64_t& out_ui64GapCjsl, simutgw::uint64_t_Money& out_ui64mGapCjje,
		bool& out_bIsRecordBefore );

	// 储存自用行情
	int StoreSelfUseQuotation(const struct AStockQuot& in_struAstockQuot, 
		const simutgw::uint64_t_Money& in_ui64mGapCjsl, const simutgw::uint64_t_Money& in_ui64mGapCjje,
		bool in_bIsRecordBefore);

	virtual int TaskProc(void);

private:
};


#endif