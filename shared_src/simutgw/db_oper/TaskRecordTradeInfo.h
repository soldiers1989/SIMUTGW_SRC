#ifndef __TASK_RECORD_TRADE_INFO_H__
#define __TASK_RECORD_TRADE_INFO_H__

#include "thread_pool_base/TaskBase.h"
#include "order/define_order_msg.h"

namespace AsyncDbTask
{
	enum UpdateTaskType
	{
		notype = 0,
		match = 1,
		cancel_succ = 2,
		cancel_fail = 3,
		error = 4
	};
}

/*
	���׺�������ݿ���Ϣ������
*/
class TaskRecordTradeInfo : public TaskBase
{
	//
	// member
	//
	AsyncDbTask::UpdateTaskType m_emType;
	std::shared_ptr<struct simutgw::OrderMessage> m_ptrReport;

	//
	// function
	//
public:
	explicit TaskRecordTradeInfo(unsigned int uiId);
	virtual ~TaskRecordTradeInfo();

	void SetTask(AsyncDbTask::UpdateTaskType& in_emType,
		std::shared_ptr<struct simutgw::OrderMessage>& in_ptrReport)
	{
		m_emType = in_emType;
		m_ptrReport = in_ptrReport;
	}

	virtual int TaskProc(void);

private:
	/*
	�����ݿ���д��ɽ�������
	�ɽ�
	Param :

	Return :
	0 -- д��ɹ�
	��0 -- д��ʧ��
	*/
	int WriteTransInfoInDb();

	/*
	�����ݿ���д��ɽ�������
	�����ɹ�
	Param :

	Return :
	0 -- д��ɹ�
	��0 -- д��ʧ��
	*/
	int WriteTransInfoInDb_CancelSuccess();

	/*
	�����ݿ���д��ɽ�������
	����ʧ��
	Param :

	Return :
	0 -- д��ɹ�
	��0 -- д��ʧ��
	*/
	int WriteTransInfoInDb_CancelFail();

	/*
	�����ݿ���д��ɽ�������
	�ϵ�
	Param :

	Return :
	0 -- д��ɹ�
	��0 -- д��ʧ��
	*/
	int WriteTransInfoInDb_Error();
};

#endif