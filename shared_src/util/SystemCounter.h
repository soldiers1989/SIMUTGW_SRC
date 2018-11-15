#ifndef __SYSTEM_COUNTER_H__
#define __SYSTEM_COUNTER_H__

#include <map>
#include <string>
#include <stdint.h>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "util/EzLog.h"

#include "boost/thread/locks.hpp"
#include "boost/thread/shared_mutex.hpp"

#include "cache/WorkCounter_atomic.h"
#include "cache/WorkCounter_nolock.h"

class SystemCounter
{
	//
	//	member
	//
private:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	//
	boost::mutex m_mutex;

	// ϵͳ�ڲ�ͳ��
	// ͳ�Ƶ���ˮ���ж�������
	// ����
	std::shared_ptr<WorkCounter_atomic> m_Counter_inner_sz;
	// �Ϻ�
	std::shared_ptr<WorkCounter_atomic> m_Counter_inner_sh;

	// ������·ͳ��
	// ͳ����·�ж�������״̬
	// ע�⣺��map�����еļ���������ͳ������д���ͽ����ۺ�
	std::map<std::string, std::shared_ptr<WorkCounter_nolock>> m_mapCounters_link_sz;
	// �Ϻ���·ͳ��
	// ͳ����·�ж�������״̬
	// ע�⣺��map�����еļ���������ͳ������д���ͽ����ۺ�
	std::map<std::string, std::shared_ptr<WorkCounter_nolock>> m_mapCounters_link_sh;

	//
	//	function
	//
public:
	SystemCounter(void);
	virtual ~SystemCounter(void);

	const std::shared_ptr<WorkCounter_atomic>& GetSz_InnerCounter()
	{
		return m_Counter_inner_sz;
	}

	const std::shared_ptr<WorkCounter_atomic>& GetSh_InnerCounter()
	{
		return m_Counter_inner_sh;
	}

	// ����������·������
	void AddSz_LinkCounter(const std::string& sName);

	// �����Ϻ���·������
	void AddSh_LinkCounter(const std::string& sName);

	// ����������·���� receive
	void IncSz_Link_Receive(const std::string& sName);

	// ����������·���� SendBack
	void IncSz_Link_SendBack(const std::string& sName);

	// �����Ϻ���·���� receive
	void IncSh_Link_Receive(const std::string& sName);

	// �����Ϻ���·���� SendBack
	void IncSh_Link_SendBack(const std::string& sName);

	// ��ӡ�ɽ�ͳ����Ϣ
	int Print(rapidjson::Document& doc, rapidjson::Document::AllocatorType &allocator);

	/*
	ȡ�����ڲ��������

	@param uint64_t& out_ui64_receive : �յ�����
	@param uint64_t& out_ui64_confirm : ȷ�ϼ���
	@param uint64_t& out_ui64_matchall : ȫ���ɽ�����
	@param uint64_t& out_ui64_matchpart : �ֱʳɽ�����
	@param uint64_t& out_ui64_matchcancel : ��������
	@param uint64_t& out_ui64_error : ���󵥼���
	*/
	void GetSz_Inner(uint64_t& out_ui64_receive,
		uint64_t& out_ui64_confirm,
		uint64_t& out_ui64_matchall,
		uint64_t& out_ui64_matchpart,
		uint64_t& out_ui64_matchcancel,
		uint64_t& out_ui64_error);

	// ȡ������·����
	void GetSz_Link(rapidjson::Document& doc, rapidjson::Document::AllocatorType &allocator, uint64_t& out_ui64Sendback);

	/*
	ȡ�Ϻ��ڲ��������

	@param uint64_t& out_ui64_receive : �յ�����
	@param uint64_t& out_ui64_confirm : ȷ�ϼ���
	@param uint64_t& out_ui64_matchall : ȫ���ɽ�����
	@param uint64_t& out_ui64_matchpart : �ֱʳɽ�����
	@param uint64_t& out_ui64_matchcancel : ��������
	@param uint64_t& out_ui64_error : ���󵥼���
	*/
	void GetSh_Inner(uint64_t& out_ui64_receive,
		uint64_t& out_ui64_confirm,
		uint64_t& out_ui64_matchall,
		uint64_t& out_ui64_matchpart,
		uint64_t& out_ui64_matchcancel,
		uint64_t& out_ui64_error);

	// ȡ�Ϻ���·����
	void GetSh_Link(rapidjson::Document& doc, rapidjson::Document::AllocatorType &allocator, uint64_t& out_ui64Sendback);;
};

#endif