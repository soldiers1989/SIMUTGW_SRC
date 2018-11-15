#ifndef __BINARY_MESSAGE_STRUCT_DEFINE_H__
#define __BINARY_MESSAGE_STRUCT_DEFINE_H__

#include <stdint.h>

namespace simutgw
{
	namespace binary
	{
		/*
		length
		*/
		// ���ͷ�����
		static const size_t SENDER_COMP_ID_LEN = 20;
		// ���շ�����
		static const size_t TARGET_COMP_ID_LEN = 20;
		// ����
		static const size_t PASSWORD_LEN = 16;
		// ������Э��汾����ͨ�Ű汾��
		static const size_t DEFAULT_APPL_VER_ID_LEN = 32;
		// �ı�
		static const size_t TEXT_LEN = 200;
		// Ӧ�ñ�ʶ
		static const size_t APPL_ID_LEN = 3;
		// PBUID ���׵�Ԫ����
		static const size_t PBUID_LEN = 6;
		// ֤ȯ����
		static const size_t SECURITY_ID_LEN = 8;
		// ֤ȯ����Դ 102 = ����֤ȯ������
		static const size_t SECURITY_ID_SOURCE_LEN = 4;
		// �����������
		static const size_t CLEARING_FIRM_LEN = 2;
		// �û�˽����Ϣ
		static const size_t USER_INFO_LEN = 8;
		// �ͻ��������
		static const size_t CL_ORD_ID = 10;
		// ֤ȯ�˻�
		static const size_t ACCOUNT_ID_LEN = 12;
		// Ӫҵ������
		static const size_t BRANCH_ID_LEN = 4;
		// �����޶�
		static const size_t ORDER_RESTICTIONS_LEN = 4;
		// ��������
		static const size_t SIDE_LEN = 1;
		// �������
		static const size_t ORD_TYPE_LEN = 1;
		// ������������� char OrderID[16];
		static const size_t ORDER_ID_LEN = 16;
		// ִ�б�� char ExecID[16];
		static const size_t EXEC_ID_LEN = 16;

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// ��Ϣͷ
		// ����    �ֶ�����
		// MsgType  ��Ϣ����
		// BodyLength ��Ϣ�峤��
		struct BINARY_HEAD
		{
			// ��Ϣ����
			uint32_t MsgType;
			// ��Ϣ�峤��
			uint32_t BodyLength;

			BINARY_HEAD()
			{
				memset(this, 0x00, sizeof(BINARY_HEAD));
			}
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// ��Ϣβ
		// ��Ϣβ��������Ϣ��У��ͣ������ܴ��䡣У��͵ļ��㷶Χ����Ϣͷ��ʼ��������Ϣͷ��һֱ����Ϣ�������
		// ����    �ֶ�����
		// Checksum  У���
		struct BINARY_TAIL
		{
			// У���
			uint32_t Checksum;

			BINARY_TAIL()
			{
				memset(this, 0x00, sizeof(BINARY_TAIL));
			}
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// ��¼��Ϣ Logon
		struct BINARY_LOGON
		{
			// ���ͷ�����
			char SenderCompId[20];
			// ���շ�����
			char TargetCompId[20];

			// �����������λΪ�롣��������ϵͳϵͳ��½ʱ�ṩ����������
			uint32_t HeartBtInt;

			// ����
			char Password[16];
			// ������Э��汾����ͨ�Ű汾��
			// ��дΪ n.xy
			char DefaultApplVerID[32];

			BINARY_LOGON()
			{
				memset(this, 0x00, sizeof(BINARY_LOGON));
			}
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// ע����Ϣ Logout
		struct BINARY_LOGOUT
		{

			// �˳�ʱ�ĻỰ״̬
			// 0 = �Ự��Ծ
			// 1 = �Ự�����Ѹ���
			// 2 = �����ڵĻỰ����
			// 3 = �»Ự������Ϲ淶
			// 4 = �Ự�˵����
			// 5 = ���Ϸ����û��������
			// 6 = �˻�����
			// 7 = ��ǰʱ�䲻�����¼
			// 8 = �������
			// 9 = �յ��� MsgSeqNum(34)̫С
			// 10 = �յ��� NextExpectedMsgSeqNum(789)̫��.
			// 101 = ����
			// 102 = ��Ч��Ϣ
			int32_t	SessionStatus;

			// �ı�
			// ע��ԭ��Ľ�һ������˵��
			char Text[200];

			BINARY_LOGOUT()
			{
				memset(this, 0x00, sizeof(BINARY_LOGOUT));
			}
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// ƽ̨״̬��Ϣ Platform State Info
		struct PLATFORM_STATE_INFO
		{
			// ƽ̨�� uInt16
			// 1 = �ֻ����о��۽���ƽ̨
			// 2 = �ۺϽ��ڷ���ƽ̨
			// 3 = �ǽ��״���ƽ̨
			// 4 = ����Ʒ���о��۽���ƽ̨
			// 5 = �����г�����ƽ̨
			uint16_t PlatformID;

			// ƽ̨״̬
			// 0 = PreOpen��δ����
			// 1 = OpenUpComing����������
			// 2 = Open������
			// 3 = Halt����ͣ����
			// 4 = Close���ر�
			uint16_t PlatformState;

			PLATFORM_STATE_INFO()
			{
				memset(this, 0x00, sizeof(PLATFORM_STATE_INFO));
			}
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// �ر�ͬ����Ϣ Report Synchronization
		struct REPORT_SYNCHRONIZATION
		{
			// OMS �������յ���һ���ر��ļ�¼�ţ���
			// ����¼�Ŵ�1 ��ʼ������ţ���
			// �յ�����Ϣ��TGW ��Ӹü�¼�ſ�ʼ��
			// OMS ���ͻر������û���յ�����Ϣ,
			// TGW ������OMS ���ͻر���
			int64_t ReportIndex;

			REPORT_SYNCHRONIZATION()
			{
				memset(this, 0x00, sizeof(REPORT_SYNCHRONIZATION));
			}
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// �¶��� New Order
		struct NEW_ORDER
		{
			// Standard Header ��Ϣͷ MsgType = 1xxx01
			// Ӧ�ñ�ʶ
			char ApplID[3];
			// �걨���׵�Ԫ
			char SubmittingPBUID[6];
			// ֤ȯ����
			char SecurityID[8];
			// ֤ȯ����Դ 102 = ����֤ȯ������
			char SecurityIDSource[4];
			// ��������������
			uint16_t OwnerType;
			// �����������
			char ClearingFirm[2];
			// ί��ʱ��
			int64_t TransactTime;
			// �û�˽����Ϣ
			char UserInfo[8];
			// �ͻ��������
			char ClOrdID[10];
			// ֤ȯ�˻�
			char AccountID[12];
			// Ӫҵ������
			char BranchID[4];
			// �����޶�
			char OrderRestrictions[4];
			// ��������
			char Side;
			// �������
			// 1 ��ʾ�м�
			// 2 ��ʾ�޼�
			// U ��������
			char OrdType;
			// ��������
			int64_t OrderQty;
			// �۸�
			int64_t Price;
			// ��ҵ����չ�ֶ�
			// Extend Fields 

			NEW_ORDER()
			{
				memset(this, 0x00, sizeof(NEW_ORDER));
			}
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// �ֻ����о��۽���ҵ���¶�����չ�ֶ� 100101
		struct NEW_ORDER_EXT_100101
		{
			// ֹ���
			int64_t StopPx;
			// ��ͳɽ�����
			int64_t MinQty;
			// ���ɽ���λ��
			// 0 ��ʾ�����Ƴɽ���λ��
			uint16_t MaxPriceLevels;
			// ������Чʱ������
			char TimeInForce;
			// ���ñ�ʶ
			// 1 = Cash����ͨ����
			// 2 = Open��������ȯ����
			// 3 = Close��������ȯƽ��
			char CashMargin;

			/*
			NEW_ORDER_EXT_100101()
			{
			memset(this, 0x00, sizeof(NEW_ORDER_EXT_100101));
			}
			*/
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// ������Ӧ�������ɹ�ִ�б��� Execution Report
		struct EXECUTION_REPORT
		{
			// Standard Header ��Ϣͷ MsgType = 2xxx02
			// �ر���¼��
			int64_t ReportIndex;
			// Ӧ�ñ�ʶ
			char ApplID[3];
			// �ر����׵�Ԫ
			char ReportingPBUID[6];
			// �걨���׵�Ԫ
			char SubmittingPBUID[6];
			// ֤ȯ����
			char SecurityID[8];
			// ֤ȯ����Դ
			char SecurityIDSource[4];
			// ��������������
			uint16_t OwnerType;
			// �����������
			char ClearingFirm[2];
			// �ر�ʱ��
			int64_t TransactTime;
			// �û�˽����Ϣ
			char UserInfo[8];
			// �������������
			char OrderID[16];
			// �ͻ�������� ����Ǳ���ί�еĳɽ�����д QuoteMsgID
			char ClOrdID[10];
			// ԭʼ�����ͻ��������
			char OrigClOrdID[10];
			// ִ�б��
			char ExecID[16];
			// ִ������
			char ExecType;
			// ����״̬
			char OrdStatus;
			// ���� / �ܾ�ԭ�����
			uint16_t OrdRejReason;
			// ����ʣ������
			int64_t LeavesQty;
			// �ۼ�ִ������
			int64_t CumQty;
			// ��������
			char Side;
			// �������
			char OrdType;
			// ��������
			int64_t OrderQty;
			// �۸�
			int64_t Price;
			// ֤ȯ�˻�
			char AccountID[12];
			// Ӫҵ������
			char BranchID[4];
			// �����޶�
			char OrderRestrictions[4];
			// Extend Fields ��ҵ����չ�ֶ�

			EXECUTION_REPORT()
			{
				memset(this, 0x00, sizeof(EXECUTION_REPORT));
			}
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// �ֻ����о��۽���ҵ��ִ�б�����չ�ֶ� 200102
		struct EXECUTION_REPORT_EXT_200102
		{
			// ֹ���
			int64_t StopPx;
			// ��ͳɽ�����
			int64_t MinQty;
			// ���ɽ���λ��
			// 0 ��ʾ�����Ƴɽ���λ��
			uint16_t MaxPriceLevels;
			// ������Чʱ������
			char TimeInForce;
			// ���ñ�ʶ
			// 1 = Cash����ͨ����
			// 2 = Open��������ȯ����
			// 3 = Close��������ȯƽ��
			char CashMargin;

			EXECUTION_REPORT_EXT_200102()
			{
				memset(this, 0x00, sizeof(EXECUTION_REPORT_EXT_200102));
			}
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// �����ɽ�ִ�б��� Execution Report
		struct EXECUTION_REPORT_MATCHED
		{
			// Standard Header ��Ϣͷ MsgType = 2xxx15
			// �ر���¼��
			int64_t ReportIndex;
			// Ӧ�ñ�ʶ
			char ApplID[3];
			// �ر����׵�Ԫ
			char ReportingPBUID[6];
			// �걨���׵�Ԫ
			char SubmittingPBUID[6];
			// ֤ȯ����
			char SecurityID[8];
			// ֤ȯ����Դ
			char SecurityIDSource[4];
			// ��������������
			uint16_t OwnerType;
			// �����������
			char ClearingFirm[2];
			// �ر�ʱ��
			int64_t TransactTime;
			// �û�˽����Ϣ
			char UserInfo[8];
			// �������������
			char OrderID[16];
			// �ͻ�������� ����Ǳ���ί�еĳɽ�����д QuoteMsgID
			char ClOrdID[10];
			// ִ�б��
			char ExecID[16];
			// ִ������
			char ExecType;
			// ����״̬
			char OrdStatus;
			// �ɽ���
			int64_t LastPx;
			// �ɽ�����
			int64_t LastQty;
			// ����ʣ������
			int64_t LeavesQty;
			// �ۼ�ִ������
			int64_t CumQty;
			// ��������
			char Side;
			// ֤ȯ�˻�
			char AccountID[12];
			// Ӫҵ������
			char BranchID[4];
			// Extend Fields ��ҵ����չ�ֶ�

			EXECUTION_REPORT_MATCHED()
			{
				memset(this, 0x00, sizeof(EXECUTION_REPORT_MATCHED));
			}
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)

		// ���ֽڶ��뷽ʽ��Ϊ1
#pragma pack(push,1)
		// �ֻ����о��۽���ҵ��ɽ�ִ�б�����չ�ֶ� 200115
		struct EXECUTION_REPORT_MATCHED_EXT_200115
		{
			// ���ñ�ʶ
			// 1 = Cash����ͨ����
			// 2 = Open��������ȯ����
			// 3 = Close��������ȯƽ��
			char CashMargin;

			EXECUTION_REPORT_MATCHED_EXT_200115()
			{
				memset(this, 0x00, sizeof(EXECUTION_REPORT_MATCHED_EXT_200115));
			}
		};
		// ����ǰ�ֽڶ���ֵ��ΪĬ��ֵ(ͨ����4)
#pragma pack(pop)
	};
};


#endif