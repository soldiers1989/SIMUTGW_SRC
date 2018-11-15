#ifndef __RULE_WORD_PROC_SZ_EZSTEP_H__
#define __RULE_WORD_PROC_SZ_EZSTEP_H__

#include "RuleWordProc_Base.h"

/*
成交规则参数配置 中具体单字的处理类
深圳市场EzSTEP报盘
*/
class RuleWordProc_SzEzSTEP
	: public RuleWordProc_Base
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// 交易席位
	std::string m_strSeat;

	// 交易账户
	std::string m_strAccount;

	//
	// Functions
	//
public:
	RuleWordProc_SzEzSTEP(void);

	virtual ~RuleWordProc_SzEzSTEP(void);

	/*
	按成交规则配置 解析字符串

	@return 0 : 替换成功
	@return 1 : 无替换对象
	@return -1 : 替换失败
	*/
	int ResolveRule(rapidjson::Value& in_docRule,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::Message& out_fixReport);

protected:
	/*
	取的席位、账号、营业部代码
	@param msessage 收到的委托消息
	@param strSeat 席位
	@param strAccount 账号
	*/
	int GetSeat_Account(const FIX::Message& message,
		std::string& strSeat,
		std::string& strAccount);

	/*
	按成交规则配置 解析单个JSON元素

	@return 0 : 替换成功
	@return 1 : 无替换对象
	@return -1 : 替换失败
	*/
	int ResolveElement(rapidjson::Value& in_docRule,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		FIX::FieldMap& out_fixReport);

	/*
	按指定规则 替换字符串
	@param const std::string& in_strRuleValue : 规则字符串
	@param std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport : 订单数据
	@param std::string& out_strWord : 转换后的字符

	@return 0 : 替换成功
	@return 1 : 无替换对象
	@return -1 : 替换失败
	*/
	int ReplaceWord(const std::string& in_strRuleValue,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		std::string& out_strWord);

	/*
	按指定规则 替换字符串，转换为四则运算所需的整型数值
	@param const std::string& in_strRuleValue : 规则字符串
	@param std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport : 订单数据
	@param double& out_dIntvalue : 转换后的doubel数值

	@return 0 : 替换成功
	@return 1 : 无替换对象
	@return -1 : 替换失败
	*/
	int ReplaceWordDouble(const std::string& in_strRuleValue,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport,
		double& out_dIntvalue);
};

#endif