#ifndef __TGW_MARKET_INFO_PROC_H__
#define __TGW_MARKET_INFO_PROC_H__

#include "simutgw_flowwork/FlowWorkBase.h"

#include "config/conf_msg.h"

#include "util/EzLog.h"

/*
��Ͻ���ģ��ƽ̨�����鴦����
*/
class TgwMarketInfoProc : public FlowWorkBase
{
	//
	// Members
	//

	// �г�����������Key�Ƿ��Ѵ���
	// true -- �Ѵ���
	// false -- δ����
public:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	static bool m_bIsMarketTradeVolumnCreated;

	//
	// Functions
	//
public:
	TgwMarketInfoProc(void);
	virtual ~TgwMarketInfoProc(void);

	//�Ӹ�����������д���Redis��ȡ����
	int ReadRealMarketInfo();

protected:

	/* 
	������ϴ�����֮��Ľ��ײ�
	Param :
	bool& out_bIsRecordBefore :
	true -- ������֮ǰ��Redis�д���
	false -- ������֮ǰ��Redis�в�����

	Return:
	0 -- ����
	��0 -- ����ʧ��
	*/
	int CalcQuotationGap(const struct AStockQuot& struAstockQuot, 
		uint64_t& out_ui64GapCjsl, simutgw::uint64_t_Money& out_ui64mGapCjje,
		bool& out_bIsRecordBefore );

	// ������������
	int StoreSelfUseQuotation(const struct AStockQuot& in_struAstockQuot, 
		const simutgw::uint64_t_Money& in_ui64mGapCjsl, const simutgw::uint64_t_Money& in_ui64mGapCjje,
		bool in_bIsRecordBefore);

	virtual int TaskProc(void);

private:
};


#endif