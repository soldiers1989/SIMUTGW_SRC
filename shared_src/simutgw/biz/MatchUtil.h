#ifndef __MATCH_UTIL_H__
#define __MATCH_UTIL_H__

#include <memory>

#include "order/define_order_msg.h"

/*
��Ͻ���ͨ�÷�����
*/
class MatchUtil
{
	//
	// Members
	//
protected:

	//
	// Functions
	//
public:	
	virtual ~MatchUtil( void );

	/*
	�鿴�Ƿ�ͣ��
	Return:
	0 -- δͣ��
	1 -- ��ͣ��
	*/
	static int CheckTPBZ(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg, const string& in_strTpbz);

	/*
	�鿴�Ƿ񳬳��ǵ���
	Return:
	0 -- δ��
	1 -- �ѳ�
	*/
	static int Check_MaxGain_And_MinFall(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const simutgw::uint64_t_Money in_ui64mMaxGain,const simutgw::uint64_t_Money in_ui64mMinFall);

	/*
	�жϳɽ�����
	Param:
	bLimit -- true ���޼�
	-- false �м�
	Return:
	0 -- �ɳɽ������ֻ���ȫ��
	-1 -- ���ɽ���������߹ҵ�
	*/
	static int Check_Match_Method(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl,const simutgw::uint64_t_Money in_ui64mCjje, bool bLimit = true);

	/*
	�жϳɽ����ͣ������ʽ�͹ɷ�
	Param:
	bLimit -- true ���޼�
	-- false �м�
	Return:
	0 -- �ɳɽ������ֻ���ȫ��
	-1 -- ���ɽ���������߹ҵ�
	*/
	static int Check_Match_Method_WithoutAccount(std::shared_ptr<struct simutgw::OrderMessage>& io_orderMsg,
		const uint64_t in_ui64Cjsl,const simutgw::uint64_t_Money in_ui64mCjje, bool bLimit = true);

	/*
	 ȡһ��ί�е����齻��Ȧ
	*/
	static void Get_Order_CircleID(const std::shared_ptr<struct simutgw::OrderMessage>& orderMsg, 
		string& out_strCircleID);

private:
	// ��ֹʹ��Ĭ�Ϲ��캯��
	MatchUtil( void );
};

#endif
