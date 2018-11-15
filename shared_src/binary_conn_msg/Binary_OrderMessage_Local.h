#ifndef __BINARY_ORDER_MESSAGE_LOCAL_H__
#define __BINARY_ORDER_MESSAGE_LOCAL_H__

#include <memory>
#include <string>
#include <stdint.h>
namespace simutgw
{
	namespace binary
	{
		// 新订单 New Order
		struct NEW_ORDER_LOCAL
		{
			// Standard Header 消息头 MsgType = 1xxx01
			// 应用标识 char ApplID[3];
			std::string ApplID;

			// 申报交易单元 char SubmittingPBUID[6];
			std::string SubmittingPBUID;

			// 证券代码 char SecurityID[8];
			std::string SecurityID;

			// 证券代码源 102 = 深圳证券交易所 char SecurityIDSource[4];
			std::string SecurityIDSource;

			// 订单所有者类型
			uint16_t OwnerType;

			// 结算机构代码 char ClearingFirm[2];
			std::string ClearingFirm;

			// 委托时间
			int64_t TransactTime;

			// 用户私有信息 char UserInfo[8];
			std::string UserInfo;

			// 客户订单编号 char ClOrdID[10];
			std::string ClOrdID;

			// 证券账户 char AccountID[12];
			std::string AccountID;

			// 营业部代码 char BranchID[4];
			std::string BranchID;

			// 订单限定 char OrderRestrictions[4];
			std::string OrderRestrictions;

			// 买卖方向 char Side;
			char Side;

			// 订单类别 1 表示市价 2 表示限价 U 本方最优
			char OrdType;

			// 订单数量
			int64_t OrderQty;
			// 价格
			int64_t Price;
			// 各业务扩展字段
			// Extend Fields 

			NEW_ORDER_LOCAL()
			{
				OwnerType = 0;
				TransactTime = 0;
				Side = '\0';
				OrdType = '\0';
				OrderQty = 0;
				Price = 0;
			}
		};

		// 现货集中竞价交易业务新订单扩展字段 100101
		struct NEW_ORDER_EXT_100101_LOCAL
		{
			// 止损价
			int64_t StopPx;
			// 最低成交数量
			int64_t MinQty;
			// 最多成交价位数
			// 0 表示不限制成交价位数
			uint16_t MaxPriceLevels;
			// 订单有效时间类型
			char TimeInForce;
			// 信用标识
			// 1 = Cash，普通交易
			// 2 = Open，融资融券开仓
			// 3 = Close，融资融券平仓
			char CashMargin;

			NEW_ORDER_EXT_100101_LOCAL()
			{
				StopPx = 0;
				MinQty = 0;
				MaxPriceLevels = 0;
				TimeInForce = '\0';
				CashMargin = '\0';
			}
		};
	};
};

#endif