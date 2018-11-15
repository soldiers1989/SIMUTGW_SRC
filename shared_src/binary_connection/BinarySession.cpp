#include "BinarySession.h"

BinarySession::BinarySession(const uint64_t cid, const std::string& strSenderCompId,
	const std::string& strTargetCompId, uint32_t uiHeartBtInt)
	:m_scl(keywords::channel = "BinarySession"), m_id(cid),
	m_strSenderCompId(strSenderCompId), m_strTargetCompId(strTargetCompId),
	m_uiHeartBtInt(uiHeartBtInt), m_i64NextSendSeq(1)
{
	m_info = "SenderCompId=";
	m_info += m_strSenderCompId;
	m_info += ",TargetCompId=";
	m_info += m_strTargetCompId;

	// �����ϴη���ʱ��
	time(&m_tm_Self_LastSendTime);

	// �Զ��ϴη���ʱ��
	time(&m_tm_Counterparty_LastSendTime);
}

BinarySession::~BinarySession()
{
}

/*
����Ƿ���Ҫ����������Ϣ

@return 0 : ����Ҫ����
@return 1 : ��Ҫ���˷���������Ϣ
@return -1 : �Զ�������ʱ����Ҫ�Ͽ�����
*/
int BinarySession::IsHeartBeatNeeded(void)
{
	static const string ftag("IsHeartBeatNeeded() ");

	time_t tmNow = time(NULL);

	// �����ϴη���ʱ�� ���
	time_t tmSelfSendGap = tmNow - m_tm_Self_LastSendTime;

	// �Զ��ϴη���ʱ�� ���
	time_t tmCounterpartySendGap = tmNow - m_tm_Counterparty_LastSendTime;

	// �ж϶Զ������Ƿ����
	if ( tmCounterpartySendGap >= m_uiHeartBtInt * 5 )
	{
		// ����5������ʱ�䣬��Ҫ�Ͽ�����
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag
			<< " breaking out because of heartbeat overtime=" << tmCounterpartySendGap << "s, info=" << m_info;
		return -1;
	}

	// �жϱ��������Ƿ����
	if ( m_uiHeartBtInt < tmSelfSendGap )
	{
		// ��Ҫ��������
		return 1;
	}

	return 0;
}