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

	// 本端上次发送时间
	time(&m_tm_Self_LastSendTime);

	// 对端上次发送时间
	time(&m_tm_Counterparty_LastSendTime);
}

BinarySession::~BinarySession()
{
}

/*
检测是否需要发送心跳消息

@return 0 : 不需要发送
@return 1 : 需要本端发送心跳消息
@return -1 : 对端心跳超时，需要断开连接
*/
int BinarySession::IsHeartBeatNeeded(void)
{
	static const string ftag("IsHeartBeatNeeded() ");

	time_t tmNow = time(NULL);

	// 本端上次发送时间 间隔
	time_t tmSelfSendGap = tmNow - m_tm_Self_LastSendTime;

	// 对端上次发送时间 间隔
	time_t tmCounterpartySendGap = tmNow - m_tm_Counterparty_LastSendTime;

	// 判断对端心跳是否过期
	if ( tmCounterpartySendGap >= m_uiHeartBtInt * 5 )
	{
		// 超过5倍心跳时间，需要断开连接
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag
			<< " breaking out because of heartbeat overtime=" << tmCounterpartySendGap << "s, info=" << m_info;
		return -1;
	}

	// 判断本端心跳是否过期
	if ( m_uiHeartBtInt < tmSelfSendGap )
	{
		// 需要发送心跳
		return 1;
	}

	return 0;
}