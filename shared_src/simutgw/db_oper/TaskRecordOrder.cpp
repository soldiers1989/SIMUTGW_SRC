#include "TaskRecordOrder.h"

#include "simutgw/stgw_config/g_values_inner.h"
#include "config/conf_mysql_table.h"

#include "simutgw/msg_biz/ProcCancelOrder.h"


TaskRecordOrder::TaskRecordOrder(const uint64_t uiId) :TaskBase(uiId),
	m_scl(keywords::channel = "TaskRecordOrder")
{
}


TaskRecordOrder::~TaskRecordOrder()
{
}

int TaskRecordOrder::TaskProc(void)
{
	RecordOrderInDb();
	return 0;
}

//	将下单消息写入到数据库，包括流水表、买表和卖表
int TaskRecordOrder::RecordOrderInDb()
{
	static const string ftag("TaskRecordOrder::RecordOrderInDb() ");

	//	从mysql连接池取连接
	std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
	if (NULL == mysqlConn)
	{
		//	取出的mysql连接为NULL

		//	归还连接
		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Get Connection is NULL";

		return -1;
	}

	{
		string strDebug("Write clordid[");
		strDebug += m_orderMsg->strClordid;
		BOOST_LOG_SEV(m_scl, trivial::debug) << ftag << strDebug;
	}

	int iRes = 0;

	//	下单消息写入数据库
	if (0 == m_orderMsg->strMsgType.compare("F"))
	{
		//撤单消息
		iRes = RecordNewOrder(m_orderMsg, mysqlConn);
	}
	else if (0 == m_orderMsg->strMsgType.compare("D"))
	{
		//	买卖单消息
		if (0 == m_orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_B)
			|| 0 == m_orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_BUY_1) 
			|| 0 == m_orderMsg->strSide.compare("D") ||
			0 == m_orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_S)
			|| 0 == m_orderMsg->strSide.compare(simutgw::STEPMSG_SIDE_SELL_2) 
			|| 0 == m_orderMsg->strSide.compare("E"))
		{
			iRes = RecordNewOrder(m_orderMsg, mysqlConn);
		}
		else
		{
			string strError = "未知的买卖方向[";
			strError += m_orderMsg->strSide;
			strError += "].clordid[";
			strError += m_orderMsg->strClordid;
			strError += "]";

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strError;
			iRes = -1;
		}
	}
	else
	{
		//nothing
		string strError = "未知的MsgType[";
		strError += m_orderMsg->strMsgType;
		strError += "].clordid[";
		strError += m_orderMsg->strClordid;
		strError += "]";

		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strError;
		iRes = -1;
	}

	if (iRes < 0)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "Write msg to table faild";
	}

	//	归还连接
	simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

	return iRes;
}

/*
将下单消息插入到表中
Param :
iType :
0 -- new_order

Return :
*/
int TaskRecordOrder::InsertDb_neworder_buy_sell( int iType, std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg,
	std::shared_ptr<MySqlCnnC602> &in_mysqlConn)
{
	static const string ftag("TaskRecordOrder::InsertDb_neworder_buy_sell() ");

	if (nullptr == in_OrderMsg)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "null order ptr";
	}	

	// 转换tmp
	string strTran;

	string strSql = "INSERT INTO ";
	switch (iType)
	{
	case 0:
		strSql += simutgw::g_strSQL_NewOrder_TableName;
		strSql += " (`trade_market`,`trade_type`,"
"`sessionid`,`security_account`,`security_seat`,`trade_group`,`beginstring`,"
"`bodylength`,`checksum`,`clordid`,`securityidsource`,`msgseqnum`,"
"`msgtype`,`orderqty_origin`,`leavesqty`,`ordtype`,`origclordid`,"
"`possdupflag`,`order_price`,`securityid`,`sendercompid`,`sendingtime`,"
"`side`,`targetcompid`,`text`,`timeinforce`,`transacttime`,"
"`positioneffect`,`stoppx`,`minqty`,`origsendingtime`,`cashorderqty`,"
"`coveredoruncovered`,`nopartyids`,`ownertype`,`orderrestrictions`,`cashmargin`,"
"`confirmid`,`maxpricelevels`,`applid`,`market_branchid`,`oper_time`) VALUES('";
		break;	

	default:
		return -1;
	}
	//`trade_market`,`trade_type`,
	if (in_OrderMsg->strTrade_market.empty())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "trade_market不能为空";
		return -1;
	}
	strSql += in_OrderMsg->strTrade_market;
	strSql += "',";

	strSql += sof_string::itostr(in_OrderMsg->iTrade_type, strTran);
	strSql += ",";

	//`sessionid`,`security_account`,`security_seat`,`trade_group`,`beginstring`,
	if (in_OrderMsg->strSessionId.empty())
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "sessionid不能为空";
		return -1;
	}
	strSql += "'";
	strSql += in_OrderMsg->strSessionId;
	strSql += "',";

	if (in_OrderMsg->strMsgType == simutgw::STEPMSG_MSGTYPE_ORDER_CACEL)
	{
		strSql += "null,";
	}
	else
	{
		if (in_OrderMsg->strAccountId.empty())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "security_account不能为空";
			return -1;
		}
		strSql += "'";
		strSql += in_OrderMsg->strAccountId;
		strSql += "',";
	}


	if (in_OrderMsg->strSecurity_seat.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strSecurity_seat;
		strSql += "',";
	}

	if (in_OrderMsg->strTrade_group.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strTrade_group;
		strSql += "',";
	}

	if (in_OrderMsg->strBeginString.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strBeginString;
		strSql += "',";
	}


	//`bodylength`,`checksum`,`clordid`,`securityidsource`,`msgseqnum`,
	if (in_OrderMsg->strBodyLength.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strBodyLength;
		strSql += "',";
	}

	if (in_OrderMsg->strCheckSum.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strCheckSum;
		strSql += "',";
	}

	if (in_OrderMsg->strClordid.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strClordid;
		strSql += "',";
	}

	if (in_OrderMsg->strSecurityIDSource.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strSecurityIDSource;
		strSql += "',";
	}


	if (in_OrderMsg->strMsgSeqNum.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strMsgSeqNum;
		strSql += "',";
	}


	//`msgtype`,`orderqty_origin`,`leavesqty`,`ordtype`,
	if (in_OrderMsg->strMsgType.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strMsgType;
		strSql += "',";
	}

	if (in_OrderMsg->strOrderqty_origin.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strOrderqty_origin;
		strSql += "',";
	}

	if (in_OrderMsg->strLeavesQty.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strLeavesQty;
		strSql += "',";
	}

	if (in_OrderMsg->strOrdType.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strOrdType;
		strSql += "',";
	}

	if (0 == iType)
	{
		//撤单需要原始订单号
		if (0 == in_OrderMsg->strMsgType.compare(simutgw::STEPMSG_MSGTYPE_ORDER_CACEL) &&
			in_OrderMsg->strOrigClordid.empty())
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "OrigClordid不能为空";
			return -1;
		}
		else
		{
			strSql += "'";
			strSql += in_OrderMsg->strOrigClordid;
			strSql += "',";
		}
	}


	//`possdupflag`,`order_price`,`securityid`,`sendercompid`,`sendingtime`,
	if (in_OrderMsg->strPossDupFlag.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strPossDupFlag;
		strSql += "',";
	}

	if (in_OrderMsg->strOrderPrice.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strOrderPrice;
		strSql += "',";
	}

	if (in_OrderMsg->strStockID.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strStockID;
		strSql += "',";
	}

	if (in_OrderMsg->strSenderCompID.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strSenderCompID;
		strSql += "',";
	}


	if (in_OrderMsg->strSendingTime.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strSendingTime;
		strSql += "',";
	}

	//`side`,`targetcompid`,`text`,`timeinforce`,`transacttime`,
	if (in_OrderMsg->strSide.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strSide;
		strSql += "',";
	}

	if (in_OrderMsg->strTargetCompID.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strTargetCompID;
		strSql += "',";
	}

	if (in_OrderMsg->strText.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strText;
		strSql += "',";
	}

	if (in_OrderMsg->strTimeInForce.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strTimeInForce;
		strSql += "',";
	}


	if (in_OrderMsg->strTransactTime.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strTransactTime;
		strSql += "',";
	}

	//`positioneffect`,`stoppx`,`minqty`,`origsendingtime`,`cashorderqty`,
	if (in_OrderMsg->strPositionEffect.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strPositionEffect;
		strSql += "',";
	}

	if (in_OrderMsg->strStoppx.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strStoppx;
		strSql += "',";
	}

	if (in_OrderMsg->strMinQty.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strMinQty;
		strSql += "',";
	}

	if (in_OrderMsg->strOrigSendingTime.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strOrigSendingTime;
		strSql += "',";
	}


	if (in_OrderMsg->strCashorderqty.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strCashorderqty;
		strSql += "',";
	}

	//`coveredoruncovered`,`nopartyids`,`ownertype`,`orderrestrictions`,`cashmargin`,
	if (in_OrderMsg->strCoveredOrUncovered.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strCoveredOrUncovered;
		strSql += "',";
	}

	if (in_OrderMsg->strNoPartyIds.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strNoPartyIds;
		strSql += "',";
	}

	if (in_OrderMsg->strOwnerType.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strOwnerType;
		strSql += "',";
	}

	if (in_OrderMsg->strOrderRestrictions.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strOrderRestrictions;
		strSql += "',";
	}


	if (in_OrderMsg->strCashMargin.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strCashMargin;
		strSql += "',";
	}

	//`confirmid`,`maxpricelevels`,`applid`,`market_branchid`,`opertime`,;
	if (in_OrderMsg->strConfirmID.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strConfirmID;
		strSql += "',";
	}

	if (in_OrderMsg->strMaxPriceLevels.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strMaxPriceLevels;
		strSql += "',";
	}

	if (in_OrderMsg->strApplID.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strApplID;
		strSql += "',";
	}

	if (in_OrderMsg->strMarket_branchid.empty())
	{
		strSql += "null,";
	}
	else
	{
		strSql += "'";
		strSql += in_OrderMsg->strMarket_branchid;
		strSql += "',";
	}
	strSql += "now());";

	MYSQL_RES *pResultSet = NULL;

	try
	{
		unsigned long ulAffectedRows = 0;

		int iRes = in_mysqlConn->Query(strSql, &pResultSet, ulAffectedRows);
		if (2 != iRes)
		{
			in_mysqlConn->FreeResult(&pResultSet);
			pResultSet = NULL;

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strSql;
			return -1;
		}
		in_mysqlConn->FreeResult(&pResultSet);
		pResultSet = NULL;
	}
	catch (exception &e)
	{
		in_mysqlConn->FreeResult(&pResultSet);
		pResultSet = NULL;
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << e.what();
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}

/*
将下单消息插入到表中
Param :
Return :
0 -- 成功
1 -- 重单
*/
int TaskRecordOrder::RecordNewOrder(std::shared_ptr<struct simutgw::OrderMessage> &in_OrderMsg,
	std::shared_ptr<MySqlCnnC602> &in_mysqlConn)
{
	static const string ftag("TaskRecordOrder::RecordNewOrder() ");
	
	try
	{
		int iRes = 0;
		
		//	不重单写入order_record表
		iRes = in_mysqlConn->StartTransaction();
		if (0 != iRes)
		{
			return -1;
		}

		iRes = InsertDb_neworder_buy_sell( 0, in_OrderMsg, in_mysqlConn );
		if (0 != iRes)
		{
			in_mysqlConn->RollBack();
			return -1;
		}

		iRes = in_mysqlConn->Commit();
		if (0 != iRes)
		{
			string strItoa;
			string strDebug("运行[");
			strDebug += "commit";
			strDebug += "]得到";
			strDebug += sof_string::itostr(iRes, strItoa);
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;
		}
	}
	catch (exception &e)
	{
		in_mysqlConn->RollBack();
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << e.what();
		EzLog::ex(ftag, e);

		return -1;
	}

	return 0;
}