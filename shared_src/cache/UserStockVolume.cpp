#include "UserStockVolume.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"

UserStockVolume::UserStockVolume()
	:m_scl(keywords::channel = "UserStockVolume")
{
	m_bStored = false;
	m_ultime_interval = 600;
	m_bExpire = false;
	m_ullast_access_time = TimeStringUtil::GetTimeStamp();
}

UserStockVolume::UserStockVolume(bool bStored, uint64_t stkBalance, uint64_t stk_act_pch_Balance, uint64_t stk_stp_pch_Balance,
	uint64_t stk_etf_rdp_Balance, uint64_t stk_crt_Balance, uint64_t stk_avl_Balance,
	uint64_t stk_last_Balance, unsigned long ulinterval)
	:m_scl(keywords::channel = "UserStockVolume")
{
	m_bStored = bStored;
	m_stkBalance.Init(stkBalance);
	m_stk_act_pch_Balance.Init(stk_act_pch_Balance);
	m_stk_stp_pch_Balance.Init(stk_stp_pch_Balance);
	m_stk_etf_rdp_Balance.Init(stk_etf_rdp_Balance);
	m_stk_crt_Balance.Init(stk_crt_Balance);
	m_stk_avl_Balance.Init(stk_avl_Balance);
	m_stk_last_Balance.Init(stk_last_Balance);

	m_ultime_interval = ulinterval;
	m_bExpire = false;
	m_ullast_access_time = TimeStringUtil::GetTimeStamp();
}

UserStockVolume::~UserStockVolume()
{
}

//------------------------------股份卖出冻结股份--------------------------------//
/*
冻结普通股票股份

Param:
out_ui64etf -- 源于etf赎回的股份数量
out_ui64avl -- 源于可用股份数量

Return:
0	-- 冻结成功
-1	-- 冻结失败
*/
int UserStockVolume::SellFroze(uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl)
{
	static const std::string ftag("UserStockVolume::SellFroze() ");

	IsExpired();

	// 先用etf赎回部分
	out_ui64etf = out_ui64avl = 0;
	int iRes = m_stk_etf_rdp_Balance.Froze(ui64Froze, &out_ui64etf);

	// 
	if (out_ui64etf == ui64Froze)
	{
		return 0;
	}

	// 再用可用余额
	iRes = m_stk_avl_Balance.Froze(ui64Froze - out_ui64etf, &out_ui64avl);

	if ((out_ui64etf + out_ui64avl) < ui64Froze)
	{
		// 冻结失败
		if (0 != out_ui64etf)
		{
			iRes = m_stk_etf_rdp_Balance.Defroze(out_ui64etf);
		}

		if (0 != out_ui64avl)
		{
			iRes = m_stk_avl_Balance.Defroze(out_ui64avl);
		}

		std::string strDebug, strTrans;
		strDebug += "Target=";
		strDebug += sof_string::itostr(ui64Froze, strTrans);
		strDebug += ",etf_rdp=";
		strDebug += sof_string::itostr(out_ui64etf, strTrans);
		strDebug += ",avl=";
		strDebug += sof_string::itostr(out_ui64avl, strTrans);
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strDebug;

		return -1;
	}

	// 冻结总额
	iRes = FrozeStkBalance(ui64Froze);

	return iRes;
}

/*
解除冻结普通股票股份

Param:
in_ui64etf -- 源于etf赎回的股份数量
in_ui64avl -- 源于可用股份数量

Return:
0	-- 解除冻结成功
-1	-- 解除冻结失败
*/
int UserStockVolume::SellDeFroze(uint64_t in_ui64etf, uint64_t in_ui64avl)
{
	IsExpired();

	int iRes = 0;
	if (0 != in_ui64etf)
	{
		iRes = m_stk_etf_rdp_Balance.Defroze(in_ui64etf);
		if (0 != iRes)
		{
			return -1;
		}
	}

	if (0 != in_ui64avl)
	{
		iRes = m_stk_avl_Balance.Defroze(in_ui64avl);
		if (0 != iRes)
		{
			return -1;
		}
	}

	// 解冻总额
	iRes = DeFrozeStkBalance(in_ui64etf + in_ui64avl);

	return iRes;
}

/*
确认普通股份交易额，将从冻结的部分扣除相应额度
Param:
in_ui64etf -- 源于etf赎回的股份数量
in_ui64avl -- 源于可用股份数量

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int UserStockVolume::SellConfirm(uint64_t in_ui64etf, uint64_t in_ui64avl)
{
	IsExpired();

	int iRes = 0;
	if (0 != in_ui64etf)
	{
		iRes = m_stk_etf_rdp_Balance.Confirm(in_ui64etf);
		if (0 != iRes)
		{
			return -1;
		}
	}

	if (0 != in_ui64avl)
	{
		iRes = m_stk_avl_Balance.Confirm(in_ui64avl);
		if (0 != iRes)
		{
			return -1;
		}
	}

	iRes = m_stkBalance.Confirm(in_ui64etf + in_ui64avl);

	return iRes;
}

/*
与数据库信息同步，更新最近同步计数和未冻结计数
*/
int UserStockVolume::SellDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
	uint64_t &out_etf_origin, uint64_t& out_etf_update,
	uint64_t &out_avl_origin, uint64_t& out_avl_update, uint64_t& out_last_origin)
{
	m_stkBalance.DBSyn(out_stk_origin, out_stk_update);
	m_stk_etf_rdp_Balance.DBSyn(out_etf_origin, out_etf_update);
	m_stk_avl_Balance.DBSyn(out_avl_origin, out_avl_update);
	m_stk_last_Balance.ResetSyn(out_stk_origin, &out_last_origin);

	return 0;
}

//------------------------------etf卖出冻结etf份额--------------------------------//
/*
冻结etf股份
Param:
out_ui64etf -- 源于申购的份额
out_ui64avl -- 源于可用etf份额

Return:
0	-- 冻结成功
-1	-- 冻结失败
*/
int UserStockVolume::SellEtfFroze(uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl)
{
	IsExpired();

	// 先用etf申购部分
	out_ui64etf = out_ui64avl = 0;
	int iRes = m_stk_crt_Balance.Froze(ui64Froze, &out_ui64etf);

	// 
	if (out_ui64etf == ui64Froze)
	{
		return 0;
	}

	// 再用可用余额
	iRes = m_stk_avl_Balance.Froze(ui64Froze - out_ui64etf, &out_ui64avl);

	if ((out_ui64etf + out_ui64avl) < ui64Froze)
	{
		// 冻结失败
		if (0 != out_ui64etf)
		{
			iRes = m_stk_crt_Balance.Defroze(out_ui64etf);
		}

		if (0 != out_ui64avl)
		{
			iRes = m_stk_avl_Balance.Defroze(out_ui64avl);
		}

		return -1;
	}

	// 冻结总额
	iRes = FrozeStkBalance(ui64Froze);

	return iRes;
}

/*
解除冻结etf股份
Param:
in_ui64etf -- 源于申购的份额
in_ui64avl -- 源于可用etf份额

Return:
0	-- 解除冻结成功
-1	-- 解除冻结失败
*/
int UserStockVolume::SellEtfDeFroze(uint64_t in_ui64etf, uint64_t in_ui64avl)
{
	IsExpired();

	int iRes = 0;
	if (0 != in_ui64etf)
	{
		iRes = m_stk_crt_Balance.Defroze(in_ui64etf);
		if (0 != iRes)
		{
			return -1;
		}
	}

	if (0 != in_ui64avl)
	{
		iRes = m_stk_avl_Balance.Defroze(in_ui64avl);
		if (0 != iRes)
		{
			return -1;
		}
	}

	// 解冻总额
	iRes = DeFrozeStkBalance(in_ui64etf + in_ui64avl);

	return iRes;
}

/*
确认etf交易额，将从冻结的部分扣除相应额度
Param:
in_ui64etf -- 源于申购的份额
in_ui64avl -- 源于可用etf份额

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int UserStockVolume::SellEtfConfirm(uint64_t in_ui64etf, uint64_t in_ui64avl)
{
	IsExpired();

	int iRes = 0;
	if (0 != in_ui64etf)
	{
		iRes = m_stk_crt_Balance.Confirm(in_ui64etf);
		if (0 != iRes)
		{
			return -1;
		}
	}

	if (0 != in_ui64avl)
	{
		iRes = m_stk_avl_Balance.Confirm(in_ui64avl);
		if (0 != iRes)
		{
			return -1;
		}
	}

	iRes = m_stkBalance.Confirm(in_ui64etf + in_ui64avl);

	return iRes;
}

/*
与数据库信息同步，更新最近同步计数和未冻结计数
*/
int UserStockVolume::SellEtfDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
	uint64_t &out_etf_origin, uint64_t& out_etf_update,
	uint64_t &out_avl_origin, uint64_t& out_avl_update, uint64_t& out_last_origin)
{
	m_stkBalance.DBSyn(out_stk_origin, out_stk_update);
	m_stk_crt_Balance.DBSyn(out_etf_origin, out_etf_update);
	m_stk_avl_Balance.DBSyn(out_avl_origin, out_avl_update);
	m_stk_last_Balance.ResetSyn(out_stk_origin, &out_last_origin);

	return 0;
}

//------------------------------etf申购冻结成分股--------------------------------//
/*
申购成分股的最大可用股份查询

Return:
0 -- 足够
-1 -- 不够
*/
int UserStockVolume::CreationQuery(uint64_t ui64Query, uint64_t &out_ui64max)
{
	IsExpired();

	out_ui64max = ui64Query;

	uint64_t ui64act, ui64avl;
	ui64act = ui64avl = 0;
	int iRes = m_stk_act_pch_Balance.Query(ui64Query, &ui64act);

	// 
	if (0 == iRes)
	{
		return 0;
	}

	// 再用可用余额
	iRes = m_stk_avl_Balance.Query(ui64Query - ui64act, &ui64avl);
	out_ui64max = ui64act + ui64avl;

	return iRes;
}

/*
冻结申购etf的成分股股份

Return:
0	-- 冻结成功
-1	-- 冻结失败
*/
int UserStockVolume::CreationFroze(uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl)
{
	IsExpired();

	// 先用竞价买入量
	out_ui64act = out_ui64avl = 0;
	int iRes = m_stk_act_pch_Balance.Froze(ui64Froze, &out_ui64act);

	// 
	if (out_ui64act == ui64Froze)
	{
		return 0;
	}

	// 再用可用余额
	iRes = m_stk_avl_Balance.Froze(ui64Froze - out_ui64act, &out_ui64avl);

	if ((out_ui64act + out_ui64avl) < ui64Froze)
	{
		// 冻结失败
		if (0 != out_ui64act)
		{
			iRes = m_stk_etf_rdp_Balance.Defroze(out_ui64act);
		}

		if (0 != out_ui64avl)
		{
			iRes = m_stk_avl_Balance.Defroze(out_ui64avl);
		}

		return -1;
	}

	// 冻结总额
	iRes = FrozeStkBalance(ui64Froze);

	return iRes;
}

/*
解除冻结申购etf的成分股股份

Param:
in_ui64act -- 源于竞价买入股份数量
in_ui64avl -- 源于可用股份数量

Return:
0	-- 解除冻结成功
-1	-- 解除冻结失败
*/
int UserStockVolume::CreationDeFroze(uint64_t in_ui64act, uint64_t in_ui64avl)
{
	IsExpired();

	int iRes = 0;
	if (0 != in_ui64act)
	{
		iRes = m_stk_act_pch_Balance.Defroze(in_ui64act);
		if (0 != iRes)
		{
			return -1;
		}
	}

	if (0 != in_ui64avl)
	{
		iRes = m_stk_avl_Balance.Defroze(in_ui64avl);
		if (0 != iRes)
		{
			return -1;
		}
	}

	// 解冻总额
	iRes = DeFrozeStkBalance(in_ui64act + in_ui64avl);

	return iRes;
}

/*
与数据库信息同步，更新最近同步计数和未冻结计数
申购成分股同步
*/
int UserStockVolume::CreationComponentDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
	uint64_t &out_act_origin, uint64_t& out_act_update,
	uint64_t &out_avl_origin, uint64_t& out_avl_update, uint64_t& out_last_origin)
{
	m_stkBalance.DBSyn(out_stk_origin, out_stk_update);
	m_stk_act_pch_Balance.DBSyn(out_act_origin, out_act_update);
	m_stk_avl_Balance.DBSyn(out_avl_origin, out_avl_update);
	m_stk_last_Balance.ResetSyn(out_stk_origin, &out_last_origin);

	return 0;
}

/*
与数据库信息同步，更新最近同步计数和未冻结计数
申购ETF同步
*/
int UserStockVolume::CreationEtfDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
	uint64_t &out_crt_origin, uint64_t& out_crt_update, uint64_t& out_last_origin)
{
	m_stkBalance.DBSyn(out_stk_origin, out_stk_update);
	m_stk_crt_Balance.DBSyn(out_crt_origin, out_crt_update);
	m_stk_last_Balance.ResetSyn(out_stk_origin, &out_last_origin);

	return 0;
}

/*
确认申购etf的成分股交易额，将从冻结的部分扣除相应额度
Param:
in_ui64act -- 源于竞价买入股份数量
in_ui64avl -- 源于可用股份数量

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int UserStockVolume::CreationConfirm(uint64_t in_ui64act, uint64_t in_ui64avl)
{
	IsExpired();

	int iRes = 0;
	if (0 != in_ui64act)
	{
		iRes = m_stk_act_pch_Balance.Confirm(in_ui64act);
		if (0 != iRes)
		{
			return -1;
		}
	}

	if (0 != in_ui64avl)
	{
		iRes = m_stk_avl_Balance.Confirm(in_ui64avl);
		if (0 != iRes)
		{
			return -1;
		}
	}

	m_stkBalance.Confirm(in_ui64act + in_ui64avl);

	return 0;
}

//------------------------------etf赎回冻结etf份额--------------------------------//
/*
冻结赎回etf的份额
Param:
out_ui64act -- 源于买入的份额
out_ui64avl -- 源于可用etf份额

Return:
0	-- 冻结成功
-1	-- 冻结失败
*/
int UserStockVolume::RedemptionFroze(uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl)
{
	IsExpired();

	// 先用竞价买入量
	out_ui64act = out_ui64avl = 0;
	int iRes = m_stk_act_pch_Balance.Froze(ui64Froze, &out_ui64act);

	// 
	if (out_ui64act == ui64Froze)
	{
		return 0;
	}

	// 再用可用余额
	iRes = m_stk_avl_Balance.Froze(ui64Froze - out_ui64act, &out_ui64avl);

	if ((out_ui64avl + out_ui64act) < ui64Froze)
	{
		// 冻结失败
		if (0 != out_ui64act)
		{
			iRes = m_stk_etf_rdp_Balance.Defroze(out_ui64act);
		}

		if (0 != out_ui64avl)
		{
			iRes = m_stk_avl_Balance.Defroze(out_ui64avl);
		}

		return -1;
	}

	// 冻结总额
	iRes = FrozeStkBalance(ui64Froze);

	return iRes;
}

/*
解除冻结赎回etf的份额

Param:
in_ui64act -- 源于竞价买入股份数量
in_ui64avl -- 源于可用股份数量

Return:
0	-- 解除冻结成功
-1	-- 解除冻结失败
*/
int UserStockVolume::RedemptionDeFroze(uint64_t in_ui64act, uint64_t in_ui64avl)
{
	IsExpired();

	int iRes = 0;
	if (0 != in_ui64act)
	{
		iRes = m_stk_act_pch_Balance.Defroze(in_ui64act);
		if (0 != iRes)
		{
			return -1;
		}
	}

	if (0 != in_ui64avl)
	{
		iRes = m_stk_avl_Balance.Defroze(in_ui64avl);
		if (0 != iRes)
		{
			return -1;
		}
	}

	// 解冻总额
	iRes = DeFrozeStkBalance(in_ui64act + in_ui64avl);

	return iRes;
}

/*
确认赎回etf的份额，将从冻结的部分扣除相应额度
Param:
in_ui64act -- 源于竞价买入股份数量
in_ui64avl -- 源于可用股份数量

Return:
0 -- 确认成功
-1 -- 确认失败
*/
int UserStockVolume::RedemptionConfirm(uint64_t in_ui64act, uint64_t in_ui64avl)
{
	IsExpired();

	int iRes = 0;
	if (0 != in_ui64act)
	{
		iRes = m_stk_act_pch_Balance.Confirm(in_ui64act);
		if (0 != iRes)
		{
			return -1;
		}
	}

	if (0 != in_ui64avl)
	{
		iRes = m_stk_avl_Balance.Confirm(in_ui64avl);
		if (0 != iRes)
		{
			return -1;
		}
	}

	m_stkBalance.Confirm(in_ui64act + in_ui64avl);

	return 0;
}

//------------------------------增加股份--------------------------------//
/*
竞价买入增加（普通股份、etf份额）

Return:
0 -- 成功
-1 -- 失败
*/
int UserStockVolume::AddAct(uint64_t ui64StpNum)
{
	IsExpired();

	m_stk_act_pch_Balance.Add(ui64StpNum);

	m_stkBalance.Add(ui64StpNum);

	return 0;
}

/*
申购买入增加(etf份额)

Return:
0 -- 成功
-1 -- 失败
*/
int UserStockVolume::AddCrt(uint64_t ui64CrtNum)
{
	IsExpired();

	m_stk_crt_Balance.Add(ui64CrtNum);

	m_stkBalance.Add(ui64CrtNum);

	return 0;
}

/*
赎回etf份额时增加(成分股股份)

Return:
0 -- 成功
-1 -- 失败
*/
int UserStockVolume::AddRdp(uint64_t ui64RdpNum)
{
	IsExpired();

	m_stk_etf_rdp_Balance.Add(ui64RdpNum);

	m_stkBalance.Add(ui64RdpNum);

	return 0;
}

/*
与数据库信息同步，更新最近同步计数和未冻结计数
竞价买入增加
*/
int UserStockVolume::AddActSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
	uint64_t &out_act_origin, uint64_t& out_act_update, uint64_t& out_last_origin)
{
	m_stkBalance.DBSyn(out_stk_origin, out_stk_update);
	m_stk_act_pch_Balance.DBSyn(out_act_origin, out_act_update);
	m_stk_last_Balance.ResetSyn(out_stk_origin, &out_last_origin);

	return 0;
}

/*
与数据库信息同步，更新最近同步计数和未冻结计数
申购增加(etf份额)
*/
int UserStockVolume::AddEtfSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
	uint64_t &out_etf_origin, uint64_t& out_etf_update, uint64_t& out_last_origin)
{
	m_stkBalance.DBSyn(out_stk_origin, out_stk_update);
	m_stk_crt_Balance.DBSyn(out_etf_origin, out_etf_update);
	m_stk_last_Balance.ResetSyn(out_stk_origin, &out_last_origin);

	return 0;
}

/*
与数据库信息同步，更新最近同步计数和未冻结计数
赎回减少(etf份额)
*/
int UserStockVolume::RdpEtfSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
	uint64_t &out_act_origin, uint64_t& out_act_update,
	uint64_t &out_avl_origin, uint64_t& out_avl_update,
	uint64_t& out_last_origin)
{
	m_stkBalance.DBSyn(out_stk_origin, out_stk_update);
	m_stk_act_pch_Balance.DBSyn(out_act_origin, out_act_update);
	m_stk_avl_Balance.DBSyn(out_avl_origin, out_avl_update);
	m_stk_last_Balance.ResetSyn(out_stk_origin, &out_last_origin);

	return 0;
}

/*
与数据库信息同步，更新最近同步计数和未冻结计数
赎回etf份额时增加(成分股股份)
*/
int UserStockVolume::AddRdpComponentSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
	uint64_t &out_rdp_origin, uint64_t& out_rdp_update, uint64_t& out_last_origin)
{
	m_stkBalance.DBSyn(out_stk_origin, out_stk_update);
	m_stk_etf_rdp_Balance.DBSyn(out_rdp_origin, out_rdp_update);
	m_stk_last_Balance.ResetSyn(out_stk_origin, &out_last_origin);

	return 0;
}

/*
查看是否过期
Return:
true -- 过期
false -- 未过期
*/
bool UserStockVolume::IsExpired()
{
	unsigned long ulNow = TimeStringUtil::GetTimeStamp();

	if ((ulNow - m_ullast_access_time) < m_ultime_interval)
	{
		m_ullast_access_time = ulNow;
		m_bExpire = false;
	}

	m_bExpire = true;

	return m_bExpire;
}

/*
冻结总额
Return:
0 -- 成功
-1 -- 失败
*/
int UserStockVolume::FrozeStkBalance(uint64_t ui64Froze)
{
	static const std::string ftag("UserStockVolume::FrozeStkBalance() ");
	int iRes = m_stkBalance.Froze(ui64Froze);
	if (0 != iRes)
	{
		std::string strVolumeInfo;
		m_stkBalance.GetVolumeInfo(strVolumeInfo);

		string strDebug("Target="), strTrans;
		strDebug += sof_string::itostr(ui64Froze, strTrans);
		strDebug += ",";
		strDebug += strVolumeInfo;
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strVolumeInfo;

		return -1;
	}

	return 0;
}

/*
解冻总额
Return:
0 -- 成功
-1 -- 失败
*/
int UserStockVolume::DeFrozeStkBalance(uint64_t ui64Froze)
{
	static const std::string ftag("UserStockVolume::DeFrozeStkBalance() ");
	int iRes = m_stkBalance.Defroze(ui64Froze);
	if (0 != iRes)
	{
		std::string strVolumeInfo;
		m_stkBalance.GetVolumeInfo(strVolumeInfo);

		string strDebug("Target="), strTrans;
		strDebug += sof_string::itostr(ui64Froze, strTrans);
		strDebug += ",";
		strDebug += strVolumeInfo;
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << strVolumeInfo;

		return -1;
	}

	return 0;
}