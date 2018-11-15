#ifndef __RULE_WORD_PROC_SH_DB_H__
#define __RULE_WORD_PROC_SH_DB_H__

#include "RuleWordProc_Base.h"

/*
�ɽ������������ �о��嵥�ֵĴ�����
�Ϻ��г����ݿⱨ��
*/
class RuleWordProc_ShDb
	: public RuleWordProc_Base
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	//
	// Functions
	//
public:
	RuleWordProc_ShDb(void);

	virtual ~RuleWordProc_ShDb(void);

	/*
	���ɽ��������� �����ַ���

	@return 0 : �滻�ɹ�
	@return 1 : ���滻����
	@return -1 : �滻ʧ��
	*/
	int ResolveRule(rapidjson::Value& in_docRule,
		const std::string& in_strRuleKey,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::string& out_strTrans);

protected:
	/*
	��ָ������ �滻�ַ���
	@param const std::string& in_strRuleValue : �����ַ���
	@param std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport : ��������
	@param std::string& out_strWord : ת������ַ�

	@return 0 : �滻�ɹ�
	@return 1 : ���滻����
	@return -1 : �滻ʧ��
	*/
	int ReplaceWord(const std::string& in_strRuleValue,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::string& out_strWord);

	/*
	��ָ������ �滻�ַ�����ת��Ϊ�������������������ֵ
	@param const std::string& in_strRuleValue : �����ַ���
	@param std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport : ��������
	@param double& out_dIntvalue : ת�����doubel��ֵ

	@return 0 : �滻�ɹ�
	@return 1 : ���滻����
	@return -1 : �滻ʧ��
	*/
	int ReplaceWordDouble(const std::string& in_strRuleValue,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		double& out_dIntvalue);
};

#endif