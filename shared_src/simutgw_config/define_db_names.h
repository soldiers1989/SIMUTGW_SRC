#ifndef __DEFINE_DB_NAMES_H__
#define __DEFINE_DB_NAMES_H__

#include <memory>
#include <stdint.h>  
#include <string>

/*
�������ݿ����
*/
namespace simutgw
{
	//
	// Members
	//
	
	// �Ϻ��ɽ������������
	static const std::string g_DBTable_match_rule_sh("`match_rule_sh`");

	// ���ڳɽ������������
	static const std::string g_DBTable_match_rule_sz("`match_rule_sz`");

	// �Ϻ�������ETF PCF�ļ���Ϣ���ṹ�������ڵ���Ϣ��׼���ο��ĵ�������֤ȯ�����������ļ������ӿڹ淶��Ver1.05��.pdf������IS101 �Ϻ�֤ȯ���������۴��ƽ̨�г������߽ӿڹ��˵����1.38��_20180522.docx��
	static const std::string g_DBTable_etf_info("`etf_info`");
	
	// �Ϻ�������ETF�ɷݹ��б� Components���ṹ�������ڵ���Ϣ��׼���ο��ĵ�������֤ȯ�����������ļ������ӿڹ淶��Ver1.05��.pdf������IS101 �Ϻ�֤ȯ���������۴��ƽ̨�г������߽ӿڹ��˵����1.38��_20180522.docx��
	static const std::string g_DBTable_etf_component("`etf_component`");
};

#endif
