#include "FixAcceptor.h"

#include <exception>
#include "util/EzLog.h"

using namespace std;

/*
����ThreadedSocketAcceptor

@param const std::string& confFile : �����ļ�·��

@return 0: �����ɹ�
@return 1: ��������������������
@return -1: ����ʧ��
*/
int FixAcceptorManager::Start(const std::string& confFile)
{
	const static string ftag("FixAcceptorManager::Start() ");
	if (m_bIsStarted)
	{
		// ������������
		return 1;
	}

	if (confFile.empty())
	{
		// �ļ�����
		EzLog::e(ftag, "empty filename");
		return -1;
	}

	try
	{
		m_p_settings = new FIX::SessionSettings(confFile);
		if (nullptr == m_p_settings)
		{
			EzLog::e(ftag, "new FIX::SessionSettings failed");
			CleanUp();
			return -1;
		}

		m_p_application = new StgwApplication();
		if (nullptr == m_p_application)
		{
			EzLog::e(ftag, "new StgwApplication failed");
			CleanUp();
			return -1;
		}

		m_p_storeFactory = new FIX::FileStoreFactory(*m_p_settings);
		if (nullptr == m_p_storeFactory)
		{
			EzLog::e(ftag, "new FIX::FileStoreFactory failed");
			CleanUp();
			return -1;
		}

		m_p_logFactory = new FIX::FileLogFactory(*m_p_settings);
		if (nullptr == m_p_logFactory)
		{
			EzLog::e(ftag, "new FIX::FileLogFactory failed");
			CleanUp();
			return -1;
		}

		m_p_acceptor = new FIX::ThreadedSocketAcceptor(*m_p_application, *m_p_storeFactory, *m_p_settings, *m_p_logFactory);
		if (nullptr == m_p_acceptor)
		{
			EzLog::e(ftag, "new FIX::ThreadedSocketAcceptor failed");
			CleanUp();
			return -1;
		}

		m_p_acceptor->start();

		m_bIsStarted = true;

		return 0;
	}
	catch (std::exception & e)
	{
		string sErr("exception ");
		sErr += e.what();
		EzLog::e(ftag, sErr);
		EzLog::ex(ftag, e);

		return -1;
	}
}


/*
����ThreadedSocketAcceptor

@param const std::string& configuration : �����ַ���

@return 0: �����ɹ�
@return 1: ��������������������
@return -1: ����ʧ��
*/
int FixAcceptorManager::StartByStrconfig(const std::string& configuration)
{
	const static string ftag("FixAcceptorManager::StartByStrconfig() ");
	if (m_bIsStarted)
	{
		// ������������
		return 1;
	}

	if (configuration.empty())
	{
		// �ļ�����
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "empty configuration string";
		return -1;
	}

	try
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "try to start fix with config=" << configuration;		
		std::istringstream input(configuration);

		m_p_settings = new FIX::SessionSettings();
		if (nullptr == m_p_settings)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new FIX::SessionSettings failed";
			CleanUp();
			return -1;
		}

		input >> (*m_p_settings);

		m_p_application = new StgwApplication();
		if (nullptr == m_p_application)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new StgwApplication failed";
			CleanUp();
			return -1;
		}

		m_p_storeFactory = new FIX::FileStoreFactory(*m_p_settings);
		if (nullptr == m_p_storeFactory)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new FIX::FileStoreFactory failed";
			CleanUp();
			return -1;
		}

		m_p_logFactory = new FIX::FileLogFactory(*m_p_settings);
		if (nullptr == m_p_logFactory)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new FIX::FileLogFactory failed";
			CleanUp();
			return -1;
		}

		m_p_acceptor = new FIX::ThreadedSocketAcceptor(*m_p_application, *m_p_storeFactory, *m_p_settings, *m_p_logFactory);
		if (nullptr == m_p_acceptor)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "new FIX::ThreadedSocketAcceptor failed";
			CleanUp();
			return -1;
		}

		m_p_acceptor->start();

		m_bIsStarted = true;

		return 0;
	}
	catch (std::exception & e)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << e.what();
		EzLog::ex(ftag, e);

		return -1;
	}
}

/*
�ر�ThreadedSocketAcceptor
*/
void FixAcceptorManager::Stop(void)
{
	if (!m_bIsStarted)
	{
		// �ѹرգ�����
		return;
	}

	if (nullptr != m_p_acceptor)
	{
		m_p_acceptor->stop();
	}

	CleanUp();

	return;
}

/*
������Ϣ
0 -- �ɹ�
-1 -- ʧ��
*/
int FixAcceptorManager::SendMsg(FIX::Message& message)
{
	if (!m_bIsStarted)
	{
		return -1;
	}

	m_p_application->SendFIXMessage(message, message.getSessionID());

	return 0;
}