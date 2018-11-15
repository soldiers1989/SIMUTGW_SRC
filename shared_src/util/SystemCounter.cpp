#include "SystemCounter.h"

#include "tool_string/TimeStringUtil.h"

#include "simutgw/stgw_config/g_values_biz.h"

SystemCounter::SystemCounter(void)
	:m_scl(keywords::channel = "SystemCounter"),
	m_Counter_inner_sz(new WorkCounter_atomic()),
	m_Counter_inner_sh(new WorkCounter_atomic())
{


}

SystemCounter::~SystemCounter(void)
{
}

// 增加深圳链路计数器
void SystemCounter::AddSz_LinkCounter(const std::string& sName)
{
	static const string ftag("AddSz_LinkCounter() ");
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	std::map<std::string, std::shared_ptr<WorkCounter_nolock>>::iterator it = m_mapCounters_link_sz.find(sName);
	if (m_mapCounters_link_sz.end() == it)
	{
		// not existed
		std::shared_ptr<WorkCounter_nolock> ptrCnt(new WorkCounter_nolock(sName));
		m_mapCounters_link_sz.insert(std::pair<std::string, std::shared_ptr<WorkCounter_nolock>>(sName, ptrCnt));
	}
}

// 增加上海链路计数器
void SystemCounter::AddSh_LinkCounter(const std::string& sName)
{
	static const string ftag("AddSh_LinkCounter() ");
	boost::unique_lock<boost::mutex> Locker(m_mutex);

	std::map<std::string, std::shared_ptr<WorkCounter_nolock>>::iterator it = m_mapCounters_link_sh.find(sName);

	if (m_mapCounters_link_sh.end() == it)
	{
		// not existed
		std::shared_ptr<WorkCounter_nolock> ptrCnt(new WorkCounter_nolock(sName));
		m_mapCounters_link_sh.insert(std::pair<std::string, std::shared_ptr<WorkCounter_nolock>>(sName, ptrCnt));
	}
}

// 增加深圳链路计数 receive
void SystemCounter::IncSz_Link_Receive(const string& sName)
{
	const string ftag("IncSz_Link_Receive() ");

	std::map<std::string, std::shared_ptr<WorkCounter_nolock>>::iterator it = m_mapCounters_link_sz.find(sName);
	if (m_mapCounters_link_sz.end() == it)
	{
		BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << " element " << sName << " not existed in m_mapCounters_link_sz";
	}
	else
	{
		it->second->Inc_Received();
	}
}

// 增加深圳链路计数 Sendback
void SystemCounter::IncSz_Link_SendBack(const string& sName)
{
	static const string ftag("IncSz_Link_SendBack() ");

	std::map<std::string, std::shared_ptr<WorkCounter_nolock>>::iterator it = m_mapCounters_link_sz.find(sName);
	if (m_mapCounters_link_sz.end() == it)
	{
		BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << " element " << sName << " not existed in m_mapCounters_link_sz";
	}
	else
	{
		// 使用确认数代表回报数量
		it->second->Inc_Confirm();
	}
}

// 增加上海链路计数 receive
void SystemCounter::IncSh_Link_Receive(const string& sName)
{
	const string ftag("IncSh_Link_Receive() ");

	std::map<std::string, std::shared_ptr<WorkCounter_nolock>>::iterator it = m_mapCounters_link_sh.find(sName);
	if (m_mapCounters_link_sh.end() == it)
	{
		BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << " element " << sName << " not existed in m_mapCounters_link_sh";
	}
	else
	{
		it->second->Inc_Received();
	}
}

// 增加上海链路计数 Sendback
void SystemCounter::IncSh_Link_SendBack(const string& sName)
{
	const string ftag("IncSh_Link_SendBack() ");

	std::map<std::string, std::shared_ptr<WorkCounter_nolock>>::iterator it = m_mapCounters_link_sh.find(sName);
	if (m_mapCounters_link_sh.end() == it)
	{
		BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << " element " << sName << " not existed in m_mapCounters_link_sh";
	}
	else
	{
		// 使用确认数代表回报数量
		it->second->Inc_Confirm();
	}
}

/*
取深圳内部处理计数

@param uint64_t& out_ui64_receive : 收到计数
@param uint64_t& out_ui64_confirm : 确认计数
@param uint64_t& out_ui64_matchall : 全部成交计数
@param uint64_t& out_ui64_matchpart : 分笔成交计数
@param uint64_t& out_ui64_matchcancel : 撤单计数
@param uint64_t& out_ui64_error : 错误单计数
*/
void SystemCounter::GetSz_Inner(uint64_t& out_ui64_receive,
	uint64_t& out_ui64_confirm,
	uint64_t& out_ui64_matchall,
	uint64_t& out_ui64_matchpart,
	uint64_t& out_ui64_matchcancel,
	uint64_t& out_ui64_error)
{
	// 内部

	// 收到计数
	uint64_t ui64Ord_receive = 0;
	// 确认计数
	uint64_t ui64Ord_confirm = 0;
	// 全部成交计数
	uint64_t ui64Ord_matchall = 0;
	// 分笔成交计数
	uint64_t ui64Ord_matchpart = 0;
	// 撤单计数
	uint64_t ui64Ord_matchcancel = 0;
	// 错误单计数
	uint64_t ui64Ord_error = 0;

	ui64Ord_receive = m_Counter_inner_sz->Get_Receive();

	ui64Ord_confirm = m_Counter_inner_sz->Get_Confirm();

	ui64Ord_matchall = m_Counter_inner_sz->Get_MatchAll();

	ui64Ord_matchpart = m_Counter_inner_sz->Get_MatchPart();

	ui64Ord_matchcancel = m_Counter_inner_sz->Get_MatchCancel();

	ui64Ord_error = m_Counter_inner_sz->Get_Error();

	out_ui64_receive = ui64Ord_receive;
	out_ui64_confirm = ui64Ord_confirm;
	out_ui64_matchall = ui64Ord_matchall;
	out_ui64_matchpart = ui64Ord_matchpart;
	out_ui64_matchcancel = ui64Ord_matchcancel;
	out_ui64_error = ui64Ord_error;
}

// 取深圳链路计数
void SystemCounter::GetSz_Link(rapidjson::Document& doc, rapidjson::Document::AllocatorType &allocator, uint64_t& out_ui64Sendback)
{
	static const char cs_linkid[] = "linkid";
	static const char cs_targetCompId[] = "targetCompId";
	static const char cs_receive[] = "receive";
	static const char cs_sendback[] = "sendback";
	static const char cs_link[] = "sz_link";

	rapidjson::Value linkArray(rapidjson::kArrayType);

	uint64_t ui64_linkSendback = 0;

	std::map<std::string, std::shared_ptr<WorkCounter_nolock>>::iterator it = m_mapCounters_link_sz.begin();
	for (; m_mapCounters_link_sz.end() != it; ++it)
	{
		rapidjson::Value elem(rapidjson::kObjectType);

		// 记录深圳接口和Web linkid的对应关系
		string strWebLinkid("");
		std::map<string, struct Connection_webConfig>::const_iterator cit = simutgw::g_mapSzConn_webConfig.find(it->first);
		if (simutgw::g_mapSzConn_webConfig.end() != cit)
		{
			strWebLinkid = cit->second.strWebLinkid;
		}

		elem.AddMember(cs_linkid, rapidjson::Value(strWebLinkid.c_str(), allocator), allocator);
		elem.AddMember(cs_targetCompId, rapidjson::Value(it->first.c_str(), allocator), allocator);
		elem.AddMember(cs_receive, it->second->Get_Receive(), allocator);

		ui64_linkSendback = it->second->Get_Confirm();
		elem.AddMember(cs_sendback, ui64_linkSendback, allocator);

		out_ui64Sendback += ui64_linkSendback;

		linkArray.PushBack(elem, allocator);
	}

	doc.AddMember(cs_link, linkArray, allocator);
}

/*
取上海内部处理计数

@param uint64_t& out_ui64_receive : 收到计数
@param uint64_t& out_ui64_confirm : 确认计数
@param uint64_t& out_ui64_matchall : 全部成交计数
@param uint64_t& out_ui64_matchpart : 分笔成交计数
@param uint64_t& out_ui64_matchcancel : 撤单计数
@param uint64_t& out_ui64_error : 错误单计数
*/
void SystemCounter::GetSh_Inner(uint64_t& out_ui64_receive,
	uint64_t& out_ui64_confirm,
	uint64_t& out_ui64_matchall,
	uint64_t& out_ui64_matchpart,
	uint64_t& out_ui64_matchcancel,
	uint64_t& out_ui64_error)
{
	// 内部

	// 收到计数
	uint64_t ui64Ord_receive = 0;
	// 确认计数
	uint64_t ui64Ord_confirm = 0;
	// 全部成交计数
	uint64_t ui64Ord_matchall = 0;
	// 分笔成交计数
	uint64_t ui64Ord_matchpart = 0;
	// 撤单计数
	uint64_t ui64Ord_matchcancel = 0;
	// 错误单计数
	uint64_t ui64Ord_error = 0;


	ui64Ord_receive = m_Counter_inner_sh->Get_Receive();

	ui64Ord_confirm = m_Counter_inner_sh->Get_Confirm();

	ui64Ord_matchall = m_Counter_inner_sh->Get_MatchAll();

	ui64Ord_matchpart = m_Counter_inner_sh->Get_MatchPart();

	ui64Ord_matchcancel = m_Counter_inner_sh->Get_MatchCancel();

	ui64Ord_error = m_Counter_inner_sh->Get_Error();


	out_ui64_receive = ui64Ord_receive;
	out_ui64_confirm = ui64Ord_confirm;
	out_ui64_matchall = ui64Ord_matchall;
	out_ui64_matchpart = ui64Ord_matchpart;
	out_ui64_matchcancel = ui64Ord_matchcancel;
	out_ui64_error = ui64Ord_error;
}

// 取上海全部链路计数
void SystemCounter::GetSh_Link(rapidjson::Document& doc, rapidjson::Document::AllocatorType &allocator, uint64_t& out_ui64Sendback)
{
	static const char cs_name[] = "linkid";
	static const char cs_receive[] = "receive";
	static const char cs_sendback[] = "sendback";
	static const char cs_link[] = "sh_link";

	rapidjson::Value linkArray(rapidjson::kArrayType);

	uint64_t ui64_linkSendback = 0;

	std::map<std::string, std::shared_ptr<WorkCounter_nolock>>::iterator it = m_mapCounters_link_sh.begin();
	for (; m_mapCounters_link_sh.end() != it; ++it)
	{
		rapidjson::Value elem(rapidjson::kObjectType);
		elem.AddMember(cs_name, rapidjson::Value(it->first.c_str(), allocator), allocator);
		elem.AddMember(cs_receive, it->second->Get_Receive(), allocator);

		ui64_linkSendback = it->second->Get_Confirm();
		elem.AddMember(cs_sendback, ui64_linkSendback, allocator);

		out_ui64Sendback += ui64_linkSendback;

		linkArray.PushBack(elem, allocator);
	}

	doc.AddMember(cs_link, linkArray, allocator);
}


// 打印成交统计信息
int SystemCounter::Print(rapidjson::Document& doc, rapidjson::Document::AllocatorType &allocator)
{
	static const string strTag("SystemCounter::Print()");

	// time stamp
	doc.AddMember("timestamp", (long)TimeStringUtil::GetTimeStamp(), allocator);

	// sz
	{
		// 发送消息总计数
		uint64_t ui64Ord_sendback = 0;

		// 收到计数
		uint64_t ui64Ord_receive = 0;
		// 确认计数
		uint64_t ui64Ord_confirm = 0;
		// 回报计数
		uint64_t ui64Ord_report = 0;
		// 全部成交计数
		uint64_t ui64Ord_matchall = 0;
		// 分笔成交计数
		uint64_t ui64Ord_matchpart = 0;
		// 撤单计数
		uint64_t ui64Ord_matchcancel = 0;
		// 错误单计数
		uint64_t ui64Ord_error = 0;

		GetSz_Link(doc, allocator, ui64Ord_sendback);

		GetSz_Inner(ui64Ord_receive, ui64Ord_confirm, ui64Ord_matchall, ui64Ord_matchpart, ui64Ord_matchcancel, ui64Ord_error);

		ui64Ord_report = ui64Ord_sendback - ui64Ord_confirm;

		doc.AddMember("sz_inner_receive", ui64Ord_receive, allocator);
		doc.AddMember("sz_inner_confirm", ui64Ord_confirm, allocator);
		doc.AddMember("sz_inner_report", ui64Ord_report, allocator);
		doc.AddMember("sz_inner_matchall", ui64Ord_matchall, allocator);
		doc.AddMember("sz_inner_matchpart", ui64Ord_matchpart, allocator);
		doc.AddMember("sz_inner_matchcancel", ui64Ord_matchcancel, allocator);
		doc.AddMember("sz_inner_error", ui64Ord_error, allocator);
	}

	// sh
	{
		// 发送消息总计数
		uint64_t ui64Ord_sendback = 0;

		// 收到计数
		uint64_t ui64Ord_receive = 0;
		// 确认计数
		uint64_t ui64Ord_confirm = 0;
		// 回报计数
		uint64_t ui64Ord_report = 0;
		// 全部成交计数
		uint64_t ui64Ord_matchall = 0;
		// 分笔成交计数
		uint64_t ui64Ord_matchpart = 0;
		// 撤单计数
		uint64_t ui64Ord_matchcancel = 0;
		// 错误单计数
		uint64_t ui64Ord_error = 0;

		GetSh_Link(doc, allocator, ui64Ord_sendback);

		GetSh_Inner(ui64Ord_receive, ui64Ord_confirm, ui64Ord_matchall, ui64Ord_matchpart, ui64Ord_matchcancel, ui64Ord_error);

		ui64Ord_report = ui64Ord_sendback - ui64Ord_confirm;

		doc.AddMember("sh_inner_receive", ui64Ord_receive, allocator);
		doc.AddMember("sh_inner_confirm", ui64Ord_confirm, allocator);
		doc.AddMember("sh_inner_report", ui64Ord_report, allocator);
		doc.AddMember("sh_inner_matchall", ui64Ord_matchall, allocator);
		doc.AddMember("sh_inner_matchpart", ui64Ord_matchpart, allocator);
		doc.AddMember("sh_inner_matchcancel", ui64Ord_matchcancel, allocator);
		doc.AddMember("sh_inner_error", ui64Ord_error, allocator);
	}

	return 0;
}