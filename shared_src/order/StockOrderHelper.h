#ifndef __STOCK_ORDER_HELPER_H__
#define __STOCK_ORDER_HELPER_H__

#include <string>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif

#include "order/define_order_msg.h"


/*
�µ���Ϣ������
*/
class StockOrderHelper
{
public:
	StockOrderHelper(void);
	virtual ~StockOrderHelper(void);

	/*
	��֤�µ����ݵĺϷ���

	Return :
	0 -- �Ϸ�
	��0 -- ���Ϸ�
	*/
	static int OrderMsgValidate(std::shared_ptr<struct simutgw::OrderMessage> &io_orderMsg,
		int& out_iStockId);

	/*
	ת��Ϊ��Ļ��ӡ��������

	*/
	static std::string& OrderToScreenOut(const std::shared_ptr<struct simutgw::OrderMessage>& order,
		std::string& out_strScreenOut);

	/*
	���ݹ�Ʊ�����ȡ��Ʊ��������
	"0" -- A��
	"1" -- B��
	"2" -- ���ʽ���
	"3" -- ��ȯ����

	����A�ɵĴ�������000��ͷ��
	����B�ɵĴ�������200��ͷ��

	����A�ɵĴ�������600��601��603��ͷ��
	����B�ɵĴ�������900��ͷ��

	Return:
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	static int CheckOrder_TradeType(std::shared_ptr<struct simutgw::OrderMessage> &io_OrderMsg);

	/*
	�ж϶��������ĸ����׹���

	@param const std::map<uint64_t, std::string>& in_mapLinkRules : �ɽ����ú�ͨ���Ĺ�ϵ

	Return:
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	static int GetOrderMatchRule(std::shared_ptr<struct simutgw::OrderMessage> &io_OrderMsg,
		const std::map<uint64_t, uint64_t>& in_mapLinkRules);

private:

};

#endif