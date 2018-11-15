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
binary 相关的socket session类
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

	// 发送方代码
	std::string m_strSenderCompId;

	// 接收方代码
	std::string m_strTargetCompId;

	// 心跳间隔，单位为秒。订单管理系统系统登陆时提供给交易网关
	uint32_t m_uiHeartBtInt;

	// mutex for NextSendSeq
	boost::mutex m_mutex_NextSendSeq;

	// 下一条发送消息序号
	int64_t m_i64NextSendSeq;

	// 交易网关和订单管理系统之间用Heartbeat 消息来检测TCP 连接的状态，因此当一
	// 方处于数据发送空闲期时，需要定时发送Heartbeat 消息以供检测连接的健康度。

	// 本端上次发送时间
	time_t m_tm_Self_LastSendTime;

	// 对端上次发送时间
	time_t m_tm_Counterparty_LastSendTime;

	//
	// Functions
	//
public:
	/*
	@param const std::string strSenderCompId : 发送方代码
	@param const std::string strTargetCompId : 接收方代码
	@param uint32_t uiHeartBtInt : 心跳间隔，单位为秒。订单管理系统系统登陆时提供给交易网关
	*/
	BinarySession(const uint64_t cid, const std::string& strSenderCompId,
		const std::string& strTargetCompId, uint32_t uiHeartBtInt);

	virtual ~BinarySession();

	/*
	设置下一条发送消息序号
	*/
	void SetSeq(int64_t i64Seq)
	{
		m_i64NextSendSeq = i64Seq;
	}

	/*
	取下一条发送消息序号
	*/
	bool GetSeq(int64_t& out_i64Seq)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutex_NextSendSeq);

		out_i64Seq = m_i64NextSendSeq++;

		return true;
	}

	/*
	更新心跳时间 本端
	*/
	void UpdateHeartBeat_Self(void)
	{
		time(&m_tm_Self_LastSendTime);
	}

	/*
	更新心跳时间 对端
	*/
	void UpdateHeartBeat_Counterparty(void)
	{
		time(&m_tm_Counterparty_LastSendTime);
	}

	/*
	检测是否需要发送心跳消息

	@return 0 : 不需要发送
	@return 1 : 需要本端发送心跳消息
	@return -1 : 对端心跳超时，需要断开连接
	*/
	int IsHeartBeatNeeded(void);

protected:
	// 禁止使用默认构造函数
	BinarySession();
};

#endif