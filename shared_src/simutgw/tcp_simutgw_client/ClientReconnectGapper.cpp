#include "ClientReconnectGapper.h"

#include "tool_string/TimeStringUtil.h"

const unsigned long ClientReconnectGapper::DISCONNECT_TIME_GAP = 5 * 60;

ClientReconnectGapper::ClientReconnectGapper(void)
{
	m_ulDisconnectTime = 0;
}

ClientReconnectGapper::~ClientReconnectGapper(void)
{
}

void ClientReconnectGapper::RecordDisconnectTime(void)
{
	m_ulDisconnectTime = TimeStringUtil::GetTimeStamp();
}

/*
对比上次断线时间，是否已经过了间隔，允许断线重连
*/
bool ClientReconnectGapper::CanReconnect(void)
{
	unsigned long ulNow = TimeStringUtil::GetTimeStamp();

	unsigned long ulGap = ulNow - m_ulDisconnectTime;

	if ( DISCONNECT_TIME_GAP < ulGap )
	{
		// 超过时间间隔，允许重连
		return true;
	}

	return false;
}