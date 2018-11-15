#ifndef __BINARY_SESSION_H__
#define __BINARY_SESSION_H__

#include <string>
#include <stdint.h>
#include <memory.h>
#include <vector>
#include <time.h>

#include "boost/thread/mutex.hpp"

#include "util/EzLog.h"

/*
binary ��ص�socket session��
*/
class BinarySession
{
	//
	// Members
	//
public:
	string m_info;

protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// id for this connection
	uint64_t m_id;

	// ���ͷ�����
	std::string m_strSenderCompId;

	// ���շ�����
	std::string m_strTargetCompId;

	// �����������λΪ�롣��������ϵͳϵͳ��½ʱ�ṩ����������
	uint32_t m_uiHeartBtInt;

	// mutex for NextSendSeq
	boost::mutex m_mutex_NextSendSeq;

	// ��һ��������Ϣ���
	int64_t m_i64NextSendSeq;

	// �������غͶ�������ϵͳ֮����Heartbeat ��Ϣ�����TCP ���ӵ�״̬����˵�һ
	// ���������ݷ��Ϳ�����ʱ����Ҫ��ʱ����Heartbeat ��Ϣ�Թ�������ӵĽ����ȡ�

	// �����ϴη���ʱ��
	time_t m_tm_Self_LastSendTime;

	// �Զ��ϴη���ʱ��
	time_t m_tm_Counterparty_LastSendTime;

	//
	// Functions
	//
public:
	/*
	@param const std::string strSenderCompId : ���ͷ�����
	@param const std::string strTargetCompId : ���շ�����
	@param uint32_t uiHeartBtInt : �����������λΪ�롣��������ϵͳϵͳ��½ʱ�ṩ����������
	*/
	BinarySession(const uint64_t cid, const std::string& strSenderCompId,
		const std::string& strTargetCompId, uint32_t uiHeartBtInt);

	virtual ~BinarySession();

	/*
	������һ��������Ϣ���
	*/
	void SetSeq(int64_t i64Seq)
	{
		m_i64NextSendSeq = i64Seq;
	}

	/*
	ȡ��һ��������Ϣ���
	*/
	bool GetSeq(int64_t& out_i64Seq)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutex_NextSendSeq);

		out_i64Seq = m_i64NextSendSeq++;

		return true;
	}

	/*
	��������ʱ�� ����
	*/
	void UpdateHeartBeat_Self(void)
	{
		time(&m_tm_Self_LastSendTime);
	}

	/*
	��������ʱ�� �Զ�
	*/
	void UpdateHeartBeat_Counterparty(void)
	{
		time(&m_tm_Counterparty_LastSendTime);
	}

	/*
	����Ƿ���Ҫ����������Ϣ

	@return 0 : ����Ҫ����
	@return 1 : ��Ҫ���˷���������Ϣ
	@return -1 : �Զ�������ʱ����Ҫ�Ͽ�����
	*/
	int IsHeartBeatNeeded(void);

protected:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	BinarySession();
};

#endif