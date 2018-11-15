#ifndef __BINARY_ORDER_MESSAGE_LOCAL_H__
#define __BINARY_ORDER_MESSAGE_LOCAL_H__

#include <memory>
#include <string>
#include <stdint.h>
namespace simutgw
{
	namespace binary
	{
		// �¶��� New Order
		struct NEW_ORDER_LOCAL
		{
			// Standard Header ��Ϣͷ MsgType = 1xxx01
			// Ӧ�ñ�ʶ char ApplID[3];
			std::string ApplID;

			// �걨���׵�Ԫ char SubmittingPBUID[6];
			std::string SubmittingPBUID;

			// ֤ȯ���� char SecurityID[8];
			std::string SecurityID;

			// ֤ȯ����Դ 102 = ����֤ȯ������ char SecurityIDSource[4];
			std::string SecurityIDSource;

			// ��������������
			uint16_t OwnerType;

			// ����������� char ClearingFirm[2];
			std::string ClearingFirm;

			// ί��ʱ��
			int64_t TransactTime;

			// �û�˽����Ϣ char UserInfo[8];
			std::string UserInfo;

			// �ͻ�������� char ClOrdID[10];
			std::string ClOrdID;

			// ֤ȯ�˻� char AccountID[12];
			std::string AccountID;

			// Ӫҵ������ char BranchID[4];
			std::string BranchID;

			// �����޶� char OrderRestrictions[4];
			std::string OrderRestrictions;

			// �������� char Side;
			char Side;

			// ������� 1 ��ʾ�м� 2 ��ʾ�޼� U ��������
			char OrdType;

			// ��������
			int64_t OrderQty;
			// �۸�
			int64_t Price;
			// ��ҵ����չ�ֶ�
			// Extend Fields 

			NEW_ORDER_LOCAL()
			{
				OwnerType = 0;
				TransactTime = 0;
				Side = '\0';
				OrdType = '\0';
				OrderQty = 0;
				Price = 0;
			}
		};

		// �ֻ����о��۽���ҵ���¶�����չ�ֶ� 100101
		struct NEW_ORDER_EXT_100101_LOCAL
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

			NEW_ORDER_EXT_100101_LOCAL()
			{
				StopPx = 0;
				MinQty = 0;
				MaxPriceLevels = 0;
				TimeInForce = '\0';
				CashMargin = '\0';
			}
		};
	};
};

#endif