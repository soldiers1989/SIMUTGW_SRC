#include "MatchUtil.h"

#include "order/StockOrderHelper.h"

#include "tool_string/Tgw_StringUtil.h"

#include "simutgw/order/OrderMemoryStoreFactory.h"

#include "config/conf_mysql_table.h"
#include "simutgw_config/g_values_sys_run_config.h"
#include "simutgw/stgw_config/g_values_inner.h"

#include "simutgw/db_oper/RecordNewOrderHelper.h"
#include "cache/UserStockHelper.h"

/*
�鿴�Ƿ�ͣ��
Return:
0 -- δͣ��
1 -- ��ͣ��
*/
int MatchUtil::CheckTPBZ(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const string& in_strTpbz)
{
	static const string strTag("MatchUtil::CheckTPBZ() ");

	int iReturn = 0;

	if (0 != in_strTpbz.compare("F"))
	{
		io_orderMsg->enMatchType = simutgw::StopTrans;

		io_orderMsg->bDataError = true;
		io_orderMsg->strError = "�ù���ͣ��";

		if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
		{
			// ����
			io_orderMsg->strErrorCode = simutgw::SZ_ERRCODE::c20007;
		}
		else if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
		{
			// �Ϻ�
			io_orderMsg->strErrorCode = simutgw::SH_ERRCODE::c203;
		}
		else
		{
			// 
		}

		iReturn = -1;
	}

	return iReturn;
}

/*
�鿴�Ƿ񳬳��ǵ���
Return:
0 -- δ��
1 -- �ѳ�
*/
int MatchUtil::Check_MaxGain_And_MinFall(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const simutgw::uint64_t_Money in_ui64mMaxGain, const simutgw::uint64_t_Money in_ui64mMinFall)
{
	static const string strTag("MatchUtil::Check_MaxGain_And_MinFall() ");

	int iReturn = 0;

	// ��������¹�Ʊ��û���ǵ����ƣ��������
	if ((0 == in_ui64mMaxGain) && (0 == in_ui64mMinFall))
	{

	}
	else
	{
		// �����ǵ���
		if (in_ui64mMaxGain < io_orderMsg->ui64mOrderPrice || in_ui64mMinFall > io_orderMsg->ui64mOrderPrice)
		{
			// �����ǵ�ͣ�۸�Χ
			io_orderMsg->enMatchType = simutgw::OutOfRange;

			string strTransTmp;
			io_orderMsg->strError = "���ǵ���λ.max";
			io_orderMsg->strError += sof_string::itostr(in_ui64mMaxGain, strTransTmp);
			io_orderMsg->strError += ".min";
			io_orderMsg->strError += sof_string::itostr(in_ui64mMinFall, strTransTmp);

			io_orderMsg->bDataError = true;
			if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SZ))
			{
				// ����
				io_orderMsg->strErrorCode = simutgw::SZ_ERRCODE::c20009;
			}
			else if (0 == io_orderMsg->strTrade_market.compare(simutgw::TRADE_MARKET_SH))
			{
				// �Ϻ�
				io_orderMsg->strErrorCode = simutgw::SH_ERRCODE::c212;
			}
			else
			{
				// 
			}

			iReturn = -1;
		}
	}

	return iReturn;
}

/*
�жϳɽ�����
Param:
bLimit -- true ���޼�
-- false �м�

Return:
0 -- �ɳɽ������ֻ���ȫ��
-1 -- ���ɽ���������߹ҵ�
*/
int MatchUtil::Check_Match_Method(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje,
	bool bLimit)
{
	static const string strTag("MatchUtil::Check_Match_Method() ");

	// Ԥ�� �ɽ�ʱ�ĳɽ�����
	uint64_t ui64Cjsl_predict_match = 0;
	// Ԥ�� �ɽ�ʱ�ĳɽ����
	simutgw::uint64_t_Money ui64mCjje_predict_match = 0;

	// �����г��ľ���
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;

	// �жϴ�ϳɽ����ͣ�Ԥ���ɽ�����������Ϊ���ּ�ȫ���ɽ�
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// �г�����������ֻ�ܲ��ֳɽ�

		// Ԥ�� �ɽ�ʱ�ĳɽ�����
		ui64Cjsl_predict_match = in_ui64Cjsl;
		// Ԥ�� �ɽ�ʱ�ĳɽ����
		ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;

	}
	else
	{
		// �г������㹻������ȫ���ɽ�

		// Ԥ�� �ɽ�ʱ�ĳɽ�����
		ui64Cjsl_predict_match = io_orderMsg->ui64LeavesQty;
		// Ԥ�� �ɽ�ʱ�ĳɽ����
		ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;
	}

	// �����۸񼰿���ж�
	if (1 == io_orderMsg->iSide)
	{
		// ��
		if (bLimit && io_orderMsg->ui64mOrderPrice < ui64mAveragePrice)
		{
			// �޼ۣ���۱Ⱦ��ۻ���
			io_orderMsg->enMatchType = simutgw::NotMatch;

			string strTransTmp;
			io_orderMsg->strError = "��۱Ⱦ��ۻ���.average:";
			io_orderMsg->strError += sof_string::itostr(ui64mAveragePrice, strTransTmp);

			return -1;
		}
	}
	else if (2 == io_orderMsg->iSide)
	{
		// ����
		if (bLimit && io_orderMsg->ui64mOrderPrice > ui64mAveragePrice)
		{
			// �޼ۣ����۱Ⱦ��ۻ���

			io_orderMsg->enMatchType = simutgw::NotMatch;

			string strTransTmp;
			io_orderMsg->strError = "���۱Ⱦ��ۻ���.average:";
			io_orderMsg->strError += sof_string::itostr(ui64mAveragePrice, strTransTmp);

			return -1;
		}
	}
	else
	{
		string strTransTmp;
		io_orderMsg->strError = "δ֪��������.side:";
		io_orderMsg->strError += io_orderMsg->strSide;

		io_orderMsg->enMatchType = simutgw::ErrorMatch;
		io_orderMsg->bDataError = true;

		return -1;
	}

	return 0;
}

/*
�жϳɽ����ͣ������ʽ�͹ɷ�
Param:
bLimit -- true ���޼�
-- false �м�
Return:
0 -- �ɳɽ������ֻ���ȫ��
-1 -- ���ɽ���������߹ҵ�
*/
int MatchUtil::Check_Match_Method_WithoutAccount(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
	const uint64_t in_ui64Cjsl, const simutgw::uint64_t_Money in_ui64mCjje, bool bLimit)
{
	static const string strTag("MatchUtil::Check_Match_Method_WithoutAccount() ");

	// Ԥ�� �ɽ�ʱ�ĳɽ�����
	uint64_t ui64Cjsl_predict_match = 0;
	// Ԥ�� �ɽ�ʱ�ĳɽ����
	simutgw::uint64_t_Money ui64mCjje_predict_match = 0;

	// �����г��ľ���
	simutgw::uint64_t_Money ui64mAveragePrice = in_ui64mCjje / in_ui64Cjsl;


	// �жϴ�ϳɽ����ͣ�Ԥ���ɽ�����������Ϊ���ּ�ȫ���ɽ�
	if (in_ui64Cjsl < io_orderMsg->ui64LeavesQty)
	{
		// �г�����������ֻ�ܲ��ֳɽ�

		// Ԥ�� �ɽ�ʱ�ĳɽ�����
		ui64Cjsl_predict_match = in_ui64Cjsl;
		// Ԥ�� �ɽ�ʱ�ĳɽ����
		ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;

	}
	else
	{
		// �г������㹻������ȫ���ɽ�

		// Ԥ�� �ɽ�ʱ�ĳɽ�����
		ui64Cjsl_predict_match = io_orderMsg->ui64LeavesQty;
		// Ԥ�� �ɽ�ʱ�ĳɽ����
		ui64mCjje_predict_match = ui64mAveragePrice * ui64Cjsl_predict_match;
	}

	// �����۸񼰿���ж�
	if (1 == io_orderMsg->iSide)
	{
		// ��
		if (bLimit && io_orderMsg->ui64mOrderPrice < ui64mAveragePrice)
		{
			// �޼ۣ���۱Ⱦ��ۻ���
			io_orderMsg->enMatchType = simutgw::NotMatch;

			string strTransTmp;
			io_orderMsg->strError = "��۱Ⱦ��ۻ���.average:";
			io_orderMsg->strError += sof_string::itostr(ui64mAveragePrice, strTransTmp);

			return -1;
		}
	}
	else if (2 == io_orderMsg->iSide)
	{
		// ����
		if (bLimit && io_orderMsg->ui64mOrderPrice > ui64mAveragePrice)
		{
			// �޼ۣ����۱Ⱦ��ۻ���
			io_orderMsg->enMatchType = simutgw::NotMatch;

			string strTransTmp;
			io_orderMsg->strError = "���۱Ⱦ��ۻ���.average:";
			io_orderMsg->strError += sof_string::itostr(ui64mAveragePrice, strTransTmp);

			return -1;
		}
	}
	else
	{
		string strTransTmp;
		io_orderMsg->strError = "δ֪��������.side:";
		io_orderMsg->strError += io_orderMsg->strSide;
		io_orderMsg->bDataError = true;

		io_orderMsg->enMatchType = simutgw::ErrorMatch;

		return -1;
	}

	return 0;
}

/*
ȡһ��ί�е����齻��Ȧ
*/
void MatchUtil::Get_Order_CircleID(const std::shared_ptr<struct simutgw::OrderMessage>& orderMsg,
	string& out_strCircleID)
{
	out_strCircleID = orderMsg->strTrade_group;
	if (out_strCircleID.empty())
	{
		out_strCircleID = orderMsg->strMarket_branchid;
	}

	if (out_strCircleID.empty())
	{
		out_strCircleID = "C1";
	}
}