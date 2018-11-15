#ifndef __BINARY_MSG_TYPE_DEFINE_H__
#define __BINARY_MSG_TYPE_DEFINE_H__

#include <stdint.h>

namespace simutgw
{
	namespace BINARY
	{
		// 登录
		const uint32_t MsgType_Logon1 = 1;

		// 登出
		const uint32_t MsgType_Logout2 = 2;

		// 心跳
		const uint32_t MsgType_HeartBeat3 = 3;

		// 业务拒绝
		const uint32_t MsgType_BusinessReject4 = 4;

		// 回报同步
		const uint32_t MsgType_ReportSynchronization5 = 5;

		// 平台状态
		const uint32_t MsgType_PlatformStateInfo6 = 6;

		// 回报结束
		const uint32_t MsgType_ReportFinished7 = 7;

		// 新订单 1xxx01
		const uint32_t MsgType_NewOrder100101 = 100101; //现货（股票，基金，债券等）集中竞价交易申报
		const uint32_t MsgType_NewOrder100201 = 100201; //质押式回购交易申报
		const uint32_t MsgType_NewOrder100301 = 100301; //债券分销申报
		const uint32_t MsgType_NewOrder100401 = 100401; //期权集中竞价交易申报
		const uint32_t MsgType_NewOrder100501 = 100501; //协议交易定价申报\协议交易点击成申报
		const uint32_t MsgType_NewOrder100601 = 100601; //以收盘价交易的盘后定价交易申报\以成交量加权平均价交易的盘后定价交易申报
		const uint32_t MsgType_NewOrder100701 = 100701; //转融通证券出借非约定申报
		const uint32_t MsgType_NewOrder101201 = 101201; //ETF实时申购赎回申报
		const uint32_t MsgType_NewOrder101301 = 101301; //网上发行认购申报
		const uint32_t MsgType_NewOrder101401 = 101401; //配股认购申报
		const uint32_t MsgType_NewOrder101501 = 101501; //债券转股申报\债券回售申报
		const uint32_t MsgType_NewOrder101601 = 101601; //期权行权申报
		const uint32_t MsgType_NewOrder101701 = 101701; //开放式基金申购赎回申报
		const uint32_t MsgType_NewOrder101801 = 101801; //要约收购预受要约申报\要约收购解除预受要约申报
		const uint32_t MsgType_NewOrder101901 = 101901; //质押式回购质押申报\质押式回购解押申报
		const uint32_t MsgType_NewOrder102201 = 102201; //黄金ETF实物申购赎回申报（该委托只能由上海黄金交易所申报）
		const uint32_t MsgType_NewOrder102301 = 102301; //权证行权申报
		const uint32_t MsgType_NewOrder102601 = 102601; //备兑锁定申报\备兑解锁申报
		const uint32_t MsgType_NewOrder102701 = 102701; //转处置扣券申报\转处置还券申报
		const uint32_t MsgType_NewOrder102801 = 102801; //垫券申报\还券申报
		const uint32_t MsgType_NewOrder102901 = 102901; //待清偿扣划客户申报\待清偿扣划自营申报
		const uint32_t MsgType_NewOrder103101 = 103101; //分级基金实时分拆申报\分级基金实时合并申报
		const uint32_t MsgType_NewOrder103301 = 103301; //债券质押式三方回购出入库申报  
		const uint32_t MsgType_NewOrder106301 = 106301; //港股通申报 国际市场互联平台

		// 撤单请求
		const uint32_t MsgType_CancelRequest190007 = 190007;  //撤单请求

		// 撤单响应失败
		const uint32_t MsgType_CancelReject290008 = 290008;   //撤单响应失败

		//订单响应及撤单成功执行报告  2xxx02
		const uint32_t MsgType_ExecutionReport200102 = 200102; //现货（股票，基金，债券等）集中竞价交易执行报告
		const uint32_t MsgType_ExecutionReport200202 = 200202; //质押式回购交易执行报告
		const uint32_t MsgType_ExecutionReport200302 = 200302; //债券分销执行报告
		const uint32_t MsgType_ExecutionReport200402 = 200402; //期权集中竞价交易执行报告
		const uint32_t MsgType_ExecutionReport200502 = 200502; //协议交易定价执行报告\协议交易点击成交执行报告
		const uint32_t MsgType_ExecutionReport200602 = 200602; //以收盘价交易的盘后定价交易执行报告\以成交量加权平均价交易的盘后定价交易执行报告
		const uint32_t MsgType_ExecutionReport200702 = 200702; //转融通证券出借非约定执行报告
		const uint32_t MsgType_ExecutionReport201202 = 201202; //股票/债券/货币ETF实时申购赎回执行报告\  黄金ETF现金申购赎回执行报告
		const uint32_t MsgType_ExecutionReport201302 = 201302; //网上发行认购执行报告
		const uint32_t MsgType_ExecutionReport201402 = 201402; //配股认购执行报告
		const uint32_t MsgType_ExecutionReport201502 = 201502; //债券转股执行报告\债券回售执行报告
		const uint32_t MsgType_ExecutionReport201602 = 201602; //期权行权执行报告
		const uint32_t MsgType_ExecutionReport201702 = 201702; //开放式基金申购赎回执行报告
		const uint32_t MsgType_ExecutionReport201802 = 201802; //要约收购预受要约执行报告\要约收购解除预受要约执行报告
		const uint32_t MsgType_ExecutionReport201902 = 201902; //质押式回购质押执行报告\质押式回购解押执行报告
		const uint32_t MsgType_ExecutionReport202202 = 202202; //黄金ETF实物申购赎回执行报告
		const uint32_t MsgType_ExecutionReport202302 = 202302; //权证行权执行报告
		const uint32_t MsgType_ExecutionReport202602 = 202602; //备兑锁定执行报告\备兑解锁执行报告
		const uint32_t MsgType_ExecutionReport202702 = 202702; //转处置扣券执行报告\转处置还券执行报告
		const uint32_t MsgType_ExecutionReport202802 = 202802; //垫券执行报告\还券执行报告
		const uint32_t MsgType_ExecutionReport202902 = 202902; //待清偿扣划客户申报执行报告\待清偿扣划自营申报执行报告
		const uint32_t MsgType_ExecutionReport203102 = 203102; //分级基金实时分拆申报\分级基金实时合并申报
		const uint32_t MsgType_ExecutionReport206302 = 206302; //港股通订单执行报告

		// 订单成交执行报告 2xxx15
		const uint32_t MsgType_MatchedExecutionReport200115 = 200115; //现货（股票，基金，债券等）集中竞价交易执行报告
		const uint32_t MsgType_MatchedExecutionReport200215 = 200215; //质押式回购交易执行报告
		const uint32_t MsgType_MatchedExecutionReport200315 = 200315; //债券分销执行报告
		const uint32_t MsgType_MatchedExecutionReport200415 = 200415; //期权集中竞价交易执行报告
		const uint32_t MsgType_MatchedExecutionReport200515 = 200515; //协议交易定价执行报告\协议交易点击成交执行报告
		const uint32_t MsgType_MatchedExecutionReport200615 = 200615; //以收盘价交易的盘后定价交易执行报告\以成交量加权平均价交易的盘后定价交易执行报告
		const uint32_t MsgType_MatchedExecutionReport200715 = 200715; //转融通证券出借非约定执行报告
		const uint32_t MsgType_MatchedExecutionReport202615 = 202615; //备兑锁定执行报告\备兑解锁执行报告
		const uint32_t MsgType_MatchedExecutionReport206315 = 206315; //港股通订单执行报告

		// 报价 1xxx05
		const uint32_t MsgType_Quote100405 = 100405;//期权集中竞价交易报价申报
		const uint32_t MsgType_Quote100505 = 100505;//协议交易报价申报

		// 报价状态回报  2xxx06
		const uint32_t MsgType_QuoteStatusReport200406 = 200406;//期权集中竞价交易报价状态
		const uint32_t MsgType_QuoteStatusReport200506 = 200506;//协议交易报价状态回报

		// 询价申报 1xxx17
		const uint32_t MsgType_QuoteRequest100417 = 100417;//期权集中竞价交易询价申报

		// 询价申报接受响应 2xxx17
		const uint32_t MsgType_QuoteRequestAck200417 = 200417;//期权集中竞价交易询价申报

		// 询价申报拒绝响应 2xxx18
		const uint32_t MsgType_QuoteRequestReject200418 = 200418;//期权集中竞价交易询价申报

		// 意向申报 1xxx09
		const uint32_t MsgType_IndicationOfInterest100509 = 100509;//协议交易意向申报

		// 意向申报响应 2xxx10
		const uint32_t MsgType_QuoteResponse200510 = 200510;//协议交易意向申报响应

		// 成交申报 1xxx03
		const uint32_t MsgType_TradeCapture100503 = 100503;//协议交易双方协议成交申报
		const uint32_t MsgType_TradeCapture100703 = 100703;//转融通证券出借约定申报
		const uint32_t MsgType_TradeCapture100803 = 100803;//资产管理计划份额转让
		const uint32_t MsgType_TradeCapture100903 = 100903;//股票质押式回购
		const uint32_t MsgType_TradeCapture101003 = 101003;//约定购回
		const uint32_t MsgType_TradeCapture101103 = 101103;//质押式报价回购
		const uint32_t MsgType_TradeCapture103003 = 103003;//债券质押式协议回购
		const uint32_t MsgType_TradeCapture103203 = 103203;//债券质押式三方回购交易申报


		// 成交申报 成交申报响应 2xxx04
		const uint32_t MsgType_TradeCaptureReportAck200504 = 200504;//协议交易双方协议成交申报响应
		const uint32_t MsgType_TradeCaptureReportAck200704 = 200704;//转融通证券出借约定申报响应
		const uint32_t MsgType_TradeCaptureReportAck200804 = 200804;//资产管理计划份额转让成交申报响应
		const uint32_t MsgType_TradeCaptureReportAck200904 = 200904;//股票质押式回购成交申报响应
		const uint32_t MsgType_TradeCaptureReportAck201004 = 201004;//约定购回成交申报响应
		const uint32_t MsgType_TradeCaptureReportAck201104 = 201104;//质押式报价回购成交申报响应
		const uint32_t MsgType_TradeCaptureReportAck203004 = 203004;//债券质押式协议回购成交申报响应
		const uint32_t MsgType_TradeCaptureReportAck203204 = 203204;//债券质押式三方回购成交申报响应

		// 成交确认 2xxx03
		const uint32_t MsgType_TradeCaptureReport200503 = 200503;//协议交易双方协议成交申报确认
		const uint32_t MsgType_TradeCaptureReport200703 = 200703;//转融通证券出借约定申报确认
		const uint32_t MsgType_TradeCaptureReport203003 = 203003;//债券质押式协议回购成交申报确认
		const uint32_t MsgType_TradeCaptureReport203203 = 203203;//债券质押式协议回购成交申报确认

		// 注册
		const uint32_t MsgType_Designation102099 = 102099;//注册

		// 注册执行报告
		const uint32_t MsgType_DesignationReport202098 = 202098;//注册执行报告

		// 投票
		const uint32_t MsgType_Evote102197 = 102197; //投票

		// 投票执行报告
		const uint32_t MsgType_EvoteReport202196 = 202196;//投票执行报告

		// 密码服务
		const uint32_t MsgType_PasswordService102489 = 102489;// 密码服务

		// 密码服务执行报告
		const uint32_t MsgType_PasswordServiceReport202488 = 202488;//密码服务执行报告

		// 保证金查询
		const uint32_t MsgType_MarginQuery102587 = 102587;// 保证金查询

		// 保证金查询结果
		const uint32_t MsgType_MarginQueryResult202586 = 202586;//保证金查询结果

		const uint32_t MsgType_ForwardReport203220 = 203220;  //转发成交申报
	};
};
#endif