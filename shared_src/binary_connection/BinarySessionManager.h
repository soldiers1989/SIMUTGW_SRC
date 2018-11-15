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
	����session������������ɴ����������б�
	ͬʱ���±����ѷ�������ʱ��

	@param vector<uint64_t>& out_vctNeedHeartbtIds : ��Ҫ��������id�б�
	@param vector<uint64_t>& out_vctNeedDisconnectIds : ��Ҫ�Ͽ����ӵ�id�б�

	@return
	*/
	int Gen_SendHeartBeatList(vector<uint64_t>& out_vctNeedHeartbtIds, vector<uint64_t>& out_vctNeedDisconnectIds);

	/*
	��������ʱ��

	@param uint64_t clientId : �ͻ���id
	@param bool bSelf : �Ƿ���Ҫ���±��˷�������
	true -- ��Ҫ
	false -- ����Ҫ
	@param bool bCounterParty : �Ƿ���Ҫ���¶Զ˷�������
	true -- ��Ҫ
	false -- ����Ҫ
	*/
	int UpdateHeartBeatTime(uint64_t clientId, bool bSelf, bool bCounterParty);
	
	/*
	������һ��������Ϣ���
	*/
	int SetSeq(uint64_t clientId, int64_t i64Seq);

	/*
	ȡ��һ��������Ϣ���
	*/
	int GetSeq(uint64_t clientId, int64_t& out_i64Seq);
};

#endif