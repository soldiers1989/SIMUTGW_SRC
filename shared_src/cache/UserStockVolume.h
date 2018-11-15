#ifndef __USER_STOCK_VOLUME_H__
#define __USER_STOCK_VOLUME_H__

#include "cache/FrozeVolume.h"
#include "tool_string/Tgw_StringUtil.h"
#include "tool_string/TimeStringUtil.h"

/*
	�û��ɷ�����
	*/
class UserStockVolume
{
	//
	// member
	//
private:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// ֤ȯ�˺�
	std::string m_strUserAccount;
	// ֤ȯ����
	std::string m_strSecurityID;

	// DB���Ƿ����
	bool m_bStored;

	// `stock_balance` '֤ȯ�����������ɷ����',
	FrozeVolume m_stkBalance;

	// `stock_auction_purchase_balance` '����������',
	FrozeVolume m_stk_act_pch_Balance;

	// `stock_staple_purchase_balance` '����������',
	FrozeVolume m_stk_stp_pch_Balance;

	// `stock_etf_redemption_balance` '��ͨ��Ʊetf��������ɾ�������',
	FrozeVolume m_stk_etf_rdp_Balance;

	// `stock_creation_balance` 'etf�깺�����ɾ�������',
	FrozeVolume m_stk_crt_Balance;

	// `stock_available`'���п�����������ͨ��Ʊ���������깺etf�ݶ�Ϳɾ�������������etf������etf�ݶ����غͿɾ�������',
	FrozeVolume m_stk_avl_Balance;

	// `stock_last_balance` '�ϴ����',
	FrozeVolume m_stk_last_Balance;

	// ���һ�η���ʱ��
	unsigned long m_ullast_access_time;

	// ����ʱ������Ĭ��ʮ����600s
	unsigned long m_ultime_interval;

	// �Ƿ���
	bool m_bExpire;

	//
	// function
	//
public:
	UserStockVolume();

	UserStockVolume(bool bStored, uint64_t stkBalance, uint64_t stk_act_pch_Balance, uint64_t stk_stp_pch_Balance,
		uint64_t stk_etf_rdp_Balance, uint64_t stk_crt_Balance, uint64_t stk_avl_Balance,
		uint64_t stk_last_Balance, unsigned long ulinterval = 600);

	virtual ~UserStockVolume();

	/* �鿴�Ƿ���mysql���м�¼ */
	bool CheckStore()
	{
		return m_bStored;
	}

	/* �鿴�Ƿ���mysql���м�¼ */
	void SetStore(bool b = true)
	{
		m_bStored = b;
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
	int SellFroze(uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl);

	/*
		���������ͨ��Ʊ�ɷ�

		Param:
		in_ui64etf -- Դ��etf��صĹɷ�����
		in_ui64avl -- Դ�ڿ��ùɷ�����

		Return:
		0	-- �������ɹ�
		-1	-- �������ʧ��
		*/
	int SellDeFroze(uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
	ȷ����ͨ�ɷݽ��׶���Ӷ���Ĳ��ֿ۳���Ӧ���
	Param:
	in_ui64etf -- Դ��etf��صĹɷ�����
	in_ui64avl -- Դ�ڿ��ùɷ�����

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	int SellConfirm(uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
		�����ݿ���Ϣͬ�����������ͬ��������δ�������
		*/
	int SellDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_etf_origin, uint64_t& out_etf_update,
		uint64_t &out_avl_origin, uint64_t& out_avl_update, uint64_t& out_last_origin);

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
	int SellEtfFroze(uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl);

	/*
	�������etf�ɷ�
	Param:
	in_ui64etf -- Դ���깺�ķݶ�
	in_ui64avl -- Դ�ڿ���etf�ݶ�

	Return:
	0	-- �������ɹ�
	-1	-- �������ʧ��
	*/
	int SellEtfDeFroze(uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
	ȷ��etf���׶���Ӷ���Ĳ��ֿ۳���Ӧ���
	Param:
	in_ui64etf -- Դ���깺�ķݶ�
	in_ui64avl -- Դ�ڿ���etf�ݶ�

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	int SellEtfConfirm(uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
	�����ݿ���Ϣͬ�����������ͬ��������δ�������
	*/
	int SellEtfDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_etf_origin, uint64_t& out_etf_update,
		uint64_t &out_avl_origin, uint64_t& out_avl_update, uint64_t& out_last_origin);

	//------------------------------etf�깺����ɷֹ�--------------------------------//
	/*
		�깺�ɷֹɵ������ùɷݲ�ѯ

		Return:
		0 -- �㹻
		-1 -- ����
		*/
	int CreationQuery(uint64_t ui64Query, uint64_t &out_ui64max);

	/*
	�����깺etf�ĳɷֹɹɷ�

	Return:
	0	-- ����ɹ�
	-1	-- ����ʧ��
	*/
	int CreationFroze(uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl);

	/*
	��������깺etf�ĳɷֹɹɷ�

	Param:
	in_ui64act -- Դ�ھ�������ɷ�����
	in_ui64avl -- Դ�ڿ��ùɷ�����

	Return:
	0	-- �������ɹ�
	-1	-- �������ʧ��
	*/
	int CreationDeFroze(uint64_t in_ui64act, uint64_t in_ui64avl);

	/*
	�����ݿ���Ϣͬ�����������ͬ��������δ�������
	�깺�ɷֹ�ͬ��
	*/
	int CreationComponentDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_act_origin, uint64_t& out_act_update,
		uint64_t &out_avl_origin, uint64_t& out_avl_update, uint64_t& out_last_origin);

	/*
	�����ݿ���Ϣͬ�����������ͬ��������δ�������
	�깺ETFͬ��
	*/
	int CreationEtfDBSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_crt_origin, uint64_t& out_crt_update, uint64_t& out_last_origin);

	/*
	ȷ���깺etf�ĳɷֹɽ��׶���Ӷ���Ĳ��ֿ۳���Ӧ���
	Param:
	in_ui64act -- Դ�ھ�������ɷ�����
	in_ui64avl -- Դ�ڿ��ùɷ�����

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	int CreationConfirm(uint64_t in_ui64act, uint64_t in_ui64avl);

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
	int RedemptionFroze(uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl);

	/*
	����������etf�ķݶ�

	Param:
	in_ui64act -- Դ�ھ�������ɷ�����
	in_ui64avl -- Դ�ڿ��ùɷ�����

	Return:
	0	-- �������ɹ�
	-1	-- �������ʧ��
	*/
	int RedemptionDeFroze(uint64_t in_ui64act, uint64_t in_ui64avl);

	/*
	ȷ�����etf�ķݶ���Ӷ���Ĳ��ֿ۳���Ӧ���
	Param:
	in_ui64act -- Դ�ھ�������ɷ�����
	in_ui64avl -- Դ�ڿ��ùɷ�����

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	int RedemptionConfirm(uint64_t in_ui64act, uint64_t in_ui64avl);

	//------------------------------���ӹɷ�--------------------------------//
	/*
		�����������ӣ���ͨ�ɷݡ�etf�ݶ

		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	int AddAct(uint64_t ui64StpNum);

	/*
	�깺��������(etf�ݶ�)

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int AddCrt(uint64_t ui64CrtNum);

	/*
	���etf�ݶ�ʱ����(�ɷֹɹɷ�)

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int AddRdp(uint64_t ui64RdpNum);

	/*
	�����ݿ���Ϣͬ�����������ͬ��������δ�������
	������������
	*/
	int AddActSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_act_origin, uint64_t& out_act_update, uint64_t& out_last_origin);

	/*
	�����ݿ���Ϣͬ�����������ͬ��������δ�������
	�깺����(etf�ݶ�)
	*/
	int AddEtfSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_etf_origin, uint64_t& out_etf_update, uint64_t& out_last_origin);

	/*
	�����ݿ���Ϣͬ�����������ͬ��������δ�������
	��ؼ���(etf�ݶ�)
	*/
	int RdpEtfSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_act_origin, uint64_t& out_act_update,
		uint64_t &out_avl_origin, uint64_t& out_avl_update,
		uint64_t& out_last_origin);

	/*
	�����ݿ���Ϣͬ�����������ͬ��������δ�������
	���etf�ݶ�ʱ����(�ɷֹɹɷ�)
	*/
	int AddRdpComponentSyn(uint64_t& out_stk_origin, uint64_t& out_stk_update,
		uint64_t &out_rdp_origin, uint64_t& out_rdp_update, uint64_t& out_last_origin);


private:
	/*
		�鿴�Ƿ����
		Return:
		true -- ����
		false -- δ����
		*/
	bool IsExpired();

	/*
		�����ܶ�
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	int FrozeStkBalance(uint64_t ui64Froze);

	/*
	�ⶳ�ܶ�
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int DeFrozeStkBalance(uint64_t ui64Froze);

	/*
		���»�����Ϣ�������Ѿ�����

		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	int UpdateUserStock();
};

#endif