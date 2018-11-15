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

//------------------------------�ɷ���������ɷ�--------------------------------//
/*
������ͨ��Ʊ�ɷ�

Param:
out_ui64etf -- Դ��etf��صĹɷ�����
out_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0	-- ����ɹ�
-1	-- ����ʧ��
*/
int UserStockVolume::SellFroze(uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl)
{
	static const std::string ftag("UserStockVolume::SellFroze() ");

	IsExpired();

	// ����etf��ز���
	out_ui64etf = out_ui64avl = 0;
	int iRes = m_stk_etf_rdp_Balance.Froze(ui64Froze, &out_ui64etf);

	// 
	if (out_ui64etf == ui64Froze)
	{
		return 0;
	}

	// ���ÿ������
	iRes = m_stk_avl_Balance.Froze(ui64Froze - out_ui64etf, &out_ui64avl);

	if ((out_ui64etf + out_ui64avl) < ui64Froze)
	{
		// ����ʧ��
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

	// �����ܶ�
	iRes = FrozeStkBalance(ui64Froze);

	return iRes;
}

/*
���������ͨ��Ʊ�ɷ�

Param:
in_ui64etf -- Դ��etf��صĹɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0	-- �������ɹ�
-1	-- �������ʧ��
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

	// �ⶳ�ܶ�
	iRes = DeFrozeStkBalance(in_ui64etf + in_ui64avl);

	return iRes;
}

/*
ȷ����ͨ�ɷݽ��׶���Ӷ���Ĳ��ֿ۳���Ӧ���
Param:
in_ui64etf -- Դ��etf��صĹɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0 -- ȷ�ϳɹ�
-1 -- ȷ��ʧ��
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
�����ݿ���Ϣͬ�����������ͬ��������δ�������
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

//------------------------------etf��������etf�ݶ�--------------------------------//
/*
����etf�ɷ�
Param:
out_ui64etf -- Դ���깺�ķݶ�
out_ui64avl -- Դ�ڿ���etf�ݶ�

Return:
0	-- ����ɹ�
-1	-- ����ʧ��
*/
int UserStockVolume::SellEtfFroze(uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl)
{
	IsExpired();

	// ����etf�깺����
	out_ui64etf = out_ui64avl = 0;
	int iRes = m_stk_crt_Balance.Froze(ui64Froze, &out_ui64etf);

	// 
	if (out_ui64etf == ui64Froze)
	{
		return 0;
	}

	// ���ÿ������
	iRes = m_stk_avl_Balance.Froze(ui64Froze - out_ui64etf, &out_ui64avl);

	if ((out_ui64etf + out_ui64avl) < ui64Froze)
	{
		// ����ʧ��
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

	// �����ܶ�
	iRes = FrozeStkBalance(ui64Froze);

	return iRes;
}

/*
�������etf�ɷ�
Param:
in_ui64etf -- Դ���깺�ķݶ�
in_ui64avl -- Դ�ڿ���etf�ݶ�

Return:
0	-- �������ɹ�
-1	-- �������ʧ��
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

	// �ⶳ�ܶ�
	iRes = DeFrozeStkBalance(in_ui64etf + in_ui64avl);

	return iRes;
}

/*
ȷ��etf���׶���Ӷ���Ĳ��ֿ۳���Ӧ���
Param:
in_ui64etf -- Դ���깺�ķݶ�
in_ui64avl -- Դ�ڿ���etf�ݶ�

Return:
0 -- ȷ�ϳɹ�
-1 -- ȷ��ʧ��
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
�����ݿ���Ϣͬ�����������ͬ��������δ�������
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

//------------------------------etf�깺����ɷֹ�--------------------------------//
/*
�깺�ɷֹɵ������ùɷݲ�ѯ

Return:
0 -- �㹻
-1 -- ����
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

	// ���ÿ������
	iRes = m_stk_avl_Balance.Query(ui64Query - ui64act, &ui64avl);
	out_ui64max = ui64act + ui64avl;

	return iRes;
}

/*
�����깺etf�ĳɷֹɹɷ�

Return:
0	-- ����ɹ�
-1	-- ����ʧ��
*/
int UserStockVolume::CreationFroze(uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl)
{
	IsExpired();

	// ���þ���������
	out_ui64act = out_ui64avl = 0;
	int iRes = m_stk_act_pch_Balance.Froze(ui64Froze, &out_ui64act);

	// 
	if (out_ui64act == ui64Froze)
	{
		return 0;
	}

	// ���ÿ������
	iRes = m_stk_avl_Balance.Froze(ui64Froze - out_ui64act, &out_ui64avl);

	if ((out_ui64act + out_ui64avl) < ui64Froze)
	{
		// ����ʧ��
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

	// �����ܶ�
	iRes = FrozeStkBalance(ui64Froze);

	return iRes;
}

/*
��������깺etf�ĳɷֹɹɷ�

Param:
in_ui64act -- Դ�ھ�������ɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0	-- �������ɹ�
-1	-- �������ʧ��
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

	// �ⶳ�ܶ�
	iRes = DeFrozeStkBalance(in_ui64act + in_ui64avl);

	return iRes;
}

/*
�����ݿ���Ϣͬ�����������ͬ��������δ�������
�깺�ɷֹ�ͬ��
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
�����ݿ���Ϣͬ�����������ͬ��������δ�������
�깺ETFͬ��
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
ȷ���깺etf�ĳɷֹɽ��׶���Ӷ���Ĳ��ֿ۳���Ӧ���
Param:
in_ui64act -- Դ�ھ�������ɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0 -- ȷ�ϳɹ�
-1 -- ȷ��ʧ��
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

//------------------------------etf��ض���etf�ݶ�--------------------------------//
/*
�������etf�ķݶ�
Param:
out_ui64act -- Դ������ķݶ�
out_ui64avl -- Դ�ڿ���etf�ݶ�

Return:
0	-- ����ɹ�
-1	-- ����ʧ��
*/
int UserStockVolume::RedemptionFroze(uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl)
{
	IsExpired();

	// ���þ���������
	out_ui64act = out_ui64avl = 0;
	int iRes = m_stk_act_pch_Balance.Froze(ui64Froze, &out_ui64act);

	// 
	if (out_ui64act == ui64Froze)
	{
		return 0;
	}

	// ���ÿ������
	iRes = m_stk_avl_Balance.Froze(ui64Froze - out_ui64act, &out_ui64avl);

	if ((out_ui64avl + out_ui64act) < ui64Froze)
	{
		// ����ʧ��
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

	// �����ܶ�
	iRes = FrozeStkBalance(ui64Froze);

	return iRes;
}

/*
����������etf�ķݶ�

Param:
in_ui64act -- Դ�ھ�������ɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0	-- �������ɹ�
-1	-- �������ʧ��
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

	// �ⶳ�ܶ�
	iRes = DeFrozeStkBalance(in_ui64act + in_ui64avl);

	return iRes;
}

/*
ȷ�����etf�ķݶ���Ӷ���Ĳ��ֿ۳���Ӧ���
Param:
in_ui64act -- Դ�ھ�������ɷ�����
in_ui64avl -- Դ�ڿ��ùɷ�����

Return:
0 -- ȷ�ϳɹ�
-1 -- ȷ��ʧ��
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

//------------------------------���ӹɷ�--------------------------------//
/*
�����������ӣ���ͨ�ɷݡ�etf�ݶ

Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int UserStockVolume::AddAct(uint64_t ui64StpNum)
{
	IsExpired();

	m_stk_act_pch_Balance.Add(ui64StpNum);

	m_stkBalance.Add(ui64StpNum);

	return 0;
}

/*
�깺��������(etf�ݶ�)

Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int UserStockVolume::AddCrt(uint64_t ui64CrtNum)
{
	IsExpired();

	m_stk_crt_Balance.Add(ui64CrtNum);

	m_stkBalance.Add(ui64CrtNum);

	return 0;
}

/*
���etf�ݶ�ʱ����(�ɷֹɹɷ�)

Return:
0 -- �ɹ�
-1 -- ʧ��
*/
int UserStockVolume::AddRdp(uint64_t ui64RdpNum)
{
	IsExpired();

	m_stk_etf_rdp_Balance.Add(ui64RdpNum);

	m_stkBalance.Add(ui64RdpNum);

	return 0;
}

/*
�����ݿ���Ϣͬ�����������ͬ��������δ�������
������������
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
�����ݿ���Ϣͬ�����������ͬ��������δ�������
�깺����(etf�ݶ�)
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
�����ݿ���Ϣͬ�����������ͬ��������δ�������
��ؼ���(etf�ݶ�)
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
�����ݿ���Ϣͬ�����������ͬ��������δ�������
���etf�ݶ�ʱ����(�ɷֹɹɷ�)
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
�鿴�Ƿ����
Return:
true -- ����
false -- δ����
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
�����ܶ�
Return:
0 -- �ɹ�
-1 -- ʧ��
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
�ⶳ�ܶ�
Return:
0 -- �ɹ�
-1 -- ʧ��
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