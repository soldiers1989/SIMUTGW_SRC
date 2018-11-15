#ifndef __CLIENT_RECONNECT_GAPPER_H__
#define __CLIENT_RECONNECT_GAPPER_H__

class ClientReconnectGapper
{
	//
	// Members
	//
protected:
	//上次断线时间
	unsigned long m_ulDisconnectTime;

	// 重连间隔
	static const unsigned long DISCONNECT_TIME_GAP;

	//
	// Functions
	//
public:
	ClientReconnectGapper(void);
	~ClientReconnectGapper(void);

	/*
	记录断线时间
	*/
	void RecordDisconnectTime(void);

	/*
	对比上次断线时间，是否已经过了间隔，允许断线重连
	*/
	bool CanReconnect(void);
};

#endif