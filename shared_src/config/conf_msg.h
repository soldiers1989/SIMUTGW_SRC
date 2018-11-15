#ifndef __CONF_MSG_H__
#define __CONF_MSG_H__

#include <string>
#include <stdint.h>

namespace simutgw
{
	// 以分为单位的金额
	typedef uint64_t uint64_t_Money;

	//
	// STEP接口消息
	//
	/*
	ExecType
	C1 执行报告类型
	0=New，表示新订单
	4=Cancelled，表示已撤销
	8=Reject，表示已拒绝
	F=Trade，表示已成交
	*/
	static const std::string STEPMSG_EXECTYPE_NEW("0");
	static const std::string STEPMSG_EXECTYPE_CANCEL("4");
	static const std::string STEPMSG_EXECTYPE_REJECT("8");
	static const std::string STEPMSG_EXECTYPE_TRADE("F");

	/*
	OrdStatus
	C1 订单状态
	0=New，表示新订单
	1=Partially filled，表示部分成交
	2=Filled，表示全部成交
	4=Cancelled，表示已撤销
	8=Reject，表示已拒绝
	*/
	static const std::string STEPMSG_ORDSTATUS_NEW("0");
	static const std::string STEPMSG_ORDSTATUS_PART_FILL("1");
	static const std::string STEPMSG_ORDSTATUS_FILL("2");
	static const std::string STEPMSG_ORDSTATUS_CANCEL("4");
	static const std::string STEPMSG_ORDSTATUS_REJECT("8");

	/*
	MsgType
	D=新订单（ New Order）,买卖单
	F=撤单请求（ Order Cancel Request）
	8=执行报告（ Execution Report）
	9=撤单失败响应（ Cancel Reject）
	*/
	static const std::string STEPMSG_MSGTYPE_NEW_ORDER("D");
	static const std::string STEPMSG_MSGTYPE_ORDER_CACEL("F");
	static const std::string STEPMSG_MSGTYPE_EXECREPORT("8");
	static const std::string STEPMSG_MSGTYPE_CANCELREJECT("9");

	/*
	Side
	B=买
	S=卖
	G=借入
	F=出借
	D=申购
	E=赎回
	*/
	static const std::string STEPMSG_SIDE_BUY_B("B");	
	static const std::string STEPMSG_SIDE_BUY_1("1");
	static const std::string STEPMSG_SIDE_SELL_S("S");
	static const std::string STEPMSG_SIDE_SELL_2("2");
	static const std::string STEPMSG_SIDE_BORROW("G");
	static const std::string STEPMSG_SIDE_LEND("F");
	static const std::string STEPMSG_SIDE_CRT("D");
	static const std::string STEPMSG_SIDE_RDP("E");

	/*
	trade_market
	交易市场
	101 -- 上海
	102 -- 深圳	
	*/
	static const std::string TRADE_MARKET_SH("101"); // 上海
	static const std::string TRADE_MARKET_SZ("102"); // 深圳
	

	namespace SZ_ERRCODE
	{
		// 深交所回报 委托申报拒绝原因代码表
		// ERRORCODE无效账户
		static const std::string c20001("20001");

		// 20007 停牌/无效证券代码
		static const std::string c20007("20007");

		// 20022 股份余额不足
		static const std::string c20022("20022");

		// 20005 业务禁止 证券的业务开关闭
		static const std::string c20005("20005");

		// 20076 申报方式错误 竞价委托申报方错误，TimeInForce、OrdType、MaxPriceLevels、MinQty字段的组合不合法
		static const std::string c20076("20076");

		// 20101 应用标识错 应用标识错误
		static const std::string c20101("20101");

		// 20106 字段错误 字段取值错误
		static const std::string c20106("20106");

		// 20010 数量非法 委托数量不是数量单位的整数倍；分级基金分拆合并委托数量非配比的整数倍
		static const std::string c20010("20010");

		// 20009 价格错误 委托价格超过涨跌幅限制；盘后定价大宗交易委托价格不为指定价格；转融通证券出借委托的费率与公布的不一致
		static const std::string c20009("20009");

		// 20095 撤单请求错误 主动撤单请求的 applid 或证券 id 与原委托的不一致
		static const std::string c20095("20095");

		// 20064 资金不足 结算参与人可用资金不足
		static const std::string c20064("20064");
	};

	namespace SH_ERRCODE
	{
		// 上交所回报 委托申报拒绝原因代码表
		// 215  无效账户
		static const std::string c215("215");

		// 203 停牌/无效证券代码
		static const std::string c203("203");

		// 212 价格错误
		static const std::string c212("212");

		// 231 股份余额不足
		static const std::string c231("231");

		// 209	交易品种不对
		static const std::string c209("209");

		// 269	申报类型错误
		static const std::string c269("269");

		// 218	无效申报价格
		static const std::string c218("218");

		// 224	无效的申报数量
		static const std::string c224("224");
	};

	/*
	深交所 证券交易统计库SJSTJ.DBF
	*/
	struct Sz_SJSTJ
	{
		std::string str_AccountId;
		//序号 字段名 字段描述 类型 长度 备注
		// 1 TJXWDM 托管编码 C 6
		std::string str_TJXWDM;

		// 2 TJZQDM 证券代码 C 6
		std::string str_TJZQDM;

		// 3 TJMRGS 买入股数 N 12,0
		//uint64_t ui64_TJMRGS;
		std::string str_TJMRGS;

		// 4 TJMRZJ 买入资金 N 15,3
		//simutgw::uint64_t_Money ui64m_TJMRZJ;
		std::string str_TJMRZJ;

		// 5 TJMCGS 卖出股数 N 12,0
		//uint64_t ui64_TJMCGS;
		std::string str_TJMCGS;

		// 6 TJMCZJ 卖出资金 N 15,3
		//simutgw::uint64_t_Money ui64m_TJMCZJ;
		std::string str_TJMCZJ;

		// 7 TJBJSF 买经手费 N 15,3
		simutgw::uint64_t_Money ui64m_TJBJSF;

		// 8 TJSJSF 卖经手费 N 15,3
		simutgw::uint64_t_Money ui64m_TJSJSF;

		// 9 TJBYHS 买印花税 N 15,3
		simutgw::uint64_t_Money ui64m_TJBYHS;

		// 10 TJSYHS 卖印花税 N 15,3
		simutgw::uint64_t_Money ui64m_TJSYHS;

		// 11 TJBGHF 买过户费 N 15,3
		simutgw::uint64_t_Money ui64m_TJBGHF;

		// 12 TJSGHF 卖过户费 N 15,3
		simutgw::uint64_t_Money ui64m_TJSGHF;

		// 13 TJCJRQ 成交日期 D 8 CCYYMMDD
		std::string str_TJCJRQ;

		// 14 TJBYBZ 备用标志	C 1 结算参与人系统自用
	};
}

#endif