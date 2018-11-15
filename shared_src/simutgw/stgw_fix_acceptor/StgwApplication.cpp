
#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 )
#else

#endif

#include "StgwApplication.h"

#include "quickfix/Session.h"

#include "quickfix/fix40/ExecutionReport.h"
#include "quickfix/fix41/ExecutionReport.h"
#include "quickfix/fix42/ExecutionReport.h"
#include "quickfix/fix43/ExecutionReport.h"
#include "quickfix/fix44/ExecutionReport.h"
#include "quickfix/fix50/ExecutionReport.h"

#include "simutgw/stgw_fix_acceptor/StgwMsgHelper.h"

#include "simutgw/stgw_config/g_values_biz.h"
#include "simutgw_config/g_values_sys_run_config.h"

#include "util/EzLog.h"


void StgwApplication::onCreate( const FIX::SessionID& sessionID )
{

}

void StgwApplication::onLogon( const FIX::SessionID& sessionID )
{
	// static const std::string ftag("StgwApplication::onLogon() ");

	StgwMsgHelper::ProcLogOn(sessionID);

	// 回平台状态消息 Platform State Info
	FIX::Message report_stateinfo;
	StgwMsgHelper::Get_PlatformStateInfo(sessionID, report_stateinfo);
	
	SendFIXMessage(report_stateinfo, sessionID);

	if (simutgw::g_bSZ_Step_ver110)
	{
		// 深圳STEP回报是Ver1.10
		// 回平台信息消息 Platform Info
		FIX::Message report_platinfo;
		StgwMsgHelper::Get_PlatformInfo(sessionID, report_platinfo);

		SendFIXMessage(report_platinfo, sessionID);
	}
}

void StgwApplication::onLogout( const FIX::SessionID& sessionID )
{
	StgwMsgHelper::ProcLogOut(sessionID);
}

void StgwApplication::toAdmin( FIX::Message& message,
	const FIX::SessionID& sessionID )
{
}

void StgwApplication::toApp( FIX::Message& message,
	const FIX::SessionID& sessionID )
	throw( FIX::DoNotSend )
{

}

void StgwApplication::fromAdmin( const FIX::Message& message,
	const FIX::SessionID& sessionID )
	throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon )
{
	static const std::string ftag("StgwApplication::fromAdmin() ");
	
	FIX::Message report;

	int iRes = StgwMsgHelper::ProcAdminMsg(message, sessionID, report);
	if (0 != iRes)
	{
		EzLog::e(ftag, "error admin msg");
	}
}

void StgwApplication::fromApp( const FIX::Message& message,
	const FIX::SessionID& sessionID )
	throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
{
	static const std::string ftag("StgwApplication::fromApp() ");

	FIX::Message report;

	int iRes = StgwMsgHelper::ProcAppMsg(message, sessionID, report);
	if (0 != iRes)
	{
		EzLog::e(ftag, "error app msg");
	}
}

void StgwApplication::onMessage( const FIX40::NewOrderSingle& message,
	const FIX::SessionID& sessionID )
{
	static const std::string ftag("StgwApplication::onMessage() ");

	EzLog::i(ftag, "FIX40 onMessage");

	FIX::Symbol symbol;
	FIX::Side side;
	FIX::OrdType ordType;
	FIX::OrderQty orderQty;
	FIX::Price price;
	FIX::ClOrdID clOrdID;
	FIX::Account account;

	message.get( ordType );

	if ( ordType != FIX::OrdType_LIMIT )
		throw FIX::IncorrectTagValue( ordType.getField() );

	message.get( symbol );
	message.get( side );
	message.get( orderQty );
	message.get( price );
	message.get( clOrdID );

	FIX40::ExecutionReport executionReport = FIX40::ExecutionReport
		( FIX::OrderID( genOrderID() ),
		FIX::ExecID( genExecID() ),
		FIX::ExecTransType( FIX::ExecTransType_NEW ),
		FIX::OrdStatus( FIX::OrdStatus_FILLED ),
		symbol,
		side,
		orderQty,
		FIX::LastShares( orderQty ),
		FIX::LastPx( price ),
		FIX::CumQty( orderQty ),
		FIX::AvgPx( price ) );

	executionReport.set( clOrdID );

	if ( message.isSet( account ) )
		executionReport.setField( message.get( account ) );

	try
	{
		FIX::Session::sendToTarget( executionReport, sessionID );
	}
	catch ( FIX::SessionNotFound& )
	{
	}
}

void StgwApplication::onMessage( const FIX41::NewOrderSingle& message,
	const FIX::SessionID& sessionID )
{
	static const std::string ftag("StgwApplication::onMessage() ");

	EzLog::i(ftag, "FIX41 onMessage");

	FIX::Symbol symbol;
	FIX::Side side;
	FIX::OrdType ordType;
	FIX::OrderQty orderQty;
	FIX::Price price;
	FIX::ClOrdID clOrdID;
	FIX::Account account;

	message.get( ordType );

	if ( ordType != FIX::OrdType_LIMIT )
		throw FIX::IncorrectTagValue( ordType.getField() );

	message.get( symbol );
	message.get( side );
	message.get( orderQty );
	message.get( price );
	message.get( clOrdID );

	FIX41::ExecutionReport executionReport = FIX41::ExecutionReport
		( FIX::OrderID( genOrderID() ),
		FIX::ExecID( genExecID() ),
		FIX::ExecTransType( FIX::ExecTransType_NEW ),
		FIX::ExecType( FIX::ExecType_FILL ),
		FIX::OrdStatus( FIX::OrdStatus_FILLED ),
		symbol,
		side,
		orderQty,
		FIX::LastShares( orderQty ),
		FIX::LastPx( price ),
		FIX::LeavesQty( 0 ),
		FIX::CumQty( orderQty ),
		FIX::AvgPx( price ) );

	executionReport.set( clOrdID );

	if ( message.isSet( account ) )
		executionReport.setField( message.get( account ) );

	try
	{
		FIX::Session::sendToTarget( executionReport, sessionID );
	}
	catch ( FIX::SessionNotFound& )
	{
	}
}

void StgwApplication::onMessage( const FIX42::NewOrderSingle& message,
	const FIX::SessionID& sessionID )
{
	static const std::string ftag("StgwApplication::onMessage() ");

	EzLog::i(ftag, "FIX42 onMessage");

	FIX::Symbol symbol;
	FIX::Side side;
	FIX::OrdType ordType;
	FIX::OrderQty orderQty;
	FIX::Price price;
	FIX::ClOrdID clOrdID;
	FIX::Account account;

	message.get( ordType );

	if ( ordType != FIX::OrdType_LIMIT )
		throw FIX::IncorrectTagValue( ordType.getField() );

	message.get( symbol );
	message.get( side );
	message.get( orderQty );
	message.get( price );
	message.get( clOrdID );

	FIX42::ExecutionReport executionReport = FIX42::ExecutionReport
		( FIX::OrderID( genOrderID() ),
		FIX::ExecID( genExecID() ),
		FIX::ExecTransType( FIX::ExecTransType_NEW ),
		FIX::ExecType( FIX::ExecType_FILL ),
		FIX::OrdStatus( FIX::OrdStatus_FILLED ),
		symbol,
		side,
		FIX::LeavesQty( 0 ),
		FIX::CumQty( orderQty ),
		FIX::AvgPx( price ) );

	executionReport.set( clOrdID );
	executionReport.set( orderQty );
	executionReport.set( FIX::LastShares( orderQty ) );
	executionReport.set( FIX::LastPx( price ) );

	if ( message.isSet( account ) )
		executionReport.setField( message.get( account ) );

	try
	{
		FIX::Session::sendToTarget( executionReport, sessionID );
	}
	catch ( FIX::SessionNotFound& )
	{
	}
}

void StgwApplication::onMessage( const FIX43::NewOrderSingle& message,
	const FIX::SessionID& sessionID )
{
	static const std::string ftag("StgwApplication::onMessage() ");

	EzLog::i(ftag, "FIX43 onMessage");

	FIX::Symbol symbol;
	FIX::Side side;
	FIX::OrdType ordType;
	FIX::OrderQty orderQty;
	FIX::Price price;
	FIX::ClOrdID clOrdID;
	FIX::Account account;

	message.get( ordType );

	if ( ordType != FIX::OrdType_LIMIT )
		throw FIX::IncorrectTagValue( ordType.getField() );

	message.get( symbol );
	message.get( side );
	message.get( orderQty );
	message.get( price );
	message.get( clOrdID );

	FIX43::ExecutionReport executionReport = FIX43::ExecutionReport
		( FIX::OrderID( genOrderID() ),
		FIX::ExecID( genExecID() ),
		FIX::ExecType( FIX::ExecType_FILL ),
		FIX::OrdStatus( FIX::OrdStatus_FILLED ),
		side,
		FIX::LeavesQty( 0 ),
		FIX::CumQty( orderQty ),
		FIX::AvgPx( price ) );

	executionReport.set( clOrdID );
	executionReport.set( symbol );
	executionReport.set( orderQty );
	executionReport.set( FIX::LastQty( orderQty ) );
	executionReport.set( FIX::LastPx( price ) );

	if ( message.isSet( account ) )
		executionReport.setField( message.get( account ) );

	try
	{
		FIX::Session::sendToTarget( executionReport, sessionID );
	}
	catch ( FIX::SessionNotFound& )
	{
	}
}

void StgwApplication::onMessage( const FIX44::NewOrderSingle& message,
	const FIX::SessionID& sessionID )
{
	static const std::string ftag("StgwApplication::onMessage() ");

	EzLog::i(ftag, "FIX44 onMessage");

	FIX::Symbol symbol;
	FIX::Side side;
	FIX::OrdType ordType;
	FIX::OrderQty orderQty;
	FIX::Price price;
	FIX::ClOrdID clOrdID;
	FIX::Account account;

	message.get( ordType );

	if ( ordType != FIX::OrdType_LIMIT )
		throw FIX::IncorrectTagValue( ordType.getField() );

	message.get( symbol );
	message.get( side );
	message.get( orderQty );
	message.get( price );
	message.get( clOrdID );

	FIX44::ExecutionReport executionReport = FIX44::ExecutionReport
		( FIX::OrderID( genOrderID() ),
		FIX::ExecID( genExecID() ),
		FIX::ExecType( FIX::ExecType_FILL ),
		FIX::OrdStatus( FIX::OrdStatus_FILLED ),
		side,
		FIX::LeavesQty( 0 ),
		FIX::CumQty( orderQty ),
		FIX::AvgPx( price ) );

	executionReport.set( clOrdID );
	executionReport.set( symbol );
	executionReport.set( orderQty );
	executionReport.set( FIX::LastQty( orderQty ) );
	executionReport.set( FIX::LastPx( price ) );

	if ( message.isSet( account ) )
		executionReport.setField( message.get( account ) );

	try
	{
		FIX::Session::sendToTarget( executionReport, sessionID );
	}
	catch ( FIX::SessionNotFound& )
	{
	}
}

void StgwApplication::onMessage( const FIX50::NewOrderSingle& message,
	const FIX::SessionID& sessionID )
{
	static const std::string ftag("StgwApplication::onMessage() ");

	EzLog::i(ftag, "FIX50 onMessage");

	FIX::Symbol symbol;
	FIX::Side side;
	FIX::OrdType ordType;
	FIX::OrderQty orderQty;
	FIX::Price price;
	FIX::ClOrdID clOrdID;
	FIX::Account account;

	message.get( ordType );

	if ( ordType != FIX::OrdType_LIMIT )
		throw FIX::IncorrectTagValue( ordType.getField() );

	message.get( symbol );
	message.get( side );
	message.get( orderQty );
	message.get( price );
	message.get( clOrdID );

	FIX50::ExecutionReport executionReport = FIX50::ExecutionReport
		( FIX::OrderID( genOrderID() ),
		FIX::ExecID( genExecID() ),
		FIX::ExecType( FIX::ExecType_FILL ),
		FIX::OrdStatus( FIX::OrdStatus_FILLED ),
		side,
		FIX::LeavesQty( 0 ),
		FIX::CumQty( orderQty ) );

	executionReport.set( clOrdID );
	executionReport.set( symbol );
	executionReport.set( orderQty );
	executionReport.set( FIX::LastQty( orderQty ) );
	executionReport.set( FIX::LastPx( price ) );
	executionReport.set( FIX::AvgPx( price ) );

	if ( message.isSet( account ) )
		executionReport.setField( message.get( account ) );

	try
	{
		FIX::Session::sendToTarget( executionReport, sessionID );
	}
	catch ( FIX::SessionNotFound& )
	{
	}
}

// 发送消息
int StgwApplication::SendFIXMessage(FIX::Message& fixReport, const FIX::SessionID& sessionID)
{
	static const std::string ftag("StgwApplication::SendFIXMessage() ");
	try
	{
		FIX::Session::sendToTarget(fixReport, sessionID);
	}
	catch (FIX::SessionNotFound& e)
	{
		EzLog::ex(ftag, e);

		return -1;
	}
	catch (exception& e)
	{
		EzLog::ex(ftag, e);
		return -1;
	}
	catch (...)
	{
		EzLog::e(ftag, "exception");
		return -1;
	}
	return 0;
}