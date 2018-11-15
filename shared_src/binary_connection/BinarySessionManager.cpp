#include "BinarySessionManager.h"

#include "util/EzLog.h"

using namespace std;

BinarySessionManager::BinarySessionManager(void)
	:m_scl(keywords::channel = "BinarySessionManager")
{
}

BinarySessionManager::~BinarySessionManager(void)
{
}

void BinarySessionManager::AddConnection(uint64_t clientId, std::shared_ptr<BinarySession> client)
{
	static const string ftag("AddConnection() ");

	boost::unique_lock<boost::mutex> Locker(m_mutex);

	std::pair<BinarySessionMap_t::iterator, bool> ret = m_connMap.insert(
		std::make_pair(clientId, client));

	bool inserted = ret.second;
	if (inserted)
	{
		BOOST_LOG_SEV(m_scl, trivial::info) << ftag << " id=" << clientId << " info " << client->m_info;
	}
}

bool BinarySessionManager::RemoveConnection(uint64_t clientId)
{
	static const string ftag("RemoveConnection() ");

	boost::unique_lock<boost::mutex> Locker(m_mutex);

	BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "RemoveConnection id=" << clientId;

	if (0 < m_connMap.erase(clientId))
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::shared_ptr<BinarySession> BinarySessionManager::GetConnection(uint64_t clientId)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	BinarySessionMap_t::iterator itr = m_connMap.find(clientId);

	if (m_connMap.end() != itr)
	{
		return itr->second;
	}

	return shared_ptr<BinarySession>();
}

void BinarySessionManager::CloseAllConnections()
{
	static const string ftag("CloseAllConnections() ");

	boost::unique_lock<boost::mutex> Locker(m_mutex);

	m_connMap.clear();

	BOOST_LOG_SEV(m_scl, trivial::info) << ftag << "CloseAllConnections";
}

int BinarySessionManager::GetAllConnectionIds(std::vector<uint64_t>& out_vIds)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	BinarySessionMap_t::iterator itr = m_connMap.begin();
	while (m_connMap.end() != itr)
	{
		out_vIds.push_back(itr->first);
		++itr;
	}

	return 0;
}

/*
����session������������ɴ����������б�

@param vector<uint64_t>& out_vctNeedHeartbtIds : ��Ҫ��������id�б�
@param vector<uint64_t>& out_vctNeedDisconnectIds : ��Ҫ�Ͽ����ӵ�id�б�

@return
*/
int BinarySessionManager::Gen_SendHeartBeatList(vector<uint64_t>& out_vctNeedHeartbtIds, vector<uint64_t>& out_vctNeedDisconnectIds)
{
	static const string ftag("Gen_SendHeartBeatList() ");

	boost::unique_lock<boost::mutex> Locker(m_mutex);

	int iHeartBtRes = 0;

	BinarySessionMap_t::iterator itr = m_connMap.begin();
	while (m_connMap.end() != itr)
	{
		if (nullptr == itr->second)
		{
			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "found nullptr id=" << itr->first;
		}
		else
		{
			iHeartBtRes = itr->second->IsHeartBeatNeeded();
			if (1 == iHeartBtRes)
			{
				// ��Ҫ���˷���������Ϣ
				out_vctNeedHeartbtIds.push_back(itr->first);

				itr->second->UpdateHeartBeat_Self();
			}
			else if (-1 == iHeartBtRes)
			{
				// �Զ�������ʱ����Ҫ�Ͽ�����
				out_vctNeedDisconnectIds.push_back(itr->first);
			}
			else
			{
				// do nothing
			}
		}

		++itr;
	}

	return 0;
}

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
int BinarySessionManager::UpdateHeartBeatTime(uint64_t clientId, bool bSelf, bool bCounterParty)
{
	static const string ftag("UpdateHeartBeatTime() ");

	boost::unique_lock<boost::mutex> Locker(m_mutex);

	BinarySessionMap_t::iterator itr = m_connMap.find(clientId);
	if (m_connMap.end() == itr)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "not found id=" << clientId;
		return -1;
	}

	if (nullptr == itr->second)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "found nullptr id=" << itr->first;
		return -1;
	}

	if (bSelf)
	{
		itr->second->UpdateHeartBeat_Self();
	}

	if (bCounterParty)
	{
		itr->second->UpdateHeartBeat_Counterparty();
	}

	return 0;
}

/*
������һ��������Ϣ���
*/
int BinarySessionManager::SetSeq(uint64_t clientId, int64_t i64Seq)
{
	static const string ftag("SetSeq() ");

	boost::unique_lock<boost::mutex> Locker(m_mutex);

	BinarySessionMap_t::iterator itr = m_connMap.find(clientId);
	if (m_connMap.end() == itr)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "not found id=" << clientId;
		return -1;
	}

	if (nullptr == itr->second)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "found nullptr id=" << itr->first;
		return -1;
	}

	itr->second->SetSeq(i64Seq);

	return 0;
}

/*
ȡ��һ��������Ϣ���
*/
int BinarySessionManager::GetSeq(uint64_t clientId, int64_t& out_i64Seq)
{
	static const string ftag("SetSeq() ");

	boost::unique_lock<boost::mutex> Locker(m_mutex);

	BinarySessionMap_t::iterator itr = m_connMap.find(clientId);
	if (m_connMap.end() == itr)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "not found id=" << clientId;
		return -1;
	}

	if (nullptr == itr->second)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "found nullptr id=" << itr->first;
		return -1;
	}

	itr->second->GetSeq(out_i64Seq);

	return 0;
}