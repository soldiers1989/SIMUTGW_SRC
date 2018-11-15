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
�Ա��ϴζ���ʱ�䣬�Ƿ��Ѿ����˼���������������
*/
bool ClientReconnectGapper::CanReconnect(void)
{
	unsigned long ulNow = TimeStringUtil::GetTimeStamp();

	unsigned long ulGap = ulNow - m_ulDisconnectTime;

	if ( DISCONNECT_TIME_GAP < ulGap )
	{
		// ����ʱ��������������
		return true;
	}

	return false;
}