#include "ReportMsg_Sh.h"

#include "simutgw/db_oper/RecordTradeInfo.h"
#include "simutgw/db_oper/RecordReportHelper.h"

#include "order/StockOrderHelper.h"
#include "simutgw/order/OrderMemoryStoreFactory.h"

#include "tool_redis/Tgw_RedisHelper.h"

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"
#include "util/TimeDuration.h"
#include "util/SystemCounter.h"

#include "config/conf_mssql_table.h"
#include "simutgw/stgw_config/g_values_biz.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "match_rule/RuleWordProc_ShDb.h"

src::severity_channel_logger<trivial::severity_level, std::string>
ReportMsg_Sh::m_scl(keywords::channel = "ReportMsg_Sh");

ReportMsg_Sh::ReportMsg_Sh()
{
}


ReportMsg_Sh::~ReportMsg_Sh()
{
}

/*
����һ���ر�

Reutrn:
0 -- �ɹ�
-1 -- ʧ��
*/
int ReportMsg_Sh::ProcSingleReport(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	vector<string>& out_vectSqlStr)
{
	static const string ftag("ReportMsg_Sh::ProcSingleReport() ");

	int iRes = 0;

	string strReport;

	if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		//	����
		string strSendLog("wrong place SZ Report clordid=");
		strSendLog += in_ptrReport->strClordid;
		strSendLog += (", client=");
		strSendLog += in_ptrReport->strSenderCompID;

		EzLog::e(ftag, strSendLog);
	}
	else if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// �Ϻ�
		if (0 != in_ptrReport->tradePolicy.ui64RuleId)
		{
			// �Ѷ����˳ɽ�����
			if (nullptr == in_ptrReport->tradePolicy.ptrRule_Sh)
			{
				string sDebug("error order cliordid=");
				sDebug += in_ptrReport->strClordid;
				sDebug += "nullptr Rule Sh, ruleId=";
				string sItoa;
				sof_string::itostr(in_ptrReport->tradePolicy.ui64RuleId, sItoa);
				sDebug += sItoa;
				EzLog::e(ftag, sDebug);

				return -1;
			}

			// report with rule
			iRes = FixToSHReport_JsonRule(in_ptrReport, out_vectSqlStr);
		}
		else
		{
			/* �ȸ��±� */
			iRes = RecordTradeInfo::WriteTransInfoInDb(in_ptrReport);

			iRes = FixToSHReport(in_ptrReport, out_vectSqlStr);
		}

		if (0 != iRes)
		{
			return iRes;
		}

		string strSendLog("Sended SH Cjhb reff=");
		strSendLog += in_ptrReport->strClordid;
		strSendLog += ", sh_conn=";
		strSendLog += in_ptrReport->strSessionId;

		EzLog::i(ftag, strSendLog);
	}
	else
	{
		//nothing
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "clordid=" << in_ptrReport->strClordid
			<< ", client=" << in_ptrReport->strSenderCompID << " error Trade_market=" << in_ptrReport->strTrade_market;

		return -1;
	}

	return iRes;
}


/*
FIXЭ���ʽ����ת����FIXЭ���ʽ�Ϻ�ȷ�ϣ�����һ��sql��

Reutrn:
0 -- �ɹ�
-1 -- ʧ��
*/
int ReportMsg_Sh::FixToSHConfirm(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::vector<std::string>& out_vectSqlStr)
{
	static const string strTag("ReportMsg_Sh::FixToSHConfirm() ");

	string strInsertConfirmSql = "INSERT INTO ";
	strInsertConfirmSql += simutgw::g_strSH_Ordwth2_TableName;
	strInsertConfirmSql += " (rec_num,date,time,reff,acc,stock,"
		"bs,price,qty,status,qty2,remark,status1,teordernum,owflag,ordrec,firmid,branchid,checkord) VALUES(";

	size_t pos = in_ptrReport->strTransactTime.find("-");
	if (string::npos == pos)
	{
		string strError("ί��clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]TransactTime[";
		strError += in_ptrReport->strTransactTime;
		strError += "]��ʽ����";
		EzLog::e(strTag, strError);

		return -1;
	}
	//�ɽ����ڣ���ʽΪYYYYMMDD, strTransactTime:YYYYMMDD-HH:MM:SS.sss
	string strTransDate = in_ptrReport->strTransactTime.substr(0, pos);

	size_t posTime = in_ptrReport->strTransactTime.find(".");
	if (string::npos == posTime)
	{
		string strError("ί��clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]TransactTime[";
		strError += in_ptrReport->strTransactTime;
		strError += "]��ʽ����";
		EzLog::e(strTag, strError);

		return -1;
	}
	//�ɽ�ʱ�䣬��ʽΪHH:MM:SS
	string strTransTime = in_ptrReport->strTransactTime.substr(pos + 1, posTime - pos - 1);

	string strValue;

	//rec_num
	uint64_t ui64Value = 0;
	string strName = in_ptrReport->strSessionId;
	// ����session
	if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
	{
		ui64Value = simutgw::g_mapShConns[strName].IncRecNum();
	}
	else
	{
		std::string strError("�Ϻ�connection[");
		strError += strName;
		strError += "]��ʧ";
		EzLog::e(strTag, strError);
		return -1;
	}

	strInsertConfirmSql += sof_string::itostr(ui64Value, strValue);
	strInsertConfirmSql += ",'";

	//date,time,reff,acc,stock,
	strInsertConfirmSql += strTransDate;
	strInsertConfirmSql += "','";

	strInsertConfirmSql += strTransTime;
	strInsertConfirmSql += "','";

	if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_NEW_ORDER))
	{
		strInsertConfirmSql += in_ptrReport->strClordid;
	}
	else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL) ||
		0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_CANCELREJECT) ||
		0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		//����ʱ��strOrigClordid�෴
		strInsertConfirmSql += in_ptrReport->strOrigClordid;
	}
	else
	{
		std::string strError("ί��clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]Msgtype[";
		strError += in_ptrReport->strMsgType;
		strError += "]����";
		EzLog::e(strTag, strError);
		return -1;
	}
	strInsertConfirmSql += "','";

	strInsertConfirmSql += in_ptrReport->strAccountId;
	strInsertConfirmSql += "','";

	strInsertConfirmSql += in_ptrReport->strStockID;
	strInsertConfirmSql += "','";

	//bs,price,qty,status,qty2,
	strInsertConfirmSql += in_ptrReport->strSide;
	strInsertConfirmSql += "','";

	Tgw_StringUtil::iLiToStr(in_ptrReport->ui64mOrderPrice, strValue, 3);

	strInsertConfirmSql += strValue;
	strInsertConfirmSql += "','";


	strInsertConfirmSql += in_ptrReport->strOrderqty_origin;
	strInsertConfirmSql += "','";

	/*
	����״̬��
	��F����ʾ��������̨�жϸö���Ϊ�ϵ���
	��E����ʾ������ǰ̨�жϸö���Ϊ�ϵ�����ʱremark���ֶ�12��������Ϣ������������롣
	��?����ʾͨ�Ź��ϡ�
	��O����ʾ�Ͻ����ɹ����ոñ��걨��
	��W����ʾ�Ͻ����ɹ����ܸñʳ�����
	����������Ϊ�����嵵��ʱ�ɽ�ʣ���Զ��������м۶���ʱ���Ҷ�����Чʱ�����ֶ�ȡֵΪ��W����

	*/
	if (simutgw::ErrorMatch == in_ptrReport->enMatchType)
	{
		strValue = "F";
	}
	else if (0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		strValue = "W";
	}
	else
	{
		if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_NEW_ORDER))
		{
			//	
			strValue = "O";
		}
		else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
		{
			//	
			strValue = "W";
		}
		else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_CANCELREJECT))
		{
			strValue = "F";
		}
		else
		{
			std::string strError("ί��clordid[");
			strError += in_ptrReport->strClordid;
			strError += "]Msgtype[";
			strError += in_ptrReport->strMsgType;
			strError += "]����";
			EzLog::e(strTag, strError);
			return -1;
		}
	}

	strInsertConfirmSql += strValue;
	strInsertConfirmSql += "',";

	//qty2
	/*
	����������
	�����޼۶����걨��¼�����ֶ�Ϊ�գ�
	���ڳ�����¼�����ֶ�Ϊʵ�ʳ�������������
	���������嵵��ʱ�ɽ�ʣ���Զ��������м۶���������걨���ֳɽ������ֶ�ȡֵΪ�Զ�����������������걨ȫ���ɽ�������ֶ�ȡֵΪ0��

	*/
	if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_NEW_ORDER) ||
		0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_CANCELREJECT))
	{
		strInsertConfirmSql += "'','";
	}
	else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL) ||
		0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += in_ptrReport->strLastQty;
		strInsertConfirmSql += "','";
	}
	else
	{
		std::string strError("ί��clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]Msgtype[";
		strError += in_ptrReport->strMsgType;
		strError += "]����";
		EzLog::e(strTag, strError);
		return -1;
	}

	// remark,status1,teordernum,owflag,ordrec
	if (0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		strInsertConfirmSql += "0";
	}
	else
	{
		if (in_ptrReport->strErrorCode.empty())
		{
			strInsertConfirmSql += " ";
		}
		else
		{
			strInsertConfirmSql += in_ptrReport->strErrorCode;
		}
	}
	strInsertConfirmSql += "','";

	if (0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		strInsertConfirmSql += "R";
	}
	else
	{
		strInsertConfirmSql += "R";
		//strInsertConfirmSql += "P";
	}
	strInsertConfirmSql += "','";

	if (0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		uint64_t ui64Value = 0;
		// ����session
		if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
		{
			ui64Value = simutgw::g_mapShConns[strName].IncRecNum();
		}
		strInsertConfirmSql += sof_string::itostr(ui64Value, strValue);
	}
	else
	{
		//strInsertConfirmSql += sof_string::itostr(simutgw::g_iTeordernum, strValue);
		uint64_t ui64Value = 0;
		// ����session
		if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
		{
			ui64Value = simutgw::g_mapShConns[strName].GetTeordNum();
		}
		strInsertConfirmSql += sof_string::itostr(ui64Value, strValue);
	}
	strInsertConfirmSql += "','";

	if (0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		strInsertConfirmSql += "WTH";
	}
	else
	{
		strInsertConfirmSql += in_ptrReport->strOrdType;
	}
	strInsertConfirmSql += "',";

	if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_NEW_ORDER) ||
		0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_CANCELREJECT))
	{
		strInsertConfirmSql += "0,";
	}
	else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL) ||
		0 == in_ptrReport->strOrdStatus.compare(simutgw::STEPMSG_ORDSTATUS_CANCEL))
	{
		//����ʱ��strClordid�෴
		strInsertConfirmSql += "'";
		strInsertConfirmSql += in_ptrReport->strClordid;
		strInsertConfirmSql += "',";
	}
	else
	{
		std::string strError("ί��clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]Msgtype[";
		strError += in_ptrReport->strMsgType;
		strError += "]����";
		EzLog::e(strTag, strError);
		return -1;
	}

	//	firmid,branchid,checkord
	if (!in_ptrReport->strConfirmID.empty())
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += in_ptrReport->strConfirmID;
		strInsertConfirmSql += "','";
	}
	else
	{
		strInsertConfirmSql += "null,'";
	}

	strInsertConfirmSql += in_ptrReport->strMarket_branchid;
	strInsertConfirmSql += "',0x00000000000000000000000000000000);";

	out_vectSqlStr.push_back(strInsertConfirmSql);
	return 0;
}

/*
FIXЭ���ʽ���� ��JSON���ù��� ת����FIXЭ���ʽ�Ϻ�ȷ�ϣ�����һ��sql��

Reutrn:
0 -- �ɹ�
-1 -- ʧ��
*/
int ReportMsg_Sh::FixToSHConfirm_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::vector<std::string>& out_vectSqlStr)
{
	static const string ftag("ReportMsg_Sh::FixToSHConfirm_JsonRule() ");

	if (!in_ptrReport->tradePolicy.ptrRule_Sh->docRuleConfirm.IsObject())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] error Rule match, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

		return -1;
	}

	RuleWordProc_ShDb shdbRuleProc;
	int iResolveRes = 0;
	string strResolveValue("");

	rapidjson::Value& elem = in_ptrReport->tradePolicy.ptrRule_Sh->docRuleConfirm;
	if (!elem.IsObject())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] error Rule confirm, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

		return -1;
	}

	string strInsertConfirmSql = "INSERT INTO ";
	strInsertConfirmSql += simutgw::g_strSH_Ordwth2_TableName;
	strInsertConfirmSql += " (rec_num,date,time,reff,acc,stock,"
		"bs,price,qty,status,qty2,remark,status1,teordernum,owflag,ordrec,firmid,branchid,checkord) VALUES(";


	string strValue;

	//rec_num
	iResolveRes = shdbRuleProc.ResolveRule(elem, "rec_num", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- rec_num";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	//date,time,reff,acc,stock,
	iResolveRes = shdbRuleProc.ResolveRule(elem, "date", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- date";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "time", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- time";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "reff", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- reff";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "acc", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- acc";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "stock", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- stock";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	//bs,price,qty,status,qty2,
	iResolveRes = shdbRuleProc.ResolveRule(elem, "bs", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- bs";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "price", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- price";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "qty", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- qty";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}
	/*
	����״̬��
	��F����ʾ��������̨�жϸö���Ϊ�ϵ���
	��E����ʾ������ǰ̨�жϸö���Ϊ�ϵ�����ʱremark���ֶ�12��������Ϣ������������롣
	��?����ʾͨ�Ź��ϡ�
	��O����ʾ�Ͻ����ɹ����ոñ��걨��
	��W����ʾ�Ͻ����ɹ����ܸñʳ�����
	����������Ϊ�����嵵��ʱ�ɽ�ʣ���Զ��������м۶���ʱ���Ҷ�����Чʱ�����ֶ�ȡֵΪ��W����

	*/
	iResolveRes = shdbRuleProc.ResolveRule(elem, "status", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- status";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}


	//qty2
	/*
	����������
	�����޼۶����걨��¼�����ֶ�Ϊ�գ�
	���ڳ�����¼�����ֶ�Ϊʵ�ʳ�������������
	���������嵵��ʱ�ɽ�ʣ���Զ��������м۶���������걨���ֳɽ������ֶ�ȡֵΪ�Զ�����������������걨ȫ���ɽ�������ֶ�ȡֵΪ0��

	*/
	iResolveRes = shdbRuleProc.ResolveRule(elem, "qty2", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- qty2";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	// remark,status1,teordernum,owflag,ordrec
	iResolveRes = shdbRuleProc.ResolveRule(elem, "remark", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- remark";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "status1", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- status1";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "teordernum", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- teordernum";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "owflag", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- owflag";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "ordrec", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- ordrec";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	//	firmid,branchid,checkord
	iResolveRes = shdbRuleProc.ResolveRule(elem, "firmid", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- firmid";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "branchid", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- branchid";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "NULL,";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "',";
	}

	iResolveRes = shdbRuleProc.ResolveRule(elem, "checkord", in_ptrReport, strResolveValue);
	if (-1 == iResolveRes)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- checkord";
		return -1;
	}
	if (strResolveValue.empty())
	{
		strInsertConfirmSql += "0x00000000000000000000000000000000";
	}
	else
	{
		strInsertConfirmSql += "'";
		strInsertConfirmSql += strResolveValue;
		strInsertConfirmSql += "'";
	}

	strInsertConfirmSql += ");";

	out_vectSqlStr.push_back(strInsertConfirmSql);
	return 0;
}

/*
FIXЭ���ʽ����ת����FIXЭ���ʽ�Ϻ��ر�������һ��sql��
*/
int ReportMsg_Sh::FixToSHReport(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	vector<string>& out_vectSqlStr)
{
	static const string strTag("ReportMsg_Sh::FixToSHReport() ");

	string strInsertReportSql = "INSERT INTO ";
	strInsertReportSql += simutgw::g_strSH_Cjhb_TableName;
	strInsertReportSql += "(gddm,gdxm,bcrq,cjbh,gsdm,cjsl,bcye,zqdm,"
		"sbsj,cjsj,cjjg,cjje,sqbh,bs,mjbh) VALUES('";

	size_t pos = in_ptrReport->strTransactTime.find("-");
	if (string::npos == pos)
	{
		string strError("ί��clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]TransactTime[";
		strError += in_ptrReport->strTransactTime;
		strError += "]��ʽ����";
		EzLog::e(strTag, strError);

		return -1;
	}
	//�ɽ����ڣ���ʽΪYYYYMMDD, strTransactTime:YYYYMMDD-HH:MM:SS.sss
	string strTransDate = in_ptrReport->strTransactTime.substr(0, pos);

	size_t posTime = in_ptrReport->strTransactTime.find(".");
	if (string::npos == posTime)
	{
		string strError("ί��clordid[");
		strError += in_ptrReport->strClordid;
		strError += "]TransactTime[";
		strError += in_ptrReport->strTransactTime;
		strError += "]��ʽ����";
		EzLog::e(strTag, strError);

		return -1;
	}
	//�ɽ�ʱ�䣬��ʽΪHHMMSS
	string strTransTime = in_ptrReport->strTransactTime.substr(pos + 1, posTime - pos - 1);
	while (true)
	{
		if (string::npos == strTransTime.find(":"))
		{
			break;
		}

		strTransTime.replace(strTransTime.find(":"), 1, "");
	}

	//gddm,gdxm,bcrq,cjbh,gsdm,
	strInsertReportSql += in_ptrReport->strAccountId;
	strInsertReportSql += "','";

	strInsertReportSql += "1','";

	strInsertReportSql += strTransDate;
	strInsertReportSql += "','";

	uint64_t ui64Value = 0;
	string strName = in_ptrReport->strSessionId;
	// ����session
	if (simutgw::g_mapShConns.end() != simutgw::g_mapShConns.find(strName))
	{
		ui64Value = simutgw::g_mapShConns[strName].IncCjbh();
	}
	else
	{
		std::string strError("�Ϻ�connection[");
		strError += strName;
		strError += "]��ʧ";
		EzLog::e(strTag, strError);
		return -1;
	}

	sof_string::itostr(ui64Value, in_ptrReport->strOrderID);
	strInsertReportSql += in_ptrReport->strOrderID;
	strInsertReportSql += "','";

	strInsertReportSql += in_ptrReport->strSecurity_seat;
	strInsertReportSql += "','";

	//cjsl,bcye,zqdm,sbsj,cjsj
	strInsertReportSql += in_ptrReport->strLastQty;
	strInsertReportSql += "','";

	strInsertReportSql += "0000000001','";

	strInsertReportSql += in_ptrReport->strStockID;
	strInsertReportSql += "','";

	//sbsj
	strInsertReportSql += strTransTime;
	strInsertReportSql += "','";

	//cjsj
	strInsertReportSql += strTransTime;
	strInsertReportSql += "','";

	//cjjg,cjje,sqbh,bs,mjbh
	// cjjg	�ɽ��۸񣬵�λΪ�����Ԫ(A�ɡ�����ծȯ)����Ԫ(B��)��ÿ��Ԫ�ʽ����������(��ծ�ع�)������ΪС�����3λ��
	string strItoa;
	strInsertReportSql += Tgw_StringUtil::iLiToStr(in_ptrReport->ui64mPrice_matched, strItoa, 3);
	strInsertReportSql += "','";


	/*
	cjje �ɽ�������ΪС�����2λ��
	���ʵ�ʳɽ�����999,999,999.99��ϵͳ����д-1����ʹ�ø��ֶε��г����������⴦��
	��̨ϵͳӦ��ʶ�𲢼�ʱ������ֶ�����쳣���ɲ�ȡ�����м�����ֵ������̺���õǼǽ������ݻ�������ʽ�������ø��쳣��ʶ��ʹ���
	*/
	//strInsertReportSql += in_ptrReport->strCashorderqty;
	if (in_ptrReport->ui64mCashorderqty_matched > 999999999990)
	{
		strInsertReportSql += "-1";
	}
	else
	{
		strInsertReportSql += Tgw_StringUtil::iLiToStr(in_ptrReport->ui64mCashorderqty_matched, strItoa, 2);
	}
	strInsertReportSql += "','";

	strInsertReportSql += in_ptrReport->strClordid;
	strInsertReportSql += "','";

	strInsertReportSql += in_ptrReport->strSide;

	strInsertReportSql += "','";

	strInsertReportSql += "66666";
	strInsertReportSql += "');";

	out_vectSqlStr.push_back(strInsertReportSql);

	return 0;
}


/*
FIXЭ���ʽ���� ��JSON���ù��� ת����FIXЭ���ʽ�Ϻ��ر�������һ��sql��

Reutrn:
0 -- �ɹ�
-1 -- ʧ��
*/
int ReportMsg_Sh::FixToSHReport_JsonRule(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	vector<string>& out_vectSqlStr)
{
	static const string ftag("ReportMsg_Sh::FixToSHReport_JsonRule() ");

	if (!in_ptrReport->tradePolicy.ptrRule_Sh->docRuleMatch.IsArray())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
			<< "] error Rule match, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

		return -1;
	}

	RuleWordProc_ShDb shdbRuleProc;
	int iResolveRes = 0;
	string strResolveValue("");

	for (rapidjson::SizeType i = 0; i < in_ptrReport->tradePolicy.ptrRule_Sh->docRuleMatch.Size(); ++i)
	{
		rapidjson::Value& elem = in_ptrReport->tradePolicy.ptrRule_Sh->docRuleMatch[i];
		if (!elem.IsObject())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] error Rule match, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

			return -1;
		}

		string strInsertReportSql = "INSERT INTO ";
		strInsertReportSql += simutgw::g_strSH_Cjhb_TableName;
		strInsertReportSql += "(gddm,gdxm,bcrq,cjbh,gsdm,cjsl,bcye,zqdm,"
			"sbsj,cjsj,cjjg,cjje,sqbh,bs,mjbh) VALUES(";

		//gddm,gdxm,bcrq,cjbh,gsdm,
		iResolveRes = shdbRuleProc.ResolveRule(elem, "gddm", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- gddm";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "gdxm", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- gdxm";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "bcrq", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- bcrq";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "cjbh", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- cjbh";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "gsdm", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- gsdm";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		//cjsl,bcye,zqdm,sbsj,cjsj
		iResolveRes = shdbRuleProc.ResolveRule(elem, "cjsl", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- cjsl";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "bcye", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- bcye";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "zqdm", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- zqdm";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "sbsj", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- sbsj";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "cjsj", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- cjsj";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		//cjjg,cjje,sqbh,bs,mjbh
		// cjjg	�ɽ��۸񣬵�λΪ�����Ԫ(A�ɡ�����ծȯ)����Ԫ(B��)��ÿ��Ԫ�ʽ����������(��ծ�ع�)������ΪС�����3λ��
		iResolveRes = shdbRuleProc.ResolveRule(elem, "cjjg", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- cjjg";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		/*
		cjje �ɽ�������ΪС�����2λ��
		���ʵ�ʳɽ�����999,999,999.99��ϵͳ����д-1����ʹ�ø��ֶε��г����������⴦��
		��̨ϵͳӦ��ʶ�𲢼�ʱ������ֶ�����쳣���ɲ�ȡ�����м�����ֵ������̺���õǼǽ������ݻ�������ʽ�������ø��쳣��ʶ��ʹ���
		*/
		iResolveRes = shdbRuleProc.ResolveRule(elem, "cjje", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- cjje";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "sqbh", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- sqbh";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "bs", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- bs";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL,";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "',";
		}

		iResolveRes = shdbRuleProc.ResolveRule(elem, "mjbh", in_ptrReport, strResolveValue);
		if (-1 == iResolveRes)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "ί��clordid[" << in_ptrReport->strClordid
				<< "] ruleId=" << in_ptrReport->tradePolicy.ui64RuleId << " error resolve rule -- mjbh";
			return -1;
		}
		if (strResolveValue.empty())
		{
			strInsertReportSql += "NULL";
		}
		else
		{
			strInsertReportSql += "'";
			strInsertReportSql += strResolveValue;
			strInsertReportSql += "'";
		}

		strInsertReportSql += ");";

		out_vectSqlStr.push_back(strInsertReportSql);
	}

	return 0;
}

/*
�����Ϻ�����

@param std::string& out_strSql_confirm : д��ȷ�ϱ������

Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int ReportMsg_Sh::ProcSHCancelOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::vector<std::string>& out_vectSqlStr)
{
	static const string strTag("ReportMsg_Sh::ProcSHCancelOrder() ");

	int iRes = 0;

	if (in_ptrReport->enMatchType == simutgw::CancelMatch)
	{
		if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_EXECREPORT))
		{
			// �����ɹ�
			RecordTradeInfo::WriteTransInfoInDb_CancelSuccess(in_ptrReport);

			iRes = FixToSHConfirm(in_ptrReport, out_vectSqlStr);

			if (0 == iRes)
			{
				string strSendLog("Sended SH Cancel Report reff=");
				strSendLog += in_ptrReport->strClordid;
				strSendLog += ", sh_conn=";
				strSendLog += in_ptrReport->strSessionId;

				EzLog::i(strTag, strSendLog);
			}

			simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchCancel();

		}
		else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_CANCELREJECT))
		{
			/* �ȸ��±�*/
			RecordTradeInfo::WriteTransInfoInDb_CancelFail(in_ptrReport);

			iRes = FixToSHConfirm(in_ptrReport, out_vectSqlStr);

			if (0 == iRes)
			{
				string strSendLog("Sended SH Cancel Report reff=");
				strSendLog += in_ptrReport->strClordid;

				EzLog::i(strTag, strSendLog);
			}
		}

	}

	return 0;
}

/*
�����Ϻ����󶩵�


Return:
0 -- �ɹ�
-1 -- ʧ��
1 -- �޻ر�
*/
int ReportMsg_Sh::ProcSHErrorOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	vector<string>& out_vectSqlStr)
{
	static const string strTag("ReportMsg_Sh::ProcSHErrorOrder() ");

	int iRes = 0;

	//��mysql���ӳ�ȡ����
	RecordTradeInfo::WriteTransInfoInDb_Error(in_ptrReport);

	string strReject, strStepReject;
	if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_NEW_ORDER))
	{

		// ������
		iRes = FixToSHConfirm(in_ptrReport, out_vectSqlStr);

	}
	else if (0 == in_ptrReport->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL))
	{
		// ����

	}

	simutgw::g_counter.GetSh_InnerCounter()->Inc_Error();

	string strSendLog("Sended SH Ordwth2 reff=");
	strSendLog += in_ptrReport->strClordid;

	EzLog::i(strTag, strSendLog);

	return 0;
}

/*
�����Ϻ�ȷ��

Return:
0 -- �ɹ�
-1 -- ʧ��
1 -- �޻ر�
*/
int ReportMsg_Sh::ProcSHConfirmOrder(std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
	std::vector<std::string>& out_vectSqlStr)
{
	static const string ftag("ReportMsg_Sh::ProcSHConfirmOrder() ");

	int iRes = 0;

	if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
	{
		//	����
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "wrong place SZ Report clordid=" << in_ptrReport->strClordid
			<< ", client=" << in_ptrReport->strSenderCompID;

		return -1;
	}
	else if (0 == in_ptrReport->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
	{
		// �Ϻ�
		if (0 != in_ptrReport->tradePolicy.ui64RuleId)
		{
			// �Ѷ����˳ɽ�����
			if (nullptr == in_ptrReport->tradePolicy.ptrRule_Sh)
			{
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "error order cliordid=" << in_ptrReport->strClordid
					<< "nullptr Rule Sh, ruleId=" << in_ptrReport->tradePolicy.ui64RuleId;

				return -1;
			}

			// report with rule
			iRes = FixToSHConfirm_JsonRule(in_ptrReport, out_vectSqlStr);
		}
		else
		{
			iRes = FixToSHConfirm(in_ptrReport, out_vectSqlStr);
		}

		if (0 != iRes)
		{
			return -1;
		}

		return 0;
	}

	BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "clordid=" << in_ptrReport->strClordid
		<< ", client=" << in_ptrReport->strSenderCompID << " error Trade_market=" << in_ptrReport->strTrade_market;

	return -1;
}

/*
�����Ϻ��ر�ҵ��
Return:
0 -- �ɹ�
-1 -- ʧ��
1 -- �޻ر�
*/
int ReportMsg_Sh::Get_SHReport(map<string, vector<string>>& out_mapUpdate)
{
	static const string strTag("ReportMsg_Sh::ProcSHReport() ");

	string strSzReport;
	vector<string> vectSqlStr;

	int iMaxLoopcount = 20000;
	for (int i = 0; i < iMaxLoopcount; ++i)
	{
		// ȡ�Ϻ��ر�
		std::shared_ptr<struct simutgw::OrderMessage> ptrReport(new struct simutgw::OrderMessage);

		int iRes = simutgw::g_outMsg_buffer.PopFront_sh(ptrReport);
		if (iRes < 0)
		{
			EzLog::e(strTag, "ReadReport() faild");

			continue;
		}
		else if (iRes > 0)
		{
			// �޻ر�
			return 0;
		}
		else
		{
			vectSqlStr.clear();

			// ��¼������ˮ
			RecordReportHelper::RecordReportToDb(ptrReport);
			if (ptrReport->enMatchType == simutgw::MatchAll)
			{
				// ���������ɽ��ر�
				iRes = ProcSingleReport(ptrReport, vectSqlStr);
				if (0 == iRes)
				{
					//simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchAll();
				}
			}
			else if (ptrReport->enMatchType == simutgw::MatchPart)
			{
				// ���������ɽ��ر�
				iRes = ProcSingleReport(ptrReport, vectSqlStr);
				if (0 == iRes)
				{
					//simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchPart();
				}
			}
			else if (ptrReport->enMatchType == simutgw::CancelMatch)
			{
				// �������ر�
				iRes = ProcSHCancelOrder(ptrReport, vectSqlStr);

				if (0 == iRes)
				{
					simutgw::g_counter.GetSh_InnerCounter()->Inc_MatchCancel();
				}
			}
			else if (ptrReport->enMatchType == simutgw::ErrorMatch)
			{
				// ����ϵ��ر�
				iRes = ProcSHErrorOrder(ptrReport, vectSqlStr);

				if (0 == iRes)
				{
					simutgw::g_counter.GetSh_InnerCounter()->Inc_Error();
				}
			}
			else if (ptrReport->enMatchType == simutgw::NotMatch)
			{
				// ȷ��
				iRes = ProcSHConfirmOrder(ptrReport, vectSqlStr);
				if (0 == iRes)
				{
					//simutgw::g_counter.GetSh_InnerCounter()->Inc_Confirm();
				}
			}
			else
			{
				std::string strTrans, strError("clordid[");
				strError += ptrReport->strClordid;
				strError += "]MatchType[";
				strError += sof_string::itostr(ptrReport->enMatchType, strTrans);
				EzLog::e(strTag, strError);

				continue;
			}

			if (!vectSqlStr.empty())
			{
				if (out_mapUpdate.end() == out_mapUpdate.find(ptrReport->strSessionId))
				{
					vector<string> vecSql;
					out_mapUpdate.insert(make_pair(ptrReport->strSessionId, vecSql));
				}

				out_mapUpdate[ptrReport->strSessionId].insert(out_mapUpdate[ptrReport->strSessionId].end(),
					vectSqlStr.begin(), vectSqlStr.end());
			}

		}
	}

	return 0;
}
