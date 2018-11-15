#ifndef __BINARY_MSG_TYPE_DEFINE_H__
#define __BINARY_MSG_TYPE_DEFINE_H__

#include <stdint.h>

namespace simutgw
{
	namespace BINARY
	{
		// ��¼
		const uint32_t MsgType_Logon1 = 1;

		// �ǳ�
		const uint32_t MsgType_Logout2 = 2;

		// ����
		const uint32_t MsgType_HeartBeat3 = 3;

		// ҵ��ܾ�
		const uint32_t MsgType_BusinessReject4 = 4;

		// �ر�ͬ��
		const uint32_t MsgType_ReportSynchronization5 = 5;

		// ƽ̨״̬
		const uint32_t MsgType_PlatformStateInfo6 = 6;

		// �ر�����
		const uint32_t MsgType_ReportFinished7 = 7;

		// �¶��� 1xxx01
		const uint32_t MsgType_NewOrder100101 = 100101; //�ֻ�����Ʊ������ծȯ�ȣ����о��۽����걨
		const uint32_t MsgType_NewOrder100201 = 100201; //��Ѻʽ�ع������걨
		const uint32_t MsgType_NewOrder100301 = 100301; //ծȯ�����걨
		const uint32_t MsgType_NewOrder100401 = 100401; //��Ȩ���о��۽����걨
		const uint32_t MsgType_NewOrder100501 = 100501; //Э�齻�׶����걨\Э�齻�׵�����걨
		const uint32_t MsgType_NewOrder100601 = 100601; //�����̼۽��׵��̺󶨼۽����걨\�Գɽ�����Ȩƽ���۽��׵��̺󶨼۽����걨
		const uint32_t MsgType_NewOrder100701 = 100701; //ת��֤ͨȯ�����Լ���걨
		const uint32_t MsgType_NewOrder101201 = 101201; //ETFʵʱ�깺����걨
		const uint32_t MsgType_NewOrder101301 = 101301; //���Ϸ����Ϲ��걨
		const uint32_t MsgType_NewOrder101401 = 101401; //����Ϲ��걨
		const uint32_t MsgType_NewOrder101501 = 101501; //ծȯת���걨\ծȯ�����걨
		const uint32_t MsgType_NewOrder101601 = 101601; //��Ȩ��Ȩ�걨
		const uint32_t MsgType_NewOrder101701 = 101701; //����ʽ�����깺����걨
		const uint32_t MsgType_NewOrder101801 = 101801; //ҪԼ�չ�Ԥ��ҪԼ�걨\ҪԼ�չ����Ԥ��ҪԼ�걨
		const uint32_t MsgType_NewOrder101901 = 101901; //��Ѻʽ�ع���Ѻ�걨\��Ѻʽ�ع���Ѻ�걨
		const uint32_t MsgType_NewOrder102201 = 102201; //�ƽ�ETFʵ���깺����걨����ί��ֻ�����Ϻ��ƽ������걨��
		const uint32_t MsgType_NewOrder102301 = 102301; //Ȩ֤��Ȩ�걨
		const uint32_t MsgType_NewOrder102601 = 102601; //���������걨\���ҽ����걨
		const uint32_t MsgType_NewOrder102701 = 102701; //ת���ÿ�ȯ�걨\ת���û�ȯ�걨
		const uint32_t MsgType_NewOrder102801 = 102801; //��ȯ�걨\��ȯ�걨
		const uint32_t MsgType_NewOrder102901 = 102901; //���峥�ۻ��ͻ��걨\���峥�ۻ���Ӫ�걨
		const uint32_t MsgType_NewOrder103101 = 103101; //�ּ�����ʵʱ�ֲ��걨\�ּ�����ʵʱ�ϲ��걨
		const uint32_t MsgType_NewOrder103301 = 103301; //ծȯ��Ѻʽ�����ع�������걨  
		const uint32_t MsgType_NewOrder106301 = 106301; //�۹�ͨ�걨 �����г�����ƽ̨

		// ��������
		const uint32_t MsgType_CancelRequest190007 = 190007;  //��������

		// ������Ӧʧ��
		const uint32_t MsgType_CancelReject290008 = 290008;   //������Ӧʧ��

		//������Ӧ�������ɹ�ִ�б���  2xxx02
		const uint32_t MsgType_ExecutionReport200102 = 200102; //�ֻ�����Ʊ������ծȯ�ȣ����о��۽���ִ�б���
		const uint32_t MsgType_ExecutionReport200202 = 200202; //��Ѻʽ�ع�����ִ�б���
		const uint32_t MsgType_ExecutionReport200302 = 200302; //ծȯ����ִ�б���
		const uint32_t MsgType_ExecutionReport200402 = 200402; //��Ȩ���о��۽���ִ�б���
		const uint32_t MsgType_ExecutionReport200502 = 200502; //Э�齻�׶���ִ�б���\Э�齻�׵���ɽ�ִ�б���
		const uint32_t MsgType_ExecutionReport200602 = 200602; //�����̼۽��׵��̺󶨼۽���ִ�б���\�Գɽ�����Ȩƽ���۽��׵��̺󶨼۽���ִ�б���
		const uint32_t MsgType_ExecutionReport200702 = 200702; //ת��֤ͨȯ�����Լ��ִ�б���
		const uint32_t MsgType_ExecutionReport201202 = 201202; //��Ʊ/ծȯ/����ETFʵʱ�깺���ִ�б���\  �ƽ�ETF�ֽ��깺���ִ�б���
		const uint32_t MsgType_ExecutionReport201302 = 201302; //���Ϸ����Ϲ�ִ�б���
		const uint32_t MsgType_ExecutionReport201402 = 201402; //����Ϲ�ִ�б���
		const uint32_t MsgType_ExecutionReport201502 = 201502; //ծȯת��ִ�б���\ծȯ����ִ�б���
		const uint32_t MsgType_ExecutionReport201602 = 201602; //��Ȩ��Ȩִ�б���
		const uint32_t MsgType_ExecutionReport201702 = 201702; //����ʽ�����깺���ִ�б���
		const uint32_t MsgType_ExecutionReport201802 = 201802; //ҪԼ�չ�Ԥ��ҪԼִ�б���\ҪԼ�չ����Ԥ��ҪԼִ�б���
		const uint32_t MsgType_ExecutionReport201902 = 201902; //��Ѻʽ�ع���Ѻִ�б���\��Ѻʽ�ع���Ѻִ�б���
		const uint32_t MsgType_ExecutionReport202202 = 202202; //�ƽ�ETFʵ���깺���ִ�б���
		const uint32_t MsgType_ExecutionReport202302 = 202302; //Ȩ֤��Ȩִ�б���
		const uint32_t MsgType_ExecutionReport202602 = 202602; //��������ִ�б���\���ҽ���ִ�б���
		const uint32_t MsgType_ExecutionReport202702 = 202702; //ת���ÿ�ȯִ�б���\ת���û�ȯִ�б���
		const uint32_t MsgType_ExecutionReport202802 = 202802; //��ȯִ�б���\��ȯִ�б���
		const uint32_t MsgType_ExecutionReport202902 = 202902; //���峥�ۻ��ͻ��걨ִ�б���\���峥�ۻ���Ӫ�걨ִ�б���
		const uint32_t MsgType_ExecutionReport203102 = 203102; //�ּ�����ʵʱ�ֲ��걨\�ּ�����ʵʱ�ϲ��걨
		const uint32_t MsgType_ExecutionReport206302 = 206302; //�۹�ͨ����ִ�б���

		// �����ɽ�ִ�б��� 2xxx15
		const uint32_t MsgType_MatchedExecutionReport200115 = 200115; //�ֻ�����Ʊ������ծȯ�ȣ����о��۽���ִ�б���
		const uint32_t MsgType_MatchedExecutionReport200215 = 200215; //��Ѻʽ�ع�����ִ�б���
		const uint32_t MsgType_MatchedExecutionReport200315 = 200315; //ծȯ����ִ�б���
		const uint32_t MsgType_MatchedExecutionReport200415 = 200415; //��Ȩ���о��۽���ִ�б���
		const uint32_t MsgType_MatchedExecutionReport200515 = 200515; //Э�齻�׶���ִ�б���\Э�齻�׵���ɽ�ִ�б���
		const uint32_t MsgType_MatchedExecutionReport200615 = 200615; //�����̼۽��׵��̺󶨼۽���ִ�б���\�Գɽ�����Ȩƽ���۽��׵��̺󶨼۽���ִ�б���
		const uint32_t MsgType_MatchedExecutionReport200715 = 200715; //ת��֤ͨȯ�����Լ��ִ�б���
		const uint32_t MsgType_MatchedExecutionReport202615 = 202615; //��������ִ�б���\���ҽ���ִ�б���
		const uint32_t MsgType_MatchedExecutionReport206315 = 206315; //�۹�ͨ����ִ�б���

		// ���� 1xxx05
		const uint32_t MsgType_Quote100405 = 100405;//��Ȩ���о��۽��ױ����걨
		const uint32_t MsgType_Quote100505 = 100505;//Э�齻�ױ����걨

		// ����״̬�ر�  2xxx06
		const uint32_t MsgType_QuoteStatusReport200406 = 200406;//��Ȩ���о��۽��ױ���״̬
		const uint32_t MsgType_QuoteStatusReport200506 = 200506;//Э�齻�ױ���״̬�ر�

		// ѯ���걨 1xxx17
		const uint32_t MsgType_QuoteRequest100417 = 100417;//��Ȩ���о��۽���ѯ���걨

		// ѯ���걨������Ӧ 2xxx17
		const uint32_t MsgType_QuoteRequestAck200417 = 200417;//��Ȩ���о��۽���ѯ���걨

		// ѯ���걨�ܾ���Ӧ 2xxx18
		const uint32_t MsgType_QuoteRequestReject200418 = 200418;//��Ȩ���о��۽���ѯ���걨

		// �����걨 1xxx09
		const uint32_t MsgType_IndicationOfInterest100509 = 100509;//Э�齻�������걨

		// �����걨��Ӧ 2xxx10
		const uint32_t MsgType_QuoteResponse200510 = 200510;//Э�齻�������걨��Ӧ

		// �ɽ��걨 1xxx03
		const uint32_t MsgType_TradeCapture100503 = 100503;//Э�齻��˫��Э��ɽ��걨
		const uint32_t MsgType_TradeCapture100703 = 100703;//ת��֤ͨȯ����Լ���걨
		const uint32_t MsgType_TradeCapture100803 = 100803;//�ʲ�����ƻ��ݶ�ת��
		const uint32_t MsgType_TradeCapture100903 = 100903;//��Ʊ��Ѻʽ�ع�
		const uint32_t MsgType_TradeCapture101003 = 101003;//Լ������
		const uint32_t MsgType_TradeCapture101103 = 101103;//��Ѻʽ���ۻع�
		const uint32_t MsgType_TradeCapture103003 = 103003;//ծȯ��ѺʽЭ��ع�
		const uint32_t MsgType_TradeCapture103203 = 103203;//ծȯ��Ѻʽ�����ع������걨


		// �ɽ��걨 �ɽ��걨��Ӧ 2xxx04
		const uint32_t MsgType_TradeCaptureReportAck200504 = 200504;//Э�齻��˫��Э��ɽ��걨��Ӧ
		const uint32_t MsgType_TradeCaptureReportAck200704 = 200704;//ת��֤ͨȯ����Լ���걨��Ӧ
		const uint32_t MsgType_TradeCaptureReportAck200804 = 200804;//�ʲ�����ƻ��ݶ�ת�óɽ��걨��Ӧ
		const uint32_t MsgType_TradeCaptureReportAck200904 = 200904;//��Ʊ��Ѻʽ�ع��ɽ��걨��Ӧ
		const uint32_t MsgType_TradeCaptureReportAck201004 = 201004;//Լ�����سɽ��걨��Ӧ
		const uint32_t MsgType_TradeCaptureReportAck201104 = 201104;//��Ѻʽ���ۻع��ɽ��걨��Ӧ
		const uint32_t MsgType_TradeCaptureReportAck203004 = 203004;//ծȯ��ѺʽЭ��ع��ɽ��걨��Ӧ
		const uint32_t MsgType_TradeCaptureReportAck203204 = 203204;//ծȯ��Ѻʽ�����ع��ɽ��걨��Ӧ

		// �ɽ�ȷ�� 2xxx03
		const uint32_t MsgType_TradeCaptureReport200503 = 200503;//Э�齻��˫��Э��ɽ��걨ȷ��
		const uint32_t MsgType_TradeCaptureReport200703 = 200703;//ת��֤ͨȯ����Լ���걨ȷ��
		const uint32_t MsgType_TradeCaptureReport203003 = 203003;//ծȯ��ѺʽЭ��ع��ɽ��걨ȷ��
		const uint32_t MsgType_TradeCaptureReport203203 = 203203;//ծȯ��ѺʽЭ��ع��ɽ��걨ȷ��

		// ע��
		const uint32_t MsgType_Designation102099 = 102099;//ע��

		// ע��ִ�б���
		const uint32_t MsgType_DesignationReport202098 = 202098;//ע��ִ�б���

		// ͶƱ
		const uint32_t MsgType_Evote102197 = 102197; //ͶƱ

		// ͶƱִ�б���
		const uint32_t MsgType_EvoteReport202196 = 202196;//ͶƱִ�б���

		// �������
		const uint32_t MsgType_PasswordService102489 = 102489;// �������

		// �������ִ�б���
		const uint32_t MsgType_PasswordServiceReport202488 = 202488;//�������ִ�б���

		// ��֤���ѯ
		const uint32_t MsgType_MarginQuery102587 = 102587;// ��֤���ѯ

		// ��֤���ѯ���
		const uint32_t MsgType_MarginQueryResult202586 = 202586;//��֤���ѯ���

		const uint32_t MsgType_ForwardReport203220 = 203220;  //ת���ɽ��걨
	};
};
#endif