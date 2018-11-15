#ifndef __CONF_MSG_H__
#define __CONF_MSG_H__

#include <string>
#include <stdint.h>

namespace simutgw
{
	// �Է�Ϊ��λ�Ľ��
	typedef uint64_t uint64_t_Money;

	//
	// STEP�ӿ���Ϣ
	//
	/*
	ExecType
	C1 ִ�б�������
	0=New����ʾ�¶���
	4=Cancelled����ʾ�ѳ���
	8=Reject����ʾ�Ѿܾ�
	F=Trade����ʾ�ѳɽ�
	*/
	static const std::string STEPMSG_EXECTYPE_NEW("0");
	static const std::string STEPMSG_EXECTYPE_CANCEL("4");
	static const std::string STEPMSG_EXECTYPE_REJECT("8");
	static const std::string STEPMSG_EXECTYPE_TRADE("F");

	/*
	OrdStatus
	C1 ����״̬
	0=New����ʾ�¶���
	1=Partially filled����ʾ���ֳɽ�
	2=Filled����ʾȫ���ɽ�
	4=Cancelled����ʾ�ѳ���
	8=Reject����ʾ�Ѿܾ�
	*/
	static const std::string STEPMSG_ORDSTATUS_NEW("0");
	static const std::string STEPMSG_ORDSTATUS_PART_FILL("1");
	static const std::string STEPMSG_ORDSTATUS_FILL("2");
	static const std::string STEPMSG_ORDSTATUS_CANCEL("4");
	static const std::string STEPMSG_ORDSTATUS_REJECT("8");

	/*
	MsgType
	D=�¶����� New Order��,������
	F=�������� Order Cancel Request��
	8=ִ�б��棨 Execution Report��
	9=����ʧ����Ӧ�� Cancel Reject��
	*/
	static const std::string STEPMSG_MSGTYPE_NEW_ORDER("D");
	static const std::string STEPMSG_MSGTYPE_ORDER_CACEL("F");
	static const std::string STEPMSG_MSGTYPE_EXECREPORT("8");
	static const std::string STEPMSG_MSGTYPE_CANCELREJECT("9");

	/*
	Side
	B=��
	S=��
	G=����
	F=����
	D=�깺
	E=���
	*/
	static const std::string STEPMSG_SIDE_BUY_B("B");	
	static const std::string STEPMSG_SIDE_BUY_1("1");
	static const std::string STEPMSG_SIDE_SELL_S("S");
	static const std::string STEPMSG_SIDE_SELL_2("2");
	static const std::string STEPMSG_SIDE_BORROW("G");
	static const std::string STEPMSG_SIDE_LEND("F");
	static const std::string STEPMSG_SIDE_CRT("D");
	static const std::string STEPMSG_SIDE_RDP("E");

	/*
	trade_market
	�����г�
	101 -- �Ϻ�
	102 -- ����	
	*/
	static const std::string TRADE_MARKET_SH("101"); // �Ϻ�
	static const std::string TRADE_MARKET_SZ("102"); // ����
	

	namespace SZ_ERRCODE
	{
		// ����ر� ί���걨�ܾ�ԭ������
		// ERRORCODE��Ч�˻�
		static const std::string c20001("20001");

		// 20007 ͣ��/��Ч֤ȯ����
		static const std::string c20007("20007");

		// 20022 �ɷ�����
		static const std::string c20022("20022");

		// 20005 ҵ���ֹ ֤ȯ��ҵ�񿪹ر�
		static const std::string c20005("20005");

		// 20076 �걨��ʽ���� ����ί���걨������TimeInForce��OrdType��MaxPriceLevels��MinQty�ֶε���ϲ��Ϸ�
		static const std::string c20076("20076");

		// 20101 Ӧ�ñ�ʶ�� Ӧ�ñ�ʶ����
		static const std::string c20101("20101");

		// 20106 �ֶδ��� �ֶ�ȡֵ����
		static const std::string c20106("20106");

		// 20010 �����Ƿ� ί����������������λ�����������ּ�����ֲ�ϲ�ί����������ȵ�������
		static const std::string c20010("20010");

		// 20009 �۸���� ί�м۸񳬹��ǵ������ƣ��̺󶨼۴��ڽ���ί�м۸�Ϊָ���۸�ת��֤ͨȯ����ί�еķ����빫���Ĳ�һ��
		static const std::string c20009("20009");

		// 20095 ����������� ������������� applid ��֤ȯ id ��ԭί�еĲ�һ��
		static const std::string c20095("20095");

		// 20064 �ʽ��� ��������˿����ʽ���
		static const std::string c20064("20064");
	};

	namespace SH_ERRCODE
	{
		// �Ͻ����ر� ί���걨�ܾ�ԭ������
		// 215  ��Ч�˻�
		static const std::string c215("215");

		// 203 ͣ��/��Ч֤ȯ����
		static const std::string c203("203");

		// 212 �۸����
		static const std::string c212("212");

		// 231 �ɷ�����
		static const std::string c231("231");

		// 209	����Ʒ�ֲ���
		static const std::string c209("209");

		// 269	�걨���ʹ���
		static const std::string c269("269");

		// 218	��Ч�걨�۸�
		static const std::string c218("218");

		// 224	��Ч���걨����
		static const std::string c224("224");
	};

	/*
	��� ֤ȯ����ͳ�ƿ�SJSTJ.DBF
	*/
	struct Sz_SJSTJ
	{
		std::string str_AccountId;
		//��� �ֶ��� �ֶ����� ���� ���� ��ע
		// 1 TJXWDM �йܱ��� C 6
		std::string str_TJXWDM;

		// 2 TJZQDM ֤ȯ���� C 6
		std::string str_TJZQDM;

		// 3 TJMRGS ������� N 12,0
		//uint64_t ui64_TJMRGS;
		std::string str_TJMRGS;

		// 4 TJMRZJ �����ʽ� N 15,3
		//simutgw::uint64_t_Money ui64m_TJMRZJ;
		std::string str_TJMRZJ;

		// 5 TJMCGS �������� N 12,0
		//uint64_t ui64_TJMCGS;
		std::string str_TJMCGS;

		// 6 TJMCZJ �����ʽ� N 15,3
		//simutgw::uint64_t_Money ui64m_TJMCZJ;
		std::string str_TJMCZJ;

		// 7 TJBJSF ���ַ� N 15,3
		simutgw::uint64_t_Money ui64m_TJBJSF;

		// 8 TJSJSF �����ַ� N 15,3
		simutgw::uint64_t_Money ui64m_TJSJSF;

		// 9 TJBYHS ��ӡ��˰ N 15,3
		simutgw::uint64_t_Money ui64m_TJBYHS;

		// 10 TJSYHS ��ӡ��˰ N 15,3
		simutgw::uint64_t_Money ui64m_TJSYHS;

		// 11 TJBGHF ������� N 15,3
		simutgw::uint64_t_Money ui64m_TJBGHF;

		// 12 TJSGHF �������� N 15,3
		simutgw::uint64_t_Money ui64m_TJSGHF;

		// 13 TJCJRQ �ɽ����� D 8 CCYYMMDD
		std::string str_TJCJRQ;

		// 14 TJBYBZ ���ñ�־	C 1 ���������ϵͳ����
	};
}

#endif