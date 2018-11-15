#include "TradePolicyManage.h"
#include "tool_string/Tgw_StringUtil.h"

#include "simutgw_config/g_values_sys_run_config.h"

TradePolicyManage::TradePolicyManage()
{

}

TradePolicyManage::~TradePolicyManage()
{
}

/*
按连接设置交易策略

@param const std::string& in_strConnId : 连接Id
@return 0 : 设置成功
@return -1 : 设置失败
*/
int TradePolicyManage::Set_Sz(const std::string& in_strConnId,
	const enum simutgw::SysRunMode& in_RunMode, const bool in_checkAsset,
	const enum simutgw::SysMatchMode& in_MatchMode, const enum simutgw::QuotationType& in_quotType, const uint32_t in_Part_Match_Num)
{
	static const string ftag("TradePolicyManage::Set_Sz() ");

	//
	if (in_strConnId.empty())
	{
		EzLog::e(ftag, "in_strConnName empty ");
		return -1;
	}

	boost::unique_lock<boost::mutex> Locker(m_mutex_sz);

	MAP_SZ_POLICY::iterator itConn = m_mapSzPolicy.find(in_strConnId);

	if (m_mapSzPolicy.end() == itConn)
	{
		// 未找到
		std::shared_ptr<struct Conn_TradePolicyCell> ptr(new struct Conn_TradePolicyCell());

		ptr->stSelf.Set(in_RunMode, in_checkAsset, in_MatchMode, in_quotType, in_Part_Match_Num);

		std::pair<MAP_SZ_POLICY::iterator, bool> ret;
		ret = m_mapSzPolicy.insert(std::pair<std::string, std::shared_ptr<struct Conn_TradePolicyCell>>(in_strConnId, ptr));
		if (!ret.second)
		{
			string sdebug("element ");
			sdebug += in_strConnId;
			sdebug += " already existed";
			EzLog::e(ftag, sdebug);
		}
	}
	else
	{
		// 找到
		itConn->second->stSelf.Set(in_RunMode, in_checkAsset, in_MatchMode, in_quotType, in_Part_Match_Num);
	}

	return 0;
}

/*
按连接、席位设置交易策略

@param const std::string& in_strConnId : 连接Id
@param const std::string& in_strSetName : 席位名称
@return 0 : 设置成功
@return -1 : 设置失败
*/
int TradePolicyManage::Set_Sz(const std::string& in_strConnId, const std::string& in_strSeatName,
	const enum simutgw::SysRunMode& in_RunMode, const bool in_checkAsset,
	const enum simutgw::SysMatchMode& in_MatchMode, const enum simutgw::QuotationType& in_quotType, const uint32_t in_Part_Match_Num)
{
	static const string ftag("TradePolicyManage::Set_Sz() ");

	if (in_strConnId.empty())
	{
		EzLog::e(ftag, "in_strConnName empty ");
		return -1;
	}

	if (in_strSeatName.empty())
	{
		EzLog::e(ftag, "in_strSetName empty ");
		return -1;
	}

	boost::unique_lock<boost::mutex> Locker(m_mutex_sz);

	MAP_SZ_POLICY::iterator itConn = m_mapSzPolicy.find(in_strConnId);

	if (m_mapSzPolicy.end() == itConn)
	{
		// 未找到
		std::shared_ptr<struct Conn_TradePolicyCell> ptrConn(new struct Conn_TradePolicyCell());

		ptrConn->stSelf.Set(in_RunMode, in_checkAsset, in_MatchMode, in_quotType, in_Part_Match_Num);

		std::pair<MAP_SZ_POLICY::iterator, bool> retConn;
		retConn = m_mapSzPolicy.insert(std::pair<std::string, std::shared_ptr<struct Conn_TradePolicyCell>>(in_strConnId, ptrConn));
		if (!retConn.second)
		{
			string sdebug("element ");
			sdebug += in_strConnId;
			sdebug += " already existed";
			EzLog::e(ftag, sdebug);
		}

		// seat肯定也不存在，直接插入
		std::shared_ptr<struct TradePolicyCell> ptrSeat(new struct TradePolicyCell(in_RunMode, in_checkAsset, in_MatchMode, in_quotType, in_Part_Match_Num));
		std::pair<std::map<std::string, std::shared_ptr<struct TradePolicyCell>>::iterator, bool> retSeat;
		retSeat = (retConn.first)->second->mapSeats.insert(std::pair<std::string, std::shared_ptr<struct TradePolicyCell>>(in_strSeatName, ptrSeat));
		if (!retSeat.second)
		{
			string sdebug("in 未找到 conn ");
			sdebug += in_strConnId;
			sdebug += " element ";
			sdebug += in_strSeatName;
			sdebug += " already existed";
			EzLog::e(ftag, sdebug);
		}
	}
	else
	{
		// 找到
		std::map<std::string, std::shared_ptr<struct TradePolicyCell>>::iterator itSeats = itConn->second->mapSeats.find(in_strSeatName);
		if (itConn->second->mapSeats.end() == itSeats)
		{
			// 未找到
			std::shared_ptr<struct TradePolicyCell> ptrSeat(new struct TradePolicyCell(in_RunMode, in_checkAsset, in_MatchMode, in_quotType, in_Part_Match_Num));
			std::pair<std::map<std::string, std::shared_ptr<struct TradePolicyCell>>::iterator, bool> retSeat;
			retSeat = itConn->second->mapSeats.insert(std::pair<std::string, std::shared_ptr<struct TradePolicyCell>>(in_strSeatName, ptrSeat));
			if (!retSeat.second)
			{
				string sdebug("in 找到 conn ");
				sdebug += in_strConnId;
				sdebug += " element ";
				sdebug += in_strSeatName;
				sdebug += " already existed";
				EzLog::e(ftag, sdebug);
			}
		}
		else
		{
			// 找到
			itSeats->second->Set(in_RunMode, in_checkAsset, in_MatchMode, in_quotType, in_Part_Match_Num);
		}
	}

	return 0;
}

/*
按连接设置交易策略

@param const std::string& in_strConnId : 连接Id
@return 0 : 设置成功
@return -1 : 设置失败
*/
int TradePolicyManage::Set_Sh(const std::string& in_strConnId,
	const enum simutgw::SysRunMode& in_RunMode, const bool in_checkAsset,
	const enum simutgw::SysMatchMode& in_MatchMode, const enum simutgw::QuotationType& in_quotType, const uint32_t in_Part_Match_Num)
{
	static const string ftag("TradePolicyManage::Set_Sh() ");

	//
	if (in_strConnId.empty())
	{
		EzLog::e(ftag, "in_strConnName empty ");
		return -1;
	}

	boost::unique_lock<boost::mutex> Locker(m_mutex_sh);

	MAP_SH_POLICY::iterator itConn = m_mapShPolicy.find(in_strConnId);

	if (m_mapShPolicy.end() == itConn)
	{
		// 未找到
		std::shared_ptr<struct TradePolicyCell> ptr(new struct TradePolicyCell(in_RunMode, in_checkAsset, in_MatchMode, in_quotType, in_Part_Match_Num));

		std::pair<MAP_SH_POLICY::iterator, bool> ret;
		ret = m_mapShPolicy.insert(std::pair<std::string, std::shared_ptr<struct TradePolicyCell>>(in_strConnId, ptr));
		if (!ret.second)
		{
			string sdebug("element ");
			sdebug += in_strConnId;
			sdebug += " already existed";
			EzLog::e(ftag, sdebug);
		}
	}
	else
	{
		// 找到
		itConn->second->Set(in_RunMode, in_checkAsset, in_MatchMode, in_quotType, in_Part_Match_Num);
	}

	return 0;
}

/*
按连接获取交易策略

@param const std::string& in_strConnId : 连接Id

@return 0 : 获取成功
@return 1 : 获取返回默认值
@return -1 : 获取失败
*/
int TradePolicyManage::Get_Sz(const std::string& in_strConnId,
enum simutgw::SysRunMode& out_RunMode, bool& out_checkAsset,
enum simutgw::SysMatchMode& out_MatchMode, enum simutgw::QuotationType& out_quotType, uint32_t& out_Part_Match_Num)
{
	static const string ftag("TradePolicyManage::Set_Sh() ");

	// 默认情况
	if (in_strConnId.empty())
	{
		EzLog::w(ftag, "in_strConnName empty ");

		out_RunMode = simutgw::g_iRunMode;
		out_checkAsset = simutgw::g_bEnable_Check_Assets;
		out_MatchMode = simutgw::g_iMatchMode;
		out_quotType = simutgw::g_Quotation_Type;
		out_Part_Match_Num = simutgw::g_ui32_Part_Match_Num;

		return 1;
	}

	boost::unique_lock<boost::mutex> Locker(m_mutex_sh);

	MAP_SZ_POLICY::iterator itConn = m_mapSzPolicy.find(in_strConnId);

	if (m_mapSzPolicy.end() == itConn)
	{
		// 未找到		
		// 默认情况
		out_RunMode = simutgw::g_iRunMode;
		out_checkAsset = simutgw::g_bEnable_Check_Assets;
		out_MatchMode = simutgw::g_iMatchMode;
		out_quotType = simutgw::g_Quotation_Type;
		out_Part_Match_Num = simutgw::g_ui32_Part_Match_Num;
	}
	else
	{
		// 找到
		out_RunMode = itConn->second->stSelf.iRunMode;
		out_checkAsset = itConn->second->stSelf.bCheck_Assets;
		out_MatchMode = itConn->second->stSelf.iMatchMode;
		out_quotType = itConn->second->stSelf.iQuotationMatchType;
		out_Part_Match_Num = itConn->second->stSelf.iPart_Match_Num;
	}

	return 0;
}

/*
按连接、席位获取交易策略

@param const std::string& in_strConnId : 连接Id
@param const std::string& in_strSetName : 席位名称

@return 0 : 获取成功
@return 1 : 获取返回默认值
@return -1 : 获取失败
*/
int TradePolicyManage::Get_Sz(const std::string& in_strConnId, const std::string& in_strSeatName,
enum simutgw::SysRunMode& out_RunMode, bool& out_checkAsset,
enum simutgw::SysMatchMode& out_MatchMode, enum simutgw::QuotationType& out_quotType, uint32_t& out_Part_Match_Num)
{
	static const string ftag("TradePolicyManage::Get_Sz() ");

	if (in_strConnId.empty())
	{
		EzLog::w(ftag, "in_strConnName empty ");

		// 默认情况
		out_RunMode = simutgw::g_iRunMode;
		out_checkAsset = simutgw::g_bEnable_Check_Assets;
		out_MatchMode = simutgw::g_iMatchMode;
		out_quotType = simutgw::g_Quotation_Type;
		out_Part_Match_Num = simutgw::g_ui32_Part_Match_Num;

		return 1;
	}

	if (in_strSeatName.empty())
	{
		EzLog::w(ftag, "in_strSeatName empty ");

		// 默认情况
		out_RunMode = simutgw::g_iRunMode;
		out_checkAsset = simutgw::g_bEnable_Check_Assets;
		out_MatchMode = simutgw::g_iMatchMode;
		out_quotType = simutgw::g_Quotation_Type;
		out_Part_Match_Num = simutgw::g_ui32_Part_Match_Num;

		return 1;
	}

	boost::unique_lock<boost::mutex> Locker(m_mutex_sz);

	MAP_SZ_POLICY::iterator itConn = m_mapSzPolicy.find(in_strConnId);

	if (m_mapSzPolicy.end() == itConn)
	{
		// 未找到
		// 默认情况
		out_RunMode = simutgw::g_iRunMode;
		out_checkAsset = simutgw::g_bEnable_Check_Assets;
		out_MatchMode = simutgw::g_iMatchMode;
		out_quotType = simutgw::g_Quotation_Type;
		out_Part_Match_Num = simutgw::g_ui32_Part_Match_Num;
	}
	else
	{
		// 找到
		std::map<std::string, std::shared_ptr<struct TradePolicyCell>>::iterator itSeats = itConn->second->mapSeats.find(in_strSeatName);
		if (itConn->second->mapSeats.end() == itSeats)
		{
			// 未找到
			// 按连接的设置返回
			out_RunMode = itConn->second->stSelf.iRunMode;
			out_checkAsset = itConn->second->stSelf.bCheck_Assets;
			out_MatchMode = itConn->second->stSelf.iMatchMode;
			out_quotType = itConn->second->stSelf.iQuotationMatchType;
			out_Part_Match_Num = itConn->second->stSelf.iPart_Match_Num;
		}
		else
		{
			// 找到
			out_RunMode = itSeats->second->iRunMode;
			out_checkAsset = itSeats->second->bCheck_Assets;
			out_MatchMode = itSeats->second->iMatchMode;
			out_quotType = itSeats->second->iQuotationMatchType;
			out_Part_Match_Num = itSeats->second->iPart_Match_Num;
		}
	}

	return 0;
}

/*
按连接获取交易策略

@param const std::string& in_strConnId : 连接Id

@return 0 : 获取成功
@return 1 : 获取返回默认值
@return -1 : 获取失败
*/
int TradePolicyManage::Get_Sh(const std::string& in_strConnId,
enum simutgw::SysRunMode& out_RunMode, bool& out_checkAsset,
enum simutgw::SysMatchMode& out_MatchMode, enum simutgw::QuotationType& out_quotType, uint32_t& out_Part_Match_Num)
{
	static const string ftag("TradePolicyManage::Get_Sh() ");

	//
	if (in_strConnId.empty())
	{
		EzLog::w(ftag, "in_strConnName empty ");

		// 默认情况
		out_RunMode = simutgw::g_iRunMode;
		out_checkAsset = simutgw::g_bEnable_Check_Assets;
		out_MatchMode = simutgw::g_iMatchMode;
		out_quotType = simutgw::g_Quotation_Type;
		out_Part_Match_Num = simutgw::g_ui32_Part_Match_Num;

		return 1;
	}

	boost::unique_lock<boost::mutex> Locker(m_mutex_sh);

	MAP_SH_POLICY::iterator itConn = m_mapShPolicy.find(in_strConnId);

	if (m_mapShPolicy.end() == itConn)
	{
		// 未找到
		// 默认情况
		out_RunMode = simutgw::g_iRunMode;
		out_checkAsset = simutgw::g_bEnable_Check_Assets;
		out_MatchMode = simutgw::g_iMatchMode;
		out_quotType = simutgw::g_Quotation_Type;
		out_Part_Match_Num = simutgw::g_ui32_Part_Match_Num;
	}
	else
	{
		// 找到
		out_RunMode = itConn->second->iRunMode;
		out_checkAsset = itConn->second->bCheck_Assets;
		out_MatchMode = itConn->second->iMatchMode;
		out_quotType = itConn->second->iQuotationMatchType;
		out_Part_Match_Num = itConn->second->iPart_Match_Num;
	}

	return 0;
}

/*
Debug out
*/
std::string& TradePolicyManage::DebugOut_Sz(std::string& out_strOut)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex_sz);

	string sitoa;
	string sTmp;

	string sOutput = "====\nSz t policy size = ";
	sOutput += sof_string::itostr(m_mapSzPolicy.size(), sitoa);

	MAP_SZ_POLICY::iterator itConn = m_mapSzPolicy.begin();

	for (; m_mapSzPolicy.end() != itConn; ++itConn)
	{
		sOutput += "\nkey=";
		sOutput += itConn->first;
		sOutput += ",value=[\n    ";
		sOutput += DebugOut_Sz_Conn_TradePolicyCell(itConn->second, sTmp);
		sOutput += "]";
	}
	sOutput += "\n====\n";

	out_strOut = sOutput;

	return out_strOut;
}

/*
Debug out
*/
std::string& TradePolicyManage::DebugOut_Sh(std::string& out_strOut)
{
	boost::unique_lock<boost::mutex> Locker(m_mutex_sh);

	string sOutput;
	string sTmp;

	sOutput += "====\nSh tp ";
	sOutput += DebugOut_map(m_mapShPolicy, sTmp);
	sOutput += "\n====\n";
	out_strOut = sOutput;
	return out_strOut;
}

/*
Debug out
*/
std::string& TradePolicyManage::DebugOut_Sz_Conn_TradePolicyCell(std::shared_ptr<struct Conn_TradePolicyCell>& ptr, std::string& out_strOut)
{
	string sitoa;
	string sTmp;

	string sOutput = "self:";

	sOutput += DebugOut_TradePolicyCell(ptr->stSelf, sTmp);
	sOutput += "\n    seats:";
	sOutput += DebugOut_map(ptr->mapSeats, sTmp);

	out_strOut = sOutput;

	return out_strOut;
}

/*
Debug out
*/
std::string& TradePolicyManage::DebugOut_TradePolicyCell(struct TradePolicyCell& obj, std::string& out_strOut)
{
	string sitoa;

	string sOutput = "iRunMode=";
	sOutput += sof_string::itostr(obj.iRunMode, sitoa);
	sOutput += ", bCheck_Assets=";
	sOutput += sof_string::itostr(obj.bCheck_Assets, sitoa);
	sOutput += ", iMatchMode=";
	sOutput += sof_string::itostr(obj.iMatchMode, sitoa);
	sOutput += ", iQuotationMatchType=";
	sOutput += sof_string::itostr(obj.iQuotationMatchType, sitoa);
	sOutput += ", iPart_Match_Num=";
	sOutput += sof_string::itostr(obj.iPart_Match_Num, sitoa);
	out_strOut = sOutput;

	return out_strOut;
}

/*
Debug out
*/
std::string& TradePolicyManage::DebugOut_map(std::map<std::string, std::shared_ptr<struct TradePolicyCell>>& obj, std::string& out_strOut)
{
	string sitoa;
	string sTmp;

	string sOutput = "size=";
	sOutput += sof_string::itostr(obj.size(), sitoa);

	std::map<std::string, std::shared_ptr<struct TradePolicyCell>>::iterator it = obj.begin();

	for (; obj.end() != it; ++it)
	{
		sOutput += "\nkey=";
		sOutput += it->first;
		sOutput += ",value=[";
		sOutput += DebugOut_TradePolicyCell(*(it->second), sTmp);
		sOutput += "]";
	}

	out_strOut = sOutput;

	return out_strOut;
}
