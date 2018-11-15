#ifndef __SYS_FUNCTION_H__
#define __SYS_FUNCTION_H__

#include <vector>

#include "boost/shared_ptr.hpp"

#include "config/sys_function_base.h"

namespace simutgw
{
	/*
	�ؼ������Ҫ�������
	*/
	void SimuTgwSelfExit(void);

	/*
	�ؼ������Ҫ�������
	ָ����������
	*/
	void SimuTgwSelfExit_remoterestart(void);

	/*
	����
	@param const std::vector<std::string>& in_vctSettleGroup : ����ر���
	@param std::string& out_strDay : ��ǰ�����ַ���
	@param std::string& out_strSettlementFilePath : �����ļ�����·��

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Simutgw_Settle(const std::vector<std::string>& in_vctSettleGroup,
		std::string& out_strDay, std::string& out_strSettlementFilePath);

	/*
	����
	*/
	int Simutgw_DayEnd();
};

#endif