#ifndef __TRADE_POLICY_MANAGE_H__
#define __TRADE_POLICY_MANAGE_H__

#include <memory>

#include "boost/thread/mutex.hpp"

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"

#include "util/EzLog.h"

#include "simutgw_config/config_define.h"

/*
����ϯλ���ײ��Դ���

���� -- STEP���� -- ϯλ -- ����
�Ϻ� -- DB���� -- ����
*/
struct TradePolicyCell
{
	//
	// Members
	//

	/*
	����ģʽ
	1 -- ѹ��ģʽ
	2 -- ����ģʽ
	3 -- ��ͨģʽ
	*/
	enum simutgw::SysRunMode iRunMode;

	// �Ƿ����ʽ�ɷ�
	// true -- ���
	// false -- �����
	bool bCheck_Assets;

	/*
	�ɽ�ģʽ
	0 -- Ĭ�ϣ���������
	1 -- �������飬ȫ���ɽ�
	2 -- �������飬�ֱʳɽ�
	3 -- �������飬�ҵ������ɽ����ɳ���
	4 -- ����
	*/
	enum simutgw::SysMatchMode iMatchMode;

	/*
	ʵ��ģ������ģʽ
	0 -- ���ݱ䶯�ɽ��ܽ��ͳɽ���������ľ���
	1 --
	2 -- ����ɽ���
	*/
	enum simutgw::QuotationType iQuotationMatchType;

	// �ֱʳɽ�����
	uint32_t iPart_Match_Num;

	// ����ر���
	std::string strSettleGroupName;

	// �ɽ����ú�ͨ���Ĺ�ϵ
	// Note:ʹ��map�Ƿ������
	std::map<uint64_t, uint64_t> mapLinkRules;

	// �ɽ�����ID
	uint64_t ui64RuleId;
	
	std::shared_ptr<struct MATCH_RULE_SH> ptrRule_Sh;

	std::shared_ptr<struct MATCH_RULE_SZ> ptrRule_Sz;

	//
	// Functions
	//
	TradePolicyCell(void)
	:strSettleGroupName("")
	{
		iRunMode = simutgw::SysRunMode::NormalMode;

		bCheck_Assets = false;

		iMatchMode = simutgw::SysMatchMode::EnAbleQuta;

		iQuotationMatchType = simutgw::QuotationType::AveragePrice;

		iPart_Match_Num = 2;

		ui64RuleId = 0;
	}

	TradePolicyCell(const enum simutgw::SysRunMode& in_RunMode, const bool in_checkAsset, const enum simutgw::SysMatchMode& in_MatchMode,
		const enum simutgw::QuotationType& in_quotType, const uint32_t in_Part_Match_Num)
		:strSettleGroupName("")
	{
		iRunMode = in_RunMode;

		bCheck_Assets = in_checkAsset;

		iMatchMode = in_MatchMode;

		iQuotationMatchType = in_quotType;

		iPart_Match_Num = in_Part_Match_Num;

		ui64RuleId = 0;
	}

	void copy(const TradePolicyCell& orig)
	{
		iRunMode = orig.iRunMode;

		bCheck_Assets = orig.bCheck_Assets;

		iMatchMode = orig.iMatchMode;

		iQuotationMatchType = orig.iQuotationMatchType;

		iPart_Match_Num = orig.iPart_Match_Num;

		strSettleGroupName = orig.strSettleGroupName;

		ui64RuleId = orig.ui64RuleId;

		ptrRule_Sh = orig.ptrRule_Sh;

		ptrRule_Sz = orig.ptrRule_Sz;
	}

	TradePolicyCell(const TradePolicyCell& orig)
	{
		copy(orig);
	}

	TradePolicyCell& operator=(const TradePolicyCell& orig)
	{
		copy(orig);
		return *this;
	}

	void Set(const enum simutgw::SysRunMode& in_RunMode, const bool in_checkAsset, const enum simutgw::SysMatchMode& in_MatchMode,
		const enum simutgw::QuotationType& in_quotType, const uint32_t in_Part_Match_Num)
	{
		iRunMode = in_RunMode;

		bCheck_Assets = in_checkAsset;

		iMatchMode = in_MatchMode;

		iQuotationMatchType = in_quotType;

		iPart_Match_Num = in_Part_Match_Num;
	}

	std::string& DebugOut(std::string& out_strDebug)
	{
		out_strDebug = " .TradePolicyCell-- RunMode=";
		// ����ģʽ		
		switch (iRunMode)
		{
		case simutgw::SysRunMode::PressureMode:
			// 1 --ѹ��ģʽ
			out_strDebug += "1,ѹ��ģʽ; ";
			break;

		case simutgw::SysRunMode::MiniMode:
			// 2 --����ģʽ
			out_strDebug += "2,����ģʽ; ";
			break;

		case simutgw::SysRunMode::NormalMode:
			// 3 --��ͨģʽ
			out_strDebug += "3,��ͨģʽ; ";
			break;
		}

		if (bCheck_Assets)
		{
			out_strDebug += "Check_Assets:true,��ȯ; ";
		}
		else
		{
			out_strDebug += "Check_Assets:false,����ȯ; ";
		}

		out_strDebug += "�ɽ�ģʽ:";
		switch (iMatchMode)
		{
		case simutgw::SysMatchMode::EnAbleQuta:
			// �ɽ�ģʽ����0:ʵ��ģ��
			out_strDebug += "0,EnAbleQuta ʵ��ģ��; ";
			break;

		case simutgw::SysMatchMode::SimulMatchAll:
			// �ɽ�ģʽ����1:����ģ�� - ȫ���ɽ�
			out_strDebug += "1,SimulMatchAll ����ģ��-ȫ���ɽ�; ";
			break;

		case simutgw::SysMatchMode::SimulMatchByDivide:
			// �ɽ�ģʽ����2:����ģ��-�ֱʳɽ�
			out_strDebug += "2,SimulMatchByDivide ����ģ��-�ֱʳɽ�; ";
			break;

		case simutgw::SysMatchMode::SimulNotMatch:
			// �ɽ�ģʽ����3:����ģ�� - �ҵ�(�ɳ���)
			out_strDebug += "3,SimulNotMatch ����ģ��-�ҵ�(�ɳ���); ";
			break;

		case simutgw::SysMatchMode::SimulErrMatch:
			// �ɽ�ģʽ����4:����ģ��-��
			out_strDebug += "4,SimulErrMatch ����ģ��-��; ";
			break;

		case simutgw::SysMatchMode::SimulMatchPart:
			// �ɽ�ģʽ����5:����ģ��-���ֳɽ�
			out_strDebug += "5,SimulMatchPart ����ģ��-���ֳɽ�; ";
			break;
		}

		out_strDebug += "ʵ��ģ������ģʽ:";
		switch (iQuotationMatchType)
		{
		case simutgw::QuotationType::AveragePrice:
			// ʵ��ģ������ģʽ����0:����ξ���
			out_strDebug += "0,AveragePrice ����ξ���; ";
			break;

		case simutgw::QuotationType::SellBuyPrice:
			// ʵ��ģ������ģʽ����1:��һ��һ�۸�
			out_strDebug += "1,SellBuyPrice ��һ��һ�۸�; ";
			break;

		case simutgw::QuotationType::RecentMatchPrice:
			// ʵ��ģ������ģʽ����2:����ɽ���
			out_strDebug += "2,RecentMatchPrice ����ɽ���; ";
			break;
		}

		out_strDebug += "Part_Match_Num=";
		std::string strItoa;
		sof_string::itostr(iPart_Match_Num, strItoa);
		out_strDebug += strItoa;

		out_strDebug += ", SettleGroup=[";
		out_strDebug += strSettleGroupName;
		out_strDebug += "], RuleId=";	

		sof_string::itostr(ui64RuleId, strItoa);
		out_strDebug += strItoa;

		return out_strDebug;
	}
};

/*
��������ϯλ�Ĵ���
*/
struct Conn_TradePolicyCell
{
	// Conncetion
	TradePolicyCell stSelf;
	std::map<std::string, std::shared_ptr<struct TradePolicyCell>> mapSeats;
};

class TradePolicyManage
{
	//
	// Members
	//
protected:
	// access mutex 
	// Shenzhen
	boost::mutex m_mutex_sz;

	// access mutex
	// Shanghai
	boost::mutex m_mutex_sh;

	// ���� ���� -- ���ײ���
	// ���� ���� -- ϯλ -- ���ײ���
	typedef std::map<std::string, std::shared_ptr<struct Conn_TradePolicyCell>> MAP_SZ_POLICY;
	MAP_SZ_POLICY m_mapSzPolicy;

	// �Ϻ� ���� -- ���ײ���
	typedef std::map<std::string, std::shared_ptr<struct TradePolicyCell>> MAP_SH_POLICY;
	MAP_SH_POLICY m_mapShPolicy;

	//
	// Functions
	//
public:
	TradePolicyManage();
	~TradePolicyManage();

	/*
	���������ý��ײ���

	@param const std::string& in_strConnId : ����Id
	@return 0 : ���óɹ�
	@return -1 : ����ʧ��
	*/
	int Set_Sz(const std::string& in_strConnId,
		const enum simutgw::SysRunMode& in_RunMode, const bool in_checkAsset,
		const enum simutgw::SysMatchMode& in_MatchMode, const enum simutgw::QuotationType& in_quotType,
		const uint32_t in_Part_Match_Num);

	/*
	�����ӡ�ϯλ���ý��ײ���

	@param const std::string& in_strConnId : ����Id
	@param const std::string& in_strSetName : ϯλ����
	@return 0 : ���óɹ�
	@return -1 : ����ʧ��
	*/
	int Set_Sz(const std::string& in_strConnId, const std::string& in_strSeatName,
		const enum simutgw::SysRunMode& in_RunMode, const bool in_checkAsset,
		const enum simutgw::SysMatchMode& in_MatchMode, const enum simutgw::QuotationType& in_quotType,
		const uint32_t in_Part_Match_Num);

	/*
	���������ý��ײ���

	@param const std::string& in_strConnId : ����Id
	@return 0 : ���óɹ�
	@return -1 : ����ʧ��
	*/
	int Set_Sh(const std::string& in_strConnId,
		const enum simutgw::SysRunMode& in_RunMode, const bool in_checkAsset,
		const enum simutgw::SysMatchMode& in_MatchMode, const enum simutgw::QuotationType& in_quotType,
		const uint32_t in_Part_Match_Num);


	/*
	�����ӻ�ȡ���ײ���

	@param const std::string& in_strConnId : ����Id

	@return 0 : ��ȡ�ɹ�
	@return 1 : ��ȡ����Ĭ��ֵ
	@return -1 : ��ȡʧ��
	*/
	int Get_Sz(const std::string& in_strConnId,
	enum simutgw::SysRunMode& out_RunMode, bool& out_checkAsset,
	enum simutgw::SysMatchMode& out_MatchMode, enum simutgw::QuotationType& out_quotType, uint32_t& out_Part_Match_Num);

	/*
	�����ӡ�ϯλ��ȡ���ײ���

	@param const std::string& in_strConnId : ����Id
	@param const std::string& in_strSetName : ϯλ����

	@return 0 : ��ȡ�ɹ�
	@return 1 : ��ȡ����Ĭ��ֵ
	@return -1 : ��ȡʧ��
	*/
	int Get_Sz(const std::string& in_strConnId, const std::string& in_strSeatName,
	enum simutgw::SysRunMode& out_RunMode, bool& out_checkAsset,
	enum simutgw::SysMatchMode& out_MatchMode, enum simutgw::QuotationType& out_quotType, uint32_t& out_Part_Match_Num);

	/*
	�����ӻ�ȡ���ײ���

	@param const std::string& in_strConnId : ����Id

	@return 0 : ��ȡ�ɹ�
	@return 1 : ��ȡ����Ĭ��ֵ
	@return -1 : ��ȡʧ��
	*/
	int Get_Sh(const std::string& in_strConnId,
	enum simutgw::SysRunMode& out_RunMode, bool& out_checkAsset,
	enum simutgw::SysMatchMode& out_MatchMode, enum simutgw::QuotationType& out_quotType, uint32_t& out_Part_Match_Num);

	/*
	Debug out
	*/
	std::string& DebugOut_Sz(std::string& out_strOut);

	/*
	Debug out
	*/
	std::string& DebugOut_Sh(std::string& out_strOut);

	/*
	Debug out
	*/
	std::string& DebugOut_Sz_Conn_TradePolicyCell(std::shared_ptr<struct Conn_TradePolicyCell>& ptr, std::string& out_strOut);

	/*
	Debug out
	*/
	std::string& DebugOut_TradePolicyCell(struct TradePolicyCell& obj, std::string& out_strOut);

	/*
	Debug out
	*/
	std::string& DebugOut_map(std::map<std::string, std::shared_ptr<struct TradePolicyCell>>& obj, std::string& out_strOut);

};


#endif