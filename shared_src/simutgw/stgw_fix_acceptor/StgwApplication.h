#ifndef __STGW_APPLICATION_H__
#define __STGW_APPLICATION_H__

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/Values.h"
#include "quickfix/Utility.h"
#include "quickfix/Mutex.h"

#include "quickfix/fix40/NewOrderSingle.h"
#include "quickfix/fix41/NewOrderSingle.h"
#include "quickfix/fix42/NewOrderSingle.h"
#include "quickfix/fix43/NewOrderSingle.h"
#include "quickfix/fix44/NewOrderSingle.h"
#include "quickfix/fix50/NewOrderSingle.h"

namespace FIX
{
	//DEFINE_INT(ReportIndex);
	//DEFINE_INT(PlatformID);
	//DEFINE_INT(PlatformStatus);

	/*
	SJS Add begin
	深圳交易所自定义MsgType
	*/
	const char MsgType_ReportSynchronization[] = "U101";
	const char MsgType_PlatformStateInfo[] = "U102";
	const char MsgType_PlatformInfo[] = "U104";
	/*
	SJS Add end
	*/
	namespace FIELD
	{
		const int NoSecurity = 8902;
		const int DeliverQty = 8903;
		const int SubstCash = 8904;
		const int ReportIndex = 10179;
		const int PlatformID = 10180;
		const int PlatformStatus = 10181;
		// 平台分区数量
		const int NoPartitions = 10196;
		// 平台分区号
		const int PartitionNo = 10197;
	}
}

class StgwApplication
	: public FIX::Application, public FIX::MessageCracker
{
public:
	StgwApplication() : m_orderID( 0 ), m_execID( 0 )
	{
	}

	// Application overloads
	void onCreate( const FIX::SessionID& );
	void onLogon( const FIX::SessionID& sessionID );
	void onLogout( const FIX::SessionID& sessionID );
	void toAdmin( FIX::Message&, const FIX::SessionID& );
	void toApp( FIX::Message&, const FIX::SessionID& )
		throw( FIX::DoNotSend );
	void fromAdmin( const FIX::Message&, const FIX::SessionID& )
		throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon );
	void fromApp( const FIX::Message& message, const FIX::SessionID& sessionID )
		throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType );

	// MessageCracker overloads
	void onMessage( const FIX40::NewOrderSingle&, const FIX::SessionID& );
	void onMessage( const FIX41::NewOrderSingle&, const FIX::SessionID& );
	void onMessage( const FIX42::NewOrderSingle&, const FIX::SessionID& );
	void onMessage( const FIX43::NewOrderSingle&, const FIX::SessionID& );
	void onMessage( const FIX44::NewOrderSingle&, const FIX::SessionID& );
	void onMessage( const FIX50::NewOrderSingle&, const FIX::SessionID& );

	// 发送消息
	int SendFIXMessage(FIX::Message&, const FIX::SessionID&);

	std::string genOrderID()
	{
		std::stringstream stream;
		stream << ++m_orderID;
		return stream.str();
	}
	std::string genExecID()
	{
		std::stringstream stream;
		stream << ++m_execID;
		return stream.str();
	}
private:
	int m_orderID, m_execID;
};


#endif