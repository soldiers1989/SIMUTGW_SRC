#ifndef __PROC_SOCKET_MSG_H__
#define __PROC_SOCKET_MSG_H__

#include <string>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "config/conf_net_msg.h"

#include "order/StockHelper.h"

#include "tool_file/TgwDBFOperHelper.h"

enum SocketMsgType
{
	Socket_MsgType_AddAccount = 99,
	Socket_MsgType_Add = 100,
	Socket_MsgType_Truncate = 101,
	Socket_MsgType_ClostAccount = 102,
	Socket_MsgType_Modify = 103,
	Socket_MsgType_ReLoad = 104,
	Socket_MsgType_MatchMode_ControlCMD = 105,
	Socket_MsgType_Settlement = 106
};

struct DBF_UserStockAsset
{
	enum StockHelper::StockType enType;
	std::string strTrader;
	std::string strAccount;
	std::string strSecuritySeat;
	std::string strStockID;
	std::string strQty;
	std::string strTradeMarket;

	void clear()
	{
		strTrader.clear();
		strAccount.clear();
		strSecuritySeat.clear();
		strStockID.clear();
		strQty.clear();
		strTradeMarket.clear();
	}
};

/*
	Socket消息处理类
	*/
class ProcSocketMsg
{
public:
	ProcSocketMsg();
	virtual ~ProcSocketMsg();

	/*
		 消息处理
		 Return:
		 0 -- 成功
		 -1 -- 失败
		 */
	static int ProcMsg(const std::string& in_strMsg,
		uint64_t ui64ReportIndex, std::string& out_strReport);

	/*
	执行成功消息
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int MakeExecSuccessMsg(uint64_t in_ui64SeqNum,
		uint64_t ui64ReportIndex, std::string& out_strReport);

	/*
	执行失败消息
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int MakeExecFailMsg(uint64_t in_ui64SeqNum, uint64_t ui64ReportIndex,
		std::string& out_strReport, const std::string& in_strText);

private:
	/*
		解析命令消息
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int MsgParse(const std::string& in_strMsg, struct simutgw::SocketMsgInfo& out_msgInfo,
		std::string& out_strError);


	/*
	执行成交模式切换控制命令
	*/
	static int ExecMatchModeControlCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
		执行开户
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int ExecAddAccount(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);
	/*
	执行销户
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int ExecCloseAccount(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
		执行Add类型命令
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int ExecAddCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
		执行增加用户资金命令
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int ExecCmd_Add_UserFund(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
		执行增加用户股份命令
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int ExecCmd_Add_UserSecurity(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
		执行清空操作
		Return:
		0 -- 成功
		-1 -- 失败
		*/
	static int ExecCmdTruncate(std::string& out_strError);

	/*
	执行Modify类型命令
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int ExecModifyCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
	执行修改资金命令
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int ExecCmd_Modify_UserFund(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
	执行修改股份命令
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int ExecCmd_Modify_UserSecurity(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
	执行加载文件操作
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int ExecReloadCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
	读取zqye文件的信息
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int Get_SHzqye_Asset(const std::string& in_strFileName,
		const std::string& in_strTrader,
		std::vector<DBF_UserStockAsset>& out_vecAsset);

	/*
	zqye的一条记录转换成DBF_UserStockAsset结构
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int Trans_SHZqye(const std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& in_mapData,
		const std::string& in_strTrader,
		DBF_UserStockAsset& out_Asset);

	/*
	读取sjsdz文件的信息
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int Get_Sjsdz_Asset(const std::string& in_strFileName,
		const std::string& in_strTrader,
		std::vector<DBF_UserStockAsset>& out_vecAsset);

	/*
	sjsdz的一条记录转换成DBF_UserStockAsset结构
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int Trans_Sjsdz(const std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& in_mapData,
		const std::string& in_strTrader,
		DBF_UserStockAsset& out_Asset);

	/*
	取一行dbf数据的value，转换成string value
	*/
	static int GetDBFRowData(std::string& strValue, const TgwDBFOperHelper_DF::DataInRow& cdata);

	/*
	插入数据到stock_asset/stock_etf_asset
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int RecordAsset(std::vector<DBF_UserStockAsset>& in_vecAsset);

	/*
	mysql更新语句
	Return:
	0 -- 成功
	-1 -- 失败
	*/
	static int MysqlUpdate(const std::string& in_strQuery);

	/*
		查询account表的资金余额
		Return:
		0 -- 成功
		1 -- 无记录
		-1 -- 失败
		*/
	static int LookUpAccountBalance(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& strBalance, std::string& strHK_Balance, std::string& strUS_Balance,
		std::string& strIsClose);

	/*
		查询stock_asset表的资金余额
		Return:
		0 -- 成功
		1 -- 无记录
		-1 -- 失败
		*/
	static int LookUpAccountStock(const struct simutgw::SocketMsgInfo& in_msgInfo, uint64_t& ui64cQuantity);

	/*
	执行 生成清算文件 命令
	*/
	static int ExecCmd_Settlement(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);
};

#endif