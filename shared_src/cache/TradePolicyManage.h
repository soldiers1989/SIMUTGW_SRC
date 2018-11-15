#ifndef __TRADE_POLICY_MANAGE_H__
#define __TRADE_POLICY_MANAGE_H__

#include <memory>

#include "boost/thread/mutex.hpp"

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"

#include "util/EzLog.h"

#include "simutgw_config/config_define.h"

/*
单个席位交易策略储存

深圳 -- STEP连接 -- 席位 -- 策略
上海 -- DB连接 -- 策略
*/
struct TradePolicyCell
{
	//
	// Members
	//

	/*
	运行模式
	1 -- 压力模式
	2 -- 极简模式
	3 -- 普通模式
	*/
	enum simutgw::SysRunMode iRunMode;

	// 是否检查资金股份
	// true -- 检查
	// false -- 不检查
	bool bCheck_Assets;

	/*
	成交模式
	0 -- 默认，启用行情
	1 -- 不看行情，全部成交
	2 -- 不看行情，分笔成交
	3 -- 不看行情，挂单、不成交，可撤单
	4 -- 错误单
	*/
	enum simutgw::SysMatchMode iMatchMode;

	/*
	实盘模拟行情模式
	0 -- 根据变动成交总金额和成交总量计算的均价
	1 --
	2 -- 最近成交价
	*/
	enum simutgw::QuotationType iQuotationMatchType;

	// 分笔成交笔数
	uint32_t iPart_Match_Num;

	// 清算池别名
	std::string strSettleGroupName;

	// 成交配置和通道的关系
	// Note:使用map是方便查找
	std::map<uint64_t, uint64_t> mapLinkRules;

	// 成交规则ID
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
		// 运行模式		
		switch (iRunMode)
		{
		case simutgw::SysRunMode::PressureMode:
			// 1 --压力模式
			out_strDebug += "1,压力模式; ";
			break;

		case simutgw::SysRunMode::MiniMode:
			// 2 --极简模式
			out_strDebug += "2,极简模式; ";
			break;

		case simutgw::SysRunMode::NormalMode:
			// 3 --普通模式
			out_strDebug += "3,普通模式; ";
			break;
		}

		if (bCheck_Assets)
		{
			out_strDebug += "Check_Assets:true,验券; ";
		}
		else
		{
			out_strDebug += "Check_Assets:false,不验券; ";
		}

		out_strDebug += "成交模式:";
		switch (iMatchMode)
		{
		case simutgw::SysMatchMode::EnAbleQuta:
			// 成交模式——0:实盘模拟
			out_strDebug += "0,EnAbleQuta 实盘模拟; ";
			break;

		case simutgw::SysMatchMode::SimulMatchAll:
			// 成交模式——1:仿真模拟 - 全部成交
			out_strDebug += "1,SimulMatchAll 仿真模拟-全部成交; ";
			break;

		case simutgw::SysMatchMode::SimulMatchByDivide:
			// 成交模式——2:仿真模拟-分笔成交
			out_strDebug += "2,SimulMatchByDivide 仿真模拟-分笔成交; ";
			break;

		case simutgw::SysMatchMode::SimulNotMatch:
			// 成交模式——3:仿真模拟 - 挂单(可撤单)
			out_strDebug += "3,SimulNotMatch 仿真模拟-挂单(可撤单); ";
			break;

		case simutgw::SysMatchMode::SimulErrMatch:
			// 成交模式——4:仿真模拟-错单
			out_strDebug += "4,SimulErrMatch 仿真模拟-错单; ";
			break;

		case simutgw::SysMatchMode::SimulMatchPart:
			// 成交模式——5:仿真模拟-部分成交
			out_strDebug += "5,SimulMatchPart 仿真模拟-部分成交; ";
			break;
		}

		out_strDebug += "实盘模拟行情模式:";
		switch (iQuotationMatchType)
		{
		case simutgw::QuotationType::AveragePrice:
			// 实盘模拟行情模式——0:区间段均价
			out_strDebug += "0,AveragePrice 区间段均价; ";
			break;

		case simutgw::QuotationType::SellBuyPrice:
			// 实盘模拟行情模式——1:买一卖一价格
			out_strDebug += "1,SellBuyPrice 买一卖一价格; ";
			break;

		case simutgw::QuotationType::RecentMatchPrice:
			// 实盘模拟行情模式——2:最近成交价
			out_strDebug += "2,RecentMatchPrice 最近成交价; ";
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
连接下属席位的储存
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

	// 深圳 连接 -- 交易策略
	// 深圳 连接 -- 席位 -- 交易策略
	typedef std::map<std::string, std::shared_ptr<struct Conn_TradePolicyCell>> MAP_SZ_POLICY;
	MAP_SZ_POLICY m_mapSzPolicy;

	// 上海 连接 -- 交易策略
	typedef std::map<std::string, std::shared_ptr<struct TradePolicyCell>> MAP_SH_POLICY;
	MAP_SH_POLICY m_mapShPolicy;

	//
	// Functions
	//
public:
	TradePolicyManage();
	~TradePolicyManage();

	/*
	按连接设置交易策略

	@param const std::string& in_strConnId : 连接Id
	@return 0 : 设置成功
	@return -1 : 设置失败
	*/
	int Set_Sz(const std::string& in_strConnId,
		const enum simutgw::SysRunMode& in_RunMode, const bool in_checkAsset,
		const enum simutgw::SysMatchMode& in_MatchMode, const enum simutgw::QuotationType& in_quotType,
		const uint32_t in_Part_Match_Num);

	/*
	按连接、席位设置交易策略

	@param const std::string& in_strConnId : 连接Id
	@param const std::string& in_strSetName : 席位名称
	@return 0 : 设置成功
	@return -1 : 设置失败
	*/
	int Set_Sz(const std::string& in_strConnId, const std::string& in_strSeatName,
		const enum simutgw::SysRunMode& in_RunMode, const bool in_checkAsset,
		const enum simutgw::SysMatchMode& in_MatchMode, const enum simutgw::QuotationType& in_quotType,
		const uint32_t in_Part_Match_Num);

	/*
	按连接设置交易策略

	@param const std::string& in_strConnId : 连接Id
	@return 0 : 设置成功
	@return -1 : 设置失败
	*/
	int Set_Sh(const std::string& in_strConnId,
		const enum simutgw::SysRunMode& in_RunMode, const bool in_checkAsset,
		const enum simutgw::SysMatchMode& in_MatchMode, const enum simutgw::QuotationType& in_quotType,
		const uint32_t in_Part_Match_Num);


	/*
	按连接获取交易策略

	@param const std::string& in_strConnId : 连接Id

	@return 0 : 获取成功
	@return 1 : 获取返回默认值
	@return -1 : 获取失败
	*/
	int Get_Sz(const std::string& in_strConnId,
	enum simutgw::SysRunMode& out_RunMode, bool& out_checkAsset,
	enum simutgw::SysMatchMode& out_MatchMode, enum simutgw::QuotationType& out_quotType, uint32_t& out_Part_Match_Num);

	/*
	按连接、席位获取交易策略

	@param const std::string& in_strConnId : 连接Id
	@param const std::string& in_strSetName : 席位名称

	@return 0 : 获取成功
	@return 1 : 获取返回默认值
	@return -1 : 获取失败
	*/
	int Get_Sz(const std::string& in_strConnId, const std::string& in_strSeatName,
	enum simutgw::SysRunMode& out_RunMode, bool& out_checkAsset,
	enum simutgw::SysMatchMode& out_MatchMode, enum simutgw::QuotationType& out_quotType, uint32_t& out_Part_Match_Num);

	/*
	按连接获取交易策略

	@param const std::string& in_strConnId : 连接Id

	@return 0 : 获取成功
	@return 1 : 获取返回默认值
	@return -1 : 获取失败
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