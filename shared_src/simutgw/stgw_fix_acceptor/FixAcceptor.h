#ifndef __FIX_ACCEPTOR_H__
#define __FIX_ACCEPTOR_H__

#include <string>
#include <iostream>
#include <fstream>

#include "util/EzLog.h"

#include "quickfix/FileStore.h"
#include "quickfix/ThreadedSocketAcceptor.h"
#include "quickfix/Log.h"
#include "quickfix/FileLog.h"
#include "quickfix/SessionSettings.h"
#include "StgwApplication.h"

class FixAcceptorManager
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// Acceptor�Ƿ�������
	// true -- ������
	// false -- δ����
	bool m_bIsStarted;
	FIX::SessionSettings* m_p_settings;

	StgwApplication* m_p_application;
	FIX::FileStoreFactory* m_p_storeFactory;
	FIX::FileLogFactory* m_p_logFactory;
	FIX::ThreadedSocketAcceptor* m_p_acceptor;

	//
	// Functions
	//
public:
	FixAcceptorManager() :
		m_scl(keywords::channel = "FixAcceptorManager"),
		m_bIsStarted( false ),
		m_p_settings( nullptr ), m_p_application( nullptr ), m_p_storeFactory( nullptr ), m_p_logFactory( nullptr ), m_p_acceptor( nullptr )
	{
	}

	~FixAcceptorManager()
	{
		CleanUp();
	}

	/*
	����ThreadedSocketAcceptor

	@param const std::string& confFile : �����ļ�·��
	*/
	int Start( const std::string& confFile );

	/*
	����ThreadedSocketAcceptor

	@param const std::string& configuration : �����ַ���

	@return 0: �����ɹ�
	@return 1: ��������������������
	@return -1: ����ʧ��
	*/
	int StartByStrconfig( const std::string& configuration );

	/*
	�ر�ThreadedSocketAcceptor
	*/
	void Stop( void );

	/*
	������Ϣ
	*/
	int SendMsg(FIX::Message& message);
protected:
	/*
	Clean up
	*/
	void CleanUp( void )
	{
		if ( nullptr != m_p_acceptor )
		{
			delete m_p_acceptor;
			m_p_acceptor = nullptr;
		}

		if ( nullptr != m_p_logFactory )
		{
			delete m_p_logFactory;
			m_p_logFactory = nullptr;
		}

		if ( nullptr != m_p_storeFactory )
		{
			delete m_p_storeFactory;
			m_p_storeFactory = nullptr;
		}

		if ( nullptr != m_p_application )
		{
			delete m_p_application;
			m_p_application = nullptr;
		}
		if ( nullptr != m_p_settings )
		{
			delete m_p_settings;
			m_p_settings = nullptr;
		}

		m_bIsStarted = false;

	}

};


#endif