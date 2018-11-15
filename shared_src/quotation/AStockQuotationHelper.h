#ifndef __A_STOCK_QUOTATION_HELPER_H__
#define __A_STOCK_QUOTATION_HELPER_H__

#include "boost/date_time.hpp"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif

using namespace std;

#include "config/conf_msg.h"

/*
A股行情处理助手
*/
struct AStockQuot
{
	/*
	{"value":"{\"ZQDM\":\"300571\",\"ZQMC\":\"\",\"CJSL\":\"536524.00\",\"CJJE\":\"73703408.0700\",
	\"ZJJG\":\"136.400000\",\"ZRSP\":\"135.1000\",\"JRKP\":\"134.500000\",
	\"ZGJG\":\"139.800000\",\"ZDJG\":\"134.500000\",\"CJBS\":\"2065\",\"SJW1\":\"136.000000\",
	\"SJW2\":\"136.500000\",\"SJW3\":\"136.600000\",\"SJW4\":\"136.630000\",\"SJW5\":\"136.640000\",
	\"SSL1\":\"100.00\",\"SSL2\":\"600.00\",\"SSL3\":\"300.00\",\"SSL4\":\"100.00\",\"SSL5\":\"100.00\",
	\"BJW1\":\"135.760000\",\"BJW2\":\"135.610000\",\"BJW3\":\"135.600000\",\"BJW4\":\"135.580000\",\"BJW5\":\"135.560000\",
	\"BSL1\":\"100.00\",\"BSL2\":\"300.00\",\"BSL3\":\"800.00\",\"BSL4\":\"1000.00\",\"BSL5\":\"100.00\",
	\"SYL1\":\"211.800000\",\"SYL2\":\"0\",\"TPBZ\":\"F\",\"hqktype\":\"SZSTEP.W.010\",\"OrigTime\":\"2017-04-13 14:01:06.000\"}"};
	*/

	// 昨日收盘价(整型，单位分)
	// \"ZRSP\":\"135.1000\",
	simutgw::uint64_t_Money zrsp;

	// 今日最高涨幅
	simutgw::uint64_t_Money maxgain;

	// 今日最低跌幅
	simutgw::uint64_t_Money minfall;

	// 今日开盘价(整型，单位厘)
	// \"JRKP\":\"134.500000\",
	simutgw::uint64_t_Money jrkp;

	// 最高成交价(整型，单位厘) --变动
	// \"ZGJG\":\"139.800000\",
	simutgw::uint64_t_Money zgjg;

	// 最低成交价(整型，单位厘) --变动
	// \"ZDJG\":\"134.500000\",
	simutgw::uint64_t_Money zdjg;

	// 最近成交价(整型，单位厘) --现价 变动
	// \"ZJJG\":\"136.400000\",
	simutgw::uint64_t_Money zjjg;

	// 成交数量 --变动
	// \"CJSL\":\"536524.00\",
	uint64_t cjsl;

	// 成交金额(整型，单位厘) --变动
	// \"CJJE\":\"73703408.0700\",
	simutgw::uint64_t_Money cjje;

	// 市盈率1 --变动
	// \"SYL1\":\"211.800000\",
	double SYL1;

	// 市盈率2 --变动
	// \"SYL2\":\"0\",
	double SYL2;

	// 成交笔数 --变动
	// \"CJBS\":\"2065\",
	uint64_t cjbs;

	// 卖出价格1(整型，单位厘) --变动
	// \"SJW1\":\"136.000000\",
	simutgw::uint64_t_Money SJW1;

	// 卖出价格2(整型，单位厘)
	// \"SJW2\":\"136.500000\",
	simutgw::uint64_t_Money SJW2;

	// 卖出价格3(整型，单位厘)
	// \"SJW3\":\"136.600000\",
	simutgw::uint64_t_Money SJW3;

	// 卖出价格4(整型，单位厘)
	// \"SJW4\":\"136.630000\",
	simutgw::uint64_t_Money SJW4;

	// 卖出价格5(整型，单位厘)
	// \"SJW5\":\"136.640000\",
	simutgw::uint64_t_Money SJW5;

	// 卖出数量1 --变动
	// \"SSL1\":\"100.00\",
	uint64_t SSL1;

	// 卖出数量2 --变动
	// \"SSL2\":\"600.00\",
	uint64_t SSL2;

	// 卖出数量3 --变动
	// \"SSL3\":\"300.00\",
	uint64_t SSL3;

	// 卖出数量4 --变动
	// \"SSL4\":\"100.00\",
	uint64_t SSL4;

	// 卖出数量5 --变动
	// \"SSL5\":\"100.00\",
	uint64_t SSL5;

	// 买入价格1(整型，单位厘) --变动
	// \"BJW1\":\"135.760000\",
	simutgw::uint64_t_Money BJW1;

	// 买入价格2(整型，单位厘) --变动
	// \"BJW2\":\"135.610000\",
	simutgw::uint64_t_Money BJW2;

	// 买入价格3(整型，单位厘) --变动
	// \"BJW3\":\"135.600000\",
	simutgw::uint64_t_Money BJW3;

	// 买入价格4(整型，单位厘) --变动
	// \"BJW4\":\"135.580000\",
	simutgw::uint64_t_Money BJW4;

	// 买入价格5(整型，单位厘) --变动
	// \"BJW5\":\"135.560000\",
	simutgw::uint64_t_Money BJW5;

	// 买入数量1 --变动
	// \"BSL1\":\"100.00\",
	uint64_t BSL1;

	// 买入数量2 --变动
	// \"BSL2\":\"300.00\",
	uint64_t BSL2;

	// 买入数量3 --变动
	// \"BSL3\":\"800.00\",
	uint64_t BSL3;

	// 买入数量4 --变动
	// \"BSL4\":\"1000.00\",
	uint64_t BSL4;

	// 买入数量5 --变动
	// \"BSL5\":\"100.00\",
	uint64_t BSL5;

	// 行情的市场类型
	// 0 -- 深圳行情
	// 1 -- 上海行情
	int hqmarket;

	// 行情时间 
	boost::posix_time::ptime timehqsj;

	// 证券代码
	// \"ZQDM\":\"300571\",	
	string zqdm;

	// 证券名称
	// \"ZQMC\":\"\",
	string zqmc;

	// 行情时间
	// \"OrigTime\":\"2017-04-13 14:01:06.000\"
	string hqsj;

	// 行情数据类型 标识字母MD加类型编号 上交所行情专用
	// MDStreamID
	// MD002 表示股票（A、B股）行情数据格式类型；
	// MD003 表示债券行情数据格式类型；
	// MD004 表示基金行情数据格式类型；
	string MDStreamID;

	// 行情类型
	// \"hqktype\":\"SZSTEP.W.010\",
	string hqktype;

	// 停盘标志 --变动
	// \"TPBZ\":\"F\",
	string TPBZ;

	// PriceUpperLimit(涨停价)
	// PriceUpperLimit为999999999.9999表示无涨停价格限制		
	string PriceUpperLimit;

	// PriceLowerLimit（跌停价）
	// PriceLowerLimit为-999999999.9999表示无跌停价格限制 
	string PriceLowerLimit;

	// 原始字符串
	string OriginStr;
};

namespace AStockQuotName
{
	/*
	{"value":"{\"ZQDM\":\"300571\",\"ZQMC\":\"\",\"CJSL\":\"536524.00\",\"CJJE\":\"73703408.0700\",
	\"ZJJG\":\"136.400000\",\"ZRSP\":\"135.1000\",\"JRKP\":\"134.500000\",
	\"ZGJG\":\"139.800000\",\"ZDJG\":\"134.500000\",\"CJBS\":\"2065\",\"SJW1\":\"136.000000\",
	\"SJW2\":\"136.500000\",\"SJW3\":\"136.600000\",\"SJW4\":\"136.630000\",\"SJW5\":\"136.640000\",
	\"SSL1\":\"100.00\",\"SSL2\":\"600.00\",\"SSL3\":\"300.00\",\"SSL4\":\"100.00\",\"SSL5\":\"100.00\",
	\"BJW1\":\"135.760000\",\"BJW2\":\"135.610000\",\"BJW3\":\"135.600000\",\"BJW4\":\"135.580000\",\"BJW5\":\"135.560000\",
	\"BSL1\":\"100.00\",\"BSL2\":\"300.00\",\"BSL3\":\"800.00\",\"BSL4\":\"1000.00\",\"BSL5\":\"100.00\",
	\"SYL1\":\"211.800000\",\"SYL2\":\"0\",\"TPBZ\":\"F\",\"hqktype\":\"SZSTEP.W.010\",\"OrigTime\":\"2017-04-13 14:01:06.000\"}"};
	*/

	// 昨日收盘价
	// \"ZRSP\":\"135.1000\",
	const string zrsp("ZRSP");

	// 今日最高涨幅
	const string maxgain("maxgain");

	// 今日最低跌幅
	const string minfall("minfall");

	// 今日开盘价
	// \"JRKP\":\"134.500000\",
	const string jrkp("JRKP");

	// 最高成交价 --变动
	// \"ZGJG\":\"139.800000\",
	const string zgjg("ZGJG");

	// 最低成交价 --变动
	// \"ZDJG\":\"134.500000\",
	const string zdjg("ZDJG");

	// 最近成交价 --现价 变动
	// \"ZJJG\":\"136.400000\",
	const string zjjg("ZJJG");

	// 成交数量 --变动
	// \"CJSL\":\"536524.00\",
	const string cjsl("CJSL");

	// 成交金额 --变动
	// \"CJJE\":\"73703408.0700\",
	const string cjje("CJJE");

	// 市盈率1 --变动
	// \"SYL1\":\"211.800000\",
	const string SYL1("SYL1");

	// 市盈率2 --变动
	// \"SYL2\":\"0\",
	const string SYL2("SYL2");

	// 成交笔数 --变动
	// \"CJBS\":\"2065\",
	const string cjbs("CJBS");

	// 卖出价格1 --变动
	// \"SJW1\":\"136.000000\",
	const string SJW1("SJW1");

	// 卖出价格2
	// \"SJW2\":\"136.500000\",
	const string SJW2("SJW2");

	// 卖出价格3
	// \"SJW3\":\"136.600000\",
	const string SJW3("SJW3");

	// 卖出价格4
	// \"SJW4\":\"136.630000\",
	const string SJW4("SJW4");

	// 卖出价格5
	// \"SJW5\":\"136.640000\",
	const string SJW5("SJW5");

	// 卖出数量1 --变动
	// \"SSL1\":\"100.00\",
	const string SSL1("SSL1");

	// 卖出数量2 --变动
	// \"SSL2\":\"600.00\",
	const string SSL2("SSL2");

	// 卖出数量3 --变动
	// \"SSL3\":\"300.00\",
	const string SSL3("SSL3");

	// 卖出数量4 --变动
	// \"SSL4\":\"100.00\",
	const string SSL4("SSL4");

	// 卖出数量5 --变动
	// \"SSL5\":\"100.00\",
	const string SSL5("SSL5");

	// 买入价格1 --变动
	// \"BJW1\":\"135.760000\",
	const string BJW1("BJW1");

	// 买入价格2 --变动
	// \"BJW2\":\"135.610000\",
	const string BJW2("BJW2");

	// 买入价格3 --变动
	// \"BJW3\":\"135.600000\",
	const string BJW3("BJW3");

	// 买入价格4 --变动
	// \"BJW4\":\"135.580000\",
	const string BJW4("BJW4");

	// 买入价格5 --变动
	// \"BJW5\":\"135.560000\",
	const string BJW5("BJW5");

	// 买入数量1 --变动
	// \"BSL1\":\"100.00\",
	const string BSL1("BSL1");

	// 买入数量2 --变动
	// \"BSL2\":\"300.00\",
	const string BSL2("BSL2");

	// 买入数量3 --变动
	// \"BSL3\":\"800.00\",
	const string BSL3("BSL3");

	// 买入数量4 --变动
	// \"BSL4\":\"1000.00\",
	const string BSL4("BSL4");

	// 买入数量5 --变动
	// \"BSL5\":\"100.00\",
	const string BSL5("BSL5");

	// 行情的市场类型
	// 1 -- 深圳行情
	// 2 -- 上海行情
	const string hqmarket("hqmarket");

	// 证券代码
	// \"ZQDM\":\"300571\",	
	const string zqdm("ZQDM");

	// 证券名称
	// \"ZQMC\":\"\",
	const string zqmc("ZQMC");

	// 行情时间
	// \"OrigTime\":\"2017-04-13 14:01:06.000\"
	const string hqsj("OrigTime");

	// 行情数据类型 标识字母MD加类型编号 上交所行情专用
	// MDStreamID
	const string MDStreamID("MDStreamID");

	// 行情类型
	// \"hqktype\":\"SZSTEP.W.010\",
	const string hqktype("hqktype");

	// 停盘标志 --变动
	// \"TPBZ\":\"F\",
	const string TPBZ("TPBZ");

	// PriceUpperLimit(涨停价)
	const string PriceUpperLimit("PriceUpperLimit");

	// PriceLowerLimit（跌停价）
	const string PriceLowerLimit("PriceLowerLimit");

	// 原始字符串
	const string OriginStr("OriginStr");
}

class AStockQuotationHelper
{
	//
	// Members
	//
public:

	//
	// Functions
	//
public:
	AStockQuotationHelper(void);
	virtual ~AStockQuotationHelper(void);

	// 清空结构体内容
	static int EmptyContent(struct AStockQuot& inout_astockQuot);

	// 解析Redis获取的RedisString
	static int DecodeJsonStr(const string& strMsg, struct AStockQuot& out_astockQuot);

	/*
	解析StockQuotation的合法性
	Return :
	0 -- 解析成功
	小于0 -- 解析失败
	大于0 -- 非股票行情
	*/
	static int Validate(struct AStockQuot& io_astockQuot);

	static string& ToRedisString(struct AStockQuot& astockQuot, string& out_strRedisValue);

	/*
	根据股票代码和市场，判断是否为上证指数

	上海市场，以000打头的股票代码都是指数
	Return :
	true -- 指数
	false -- 非指数

	*/
	static bool ValidateIndexNumber(const string& in_strIndexNumber);
};

#endif