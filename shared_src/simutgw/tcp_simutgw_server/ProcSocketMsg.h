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
	Socket��Ϣ������
	*/
class ProcSocketMsg
{
public:
	ProcSocketMsg();
	virtual ~ProcSocketMsg();

	/*
		 ��Ϣ����
		 Return:
		 0 -- �ɹ�
		 -1 -- ʧ��
		 */
	static int ProcMsg(const std::string& in_strMsg,
		uint64_t ui64ReportIndex, std::string& out_strReport);

	/*
	ִ�гɹ���Ϣ
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int MakeExecSuccessMsg(uint64_t in_ui64SeqNum,
		uint64_t ui64ReportIndex, std::string& out_strReport);

	/*
	ִ��ʧ����Ϣ
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int MakeExecFailMsg(uint64_t in_ui64SeqNum, uint64_t ui64ReportIndex,
		std::string& out_strReport, const std::string& in_strText);

private:
	/*
		����������Ϣ
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int MsgParse(const std::string& in_strMsg, struct simutgw::SocketMsgInfo& out_msgInfo,
		std::string& out_strError);


	/*
	ִ�гɽ�ģʽ�л���������
	*/
	static int ExecMatchModeControlCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
		ִ�п���
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int ExecAddAccount(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);
	/*
	ִ������
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ExecCloseAccount(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
		ִ��Add��������
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int ExecAddCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
		ִ�������û��ʽ�����
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int ExecCmd_Add_UserFund(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
		ִ�������û��ɷ�����
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int ExecCmd_Add_UserSecurity(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
		ִ����ղ���
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int ExecCmdTruncate(std::string& out_strError);

	/*
	ִ��Modify��������
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ExecModifyCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
	ִ���޸��ʽ�����
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ExecCmd_Modify_UserFund(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
	ִ���޸Ĺɷ�����
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ExecCmd_Modify_UserSecurity(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
	ִ�м����ļ�����
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ExecReloadCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);

	/*
	��ȡzqye�ļ�����Ϣ
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int Get_SHzqye_Asset(const std::string& in_strFileName,
		const std::string& in_strTrader,
		std::vector<DBF_UserStockAsset>& out_vecAsset);

	/*
	zqye��һ����¼ת����DBF_UserStockAsset�ṹ
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int Trans_SHZqye(const std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& in_mapData,
		const std::string& in_strTrader,
		DBF_UserStockAsset& out_Asset);

	/*
	��ȡsjsdz�ļ�����Ϣ
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int Get_Sjsdz_Asset(const std::string& in_strFileName,
		const std::string& in_strTrader,
		std::vector<DBF_UserStockAsset>& out_vecAsset);

	/*
	sjsdz��һ����¼ת����DBF_UserStockAsset�ṹ
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int Trans_Sjsdz(const std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& in_mapData,
		const std::string& in_strTrader,
		DBF_UserStockAsset& out_Asset);

	/*
	ȡһ��dbf���ݵ�value��ת����string value
	*/
	static int GetDBFRowData(std::string& strValue, const TgwDBFOperHelper_DF::DataInRow& cdata);

	/*
	�������ݵ�stock_asset/stock_etf_asset
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int RecordAsset(std::vector<DBF_UserStockAsset>& in_vecAsset);

	/*
	mysql�������
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int MysqlUpdate(const std::string& in_strQuery);

	/*
		��ѯaccount����ʽ����
		Return:
		0 -- �ɹ�
		1 -- �޼�¼
		-1 -- ʧ��
		*/
	static int LookUpAccountBalance(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& strBalance, std::string& strHK_Balance, std::string& strUS_Balance,
		std::string& strIsClose);

	/*
		��ѯstock_asset����ʽ����
		Return:
		0 -- �ɹ�
		1 -- �޼�¼
		-1 -- ʧ��
		*/
	static int LookUpAccountStock(const struct simutgw::SocketMsgInfo& in_msgInfo, uint64_t& ui64cQuantity);

	/*
	ִ�� ���������ļ� ����
	*/
	static int ExecCmd_Settlement(const struct simutgw::SocketMsgInfo& in_msgInfo,
		std::string& out_strError);
};

#endif