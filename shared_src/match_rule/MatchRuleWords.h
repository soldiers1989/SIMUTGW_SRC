#ifndef __MATCH_RULE_WORDS_H__
#define __MATCH_RULE_WORDS_H__

#include <string>
#include <memory>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "util/EzLog.h"

#include "order/define_order_msg.h"

/*
�ɽ������������ �о��嵥�ֵĴ�����
�Ϻ��г����ݿⱨ��
*/
namespace simutgw
{
	namespace matchrule
	{
		//
		static const char cc_dollar = '$';
		static const char cc_at = '@';
		static const char cszBlank[] = { " " };
		// �����������
		static const string cszSign_Add("+");
		static const string cszSign_Minus("-");
		static const string cszSign_Times("*");
		static const string cszSign_Division("/");

		//
		// ��ϵͳ�ṩ�������ֶ�
		//
		// �Ϻ�
		// ��¼���
		static const string csz_shdb_REC_NUM("$REC_NUM");
		// ���ڣ���ʽΪYYYYMMDD
		static const string csz_shdb_DATE("$DATE");
		// ʱ�䣬��ʽΪHH:MM:SS
		static const string csz_shdb_TIME_C8("$TIME_C8");
		// ʱ�䣬��ʽΪHHMMSS
		static const string csz_shdb_TIME_C6("$TIME_C6");
		// �ɽ����
		static const string csz_shdb_CJBH("$SH_CJBH");

		//
		// ����
		// ExecID C16 �����������ִ�б�ţ������������ڲ��ظ�
		static const string csz_szstep_ExecID("$EXEC_ID");

		// OrderID C16 ����������Ķ�����ţ��罻���ղ��ظ�
		static const string csz_szstep_OrderID("$ORDER_ID");

		// LocalTimeStamp C21 ����ʱ��� YYYYMMDD-HH:MM:SS.sss(����)
		static const string csz_szstep_LocalTimeStamp("$LOCAL_TIME_STAMP");

		// ����ϯλ
		static const string csz_szstep_seat("$SEAT");
		// �����˻�
		static const string csz_szstep_account("$ACCOUNT");
		
		//
		// ԭ�Ϻ����ݿⱨ���ֶ�
		// reff	��Ա�ڲ������ţ��������걨�����������У�����ɽ��ر��У����ḽ����������Ϊ��ʶ�ֶΣ���̨ϵͳ�������ô˱�Ž��ж�Ӧ����	C10
		static const string csz_shdb_reff("@reff");
		// acc	֤ȯ�˻�	C10
		static const string csz_shdb_acc("@acc");
		// stock	֤ȯ����	C6
		static const string csz_shdb_stock("@stock");
		// bs	�������򣬡�B�����ߡ�b���������룬��S�����ߡ�s����������	C1
		static const string csz_shdb_bs("@bs");
		// price	�걨�۸�������ֶ�С��������ֳ���3λ��3λ֮�����Ϊ0	C8
		static const string csz_shdb_price("@price");
		// qty	�걨����	C8
		static const string csz_shdb_qty("@qty");
		// owflag	�������ͱ�־�����ֶ�ȡֵ��Сд������	C3
		static const string csz_shdb_owflag("@owflag");
		// ordrec	�������	C8
		static const string csz_shdb_ordrec("@ordrec");
		// firmid	B�ɽ����Ա���룬����A��Ͷ����ȡֵ������	C5
		static const string csz_shdb_firmid("@firmid");
		// branchid	Ӫҵ������	C5
		static const string csz_shdb_branchid("@branchid");
        // checkord	У���룬�Ͻ����ڲ�ʹ��	Binary
		static const string csz_shdb_checkord("@checkord");

	}
};

#endif