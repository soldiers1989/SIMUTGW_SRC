#ifndef __CLIENT_RECONNECT_GAPPER_H__
#define __CLIENT_RECONNECT_GAPPER_H__

class ClientReconnectGapper
{
	//
	// Members
	//
protected:
	//�ϴζ���ʱ��
	unsigned long m_ulDisconnectTime;

	// �������
	static const unsigned long DISCONNECT_TIME_GAP;

	//
	// Functions
	//
public:
	ClientReconnectGapper(void);
	~ClientReconnectGapper(void);

	/*
	��¼����ʱ��
	*/
	void RecordDisconnectTime(void);

	/*
	�Ա��ϴζ���ʱ�䣬�Ƿ��Ѿ����˼���������������
	*/
	bool CanReconnect(void);
};

#endif