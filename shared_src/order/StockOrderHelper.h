#ifndef __STOCK_ORDER_HELPER_H__
#define __STOCK_ORDER_HELPER_H__

#include <string>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif

#include "order/define_order_msg.h"


/*
下单消息处理类
*/
class StockOrderHelper
{
public:
	StockOrderHelper(void);
	virtual ~StockOrderHelper(void);

	/*
	验证下单数据的合法性

	Return :
	0 -- 合法
	非0 -- 不合法
	*/
	static int OrderMsgValidate(std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg,
		int& out_iStockId);

	/*
	转换为屏幕打印检视数据

	*/
	static std::string& OrderToScreenOut(const std::shared_ptr<struct simutgw::OrderMessage>& order,
		std::string& out_strScreenOut);

	/*
	根据股票代码获取股票交易类型
	"0" -- A股
	"1" -- B股
	"2" -- 融资交易
	"3" -- 融券交易

	深市A股的代码是以000打头。
	深圳B股的代码是以200打头。

	沪市A股的代码是以600、601或603打头。
	沪市B股的代码是以900打头。

	Return:
	0 -- 成功
	非0 -- 失败
	*/
	static int CheckOrder_TradeType(std::shared_ptr<struct simutgw::OrderMessage> &io_OrderMsg);

	/*
	判断订单属于哪个交易规则

	@param const std::map<uint64_t, std::string>& in_mapLinkRules : 成交配置和通道的关系

	Return:
	0 -- 成功
	非0 -- 失败
	*/
	static int GetOrderMatchRule(std::shared_ptr<struct simutgw::OrderMessage> &io_OrderMsg,
		const std::map<uint64_t, uint64_t>& in_mapLinkRules);

private:

};

#endif