#ifndef __BINARY_MESSAGE_STRUCT_DEFINE_H__
#define __BINARY_MESSAGE_STRUCT_DEFINE_H__

#include <stdint.h>

namespace simutgw
{
	namespace binary
	{
		/*
		length
		*/
		// 发送方代码
		static const size_t SENDER_COMP_ID_LEN = 20;
		// 接收方代码
		static const size_t TARGET_COMP_ID_LEN = 20;
		// 密码
		static const size_t PASSWORD_LEN = 16;
		// 二进制协议版本，即通信版本号
		static const size_t DEFAULT_APPL_VER_ID_LEN = 32;
		// 文本
		static const size_t TEXT_LEN = 200;
		// 应用标识
		static const size_t APPL_ID_LEN = 3;
		// PBUID 交易单元代码
		static const size_t PBUID_LEN = 6;
		// 证券代码
		static const size_t SECURITY_ID_LEN = 8;
		// 证券代码源 102 = 深圳证券交易所
		static const size_t SECURITY_ID_SOURCE_LEN = 4;
		// 结算机构代码
		static const size_t CLEARING_FIRM_LEN = 2;
		// 用户私有信息
		static const size_t USER_INFO_LEN = 8;
		// 客户订单编号
		static const size_t CL_ORD_ID = 10;
		// 证券账户
		static const size_t ACCOUNT_ID_LEN = 12;
		// 营业部代码
		static const size_t BRANCH_ID_LEN = 4;
		// 订单限定
		static const size_t ORDER_RESTICTIONS_LEN = 4;
		// 买卖方向
		static const size_t SIDE_LEN = 1;
		// 订单类别
		static const size_t ORD_TYPE_LEN = 1;
		// 交易所订单编号 char OrderID[16];
		static const size_t ORDER_ID_LEN = 16;
		// 执行编号 char ExecID[16];
		static const size_t EXEC_ID_LEN = 16;

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 消息头
		// 域名    字段描述
		// MsgType  消息类型
		// BodyLength 消息体长度
		struct BINARY_HEAD
		{
			// 消息类型
			uint32_t MsgType;
			// 消息体长度
			uint32_t BodyLength;

			BINARY_HEAD()
			{
				memset(this, 0x00, sizeof(BINARY_HEAD));
			}
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 消息尾
		// 消息尾定义了消息的校验和，不加密传输。校验和的计算范围从消息头开始（包括消息头）一直到消息体结束。
		// 域名    字段描述
		// Checksum  校验和
		struct BINARY_TAIL
		{
			// 校验和
			uint32_t Checksum;

			BINARY_TAIL()
			{
				memset(this, 0x00, sizeof(BINARY_TAIL));
			}
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 登录消息 Logon
		struct BINARY_LOGON
		{
			// 发送方代码
			char SenderCompId[20];
			// 接收方代码
			char TargetCompId[20];

			// 心跳间隔，单位为秒。订单管理系统系统登陆时提供给交易网关
			uint32_t HeartBtInt;

			// 密码
			char Password[16];
			// 二进制协议版本，即通信版本号
			// 填写为 n.xy
			char DefaultApplVerID[32];

			BINARY_LOGON()
			{
				memset(this, 0x00, sizeof(BINARY_LOGON));
			}
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 注销消息 Logout
		struct BINARY_LOGOUT
		{

			// 退出时的会话状态
			// 0 = 会话活跃
			// 1 = 会话口令已更改
			// 2 = 将过期的会话口令
			// 3 = 新会话口令不符合规范
			// 4 = 会话退登完成
			// 5 = 不合法的用户名或口令
			// 6 = 账户锁定
			// 7 = 当前时间不允许登录
			// 8 = 口令过期
			// 9 = 收到的 MsgSeqNum(34)太小
			// 10 = 收到的 NextExpectedMsgSeqNum(789)太大.
			// 101 = 其他
			// 102 = 无效消息
			int32_t	SessionStatus;

			// 文本
			// 注销原因的进一步补充说明
			char Text[200];

			BINARY_LOGOUT()
			{
				memset(this, 0x00, sizeof(BINARY_LOGOUT));
			}
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 平台状态消息 Platform State Info
		struct PLATFORM_STATE_INFO
		{
			// 平台号 uInt16
			// 1 = 现货集中竞价交易平台
			// 2 = 综合金融服务平台
			// 3 = 非交易处理平台
			// 4 = 衍生品集中竞价交易平台
			// 5 = 国际市场互联平台
			uint16_t PlatformID;

			// 平台状态
			// 0 = PreOpen，未开放
			// 1 = OpenUpComing，即将开放
			// 2 = Open，开放
			// 3 = Halt，暂停开放
			// 4 = Close，关闭
			uint16_t PlatformState;

			PLATFORM_STATE_INFO()
			{
				memset(this, 0x00, sizeof(PLATFORM_STATE_INFO));
			}
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 回报同步消息 Report Synchronization
		struct REPORT_SYNCHRONIZATION
		{
			// OMS 期望接收的下一条回报的记录号（回
			// 报记录号从1 开始连续编号）。
			// 收到本消息后，TGW 会从该记录号开始向
			// OMS 发送回报；如果没有收到该消息,
			// TGW 不会向OMS 发送回报。
			int64_t ReportIndex;

			REPORT_SYNCHRONIZATION()
			{
				memset(this, 0x00, sizeof(REPORT_SYNCHRONIZATION));
			}
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 新订单 New Order
		struct NEW_ORDER
		{
			// Standard Header 消息头 MsgType = 1xxx01
			// 应用标识
			char ApplID[3];
			// 申报交易单元
			char SubmittingPBUID[6];
			// 证券代码
			char SecurityID[8];
			// 证券代码源 102 = 深圳证券交易所
			char SecurityIDSource[4];
			// 订单所有者类型
			uint16_t OwnerType;
			// 结算机构代码
			char ClearingFirm[2];
			// 委托时间
			int64_t TransactTime;
			// 用户私有信息
			char UserInfo[8];
			// 客户订单编号
			char ClOrdID[10];
			// 证券账户
			char AccountID[12];
			// 营业部代码
			char BranchID[4];
			// 订单限定
			char OrderRestrictions[4];
			// 买卖方向
			char Side;
			// 订单类别
			// 1 表示市价
			// 2 表示限价
			// U 本方最优
			char OrdType;
			// 订单数量
			int64_t OrderQty;
			// 价格
			int64_t Price;
			// 各业务扩展字段
			// Extend Fields 

			NEW_ORDER()
			{
				memset(this, 0x00, sizeof(NEW_ORDER));
			}
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 现货集中竞价交易业务新订单扩展字段 100101
		struct NEW_ORDER_EXT_100101
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

			/*
			NEW_ORDER_EXT_100101()
			{
			memset(this, 0x00, sizeof(NEW_ORDER_EXT_100101));
			}
			*/
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 订单响应及撤单成功执行报告 Execution Report
		struct EXECUTION_REPORT
		{
			// Standard Header 消息头 MsgType = 2xxx02
			// 回报记录号
			int64_t ReportIndex;
			// 应用标识
			char ApplID[3];
			// 回报交易单元
			char ReportingPBUID[6];
			// 申报交易单元
			char SubmittingPBUID[6];
			// 证券代码
			char SecurityID[8];
			// 证券代码源
			char SecurityIDSource[4];
			// 订单所有者类型
			uint16_t OwnerType;
			// 结算机构代码
			char ClearingFirm[2];
			// 回报时间
			int64_t TransactTime;
			// 用户私有信息
			char UserInfo[8];
			// 交易所订单编号
			char OrderID[16];
			// 客户订单编号 如果是报价委托的成交则填写 QuoteMsgID
			char ClOrdID[10];
			// 原始订单客户订单编号
			char OrigClOrdID[10];
			// 执行编号
			char ExecID[16];
			// 执行类型
			char ExecType;
			// 订单状态
			char OrdStatus;
			// 撤单 / 拒绝原因代码
			uint16_t OrdRejReason;
			// 订单剩余数量
			int64_t LeavesQty;
			// 累计执行数量
			int64_t CumQty;
			// 买卖方向
			char Side;
			// 订单类别
			char OrdType;
			// 订单数量
			int64_t OrderQty;
			// 价格
			int64_t Price;
			// 证券账户
			char AccountID[12];
			// 营业部代码
			char BranchID[4];
			// 订单限定
			char OrderRestrictions[4];
			// Extend Fields 各业务扩展字段

			EXECUTION_REPORT()
			{
				memset(this, 0x00, sizeof(EXECUTION_REPORT));
			}
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 现货集中竞价交易业务执行报告扩展字段 200102
		struct EXECUTION_REPORT_EXT_200102
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

			EXECUTION_REPORT_EXT_200102()
			{
				memset(this, 0x00, sizeof(EXECUTION_REPORT_EXT_200102));
			}
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 订单成交执行报告 Execution Report
		struct EXECUTION_REPORT_MATCHED
		{
			// Standard Header 消息头 MsgType = 2xxx15
			// 回报记录号
			int64_t ReportIndex;
			// 应用标识
			char ApplID[3];
			// 回报交易单元
			char ReportingPBUID[6];
			// 申报交易单元
			char SubmittingPBUID[6];
			// 证券代码
			char SecurityID[8];
			// 证券代码源
			char SecurityIDSource[4];
			// 订单所有者类型
			uint16_t OwnerType;
			// 结算机构代码
			char ClearingFirm[2];
			// 回报时间
			int64_t TransactTime;
			// 用户私有信息
			char UserInfo[8];
			// 交易所订单编号
			char OrderID[16];
			// 客户订单编号 如果是报价委托的成交则填写 QuoteMsgID
			char ClOrdID[10];
			// 执行编号
			char ExecID[16];
			// 执行类型
			char ExecType;
			// 订单状态
			char OrdStatus;
			// 成交价
			int64_t LastPx;
			// 成交数量
			int64_t LastQty;
			// 订单剩余数量
			int64_t LeavesQty;
			// 累计执行数量
			int64_t CumQty;
			// 买卖方向
			char Side;
			// 证券账户
			char AccountID[12];
			// 营业部代码
			char BranchID[4];
			// Extend Fields 各业务扩展字段

			EXECUTION_REPORT_MATCHED()
			{
				memset(this, 0x00, sizeof(EXECUTION_REPORT_MATCHED));
			}
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

		// 将字节对齐方式设为1
#pragma pack(push,1)
		// 现货集中竞价交易业务成交执行报告扩展字段 200115
		struct EXECUTION_REPORT_MATCHED_EXT_200115
		{
			// 信用标识
			// 1 = Cash，普通交易
			// 2 = Open，融资融券开仓
			// 3 = Close，融资融券平仓
			char CashMargin;

			EXECUTION_REPORT_MATCHED_EXT_200115()
			{
				memset(this, 0x00, sizeof(EXECUTION_REPORT_MATCHED_EXT_200115));
			}
		};
		// 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)
	};
};


#endif