#ifndef __RULE_WORD_PROC_SZ_EZSTEP_H__
#define __RULE_WORD_PROC_SZ_EZSTEP_H__

#include "RuleWordProc_Base.h"

/*
�ɽ������������ �о��嵥�ֵĴ�����
�����г�EzSTEP����
*/
class RuleWordProc_SzEzSTEP
	: public RuleWordProc_Base
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// ����ϯλ
	std::string m_strSeat;

	// �����˻�
	std::string m_strAccount;

	//
	// Functions
	//
public:
	RuleWordProc_SzEzSTEP(void);

	virtual ~RuleWordProc_SzEzSTEP(void);

	/*
	���ɽ��������� �����ַ���

	@return 0 : �滻�ɹ�
	@return 1 : ���滻����
	@return -1 : �滻ʧ��
	*/
	int ResolveRule(rapidjson::Value& in_docRule,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& out_fixReport);

protected:
	/*
	ȡ��ϯλ���˺š�Ӫҵ������
	@param msessage �յ���ί����Ϣ
	@param strSeat ϯλ
	@param strAccount �˺�
	*/
	int GetSeat_Account(const FIX::Message& message,
		std::string& strSeat,
		std::string& strAccount);

	/*
	���ɽ��������� ��������JSONԪ��

	@return 0 : �滻�ɹ�
	@return 1 : ���滻����
	@return -1 : �滻ʧ��
	*/
	int ResolveElement(rapidjson::Value& in_docRule,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::FieldMap& out_fixReport);

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