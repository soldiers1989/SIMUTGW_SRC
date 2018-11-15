#ifndef __USER_STOCK_HELPER_H__
#define __USER_STOCK_HELPER_H__

#include <vector>
#include <memory>

#include "etf/conf_etf_info.h"
#include "order/define_order_msg.h"
#include "order/StockHelper.h"

// 
struct UserStockInfo
{
	std::string strAccount;
	std::string strStockID;
	// `stock_balance` '֤ȯ�����������ɷ����',
	uint64_t ui64stkBalance;

	// `stock_auction_purchase_balance` '����������',
	uint64_t ui64stk_act_pch_Balance;

	// `stock_staple_purchase_balance` '����������',
	uint64_t ui64stk_stp_pch_Balance;

	// `stock_etf_redemption_balance` '��ͨ��Ʊetf��������ɾ�������',
	uint64_t ui64stk_etf_rdp_Balance;

	// `stock_creation_balance` 'etf�깺�����ɾ�������',
	uint64_t ui64stk_crt_Balance;

	// `stock_available`'���п�����������ͨ��Ʊ���������깺etf�ݶ�Ϳɾ�������������etf������etf�ݶ����غͿɾ�������',
	uint64_t ui64stk_avl_Balance;

	// `stock_last_balance` '�ϴ����',
	uint64_t ui64stk_last_Balance;
};

/*
	�û��ɷ���ϢHelper��

	�ṩ���ⲿ������
	�����û����˻����Ĺɷݡ�etf
	�ⶳ�û����˻����Ĺɷݡ�etf
	ȷ���û����˻����Ĺɷݡ�etf
	�����û����˻����Ĺɷݡ�etf
	������л���Ĺɷ���Ϣ

	�ڲ�������
	������Ҫ���в������û��ɷ���Ϣ
	�����ݿ�����û��ɷ���Ϣ
	�ж���ͨ��Ʊ��etf
	*/
class UserStockHelper
{
	//
	// memebr
	//

	//
	// function
	//
public:
	UserStockHelper();
	virtual ~UserStockHelper();

	/*
	����ʱ��ջ���
	*/
	static void DayEndCleanUp(void);

	/*
	���׺�����û��ɷ�
	*/
	static int UpdateAfterTrade(std::shared_ptr<struct simutgw::OrderMessage>& io_order);

	/*
	�����ɹ�������û��ɷ�
	*/
	static int UpdateAfterCancel(std::shared_ptr<struct simutgw::OrderMessage>& io_order);

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
	static int SellFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl);

	/*
	���������ͨ��Ʊ�ɷ�

	Param:
	in_ui64etf -- Դ��etf��صĹɷ�����
	in_ui64avl -- Դ�ڿ��ùɷ�����

	Return:
	0	-- �������ɹ�
	-1	-- �������ʧ��
	*/
	static int SellDeFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
	ȷ����ͨ�ɷݽ��׶���Ӷ���Ĳ��ֿ۳���Ӧ���
	Param:
	in_ui64etf -- Դ��etf��صĹɷ�����
	in_ui64avl -- Դ�ڿ��ùɷ�����

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	static int SellConfirm(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64etf, uint64_t in_ui64avl);

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
	static int SellEtfFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t ui64Froze, uint64_t &out_ui64etf, uint64_t &out_ui64avl);

	/*
	�������etf�ɷ�
	Param:
	in_ui64etf -- Դ���깺�ķݶ�
	in_ui64avl -- Դ�ڿ���etf�ݶ�

	Return:
	0	-- �������ɹ�
	-1	-- �������ʧ��
	*/
	static int SellEtfDeFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64etf, uint64_t in_ui64avl);

	/*
	ȷ��etf���׶���Ӷ���Ĳ��ֿ۳���Ӧ���
	Param:
	in_ui64etf -- Դ���깺�ķݶ�
	in_ui64avl -- Դ�ڿ���etf�ݶ�

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	static int SellEtfConfirm(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64etf, uint64_t in_ui64avl);

	//------------------------------etf�깺����ɷֹ�--------------------------------//
	/*
	�깺etf�����гɷֹɹɷ��Ƿ��㹻
	Param:
	ui64EtfQty -- ETF����
	out_ui64CashCompnents -- �ֽ�����ܽ��

	Return:
	0	-- �㹻
	-1	-- ����
	*/
	static int CreationQuery(const std::string& strAccount, const uint64_t ui64EtfQty,
		const std::shared_ptr<struct simutgw::SzETF>& ptrEtf,
		std::vector<std::shared_ptr<struct simutgw::EtfCrt_FrozeComponent>> &io_vecFrozeCompnent);

	/*
	�����깺etf�ĳɷֹɹɷ�

	Return:
	0	-- ����ɹ�
	-1	-- ����ʧ��
	*/
	static int CreationFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl);

	/*
	��������깺etf�ĳɷֹɹɷ�

	Param:
	in_ui64act -- Դ�ھ�������ɷ�����
	in_ui64avl -- Դ�ڿ��ùɷ�����

	Return:
	0	-- �������ɹ�
	-1	-- �������ʧ��
	*/
	static int CreationDeFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64act, uint64_t in_ui64avl);

	/*
	ȷ���깺etf�ĳɷֹɽ��׶���Ӷ���Ĳ��ֿ۳���Ӧ���
	Param:
	in_ui64act -- Դ�ھ�������ɷ�����
	in_ui64avl -- Դ�ڿ��ùɷ�����

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	static int CreationConfirm(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64act, uint64_t in_ui64avl);

	//------------------------------etf��ض���etf�ݶ�--------------------------------//
	/*
	�������etf�ķݶ�

	Return:
	0	-- ����ɹ�
	-1	-- ����ʧ��
	*/
	static int RedemptionFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t ui64Froze, uint64_t &out_ui64act, uint64_t &out_ui64avl);

	/*
	����������etf�ķݶ�

	Param:
	in_ui64act -- Դ�ھ�������ɷ�����
	in_ui64avl -- Դ�ڿ��ùɷ�����

	Return:
	0	-- �������ɹ�
	-1	-- �������ʧ��
	*/
	static int RedemptionDeFroze(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64act, uint64_t in_ui64avl);

	/*
	ȷ�����etf�ķݶ���Ӷ���Ĳ��ֿ۳���Ӧ���
	Param:
	in_ui64act -- Դ�ھ�������ɷ�����
	in_ui64avl -- Դ�ڿ��ùɷ�����

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	static int RedemptionConfirm(const std::string& strAccount, const std::string& strStockID,
		uint64_t in_ui64act, uint64_t in_ui64avl);

	//------------------------------���ӹɷ�--------------------------------//
	/*
	�����������ӣ���ͨ�ɷݡ�etf�ݶ

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int AddAct(const std::string& strAccount, const std::string& strStockID, uint64_t ui64StpNum);

	/*
	�깺��������(etf�ݶ�)

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int AddCrt(const std::string& strAccount, const std::string& strStockID, uint64_t ui64CrtNum);

	/*
	���etf�ݶ�ʱ����(�ɷֹɹɷ�)

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int AddRdp(const std::string& strAccount, const std::string& strStockID, uint64_t ui64RdpNum);

private:
	/*
		�����û��ɷ���Ϣ
		���ڴ����У����سɹ�
		���ڴ����ޣ����ѯ���ݿ⣬�м��ؽ��ڴ沢���سɹ���û�з���ʧ��

		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int LookUp(const std::string& strAccount, const std::string& strStockID, enum StockHelper::StockType& out_stkType);


	/*
		�����ݿ���عɷ���Ϣ

		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int LoadUserStockFromDBAndStore(const std::string& strAccount, const std::string& strStockID);

	/*
		���׺�����û��ɷ�--��ͨ����(����etf����)
		*/
	static int UpdateTrade(std::shared_ptr<struct simutgw::OrderMessage>& io_order);

	/*
	���׺�����û��ɷ�--ETF����
	*/
	static int UpdateETFCrtRdp(std::shared_ptr<struct simutgw::OrderMessage>& io_order);

	/*
	����������û��ɷ�--��ͨ����(����etf����)
	*/
	static int UpdateCancel(std::shared_ptr<struct simutgw::OrderMessage>& io_order);
};

#endif