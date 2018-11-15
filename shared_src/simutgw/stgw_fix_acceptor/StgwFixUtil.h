#ifndef __STGW_FIX_UTIL_H__
#define __STGW_FIX_UTIL_H__

#include "simutgw/stgw_fix_acceptor/StgwApplication.h"

/*
fix��Ϣ������

��Ҫ����
@function GetNopartyIds ����NopartyIds�е�ϯλ���˺š�Ӫҵ������
*/
namespace StgwFixUtil
{
	//
	// function
	//
	// ȡnopartyids�ظ�������ݣ������˺š�ϯλ��Ӫҵ������
	// @param message fix��Ϣ
	// @param out_strSeat ȡ�õ�ϯλ
	// @param out_strAccount ȡ�õ��˺�
	// @param out_strMarket_branchid ȡ�õ�Ӫҵ������
	// @return
	// 0 -- �ɹ�
	// -1 -- ʧ��
	int GetNopartyIds(const FIX::FieldMap& message,
		std::string& out_strSeat,
		std::string& out_strAccount, 
		std::string& out_strMarket_branchid);

	// ȡ�ֶ�����
	// @param message fix��Ϣ
	// @param field �ֶ�key
	// @param strValue ����
	int GetStringField(const FIX::Message& message,
		int field, std::string& strValue);

	// set field
	// @param fixReport fix�ر�
	// @param field �ֶ�key
	// @param strValue ����
	int SetField(FIX::Message& fixReport,
		int field, const std::string& strValue);

	// set field
	int SetField(FIX::FieldMap& fixReport,
		int field, const std::string& strValue);
};

#endif