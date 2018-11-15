#ifndef __CLIENT_SOCKET_H__
#define __CLIENT_SOCKET_H__

#include <stdint.h>

#include "win_iocpbase/WinSock_Client.h"
#include "win_iocpbase/IOCPUtil.h"
#include "win_iocpbase/IocpEventHandler.h"
#include "win_iocpbase/IocpData_Client.h"
#include "win_iocpbase/IOCP_Thread_Client.h"
#include "win_iocpbase/IOCP_ThreadPool.h"

class ClientSocket
{
	//
	// Members
	//
protected:
	std::string m_strServerIp;
	u_short m_usServer_port;

	std::shared_ptr<IocpEventHandler> m_pIocpHandler;
	std::shared_ptr<IocpData_Client> m_pIocpdata;

	DWORD m_dNumOfThreads;

	std::shared_ptr<IOCP_ThreadPool<IocpData_Client, IOCP_Thread_Client>> m_pIocpThreadPool;

	uint64_t m_ui64MyCid;

	//
	// Functions
	//
public:
	explicit ClientSocket(std::shared_ptr<IocpEventHandler> pIocpHandler);
	virtual ~ClientSocket();

	/*
	��ʼ��������IOCP�̳߳�

	Param :
	void

	Return :
	0 -- ����ɹ�
	-1 -- ����ʧ��
	*/
	int Init(void);

	/*
	��server����socket tcp����

	Param :
	const string& in_ip : Server Ip address.
	const u_short in_port : Server Listen port.

	Return :
	0 -- ���ӳɹ�
	-1 -- ����ʧ��
	*/
	int Connect(const string& in_ip, const u_short in_port);

	//!***************************************************************************
	//! @details
	//! Fully disconnect from a connected client. Once all outstanding sends
	//! are completed, a corresponding OnDisconnect callback will be invoked.
	//!
	//! check class SocketServiceBase::Disconnect()
	//!***************************************************************************
	void Disconnect(void);

	/*
	��server��������

	Param :
	std::shared_ptr<std::vector<uint8_t>>& pdata : �Ѵ���ô�����unsigned char������

	Return :
	void
	*/
	void Send(std::shared_ptr<std::vector<uint8_t>>& pdata);

protected:
	/*
	��ʼ���̳߳�

	Param :
	DWORD numThread : �߳�����

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int InitializeThreadPool(DWORD numThread);

	/*
	�ر�������Դ

	Param :
	void

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int CleanUp(void);

	//!***************************************************************************
	//! @details
	//! Shutdown certain operation on the socket.
	//!
	//! check class SocketServiceBase::Shutdown()
	//!***************************************************************************
	void Shutdown(uint64_t cid, int how);

	/*
	����keepalive
	@Return
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int SetKeepAlive();

	/*
	��server���½���socket tcp����

	Return :
	0 -- ���ӳɹ�
	-1 -- ����ʧ��
	*/
	int ReConnect();
};

#endif