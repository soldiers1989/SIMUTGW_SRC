#ifndef __TASK_PRIORITY_TASK_H__
#define __TASK_PRIORITY_TASK_H__

#include <string>
#include <stdint.h>
#include <functional>

#include "tool_string/Tgw_StringUtil.h"
#include "config/conf_msg.h"

class TaskPriorityQueue;

namespace QueueTask
{
	// �������ȼ�
	enum TaskPriority
	{
		// ί�йҵ�
		Level_Standby_Order = 1,
		// ί�м��
		Level_Check_Order = 2,
		// ί�д���
		Level_Match_Order = 3,
		// ��������
		Level_Cancel_Order = 4
	};
}

/*
���ȼ�����������
*/
class TaskPriorityBase
{
	//
	// member
	//
protected:
	// id
	uint64_t m_uiId;
	// �������ʱʹ�õ�key����Ʊ����+��������
	int m_iKey;
	// ����ʱ��
	time_t m_tOrderTime;
	// �۸�
	uint64_t m_ui64Price;
	// �������
	std::string m_strClordid;
	// ��������
	std::string m_strSide;
	// �������ȼ�
	QueueTask::TaskPriority m_Prio;

	TaskPriorityQueue* m_pTaskqueue;

	// �������ǲ���ί�������
	// false �����ί���������
	// true �����ί���������
	bool m_bIsMatchOrdered;

	//
	// function
	//
public:
	explicit TaskPriorityBase(const uint64_t uiId) :
		m_uiId(uiId), m_iKey(0), m_tOrderTime(0), m_ui64Price(0),
		m_Prio(QueueTask::Level_Standby_Order),
		m_pTaskqueue(nullptr), m_bIsMatchOrdered(false)
	{
	};

	TaskPriorityBase(const uint64_t uiId, const QueueTask::TaskPriority& emPrio) :
		m_uiId(uiId), m_iKey(0), m_tOrderTime(0), m_ui64Price(0),
		m_Prio(emPrio),
		m_pTaskqueue(nullptr), m_bIsMatchOrdered(false)
	{
	};

	TaskPriorityBase(const uint64_t uiId, const int& iKey,
		const QueueTask::TaskPriority& emPrio) :
		m_uiId(uiId), m_iKey(iKey), m_tOrderTime(0), m_ui64Price(0),
		m_Prio(emPrio),
		m_pTaskqueue(nullptr), m_bIsMatchOrdered(false)
	{
	};

	TaskPriorityBase(const uint64_t uiId, const int& iKey,
		const QueueTask::TaskPriority& emPrio,
		const std::string& strSide, const uint64_t ui64Price) :
		m_uiId(uiId), m_iKey(iKey), m_tOrderTime(0), m_ui64Price(ui64Price),
		m_strSide(strSide), m_Prio(emPrio),
		m_pTaskqueue(nullptr), m_bIsMatchOrdered(false)
	{
	};

	virtual ~TaskPriorityBase()
	{
	}

	void SetId(const uint64_t in_uiId)
	{
		m_uiId = in_uiId;
	}

	uint64_t GetId() const
	{
		return m_uiId;
	}

	void SetKey(const int& in_ciKey)
	{
		m_iKey = in_ciKey;
	}

	int GetKey() const
	{
		return m_iKey;
	}

	void SetOrderTime(const time_t& in_tTime)
	{
		m_tOrderTime = in_tTime;
	}

	time_t GetOrderTime() const
	{
		return m_tOrderTime;
	}

	void SetPriority(const QueueTask::TaskPriority& emPrio)
	{
		m_Prio = emPrio;
	}

	QueueTask::TaskPriority GetPriority() const
	{
		return m_Prio;
	}

	void SetSide(const std::string& strSide)
	{
		m_strSide = strSide;
	}

	std::string GetSide() const
	{
		return m_strSide;
	}

	void SetPrice(const uint64_t cui64Price)
	{
		m_ui64Price = cui64Price;
	}

	uint64_t GetPrice() const
	{
		return m_ui64Price;
	}

	std::string GetCliordid() const
	{
		return m_strClordid;
	}

	void SetCliordid(const std::string& in_strCliordid)
	{
		m_strClordid = in_strCliordid;
	}

	void SetTaskQueue(TaskPriorityQueue* in_taskqueue)
	{
		m_pTaskqueue = in_taskqueue;
	}

	void SetIsMatchOrdered(const bool bIsOrdered)
	{
		m_bIsMatchOrdered = bIsOrdered;
	}

	bool GetIsMatchOrdered(void) const
	{
		return m_bIsMatchOrdered;
	}

	/* ����key������
	��������*1000,0000 + ��Ʊ����

	@param const bool in_bIsMatchOrdered : false �����ί���������
	true �����ί���������

	ע�⣬�Գ���ʱҲҪ���ա�true �����ί��������С��ķ�ʽ����key���������Ա�֤�������µ�����������ͬһ�߳���
	*/
	static int GenerateKey(const std::string& in_StockID, const std::string& in_Side, const bool in_bIsMatchOrdered)
	{
		int iKey = 0;

		try
		{
			int iTmp = 0;
			int iRes = 0;
			std::string sTmp;

			std::hash<std::string> hash_fn;

			if (in_bIsMatchOrdered)
			{
				// true �����ί���������
				if (0 == in_Side.compare(simutgw::STEPMSG_SIDE_BUY_B) || 0 == in_Side.compare(simutgw::STEPMSG_SIDE_BUY_1))
				{
					iTmp = 1;
				}
				else if (0 == in_Side.compare(simutgw::STEPMSG_SIDE_SELL_S) || 0 == in_Side.compare(simutgw::STEPMSG_SIDE_SELL_2))
				{
					iTmp = 2;
				}
				else
				{
					// �޷�ת����ֱ��hash
					sTmp = in_Side;
					sTmp += in_StockID;
					iKey = (int)hash_fn(sTmp);
					return iKey;
				}

				iKey = iTmp * 10000000;
				iRes = Tgw_StringUtil::String2Int_atoi(in_StockID, iTmp);
				if (0 == iRes)
				{
					iKey += iTmp;

					return iKey;
				}
			}
			else
			{
				// false �����ί���������
				iRes = Tgw_StringUtil::String2Int_atoi(in_StockID, iTmp);
				if (0 == iRes)
				{
					iKey = iTmp;

					return iKey;
				}

				// �޷�ת����ֱ��hash
				sTmp = in_StockID;
				iKey = (int)hash_fn(sTmp);
			}

		}
		catch (...)
		{
			iKey = 1;
		}

		return iKey;
	}

	virtual int TaskProc(void) = 0;

private:
	TaskPriorityBase();
};

#endif