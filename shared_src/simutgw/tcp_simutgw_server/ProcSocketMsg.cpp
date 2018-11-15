#include "ProcSocketMsg.h"
#include "util/EzLog.h"
#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw_config/g_values_sys_run_config.h"

#include "tool_file/FileOperHelper.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"
#include "tool_string/sof_string.h"

#include "simutgw/settlement/Settlement.h"
#include "simutgw/stgw_config/sys_function.h"

#include "order/StockOrderHelper.h"


ProcSocketMsg::ProcSocketMsg()
{
}


ProcSocketMsg::~ProcSocketMsg()
{
}

/*
 ��Ϣ����
 Return:
 0 -- �ɹ�
 -1 -- ʧ��
 */
int ProcSocketMsg::ProcMsg(const std::string& in_strMsg, uint64_t ui64ReportIndex, std::string& out_strReport)
{
	static const std::string ftag("ProcSocketMsg::ProcMsg() ");

	/* �Ƚ���msg */
	std::string strError;
	struct simutgw::SocketMsgInfo msgInfo;
	int iRes = 0;
	iRes = MsgParse(in_strMsg, msgInfo, strError);
	if (0 != iRes)
	{
		MakeExecFailMsg(msgInfo.ui64SeqNum, ui64ReportIndex, out_strReport, strError);
		return -1;
	}

	/* ���ݲ�ͬ��msgtype��ҵ���� */
	if (Socket_MsgType_MatchMode_ControlCMD == msgInfo.ui64MsgType)
	{
		/* ִ�гɽ�ģʽ�л��������� */
		iRes = ExecMatchModeControlCmd(msgInfo, strError);
	}
	else if (Socket_MsgType_AddAccount == msgInfo.ui64MsgType)
	{
		/* ִ������˻� */
		iRes = ExecAddAccount(msgInfo, strError);
	}
	else if (Socket_MsgType_Add == msgInfo.ui64MsgType)
	{
		/* ִ����Ӳ��� */
		iRes = ExecAddCmd(msgInfo, strError);
	}
	else if (Socket_MsgType_Truncate == msgInfo.ui64MsgType)
	{
		/* ִ����ղ��� */
		iRes = ExecCmdTruncate(strError);
	}
	else if (Socket_MsgType_ClostAccount == msgInfo.ui64MsgType)
	{
		/* ִ���������� */
		iRes = ExecCloseAccount(msgInfo, strError);
	}
	else if (Socket_MsgType_Modify == msgInfo.ui64MsgType)
	{
		/* ִ���޸Ĳ��� */
		iRes = ExecModifyCmd(msgInfo, strError);
	}
	else if (Socket_MsgType_ReLoad == msgInfo.ui64MsgType)
	{
		/* ִ�����¼��ز��� */
		iRes = ExecReloadCmd(msgInfo, strError);
	}
	else if (Socket_MsgType_Settlement == msgInfo.ui64MsgType)
	{
		/* ִ�����������ļ����� */
		iRes = ExecCmd_Settlement(msgInfo, strError);
	}	
	else
	{
		string strDebug("δ֧�ֵ�MsgType[");
		strDebug += msgInfo.strMsgType;
		EzLog::e(ftag, strDebug);

		MakeExecFailMsg(msgInfo.ui64SeqNum, ui64ReportIndex, out_strReport, strDebug);
		return -1;
	}

	if (0 != iRes)
	{
		MakeExecFailMsg(msgInfo.ui64SeqNum, ui64ReportIndex, out_strReport, strError);
		return -1;
	}

	MakeExecSuccessMsg(msgInfo.ui64SeqNum, ui64ReportIndex, out_strReport);
	return 0;
}

/*
ִ�гɽ�ģʽ�л���������
*/
int ProcSocketMsg::ExecMatchModeControlCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& out_strError)
{
	static std::string ftag("ProcSocketMsg::ExecMatchModeControlCmd() ");

	std::string strParam = in_msgInfo.strCmdValue;

	if (0 == strParam.length())
	{
		out_strError = "��Ч����";
		return -1;
	}

	if (0 == strParam.compare("SimulMatchAll"))
	{
		// ȫ���ɽ�
		simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchAll;
	}
	else if (0 == strParam.compare("SimulMatchByDivide"))
	{
		// �ֱʳɽ�
		simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchByDivide;
	}
	else if (0 == strParam.compare("SimulNotMatch"))
	{
		// ���ɽ����ҵ������Գ���
		simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulNotMatch;
	}
	else if (0 == strParam.compare("SimulErrMatch"))
	{
		// ����
		simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulErrMatch;
	}
	else if (0 == strParam.compare("SimulMatchPart"))
	{
		// ���ֳɽ�
		simutgw::g_iMatchMode = simutgw::SysMatchMode::SimulMatchPart;
	}
	else if (0 == strParam.compare("Check_Assets"))
	{
		// ����ʽ�ɷ�
		if (simutgw::g_iRunMode == simutgw::SysRunMode::PressureMode ||
			simutgw::g_iRunMode == simutgw::SysRunMode::MiniMode)
		{
			// �����ѹ��ģʽ�������ɷ�
			out_strError = "��ǰģʽ�����ɷ�";
			EzLog::e(ftag, out_strError);

			return -1;
		}
		// true -- ���
		simutgw::g_bEnable_Check_Assets = true;
	}
	else if (0 == strParam.compare("Dont_Check_Assets"))
	{
		// 
		simutgw::g_bEnable_Check_Assets = false;
	}
	else if (0 == strParam.compare("Quta_AveragePrice"))
	{
		// ʵ�� ����
		simutgw::g_iMatchMode = simutgw::SysMatchMode::EnAbleQuta;
		simutgw::g_Quotation_Type = simutgw::QuotationType::AveragePrice;
	}
	else if (0 == strParam.compare("Quta_SellBuyPrice"))
	{		
		// ʵ�� ��һ��һ
		simutgw::g_iMatchMode = simutgw::SysMatchMode::EnAbleQuta;
		simutgw::g_Quotation_Type = simutgw::QuotationType::SellBuyPrice;
	}
	else if (0 == strParam.compare("Quta_RecentMatchPrice"))
	{
		// ʵ�� ����ɽ���
		simutgw::g_iMatchMode = simutgw::SysMatchMode::EnAbleQuta;
		simutgw::g_Quotation_Type = simutgw::QuotationType::RecentMatchPrice;
	}
	else
	{
		EzLog::e(ftag, "unkown param:" + strParam);

		out_strError = "��Ч����";
		return -1;
	}

	string strItoa;
	string strDebug("match mode change to:");
	strDebug += sof_string::itostr(simutgw::g_iMatchMode, strItoa);
	EzLog::i(ftag, strDebug);

	return 0;
}

/*
ִ�п���
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::ExecAddAccount(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& out_strError)
{
	// static std::string ftag("ProcSocketMsg::ExecAddAccount() ");

	string strQuery;
	/* �Ȳ��˻��Ƿ���� */
	std::string strBalance, strHK_Balance, strUS_Balance, strIsClose;
	int iRes = LookUpAccountBalance(in_msgInfo, strBalance, strHK_Balance, strUS_Balance, strIsClose);
	if (0 == iRes)
	{
		// ����
		if (0 == strIsClose.compare("0"))
		{
			out_strError = "�˺�[";
			out_strError += in_msgInfo.strAccount;
			out_strError += "]�Ѵ���";

			return -1;
		}
		strQuery = "UPDATE `simutgw`.`account` SET `account_last_balance`=0,`account_balance`=0,\
				   `hk_account_last_balance`=0,`hk_account_balance`=0,`us_account_last_balance`=0,`us_account_balance`=0,\
				   `is_close`=0,`oper_time`=now(),`operator`='SYS' WHERE `security_account`='";
		strQuery += in_msgInfo.strAccount;
		strQuery += "' AND `is_close`=1";

		if (0 != in_msgInfo.ui64Add_Balance && !strBalance.empty())
		{ // �ʽ�Ϊ��
			strQuery += " AND `account_balance`=";

			strQuery += strBalance;
		}
		else if (0 != in_msgInfo.ui64Add_HK_Balance && !strHK_Balance.empty())
		{
			strQuery += " AND `hk_account_balance`=";

			strQuery += strHK_Balance;
		}
		else if (0 != in_msgInfo.ui64Add_US_Balance && !strHK_Balance.empty())
		{
			strQuery += " AND `us_account_balance`=";

			strQuery += strHK_Balance;
		}
	}
	else if (1 == iRes)
	{
		// ������
		strQuery = "INSERT INTO `simutgw`.`account` (`security_account`,`trade_group`,`security_seat`,\
				   `securities_trader`,`account_balance`,`account_last_balance`,`hk_account_balance`,`hk_account_last_balance`,\
				   `us_account_balance`,`us_account_last_balance`,`is_close`,`oper_time`,`operator`) VALUES('";
		strQuery += in_msgInfo.strAccount;
		strQuery += "',";
		if (in_msgInfo.strTradeGroup.empty())
		{
			strQuery += "null,'";
		}
		else
		{
			strQuery += "'";
			strQuery += in_msgInfo.strTradeGroup;
			strQuery += "','";
		}
		strQuery += in_msgInfo.strSeat;
		strQuery += "','";
		strQuery += in_msgInfo.strTrader;
		strQuery += "',0,0,0,0,0,0,0,now(),'SYS')";
	}
	else
	{
		out_strError = "ϵͳ����";
		return -1;
	}

	/*mysql����*/
	iRes = MysqlUpdate(strQuery);
	if (-1 == iRes)
	{
		out_strError = "ϵͳ����";
	}

	return iRes;
}

/*
ִ������
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::ExecCloseAccount(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& out_strError)
{
	static std::string ftag("ProcSocketMsg::ExecCloseAccount() ");
	string strQuery("UPDATE `simutgw`.`account` SET `is_close`=1, `oper_time`=now() WHERE `security_account`='");
	strQuery += in_msgInfo.strAccount;
	strQuery += "' AND `is_close`=0";
	if (!in_msgInfo.strTradeGroup.empty())
	{
		strQuery += " AND `trade_group`='";
		strQuery += in_msgInfo.strTradeGroup;
		strQuery += "'";
	}
	if (!in_msgInfo.strSeat.empty())
	{
		strQuery += " AND `security_seat`='";
		strQuery += in_msgInfo.strSeat;
		strQuery += "'";
	}
	if (!in_msgInfo.strTrader.empty())
	{
		strQuery += " AND `securities_trader`='";
		strQuery += in_msgInfo.strTrader;
		strQuery += "'";
	}

	/* �ȸ����˺ű�ʶ */
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (nullptr == mysqlConn)
	{
		EzLog::e(ftag, "Get mysql connection null");
	}

	mysqlConn->StartTransaction();
	unsigned long ulAffectedRows = 0;
	int iRes = 0, iReturn = 0;
	iRes = mysqlConn->Query(strQuery, nullptr, ulAffectedRows);
	if (2 == iRes)
	{
		// �Ǹ���
		if (1 < ulAffectedRows)
		{
			// ʧ��
			string strDebug("����["), strItoa;
			strDebug += strQuery;
			strDebug += "]�õ�AffectedRows=";
			strDebug += sof_string::itostr((uint64_t)ulAffectedRows, strItoa);
			EzLog::e(ftag, strDebug);

			out_strError = "ϵͳ����";
			iReturn = -1;

			mysqlConn->RollBack();
		}
		else if (0 == ulAffectedRows)
		{
			out_strError = "�˺Ų����ڣ�����ʧ��";

			iReturn = -1;

			mysqlConn->RollBack();
		}
		else
		{
			//
		}
	}
	else
	{
		string strDebug("����["), strItoa;
		strDebug += strQuery;
		strDebug += "]�õ�Res=";
		strDebug += sof_string::itostr(iRes, strItoa);
		EzLog::e(ftag, strDebug);

		out_strError = "ϵͳ����";
		iReturn = -1;

		mysqlConn->RollBack();
	}

	if (0 != iReturn)
	{
		/* �黹���� */
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
		return iReturn;
	}

	/* ���¹ɷݱ�ʶ */
	std::string strUpdate("UPDATE `simutgw`.`stock_asset` SET `is_close`=1, `oper_time`=now() WHERE `account_id`='");
	strUpdate += in_msgInfo.strAccount;
	strUpdate += "' AND `is_close`=0";

	iRes = mysqlConn->Query(strUpdate, nullptr, ulAffectedRows);
	if (2 == iRes)
	{
		// �Ǹ���
		if (0 < ulAffectedRows)
		{
			// ʧ��
			string strDebug("����["), strItoa;
			strDebug += strUpdate;
			strDebug += "]�õ�AffectedRows=";
			strDebug += sof_string::itostr((uint64_t)ulAffectedRows, strItoa);
			EzLog::e(ftag, strDebug);

			out_strError = "ϵͳ����";
			iReturn = -1;

			mysqlConn->RollBack();
		}
	}
	else
	{
		string strDebug("����["), strItoa;
		strDebug += strUpdate;
		strDebug += "]�õ�Res=";
		strDebug += sof_string::itostr(iRes, strItoa);
		EzLog::e(ftag, strDebug);

		out_strError = "ϵͳ����";
		iReturn = -1;

		mysqlConn->RollBack();
	}

	mysqlConn->Commit();
	/* �黹���� */
	simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

	return iReturn;
}

/*
����������Ϣ
*/
int ProcSocketMsg::MsgParse(const std::string& in_strMsg, struct simutgw::SocketMsgInfo& out_msgInfo,
	std::string& out_strError)
{
	static const std::string ftag("ProcSocketMsg::MsgParse() ");

	rapidjson::Document docData;
	docData.GetAllocator();
	if (docData.Parse<0>(in_strMsg.c_str()).HasParseError() || docData.IsNull())
	{
		//������Ϣʧ��
		out_strError = "Parse Data to Json failed";
		EzLog::e(ftag, out_strError);
		return -1;
	}

	out_msgInfo.strMsgType = (docData.HasMember("MsgType") && docData["MsgType"].IsString()) ? docData["MsgType"].GetString() : "";
	out_msgInfo.ui64ApplID = (docData.HasMember("ApplID") && docData["ApplID"].IsUint64()) ?
		docData["ApplID"].GetUint64() : 0;
	out_msgInfo.ui64SeqNum = (docData.HasMember("SeqNum") && docData["SeqNum"].IsUint64()) ?
		docData["SeqNum"].GetUint64() : 0;
	out_msgInfo.strAccount = (docData.HasMember("Account") && docData["Account"].IsString()) ?
		docData["Account"].GetString() : "";
	out_msgInfo.strTradeGroup = (docData.HasMember("TradeGroup") && docData["TradeGroup"].IsString()) ?
		docData["TradeGroup"].GetString() : "";
	out_msgInfo.strSeat = (docData.HasMember("Seat") && docData["Seat"].IsString()) ? docData["Seat"].GetString() : "";
	out_msgInfo.strTrader = (docData.HasMember("Trader") && docData["Trader"].IsString()) ? docData["Trader"].GetString() : "";

	out_msgInfo.strAdd_Balance = (docData.HasMember("Balance") && docData["Balance"].IsString()) ?
		docData["Balance"].GetString() : "";
	out_msgInfo.strAdd_HK_Balance = (docData.HasMember("HK_Balance") && docData["HK_Balance"].IsString()) ?
		docData["HK_Balance"].GetString() : "";
	out_msgInfo.strAdd_US_Balance = (docData.HasMember("US_Balance") && docData["US_Balance"].IsString()) ?
		docData["US_Balance"].GetString() : "";


	out_msgInfo.strSecurityID = (docData.HasMember("SecurityID") && docData["SecurityID"].IsString()) ?
		docData["SecurityID"].GetString() : "";
	out_msgInfo.strQuantity = (docData.HasMember("Quantity") && docData["Quantity"].IsString()) ?
		docData["Quantity"].GetString() : "";

	out_msgInfo.strFileType = (docData.HasMember("FileType") && docData["FileType"].IsString()) ?
		docData["FileType"].GetString() : "";
	out_msgInfo.strFilePath = (docData.HasMember("FilePath") && docData["FilePath"].IsString()) ?
		docData["FilePath"].GetString() : "";

	out_msgInfo.strCmdValue = (docData.HasMember("CmdValue") && docData["CmdValue"].IsString()) ?
		docData["CmdValue"].GetString() : "";

	if (out_msgInfo.strMsgType.empty())
	{
		out_strError = "field[MsgType] missed";
		EzLog::e(ftag, out_strError);
		return -1;
	}

	if (0 == out_msgInfo.ui64SeqNum)
	{
		out_strError = "field[SeqNum] missed";
		EzLog::e(ftag, out_strError);
		return -1;
	}

	Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(out_msgInfo.strAdd_Balance, out_msgInfo.ui64Add_Balance);
	Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(out_msgInfo.strAdd_HK_Balance, out_msgInfo.ui64Add_HK_Balance);
	Tgw_StringUtil::String2UInt64MoneyInLi_strtoui64(out_msgInfo.strAdd_US_Balance, out_msgInfo.ui64Add_US_Balance);

	Tgw_StringUtil::String2UInt64_strtoui64(out_msgInfo.strQuantity, out_msgInfo.ui64Add_Quantity);

	Tgw_StringUtil::String2UInt64_strtoui64(out_msgInfo.strMsgType, out_msgInfo.ui64MsgType);

	return 0;
}

/*
ִ��Add��������
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::ExecAddCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& out_strError)
{
	static const string ftag("ProcSocketMsg::ExecAddCmd() ");

	/* �����ʽ��ɷ� */
	int iRes = 0;
	if (1 == in_msgInfo.ui64ApplID)
	{
		iRes = ExecCmd_Add_UserFund(in_msgInfo, out_strError);
	}
	else if (2 == in_msgInfo.ui64ApplID)
	{
		iRes = ExecCmd_Add_UserSecurity(in_msgInfo, out_strError);
	}
	else
	{
		out_strError = "δ֧�ֵ�ApplID[";
		string strTemp;
		sof_string::itostr(in_msgInfo.ui64ApplID, strTemp);
		out_strError += strTemp;
		EzLog::e(ftag, out_strError);

		return -1;
	}

	return iRes;
}

/*
ִ�������û��ʽ�����
*/
int ProcSocketMsg::ExecCmd_Add_UserFund(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& out_strError)
{
	static const string ftag("ProcSocketMsg::ExecCmd_Add_UserFund() ");

	/* �����ʽ� */
	if (in_msgInfo.strAccount.empty())
	{
		out_strError = "field[Account] missed";
		EzLog::e(ftag, out_strError);
		return -1;
	}

	std::string strQuery;

	/* �Ȳ���� */
	std::string strBalance, strHK_Balance, strUS_Balance, strIsClose;
	int iRes = LookUpAccountBalance(in_msgInfo, strBalance, strHK_Balance, strUS_Balance, strIsClose);
	if (1 == iRes)
	{
		// δ����
		out_strError = "�˺�[";
		out_strError += in_msgInfo.strAccount;
		out_strError = "]δ����";
		EzLog::i(ftag, out_strError);
		return -1;
		//// �޼�¼������һ��
		//		strQuery = "INSERT INTO `simutgw`.`account` (`security_account`,\
		//`account_balance`,`account_last_balance`,`hk_account_balance`,`hk_account_last_balance`,\
		//`us_account_balance`,`us_account_last_balance`,`oper_time`,`operator`) VALUES('";
		//		strQuery += in_msgInfo.strAccount;
		//		strQuery += "',";
		//
		//		string strTemp;
		//		sof_string::itostr(in_msgInfo.ui64Add_Balance, strTemp);
		//		strQuery += strTemp;
		//		strQuery += ",0,";
		//
		//		sof_string::itostr(in_msgInfo.ui64Add_HK_Balance, strTemp);
		//		strQuery += strTemp;
		//		strQuery += ",0,";
		//		
		//		sof_string::itostr(in_msgInfo.ui64Add_US_Balance, strTemp);
		//		strQuery += strTemp;
		//		strQuery += ",0,";
		//		
		//		strQuery += "now(),'SYS')";
	}
	else if (0 == iRes && 0 == strIsClose.compare("0"))
	{
		// �м�¼,��δ��������
		std::string strTemp;
		uint64_t ui64Temp;
		strQuery = "UPDATE `simutgw`.`account` SET ";
		if (0 != in_msgInfo.ui64Add_Balance)
		{
			strQuery += "`account_last_balance`=";
			Tgw_StringUtil::String2UInt64_strtoui64(strBalance, ui64Temp);
			strQuery += sof_string::itostr(ui64Temp, strTemp);
			strQuery += ",`account_balance`=";
			sof_string::itostr(ui64Temp + in_msgInfo.ui64Add_Balance, strTemp);
			strQuery += strTemp;
			strQuery += ",";
		}
		else if (0 != in_msgInfo.ui64Add_HK_Balance)
		{
			strQuery += "`hk_account_last_balance`=";
			Tgw_StringUtil::String2UInt64_strtoui64(strHK_Balance, ui64Temp);
			strQuery += sof_string::itostr(ui64Temp, strTemp);
			strQuery += ",`hk_account_balance`=";
			sof_string::itostr(ui64Temp + in_msgInfo.ui64Add_HK_Balance, strTemp);
			strQuery += strTemp;
			strQuery += ",";
		}
		else if (0 != in_msgInfo.ui64Add_US_Balance)
		{
			strQuery += "`us_account_last_balance`=";
			Tgw_StringUtil::String2UInt64_strtoui64(strUS_Balance, ui64Temp);
			strQuery += sof_string::itostr(ui64Temp, strTemp);
			strQuery += ",`us_account_balance`=";
			sof_string::itostr(ui64Temp + in_msgInfo.ui64Add_US_Balance, strTemp);
			strQuery += strTemp;
			strQuery += ",";
		}
		strQuery += "`oper_time`=now(),`operator`='SYS' WHERE `security_account`='";
		strQuery += in_msgInfo.strAccount;
		strQuery += "'";

		if (0 != in_msgInfo.ui64Add_Balance && !strBalance.empty())
		{ // �ʽ�Ϊ��
			strQuery += " AND `account_balance`=";

			strQuery += strBalance;
		}
		else if (0 != in_msgInfo.ui64Add_HK_Balance && !strHK_Balance.empty())
		{
			strQuery += " AND `hk_account_balance`=";

			strQuery += strHK_Balance;
		}
		else if (0 != in_msgInfo.ui64Add_US_Balance && !strHK_Balance.empty())
		{
			strQuery += " AND `us_account_balance`=";

			strQuery += strHK_Balance;
		}
	}
	else if (-1 == iRes)
	{
		// ��ѯ����
		out_strError = "ϵͳ����";
		return -1;
	}
	else
	{
		// �˻�������
		out_strError = "�˺�[";
		out_strError += in_msgInfo.strAccount;
		out_strError = "]������";
		EzLog::i(ftag, out_strError);
		return -1;
	}

	/*mysql����*/
	iRes = MysqlUpdate(strQuery);
	if (0 == iRes)
	{
		out_strError = "ϵͳ����";
	}

	return iRes;
}

/*
ִ�������û��ɷ�����
*/
int ProcSocketMsg::ExecCmd_Add_UserSecurity(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& out_strError)
{
	static const string ftag("ProcSocketMsg::ExecCmd_Add_UserSecurity() ");

	/* ���ӹɷ� */
	if (in_msgInfo.strSecurityID.empty())
	{
		out_strError = "field[SecurityID] missed";
		EzLog::e(ftag, out_strError);
		return -1;
	}

	std::string strError;
	std::string strBalance, strIsClose;
	int iRes = LookUpAccountBalance(in_msgInfo, strBalance, strBalance, strBalance, strIsClose);
	if (1 == iRes)
	{
		// ���˻�
		out_strError = "�˺�[";
		out_strError += in_msgInfo.strAccount;
		out_strError = "]δ����";
		EzLog::i(ftag, out_strError);
		return -1;
	}
	else if (0 == iRes && 0 == strIsClose.compare("1"))
	{
		// �˻�������
		out_strError = "�˺�[";
		out_strError += in_msgInfo.strAccount;
		out_strError = "]������";
		EzLog::i(ftag, out_strError);
		return -1;
	}
	else if (-1 == iRes)
	{
		out_strError = "ϵͳ����";
		return -1;
	}
	else
	{
		//�˺�����
	}

	string strQuery;

	/* ����� */
	uint64_t ui64Quantity = 0;
	iRes = LookUpAccountStock(in_msgInfo, ui64Quantity);
	if (1 == iRes)
	{
		// �޹ɷ�,ֱ�Ӳ���һ����¼
		strQuery = "INSERT INTO `simutgw`.`stock_asset` (`account_id`,`stock_id`,`trade_market`,\
				   `stock_balance`,`stock_available`,`stock_last_balance`,`oper_time`,`operator`) VALUES('";
		strQuery += in_msgInfo.strAccount;
		strQuery += "','";
		strQuery += in_msgInfo.strSecurityID;
		strQuery += "',null,";
		strQuery += in_msgInfo.strQuantity;
		strQuery += ",";
		strQuery += in_msgInfo.strQuantity;
		strQuery += ",0,";
		strQuery += "now(),'SYS')";
	}
	else if (0 == iRes)
	{
		// �м�¼,���ӹɷ�
		std::string strTemp;
		strQuery = "UPDATE `simutgw`.`stock_asset` SET `stock_last_balance`=";
		strQuery += sof_string::itostr(ui64Quantity, strTemp);
		strQuery += ",`stock_balance`=";
		sof_string::itostr(ui64Quantity + in_msgInfo.ui64Add_Quantity, strTemp);
		strQuery += strTemp;
		strQuery += ",`stock_available`=";
		strQuery += strTemp;
		strQuery += ",`oper_time`=now(),`operator`='SYS' WHERE `account_id`='";
		strQuery += in_msgInfo.strAccount;
		strQuery += "' AND `stock_id`='";
		strQuery += in_msgInfo.strSecurityID;
		strQuery += "' AND `stock_balance`=";
		strQuery += sof_string::itostr(ui64Quantity, strTemp);
	}
	else
	{
		out_strError = "ϵͳ����";
		return -1;
	}

	/*mysql����*/
	iRes = MysqlUpdate(strQuery);
	if (-1 == iRes)
	{
		out_strError = "ϵͳ����";
	}

	return iRes;
}

/*
mysql�������
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::MysqlUpdate(const std::string& in_strQuery)
{
	static const string ftag("ProcSocketMsg::MysqlUpdate() ");

	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (nullptr == mysqlConn)
	{
		EzLog::e(ftag, "Get mysql connection null");
	}

	mysqlConn->StartTransaction();
	unsigned long ulAffectedRows = 0;
	int iRes = 0, iReturn = 0;
	iRes = mysqlConn->Query(in_strQuery, nullptr, ulAffectedRows);
	if (2 == iRes)
	{
		// �Ǹ���
		if (1 != ulAffectedRows)
		{
			// ʧ��
			string strDebug("����["), strItoa;
			strDebug += in_strQuery;
			strDebug += "]�õ�AffectedRows=";
			strDebug += sof_string::itostr((uint64_t)ulAffectedRows, strItoa);
			EzLog::e(ftag, strDebug);

			iReturn = -1;

			mysqlConn->RollBack();
		}
	}
	else
	{
		string strDebug("����["), strItoa;
		strDebug += in_strQuery;
		strDebug += "]�õ�Res=";
		strDebug += sof_string::itostr(iRes, strItoa);
		EzLog::e(ftag, strDebug);

		iReturn = -1;

		mysqlConn->RollBack();
	}

	mysqlConn->Commit();
	/* �黹���� */
	simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

	return iReturn;
}

/*
����ִ�гɹ���Ϣ
*/
int ProcSocketMsg::MakeExecSuccessMsg(uint64_t in_ui64SeqNum,
	uint64_t ui64ReportIndex, std::string& out_strReport)
{
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
	doc.SetObject();

	doc.AddMember("OriginSeqNum", in_ui64SeqNum, allocator);

	doc.AddMember(rapidjson::Value("MsgType", allocator),
		rapidjson::Value("108", allocator), allocator);

	string strTemp;
	TimeStringUtil::GetTimeStamp_intstr(strTemp);
	doc.AddMember(rapidjson::Value("SendTime", allocator),
		rapidjson::Value(strTemp.c_str(), allocator), allocator);

	//sof_string::itostr(ui64ReportIndex, strTemp);
	doc.AddMember("ReportIndex", ui64ReportIndex, allocator);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	out_strReport = buffer.GetString();

	return 0;
}

/*
����ִ��ʧ����Ϣ
*/
int ProcSocketMsg::MakeExecFailMsg(uint64_t in_ui64SeqNum, uint64_t ui64ReportIndex,
	std::string& out_strReport, const std::string& in_strText)
{
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
	doc.SetObject();

	doc.AddMember("OriginSeqNum", in_ui64SeqNum, allocator);

	doc.AddMember("MsgType", rapidjson::Value("109", allocator), allocator);

	string strTemp;
	TimeStringUtil::GetTimeStamp_intstr(strTemp);
	doc.AddMember("SendTime", rapidjson::Value(strTemp.c_str(), allocator), allocator);

	//sof_string::itostr(ui64ReportIndex, strTemp);
	doc.AddMember("ReportIndex", ui64ReportIndex, allocator);

	doc.AddMember("text", rapidjson::Value(in_strText.c_str(), allocator), allocator);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	out_strReport = buffer.GetString();

	return 0;
}

/*
ִ��ReLoad��������
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::ExecCmdTruncate(std::string& out_strError)
{
	static const string ftag("ProcSocketMsg::ExecCmdTruncate() ");

	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (nullptr == mysqlConn)
	{
		EzLog::e(ftag, "Get mysql connection null");
	}

	mysqlConn->StartTransaction();

	unsigned long ulAffectedRows = 0;
	int iRes = 0, iReturn = 0;
	string strQueryAccount("TRUNCATE `account`");
	iRes = mysqlConn->Query(strQueryAccount, nullptr, ulAffectedRows);
	if (2 == iRes)
	{
		// �Ǹ���

	}
	else
	{
		string strDebug("����["), strItoa;
		strDebug += strQueryAccount;
		strDebug += "]�õ�Res=";
		strDebug += sof_string::itostr(iRes, strItoa);
		EzLog::e(ftag, strDebug);

		out_strError = "ϵͳ����";
		iReturn = -1;

		mysqlConn->RollBack();
	}

	if (0 != iReturn)
	{
		/* �黹���� */
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
		return iReturn;
	}

	string strTruncateStock("TRUNCATE `stock_asset`");

	iRes = mysqlConn->Query(strTruncateStock, nullptr, ulAffectedRows);
	if (2 == iRes)
	{
		// �Ǹ���

	}
	else
	{
		string strDebug("����["), strItoa;
		strDebug += strTruncateStock;
		strDebug += "]�õ�Res=";
		strDebug += sof_string::itostr(iRes, strItoa);
		EzLog::e(ftag, strDebug);

		out_strError = "ϵͳ����";

		iReturn = -1;

		mysqlConn->RollBack();
	}

	string strTruncateETFStock("TRUNCATE `stock_etf_asset`");

	iRes = mysqlConn->Query(strTruncateETFStock, nullptr, ulAffectedRows);
	if (2 == iRes)
	{
		// �Ǹ���

	}
	else
	{
		string strDebug("����["), strItoa;
		strDebug += strTruncateETFStock;
		strDebug += "]�õ�Res=";
		strDebug += sof_string::itostr(iRes, strItoa);
		EzLog::e(ftag, strDebug);

		out_strError = "ϵͳ����";

		iReturn = -1;

		mysqlConn->RollBack();
	}

	mysqlConn->Commit();

	/* �黹���� */
	simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

	return iReturn;
}


/*
ִ��Modify��������
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::ExecModifyCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& out_strError)
{
	static const string ftag("ProcSocketMsg::ExecModifyCmd() ");

	/* �����ʽ��ɷ� */
	int iRes = 0;
	if (1 == in_msgInfo.ui64ApplID)
	{
		iRes = ExecCmd_Modify_UserFund(in_msgInfo, out_strError);
	}
	else if (2 == in_msgInfo.ui64ApplID)
	{
		iRes = ExecCmd_Modify_UserSecurity(in_msgInfo, out_strError);
	}
	else
	{
		out_strError = "δ֧�ֵ�ApplID[";
		string strTemp;
		sof_string::itostr(in_msgInfo.ui64ApplID, strTemp);
		out_strError += strTemp;
		EzLog::e(ftag, out_strError);

		return -1;
	}

	return iRes;
}

/*
ִ���޸��ʽ�����
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::ExecCmd_Modify_UserFund(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& out_strError)
{
	static const string ftag("ProcSocketMsg::ExecCmd_Modify_UserFund() ");

	/* �����ʽ� */
	if (in_msgInfo.strAccount.empty())
	{
		out_strError = "field[Account] missed";
		EzLog::e(ftag, out_strError);
		return -1;
	}

	std::string strQuery;

	/* �Ȳ���� */
	std::string strBalance, strHK_Balance, strUS_Balance, strIsClose;
	int iRes = LookUpAccountBalance(in_msgInfo, strBalance, strHK_Balance, strUS_Balance, strIsClose);
	if (1 == iRes)
	{
		// δ����
		out_strError = "�˺�[";
		out_strError += in_msgInfo.strAccount;
		out_strError = "]δ����";
		EzLog::i(ftag, out_strError);
		return -1;
	}
	else if (0 == iRes && 0 == strIsClose.compare("0"))
	{
		// �м�¼,��δ����,�޸�
		std::string strTemp;
		uint64_t ui64Temp;
		strQuery = "UPDATE `simutgw`.`account` SET ";

		strQuery += "`account_last_balance`=";
		Tgw_StringUtil::String2UInt64_strtoui64(strBalance, ui64Temp);
		strQuery += sof_string::itostr(ui64Temp, strTemp);
		strQuery += ",`account_balance`=";
		sof_string::itostr(in_msgInfo.ui64Add_Balance, strTemp);
		strQuery += strTemp;
		strQuery += ",";

		strQuery += "`hk_account_last_balance`=";
		Tgw_StringUtil::String2UInt64_strtoui64(strHK_Balance, ui64Temp);
		strQuery += sof_string::itostr(ui64Temp, strTemp);
		strQuery += ",`hk_account_balance`=";
		sof_string::itostr(in_msgInfo.ui64Add_HK_Balance, strTemp);
		strQuery += strTemp;
		strQuery += ",";

		strQuery += "`us_account_last_balance`=";
		Tgw_StringUtil::String2UInt64_strtoui64(strUS_Balance, ui64Temp);
		strQuery += sof_string::itostr(ui64Temp, strTemp);
		strQuery += ",`us_account_balance`=";
		sof_string::itostr(ui64Temp + in_msgInfo.ui64Add_US_Balance, strTemp);
		strQuery += strTemp;
		strQuery += ",";

		strQuery += "`oper_time`=now(),`operator`='SYS' WHERE `security_account`='";
		strQuery += in_msgInfo.strAccount;
		strQuery += "'";

		if (0 != in_msgInfo.ui64Add_Balance && !strBalance.empty())
		{ // �ʽ�Ϊ��
			strQuery += " AND `account_balance`=";

			strQuery += strBalance;
		}
		else if (0 != in_msgInfo.ui64Add_HK_Balance && !strHK_Balance.empty())
		{
			strQuery += " AND `hk_account_balance`=";

			strQuery += strHK_Balance;
		}
		else if (0 != in_msgInfo.ui64Add_US_Balance && !strHK_Balance.empty())
		{
			strQuery += " AND `us_account_balance`=";

			strQuery += strHK_Balance;
		}
	}
	else if (-1 == iRes)
	{
		// ��ѯ����
		out_strError = "ϵͳ����";
		return -1;
	}
	else
	{
		// �˻�������
		out_strError = "�˺�[";
		out_strError += in_msgInfo.strAccount;
		out_strError = "]������";
		EzLog::i(ftag, out_strError);
		return -1;
	}

	/*mysql����*/
	iRes = MysqlUpdate(strQuery);
	if (-1 == iRes)
	{
		out_strError = "ϵͳ����";
	}

	return iRes;
}

/*
ִ���޸Ĺɷ�����
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::ExecCmd_Modify_UserSecurity(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& out_strError)
{
	static const string ftag("ProcSocketMsg::ExecCmd_Modify_UserSecurity() ");

	/* ���ӹɷ� */
	if (in_msgInfo.strSecurityID.empty())
	{
		out_strError = "field[Account|SecurityID] missed";
		EzLog::e(ftag, out_strError);
		return -1;
	}

	std::string strError;
	std::string strBalance, strIsClose;
	int iRes = LookUpAccountBalance(in_msgInfo, strBalance, strBalance, strBalance, strIsClose);
	if (1 == iRes)
	{
		// ���˻�
		out_strError = "���˺�[";
		out_strError += in_msgInfo.strAccount;
		EzLog::i(ftag, out_strError);
		return -1;
	}
	else if (0 == iRes && 0 == strIsClose.compare("1"))
	{
		// �˻�������
		out_strError = "�˺�[";
		out_strError += in_msgInfo.strAccount;
		out_strError = "]������";
		EzLog::i(ftag, out_strError);
		return -1;
	}
	else if (-1 == iRes)
	{
		out_strError = "ϵͳ����";
		return -1;
	}
	else
	{
		//�˺�����
	}


	string strQuery;
	/* ����� */
	uint64_t ui64Quantity = 0;
	iRes = LookUpAccountStock(in_msgInfo, ui64Quantity);
	if (1 == iRes)
	{
		// �޹ɷ�,�����޸�
		out_strError = "�޹ɷ��˺�[";
		out_strError += in_msgInfo.strAccount;
		out_strError += "]��Ʊ����[";
		out_strError += in_msgInfo.strSecurityID;
		EzLog::i(ftag, out_strError);
		return -1;
	}
	else if (0 == iRes)
	{
		// �м�¼,��δ����,�޸Ĺɷ�
		std::string strTemp;
		strQuery = "UPDATE `simutgw`.`stock_asset` SET `stock_last_balance`=";
		strQuery += sof_string::itostr(ui64Quantity, strTemp);
		strQuery += ",`stock_balance`=";
		sof_string::itostr(in_msgInfo.ui64Add_Quantity, strTemp);
		strQuery += strTemp;
		strQuery += ",`stock_available`=";
		strQuery += strTemp;
		strQuery += ",`oper_time`=now(),`operator`='SYS' WHERE `account_id`='";
		strQuery += in_msgInfo.strAccount;
		strQuery += "' AND `stock_id`='";
		strQuery += in_msgInfo.strSecurityID;
		strQuery += "' AND `stock_balance`=";
		strQuery += sof_string::itostr(ui64Quantity, strTemp);
	}
	else
	{
		out_strError = "ϵͳ����";
		return -1;
	}

	/*mysql����*/
	iRes = MysqlUpdate(strQuery);
	if (-1 == iRes)
	{
		out_strError = "ϵͳ����";
	}

	return iRes;
}

/*
ִ�м����ļ�����
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::ExecReloadCmd(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& out_strError)
{
	static const string ftag("ProcSocketMsg::ExecReloadCmd() ");

	if (in_msgInfo.strTrader.empty())
	{
		EzLog::e(ftag, "field[Trader] missed");
		return -1;
	}

	if (in_msgInfo.strFileType.empty())
	{
		EzLog::e(ftag, "field[FileType] missed");
		return -1;
	}

	if (in_msgInfo.strFilePath.empty())
	{
		EzLog::e(ftag, "field[FilePath] missed");
		return -1;
	}

	int iRes = 0;

	vector<DBF_UserStockAsset> vecUserAsset;
	//����FileType�ж�DBF������
	if (0 == in_msgInfo.strFileType.compare("zqye"))
	{
		iRes = Get_SHzqye_Asset(in_msgInfo.strFilePath, in_msgInfo.strTrader, vecUserAsset);
		if (0 != iRes)
		{
			out_strError = "��ȡ�ļ�ʧ��[";
			out_strError += in_msgInfo.strFilePath;
			EzLog::e(ftag, out_strError);
			return -1;
		}
	}
	else if (0 == in_msgInfo.strFileType.compare("sjsdz"))
	{
		iRes = Get_Sjsdz_Asset(in_msgInfo.strFilePath, in_msgInfo.strTrader, vecUserAsset);
		if (0 != iRes)
		{
			out_strError = "��ȡ�ļ�ʧ��[";
			out_strError += in_msgInfo.strFilePath;
			EzLog::e(ftag, out_strError);
			return -1;
		}
	}
	else
	{
		out_strError = "��֧�ֵ��ļ�����[";
		out_strError += in_msgInfo.strFileType;
		EzLog::e(ftag, out_strError);
		return -1;
	}

	// ���뵽stock_asset/stock_etf_asset����
	if (0 != vecUserAsset.size())
	{
		iRes = RecordAsset(vecUserAsset);
		if (0 != iRes)
		{
			out_strError = "�洢����ʧ��";
			EzLog::e(ftag, out_strError);
			return -1;
		}
	}

	return iRes;
}

/*
��ȡzqye�ļ�����Ϣ
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::Get_SHzqye_Asset(const std::string& in_strFileName,
	const std::string& in_strTrader,
	std::vector<DBF_UserStockAsset>& out_vecAsset)
{
	static const string ftag("ProcSocketMsg::Get_SHzqye_Asset() ");

	// ��ȡ�ļ�����
	TgwDBFOperHelper dbfReader;
	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;
	SHSettle shSettle;
	shSettle.DBF_zqye(vecSetting);

	int iRes = dbfReader.Open(in_strFileName);
	if (0 != iRes)
	{
		return -1;
	}

	std::map<std::string, TgwDBFOperHelper_DF::DataInRow> mapData;
	vector<std::map<std::string, TgwDBFOperHelper_DF::DataInRow>> vecData;
	while (0 == iRes)
	{
		mapData.clear();
		iRes = dbfReader.Read(vecSetting, mapData);
		vecData.push_back(mapData);
	}

	DBF_UserStockAsset asset;
	asset.enType = StockHelper::Ordinary;
	for (size_t st = 0; st < vecData.size(); ++st)
	{
		asset.clear();
		// ת����DBF_UserStockAsset�ṹ
		iRes = Trans_SHZqye(vecData[st], in_strTrader, asset);
		if (0 == iRes)
		{
			// ת���ɹ�
			out_vecAsset.push_back(asset);
		}
	}

	return 0;
}

/*
zqye��һ����¼ת����DBF_UserStockAsset�ṹ
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::Trans_SHZqye(const std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& in_mapData,
	const std::string& in_strTrader,
	DBF_UserStockAsset& out_Asset)
{
	static const string ftag("ProcSocketMsg::Get_SHzqye_Asset() ");

	out_Asset.strTradeMarket = simutgw::TRADE_MARKET_SH;

	std::map<std::string, TgwDBFOperHelper_DF::DataInRow>::const_iterator cit = in_mapData.end();

	cit = in_mapData.find("ZQLB");
	if (in_mapData.end() == cit)
	{
		return -1;
	}

	/*
	ZQLB ˵��
	GZ �̶�������
	JJ ����
	PT ��������ͨ��
	PG ���
	PS ���۹�
	PZ Ȩ֤
	GJ ���ҹ�
	GF ���з��˹�
	JN ���ڷ��˹�
	JW ���ⷨ�˹�
	SF ��ᷨ�˹�
	XL ������ͨ��
	YX ���ȷ��˹�
	ZG ְ����
	*/
	if (0 == cit->second.strValue.compare("PT") || 0 == cit->second.strValue.compare("JJ"))
	{
		//�Ϸ������ݹ�Ʊ����������
	}
	else
	{
		return -1;
	}

	out_Asset.strTrader = in_strTrader;

	cit = in_mapData.find("ZQZH");
	if (in_mapData.end() == cit)
	{
		return -1;
	}
	GetDBFRowData(out_Asset.strAccount, cit->second);

	cit = in_mapData.find("XWH");
	if (in_mapData.end() == cit)
	{
		return -1;
	}
	GetDBFRowData(out_Asset.strSecuritySeat, cit->second);

	cit = in_mapData.find("ZQDM");
	if (in_mapData.end() == cit)
	{
		return -1;
	}
	GetDBFRowData(out_Asset.strStockID, cit->second);

	cit = in_mapData.find("YE1");
	if (in_mapData.end() == cit)
	{
		return -1;
	}
	GetDBFRowData(out_Asset.strQty, cit->second);

	int iRes = StockHelper::GetStockType(out_Asset.strStockID, out_Asset.enType);

	return iRes;
}

/*
��ȡsjsdz�ļ�����Ϣ
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::Get_Sjsdz_Asset(const std::string& in_strFileName,
	const std::string& in_strTrader,
	std::vector<DBF_UserStockAsset>& out_vecAsset)
{
	static const string ftag("ProcSocketMsg::Get_Sjsdz_Asset() ");

	// ��ȡ�ļ�����
	TgwDBFOperHelper dbfReader;
	vector<struct TgwDBFOperHelper_DF::ColumnSettingInDBF> vecSetting;
	SZSettle szSettle;
	szSettle.DBF_SJSDZ(vecSetting);

	int iRes = dbfReader.Open(in_strFileName);
	if (0 != iRes)
	{
		return -1;
	}

	std::map<std::string, TgwDBFOperHelper_DF::DataInRow> mapData;
	vector<std::map<std::string, TgwDBFOperHelper_DF::DataInRow>> vecData;
	while (0 == iRes)
	{
		mapData.clear();
		iRes = dbfReader.Read(vecSetting, mapData);
		vecData.push_back(mapData);
	}

	DBF_UserStockAsset asset;
	asset.enType = StockHelper::Ordinary;
	for (size_t st = 0; st < vecData.size(); ++st)
	{
		asset.clear();
		// ת����DBF_UserStockAsset�ṹ
		iRes = Trans_Sjsdz(vecData[st], in_strTrader, asset);
		if (0 == iRes)
		{
			// ת���ɹ�
			out_vecAsset.push_back(asset);
		}
	}

	return 0;
}

/*
sjsdz��һ����¼ת����DBF_UserStockAsset�ṹ
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::Trans_Sjsdz(const std::map<std::string, TgwDBFOperHelper_DF::DataInRow>& in_mapData,
	const std::string& in_strTrader,
	DBF_UserStockAsset& out_Asset)
{
	static const string ftag("ProcSocketMsg::Get_SHzqye_Asset() ");

	out_Asset.strTradeMarket = simutgw::TRADE_MARKET_SZ;

	std::map<std::string, TgwDBFOperHelper_DF::DataInRow>::const_iterator cit = in_mapData.end();

	/*
	��ͨ���ͣ� LTLX���������£�
	�� 0���������������������Ƿ��������䡰�ɷ����ʡ�������
	�� N�����ѵǼǣ����ݲ��������У�Ŀǰ��������Ϲ����¹�����ҵ�������ɵ�
	�ǵ���δ���н׶Σ������� IPO ҵ�񡣣�
	*/
	cit = in_mapData.find("DZLTLX");
	if (in_mapData.end() == cit)
	{
		return -1;
	}

	if (0 != cit->second.strValue.compare("0"))
	{
		return -1;
	}

	/*
	�ɷ����ʣ� GFXZ���������£�
	�� 00������������ͨ�ɣ�
	�� 01���� IPO �����۹ɣ�
	�� 02������Ȩ�������۹ɣ�
	�� 05���� IPO ǰ���۹ɡ�
	���¹ɷ����ʷǳ�����������������ʷ������ B �ɺ͸�����ʽ����
	�� 10���������˹��ҹɣ�
	�� 11���������˹��з��˹ɣ�
	�� 12���������˾��ڷ��˹ɣ�
	�� 13�������������ʷ��˹ɣ�
	�� 14������������Ȼ�˹ɣ�
	�� 22���������˾��ڷ��˹ɣ�
	�� 24������������Ȼ�˹ɡ�
	*/
	cit = in_mapData.find("DZGFXZ");
	if (in_mapData.end() == cit)
	{
		return -1;
	}

	if (0 == cit->second.strValue.compare("00"))
	{
		//�Ϸ������ݹ�Ʊ����������
	}
	else
	{
		return -1;
	}

	out_Asset.strTrader = in_strTrader;

	cit = in_mapData.find("DZZQZH");
	if (in_mapData.end() == cit)
	{
		return -1;
	}
	GetDBFRowData(out_Asset.strAccount, cit->second);

	cit = in_mapData.find("DZTGDY");
	if (in_mapData.end() == cit)
	{
		return -1;
	}
	GetDBFRowData(out_Asset.strSecuritySeat, cit->second);

	cit = in_mapData.find("DZZQDM");
	if (in_mapData.end() == cit)
	{
		return -1;
	}
	GetDBFRowData(out_Asset.strStockID, cit->second);

	cit = in_mapData.find("DZZYGS");
	if (in_mapData.end() == cit)
	{
		return -1;
	}
	GetDBFRowData(out_Asset.strQty, cit->second);

	int iRes = StockHelper::GetStockType(out_Asset.strStockID, out_Asset.enType);

	return iRes;
}

/*
ȡһ��dbf���ݵ�value��ת����string value
*/
int ProcSocketMsg::GetDBFRowData(std::string& strValue, const TgwDBFOperHelper_DF::DataInRow& cdata)
{
	if (cdata.iType == 0)
	{
		// string
		strValue = cdata.strValue;
	}
	else if (1 == cdata.iType)
	{
		// long
		sof_string::itostr(cdata.lValue, strValue);
	}
	else if (2 == cdata.iType)
	{
		// unsigned long 
		sof_string::itostr((uint64_t)cdata.ulValue, strValue);
	}
	else if (3 == cdata.iType)
	{
		// double
		sof_string::itostr((uint64_t)cdata.dValue, strValue);
	}

	return 0;
}

/*
�������ݵ�stock_asset/stock_etf_asset
Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ProcSocketMsg::RecordAsset(std::vector<DBF_UserStockAsset>& in_vecAsset)
{
	static const string ftag("ProcSocketMsg::RecordAsset() ");

	//��mysql���ӳ�ȡ����
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (NULL == mysqlConn)
	{
		//ȡ����mysql����ΪNULL

		//�黹����
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		EzLog::e(ftag, "Get Connection is NULL");

		return -1;
	}

	mysqlConn->StartTransaction();

	unsigned long ulAffectedRows = 0;

	int iRes = 0;
	std::string strTradeMarket;
	for (size_t st = 0; st < in_vecAsset.size(); ++st)
	{
		DBF_UserStockAsset &asset = in_vecAsset[st];
		std::string strQuery("INSERT INTO ");
		switch (asset.enType)
		{
		case StockHelper::Ordinary:
			strQuery += "`stock_asset`(`account_id`,`stock_id`,`trade_market`,`stock_balance`,\
						`stock_auction_purchase_balance`,`stock_staple_purchase_balance`,`stock_etf_redemption_balance`,\
						`stock_available`,`is_close`,`oper_time`,`operator`) VALUES('";
			strQuery += asset.strAccount;
			strQuery += "','";
			strQuery += asset.strStockID;
			strQuery += "',";
			strQuery += asset.strTradeMarket;
			strQuery += ",";
			strQuery += asset.strQty;
			strQuery += ",0,0,0,";
			strQuery += asset.strQty;
			strQuery += ",0,now(),'SYS')";

			break;

		case StockHelper::Etf:
			strQuery += "`stock_etf_asset`(`account_id`,`stock_id`,`trade_market`,`stock_balance`,\
						`stock_auction_purchase_balance`,`stock_staple_purchase_balance`,`stock_creation_balance`,\
						`stock_available`,`is_close`,`oper_time`,`operator`) VALUES('";
			strQuery += asset.strAccount;
			strQuery += "','";
			strQuery += asset.strStockID;
			strQuery += "',";
			strQuery += asset.strTradeMarket;
			strQuery += ",";
			strQuery += asset.strQty;
			strQuery += ",0,0,0,";
			strQuery += asset.strQty;
			strQuery += ",0,now(),'SYS')";
			break;

		default:
			continue;
			break;
		}
		iRes = mysqlConn->Query(strQuery, NULL, ulAffectedRows);
		if (2 == iRes)
		{
			// �Ǹ���
			if (1 != ulAffectedRows)
			{
				// ʧ��
				string strDebug("����["), strItoa;
				strDebug += strQuery;
				strDebug += "]�õ�AffectedRows=";
				strDebug += sof_string::itostr((uint64_t)ulAffectedRows, strItoa);
				EzLog::e(ftag, strDebug);

				mysqlConn->RollBack();
				return -1;
			}
		}
		else
		{
			string strDebug("����["), strItoa;
			strDebug += strQuery;
			strDebug += "]�õ�Res=";
			strDebug += sof_string::itostr(iRes, strItoa);
			EzLog::e(ftag, strDebug);

			mysqlConn->RollBack();
			return -1;
		}
	}

	mysqlConn->Commit();

	return 0;
}
/*
��ѯaccount����ʽ����
Return:
0 -- �ɹ�
1 -- �޼�¼
-1 -- ʧ��
*/
int ProcSocketMsg::LookUpAccountBalance(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& strBalance, std::string& strHK_Balance, std::string& strUS_Balance,
	std::string& strIsClose)
{
	static const string ftag("ProcSocketMsg::LookUpAccountBalance() ");

	/* ���ӹɷ� */
	if (in_msgInfo.strAccount.empty())
	{
		EzLog::e(ftag, "field[Account] missed");
		return -1;
	}

	string strQuery("SELECT `account_balance`,`hk_account_balance`,`us_account_balance`,`is_close` FROM `simutgw`.`account` WHERE `security_account`='");
	strQuery += in_msgInfo.strAccount;
	strQuery += "'";

	/* ��ѯ�ʽ���� */
	//��mysql���ӳ�ȡ����
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (NULL == mysqlConn)
	{
		//ȡ����mysql����ΪNULL

		//�黹����
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		EzLog::e(ftag, "Get Connection is NULL");

		return -1;
	}

	mysqlConn->StartTransaction();

	int iReturn = 0;
	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	int iRes = mysqlConn->Query(strQuery, &pResultSet, ulAffectedRows);
	if (1 == iRes)
	{
		// select
		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		if (0 != mysqlConn->FetchNextRow(&pResultSet, mapRowData))
		{
			strBalance = mapRowData["account_balance"].strValue;

			strHK_Balance = mapRowData["hk_account_balance"].strValue;

			strUS_Balance = mapRowData["us_account_balance"].strValue;

			strIsClose = mapRowData["is_close"].strValue;
		}
		else
		{
			iReturn = 1;
		}

		// �ͷ�
		mysqlConn->FreeResult(&pResultSet);
		pResultSet = NULL;

	}
	else
	{
		iReturn = -1;
	}


	//�黹����
	simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
	return iReturn;
}

/*
��ѯstock_asset����ʽ����
Return:
0 -- �ɹ�
1 -- �޼�¼
-1 -- ʧ��
*/
int ProcSocketMsg::LookUpAccountStock(const struct simutgw::SocketMsgInfo& in_msgInfo, uint64_t& ui64cQuantity)
{
	static const string ftag("ProcSocketMsg::LookUpAccountBalance() ");

	/* ���ӹɷ� */
	if (in_msgInfo.strAccount.empty())
	{
		EzLog::e(ftag, "field[Account] missed");
		return -1;
	}

	string strQuery("SELECT `stock_balance`,`is_close` FROM `simutgw`.`stock_asset` WHERE `account_id`='");
	strQuery += in_msgInfo.strAccount;
	strQuery += "' AND `stock_id`='";
	strQuery += in_msgInfo.strSecurityID;
	strQuery += "'";

	//��mysql���ӳ�ȡ����
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (NULL == mysqlConn)
	{
		//ȡ����mysql����ΪNULL

		//�黹����
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		EzLog::e(ftag, "Get Connection is NULL");

		return -1;
	}

	mysqlConn->StartTransaction();

	int iReturn = 0;
	MYSQL_RES *pResultSet = NULL;
	unsigned long ulAffectedRows = 0;

	int iRes = mysqlConn->Query(strQuery, &pResultSet, ulAffectedRows);
	if (1 == iRes)
	{
		// select
		map<string, struct MySqlCnnC602_DF::DataInRow> mapRowData;
		if (0 != mysqlConn->FetchNextRow(&pResultSet, mapRowData))
		{
			string strNum(mapRowData["stock_balance"].strValue);

			Tgw_StringUtil::String2UInt64_strtoui64(strNum, ui64cQuantity);
		}
		else
		{
			iReturn = 1;
		}

		// �ͷ�
		mysqlConn->FreeResult(&pResultSet);
		pResultSet = NULL;

	}
	else
	{
		iReturn = -1;
	}

	//�黹����
	simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);
	return iReturn;
}

/*
ִ�� ���������ļ� ����
*/
int ProcSocketMsg::ExecCmd_Settlement(const struct simutgw::SocketMsgInfo& in_msgInfo,
	std::string& out_strError)
{
	static const string ftag("ProcSocketMsg::ExecCmd_Settlement() ");

	// �����ļ�·��
	string strDay;
	string strSettlePath;
	vector<string> vctSettleGroups;

	int iRes = simutgw::Simutgw_Settle(vctSettleGroups, strDay, strSettlePath);
	if (iRes < 0)
	{
		EzLog::e(ftag, "MakeSettlement() faild");

		return -1;
	}

	std::vector<std::string> out_vectFiles;
	FileOperHelper::ListFilesInPath(strSettlePath, out_vectFiles);

	return 0;
}