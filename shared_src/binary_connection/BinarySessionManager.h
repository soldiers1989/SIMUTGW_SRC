#ifndef __BINARY_SESSION_MANAGER_H__
#define __BINARY_SESSION_MANAGER_H__

#include <map>
#include <memory>

#include "boost/thread/mutex.hpp"

#include "util/EzLog.h"

#include "BinarySession.h"

/*
Session Manager.
*/
class BinarySessionManager
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	typedef std::map<uint64_t, std::shared_ptr<BinarySession>> BinarySessionMap_t;

	BinarySessionMap_t m_connMap;

	boost::mutex m_mutex;

	//
	// Functions
	//
public:
	BinarySessionManager(void);
	virtual ~BinarySessionManager(void);

	void AddConnection(uint64_t clientId, std::shared_ptr<BinarySession> client);

	bool RemoveConnection(uint64_t clientId);

	std::shared_ptr<BinarySession> GetConnection(uint64_t clientId);

	void CloseAllConnections();

	int GetAllConnectionIds(std::vector<uint64_t>& out_vIds);

	/*
	遍历session检查心跳，生成待发送心跳列表
	同时更新本端已发送心跳时间

	@param vector<uint64_t>& out_vctNeedHeartbtIds : 需要发心跳的id列表
	@param vector<uint64_t>& out_vctNeedDisconnectIds : 需要断开连接的id列表

	@return
	*/
	int Gen_SendHeartBeatList(vector<uint64_t>& out_vctNeedHeartbtIds, vector<uint64_t>& out_vctNeedDisconnectIds);

	/*
	更新心跳时间

	@param uint64_t clientId : 客户端id
	@param bool bSelf : 是否需要更新本端发送心跳
	true -- 需要
	false -- 不需要
	@param bool bCounterParty : 是否需要更新对端发送心跳
	true -- 需要
	false -- 不需要
	*/
	int UpdateHeartBeatTime(uint64_t clientId, bool bSelf, bool bCounterParty);
	
	/*
	设置下一条发送消息序号
	*/
	int SetSeq(uint64_t clientId, int64_t i64Seq);

	/*
	取下一条发送消息序号
	*/
	int GetSeq(uint64_t clientId, int64_t& out_i64Seq);
};

#endif