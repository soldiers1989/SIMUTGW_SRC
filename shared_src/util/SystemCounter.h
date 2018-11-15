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

	// 系统内部统计
	// 统计到流水线中订单处理
	// 深圳
	std::shared_ptr<WorkCounter_atomic> m_Counter_inner_sz;
	// 上海
	std::shared_ptr<WorkCounter_atomic> m_Counter_inner_sh;

	// 深圳链路统计
	// 统计链路中订单处理状态
	// 注意：此map中所有的计数器仅由统计类进行处理和进行综合
	std::map<std::string, std::shared_ptr<WorkCounter_nolock>> m_mapCounters_link_sz;
	// 上海链路统计
	// 统计链路中订单处理状态
	// 注意：此map中所有的计数器仅由统计类进行处理和进行综合
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

	// 增加深圳链路计数器
	void AddSz_LinkCounter(const std::string& sName);

	// 增加上海链路计数器
	void AddSh_LinkCounter(const std::string& sName);

	// 增加深圳链路计数 receive
	void IncSz_Link_Receive(const std::string& sName);

	// 增加深圳链路计数 SendBack
	void IncSz_Link_SendBack(const std::string& sName);

	// 增加上海链路计数 receive
	void IncSh_Link_Receive(const std::string& sName);

	// 增加上海链路计数 SendBack
	void IncSh_Link_SendBack(const std::string& sName);

	// 打印成交统计信息
	int Print(rapidjson::Document& doc, rapidjson::Document::AllocatorType &allocator);

	/*
	取深圳内部处理计数

	@param uint64_t& out_ui64_receive : 收到计数
	@param uint64_t& out_ui64_confirm : 确认计数
	@param uint64_t& out_ui64_matchall : 全部成交计数
	@param uint64_t& out_ui64_matchpart : 分笔成交计数
	@param uint64_t& out_ui64_matchcancel : 撤单计数
	@param uint64_t& out_ui64_error : 错误单计数
	*/
	void GetSz_Inner(uint64_t& out_ui64_receive,
		uint64_t& out_ui64_confirm,
		uint64_t& out_ui64_matchall,
		uint64_t& out_ui64_matchpart,
		uint64_t& out_ui64_matchcancel,
		uint64_t& out_ui64_error);

	// 取深圳链路计数
	void GetSz_Link(rapidjson::Document& doc, rapidjson::Document::AllocatorType &allocator, uint64_t& out_ui64Sendback);

	/*
	取上海内部处理计数

	@param uint64_t& out_ui64_receive : 收到计数
	@param uint64_t& out_ui64_confirm : 确认计数
	@param uint64_t& out_ui64_matchall : 全部成交计数
	@param uint64_t& out_ui64_matchpart : 分笔成交计数
	@param uint64_t& out_ui64_matchcancel : 撤单计数
	@param uint64_t& out_ui64_error : 错误单计数
	*/
	void GetSh_Inner(uint64_t& out_ui64_receive,
		uint64_t& out_ui64_confirm,
		uint64_t& out_ui64_matchall,
		uint64_t& out_ui64_matchpart,
		uint64_t& out_ui64_matchcancel,
		uint64_t& out_ui64_error);

	// 取上海链路计数
	void GetSh_Link(rapidjson::Document& doc, rapidjson::Document::AllocatorType &allocator, uint64_t& out_ui64Sendback);;
};

#endif