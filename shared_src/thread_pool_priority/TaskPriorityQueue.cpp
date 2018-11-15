#include "TaskPriorityQueue.h"

#include "config/conf_msg.h"
#include "tool_string/sof_string.h"
#include "util/EzLog.h"

const uint64_t TaskPriorityQueue::MAX_QUEUE_SIZE = (uint64_t)(std::numeric_limits<uint64_t>::max)() - 10;

/*
��������

Param :
uint64_t& out_CurrentTaskNum : ��ǰ���߳�������������
const bool in_bIsMatchOrdered : false �����ί���������
true �����ί���������

Return :
0 -- ����ɹ�
2 -- �������������������
-1 -- ����ʧ��
*/
int TaskPriorityQueue::AddTask(T_PTR_TaskPriorityBase& in_ptrTask, uint64_t& out_CurrentTaskNum, const bool in_bIsMatchOrdered)
{
	static const std::string ftag("TaskPriorityQueue::AddTask() ");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	// ��鵱ǰ�������Ƿ��ѳ���
	if (MAX_QUEUE_SIZE <= m_uiTaskNums)
	{
		// ����ͬ�����񣬵����������������������
		// ��������̫��Խ��

		out_CurrentTaskNum = m_uiTaskNums;

		BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << "queueid="<< m_uiTaskQueId << " reach MAX_QUEUE_SIZE";

		return 2;
	}

	if (nullptr == in_ptrTask)
	{
		BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "nullptr task";
		return -1;
	}

	T_MAP_INT_DEQUE::iterator it_p3_order_que;

	switch (in_ptrTask->GetPriority())
	{
	case QueueTask::Level_Standby_Order:
		// ί�йҵ�		
		if (in_bIsMatchOrdered)
		{
			// true ������������
			it_p3_order_que = m_p3_order_que.find(in_ptrTask->GetKey());
			if (m_p3_order_que.end() == it_p3_order_que)
			{
				// �޸ö���
				std::pair<T_MAP_INT_DEQUE::iterator, bool> ret =
					m_p3_order_que.insert(
					pair<int, std::deque<T_PTR_TaskPriorityBase>>
					(in_ptrTask->GetKey(), std::deque<T_PTR_TaskPriorityBase>()));

				if (!ret.second)
				{
					// ��������ʧ��
					std::string strDebug("insert deque failed, queueid=");
					std::string strTrans;
					strDebug += sof_string::itostr(m_uiTaskQueId, strTrans);
					EzLog::e(ftag, strDebug);
				}
				else
				{
					// ֱ��push_back����
					ret.first->second.push_back(in_ptrTask);
				}

			}
			else
			{
				// �ж���
				// �۰����
				Deque_Insert(it_p3_order_que->second, in_ptrTask);
			}

		}
		else
		{
			// false ������������
			m_p1_que.push_back(in_ptrTask);
		}
		break;


	case QueueTask::Level_Check_Order:
		// ί�м��
		m_p2_que.push_back(in_ptrTask);

		break;

	case QueueTask::Level_Match_Order:
		// ί�д���
		if (in_bIsMatchOrdered)
		{
			// true ������������
			it_p3_order_que = m_p3_order_que.find(in_ptrTask->GetKey());
			if (m_p3_order_que.end() == it_p3_order_que)
			{
				// �޸ö���
				std::pair<T_MAP_INT_DEQUE::iterator, bool> ret =
					m_p3_order_que.insert(
					pair<int, std::deque<T_PTR_TaskPriorityBase>>
					(in_ptrTask->GetKey(), std::deque<T_PTR_TaskPriorityBase>()));

				if (!ret.second)
				{
					// ��������ʧ��
					std::string strDebug("insert deque failed, queueid=");
					std::string strTrans;
					strDebug += sof_string::itostr(m_uiTaskQueId, strTrans);
					EzLog::e(ftag, strDebug);
				}
				else
				{
					// ֱ��push_back����
					ret.first->second.push_back(in_ptrTask);
				}
			}
			else
			{
				// �ж���
				// �۰����
				Deque_Insert(it_p3_order_que->second, in_ptrTask);
			}

		}
		else
		{
			// false ������������
			m_p3_unorder_que.push_back(in_ptrTask);

		}
		break;

	case QueueTask::Level_Cancel_Order:
		// ��������
		m_p4_que.push_back(in_ptrTask);

		break;

	default:
		string sitoa;
		string sdebug("task id=");
		sdebug += in_ptrTask->GetCliordid();
		sdebug += ", not support priority=";
		sdebug += sof_string::itostr(in_ptrTask->GetPriority(), sitoa);

		EzLog::e(ftag, sdebug);
		return -1;
	}

	// ������������
	++m_uiTaskNums;

	out_CurrentTaskNum = m_uiTaskNums;

	return 0;
}

/*
��ȡ����

Return :
0 -- ��ȡ�ɹ�
1 -- ������
-1 -- ʧ��
*/
int TaskPriorityQueue::GetTask(T_PTR_TaskPriorityBase& out_ptrTask)
{
	static const string ftag("TaskPriorityQueue::GetTask() ");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	// reset.
	out_ptrTask = nullptr;

	// �����ȼ�Get Queues
	if (0 != m_p4_que.size())
	{
		// ��������
		out_ptrTask = m_p4_que.front();
		m_p4_que.pop_front();
	}
	else if (0 != m_p3_unorder_que.size())
	{
		// ί�д���
		// ����queue
		out_ptrTask = m_p3_unorder_que.front();
		m_p3_unorder_que.pop_front();
	}
	else if (0 != m_p3_order_que.size())
	{
		// ί�д���
		// ������۸�queue
		T_MAP_INT_DEQUE::iterator it = m_p3_order_que.begin();
		for (; m_p3_order_que.end() != it; ++it)
		{
			std::map<int, bool>::iterator it_unmatch = m_map_p3_order_queUnMatch.find(it->first);
			if (m_map_p3_order_queUnMatch.end() == it_unmatch)
			{
				// û�ҵ����ǹҵ�
				if (0 != it->second.size())
				{
					// ������ί��
					out_ptrTask = it->second.front();
					it->second.pop_front();

					break;
				}
				else
				{
					//
					continue;
				}
			}
			else
			{
				// �ҵ����ǹҵ����ҵ�����map�����ڷǹҵ�����

				BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "order hanged, key=" << it->first;
				continue;
			}
		}

		if (m_p3_order_que.end() == it)
		{
			// ֱ�����н�����û���ҵ����������ѱ�������p3_order_que�����ǹҵ�������ҵ����ٱ����´�ѭ�����±���
			m_map_p3_order_queUnMatch.clear();

			if (nullptr != out_ptrTask)
			{
				// �����������ó�data�����ش���
				BOOST_LOG_SEV(m_scl, trivial::error) << ftag << "p3_order_que end(), but have task id=" << out_ptrTask->GetCliordid();
			}
		}

	}
	else
	{
		// do nothing, just go by.
	}

	if (nullptr == out_ptrTask)
	{
		if (0 != m_p2_que.size())
		{
			out_ptrTask = m_p2_que.front();
			m_p2_que.pop_front();
		}
		else if (0 != m_p1_que.size())
		{
			// ί�йҵ���������ί������queue�Ĺҵ�
			out_ptrTask = m_p1_que.front();
			m_p1_que.pop_front();
		}
		else
		{
			// ��������
			if (0 != m_uiTaskNums)
			{
				string strTran;
				string sdebug("queid=");
				sdebug += sof_string::itostr(m_uiTaskQueId, strTran);
				sdebug += "task empty";
				EzLog::e(ftag, sdebug);
			}

			return 1;
		}

	}
	else
	{
		// do nothing, just go by.
	}

	if (0 == m_uiTaskNums)
	{
		// �д��������
		string strTran;
		string strDebug("poped, but statics empty, queue id=");
		strDebug += sof_string::itostr(m_uiTaskQueId, strTran);
		EzLog::e(ftag, strDebug);
		return 0;
	}

	--m_uiTaskNums;

	return 0;
}

/*
ɾ������(����ʱʹ��)

0 -- �и�������ɾ���ɹ�
1 -- �޸�����
*/
int TaskPriorityQueue::DeleteIfHaveTask(const int& in_iKey, const std::string& in_strClordid,
	T_PTR_TaskPriorityBase& out_ptrTask)
{
	static const string ftag("TaskPriorityQueue::DeleteIfHaveTask() ");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	bool bHave = false;
	int iRes = 1;

	string sFoundQuename;

	if (0 != m_p3_order_que.size())
	{
		// ί�д��� ������۸�queue
		sFoundQuename = "p3_order_que";

		T_MAP_INT_DEQUE::iterator it_p3_order_que = m_p3_order_que.find(in_iKey);
		if (m_p3_order_que.end() != it_p3_order_que)
		{
			// �и�key
			iRes = DeleteOrderByClordId(it_p3_order_que->second, in_strClordid, out_ptrTask);
			if (0 == iRes)
			{
				// ɾ��
				bHave = true;
			}
		}
		else
		{
			/*
			��ʱ�޳���ʱkey�����������
			*/
		}
	}

	if (!bHave && 0 != m_p3_unorder_que.size())
	{
		// ί�д��� ����queue
		iRes = DeleteOrderByClordId(m_p3_unorder_que, in_strClordid, out_ptrTask);
		if (0 == iRes)
		{
			// ɾ��
			bHave = true;
		}
	}

	if (!bHave && 0 != m_p2_que.size())
	{
		// ί�м��
		iRes = DeleteOrderByClordId(m_p2_que, in_strClordid, out_ptrTask);
		if (0 == iRes)
		{
			// ɾ��
			bHave = true;
		}
	}

	if (!bHave && 0 != m_p1_que.size())
	{
		// ί�йҵ���������ί������queue�Ĺҵ�
		iRes = DeleteOrderByClordId(m_p1_que, in_strClordid, out_ptrTask);
		if (0 == iRes)
		{
			// ɾ��
			bHave = true;
		}
	}

	if (!bHave)
	{
		// ���Զ��ж�û���ҵ�
		return 1;
	}

	// ��ǰ������������
	--m_uiTaskNums;

	// �ҵ�
	if (nullptr == out_ptrTask)
	{
		// ָ�������ǿգ���������
		string sdebug(sFoundQuename);
		sdebug += " found but nullptr clordid=";
		sdebug += in_strClordid;
		EzLog::e(ftag, sdebug);
		return 1;
	}

	return 0;
}

/*
��ȡ��ǰ�������

Return :
0 -- �ɹ�
-1 -- ʧ��
*/
int TaskPriorityQueue::GetTaskAssignInfo(string& out_strTaskInfo)
{
	static const string ftag("TaskPriorityQueue::GetTaskAssignInfo() ");

	string strTran;

	out_strTaskInfo = "Queue[";
	out_strTaskInfo += sof_string::itostr(m_uiTaskQueId, strTran);
	out_strTaskInfo += "],size[";
	out_strTaskInfo += sof_string::itostr(QueueSize(), strTran);
	out_strTaskInfo += "],";

	out_strTaskInfo += "m_p4_que={size=";
	out_strTaskInfo += sof_string::itostr(m_p4_que.size(), strTran);
	out_strTaskInfo += "},m_p3_unorder_que={size=";
	out_strTaskInfo += sof_string::itostr(m_p3_unorder_que.size(), strTran);
	out_strTaskInfo += "},m_p2_que={size=";
	out_strTaskInfo += sof_string::itostr(m_p2_que.size(), strTran);
	out_strTaskInfo += "},m_p1_que={size=";
	out_strTaskInfo += sof_string::itostr(m_p1_que.size(), strTran);
	out_strTaskInfo += "}\nm_p3_order_que={";

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);
	T_MAP_INT_DEQUE::iterator itMap = m_p3_order_que.begin();
	for (; m_p3_order_que.end() != itMap; ++itMap)
	{
		out_strTaskInfo += "[key=";
		out_strTaskInfo += sof_string::itostr(itMap->first, strTran);
		out_strTaskInfo += ",size=";
		out_strTaskInfo += sof_string::itostr(itMap->second.size(), strTran);
		std::deque<T_PTR_TaskPriorityBase>::iterator it = itMap->second.begin();
		for (; itMap->second.end() != it; ++it)
		{
			out_strTaskInfo += ",price=";
			out_strTaskInfo += sof_string::itostr((*it)->GetPrice(), strTran);
			out_strTaskInfo += ",time=";
			out_strTaskInfo += sof_string::itostr((*it)->GetOrderTime(), strTran);
		}
		out_strTaskInfo += "]";
	}
	out_strTaskInfo += "}\n";

	return 0;
}

/*
���뵽�������

@param deque& container : �����������
@param T_PTR_TaskPriorityBase& out_ptrTask : Ҫ���������

@return :
0 -- ����ɹ�
-1 -- ʧ��
*/
int TaskPriorityQueue::Deque_Insert(
	std::deque<T_PTR_TaskPriorityBase>& container,
	T_PTR_TaskPriorityBase& in_ptrTask)
{
	static const string ftag("TaskPriorityQueue::Deque_Insert() ");

	std::string strSide(in_ptrTask->GetSide());

	if (0 == simutgw::STEPMSG_SIDE_BUY_B.compare(strSide) || 0 == simutgw::STEPMSG_SIDE_BUY_1.compare(strSide))
	{
		Deque_Price_Time_Binary_Insert_Desc(container, in_ptrTask);
	}
	else if (0 == simutgw::STEPMSG_SIDE_SELL_S.compare(strSide) || 0 == simutgw::STEPMSG_SIDE_SELL_2.compare(strSide))
	{
		Deque_Price_Time_Binary_Insert_Inc(container, in_ptrTask);
	}
	else
	{
		Deque_Price_Time_Binary_Insert_Desc(container, in_ptrTask);

		std::string strDebug("side error,side=");
		strDebug += in_ptrTask->GetSide();
		strDebug += ", cliordid=";
		strDebug += in_ptrTask->GetCliordid();

		EzLog::e(ftag, strDebug);
	}

	return 0;
}

/*
���۸�ʱ���۰�����������
����

@param deque& container : �����������
@param T_PTR_TaskPriorityBase& in_ptrTask : Ҫ���������

@return :
0 -- ����ɹ�
-1 -- ʧ��
*/
int TaskPriorityQueue::Deque_Price_Time_Binary_Insert_Inc(
	std::deque<T_PTR_TaskPriorityBase>& container,
	T_PTR_TaskPriorityBase& in_ptrTask)
{
	static const string ftag("TaskPriorityQueue::Deque_Price_Time_Binary_Insert_Inc() ");

	if (0 == container.size())
	{
		// ������Ԫ��
		container.push_back(in_ptrTask);

		return 0;
	}

	const int64_t ciLen = container.size();
	int64_t iLow = 0;
	int64_t iHigh = ciLen - 1;

	int64_t iMid = 0;
	uint64_t ui64Price = in_ptrTask->GetPrice();

	// ���۸��۰����λ��
	while (iLow <= iHigh)
	{
		iMid = (iLow + iHigh) / 2;

		if (ui64Price >= container[iMid]->GetPrice())
		{
			iLow = iMid + 1;
		}
		else
		{
			iHigh = iMid - 1;
		}
	}

	time_t tTime = in_ptrTask->GetOrderTime();
	while (0 <= iMid && ciLen >= iMid)
	{
		if (ui64Price == container[iMid]->GetPrice())
		{
			// �ҵ���λ�ü۸���ͬ
			if (tTime <= container[iMid]->GetOrderTime())
			{
				// midλ�ò���
				std::deque<T_PTR_TaskPriorityBase>::iterator it = container.begin() + iMid;
				container.insert(it, in_ptrTask);

				break;
			}
			else
			{
				// ���ڵ�ǰʱ�䣬Ҫ�����ƶ�				
				if (ciLen <= iMid + 1)
				{
					// Ϊ�˷�ֹ���н���һ��Ԫ�ص����
					// ��β
					container.push_back(in_ptrTask);
					break;
				}
				else
				{
					++iMid;
					continue;
				}
			}
		}
		else if (ui64Price > container[iMid]->GetPrice())
		{
			// ���ڵ�ǰֵ������Ҫ�ڵ�ǰλ��+1���������
			if (iMid + 1 >= ciLen)
			{
				// Ϊ�˷�ֹ���н���һ��Ԫ�ص����
				// ��β
				container.push_back(in_ptrTask);
			}
			else
			{
				// mid+1λ�ò���
				std::deque<T_PTR_TaskPriorityBase>::iterator it = container.begin() + iMid + 1;
				container.insert(it, in_ptrTask);
			}

			break;
		}
		else
		{
			// С�ڵ�ǰֵ������Ҫ�ڵ�ǰλ�ò��������
			if (0 >= iMid)
			{
				// Ϊ�˷�ֹ���н���һ��Ԫ�ص����
				// ����
				container.push_front(in_ptrTask);
			}
			else
			{
				// midλ�ò���
				std::deque<T_PTR_TaskPriorityBase>::iterator it = container.begin() + iMid;
				container.insert(it, in_ptrTask);
			}

			break;
		}

		--iMid;
	}

	return 0;
}

/*
���۸�ʱ���۰�����������
����

@param deque& container : �����������
@param T_PTR_TaskPriorityBase& in_ptrTask : Ҫ���������

@return :
0 -- ����ɹ�
-1 -- ʧ��
*/
int TaskPriorityQueue::Deque_Price_Time_Binary_Insert_Desc(
	std::deque<T_PTR_TaskPriorityBase>& container,
	T_PTR_TaskPriorityBase& in_ptrTask)
{
	static const string ftag("TaskPriorityQueue::Deque_Price_Time_Binary_Insert_Desc() ");

	if (0 == container.size())
	{
		// ������Ԫ��
		container.push_back(in_ptrTask);

		return 0;
	}

	const int64_t ciLen = container.size();
	int64_t iLow = 0;
	int64_t iHigh = ciLen - 1;

	int64_t iMid = 0;
	uint64_t ui64Price = in_ptrTask->GetPrice();

	// ���۸��۰����λ��
	while (iLow <= iHigh)
	{
		iMid = (iLow + iHigh) / 2;

		if (ui64Price <= container[iMid]->GetPrice())
		{
			iLow = iMid + 1;
		}
		else
		{
			iHigh = iMid - 1;
		}
	}

	time_t tTime = in_ptrTask->GetOrderTime();
	while (0 <= iMid && ciLen >= iMid)
	{
		if (ui64Price == container[iMid]->GetPrice())
		{
			// �ҵ���λ�ü۸���ͬ
			if (tTime <= container[iMid]->GetOrderTime())
			{
				// С�ڵ�ǰʱ�䣬�ڵ�ǰλ�ò���
				std::deque<T_PTR_TaskPriorityBase>::iterator it = container.begin() + iMid;
				container.insert(it, in_ptrTask);

				break;
			}
			else
			{
				// ���ڵ�ǰʱ�䣬Ҫ�����ƶ�				
				if (ciLen <= iMid + 1)
				{
					// Ϊ�˷�ֹ���н���һ��Ԫ�ص����
					// ��β
					container.push_back(in_ptrTask);
					break;
				}
				else
				{
					++iMid;
					continue;
				}
			}
		}
		else if (ui64Price > container[iMid]->GetPrice())
		{
			// ���ڵ�ǰֵ������Ҫ�ڵ�ǰλ�ò��������
			if (0 >= iMid)
			{
				// Ϊ�˷�ֹ���н���һ��Ԫ�ص����
				// ����
				container.push_front(in_ptrTask);
			}
			else
			{
				// midλ�ò���
				std::deque<T_PTR_TaskPriorityBase>::iterator it = container.begin() + iMid;
				container.insert(it, in_ptrTask);
			}
			break;
		}
		else
		{
			// (ui64Price < container[iMid]->GetPrice())
			// С�ڵ�ǰֵ������Ҫ�ڵ�ǰλ�õ���һλ���������
			if (ciLen <= iMid + 1)
			{
				// Ϊ�˷�ֹ���н���һ��Ԫ�ص����
				// ��β
				container.push_back(in_ptrTask);
			}
			else
			{
				// midλ�ò���
				std::deque<T_PTR_TaskPriorityBase>::iterator it = container.begin() + iMid + 1;
				container.insert(it, in_ptrTask);
			}
			break;
		}

		--iMid;
	}

	return 0;
}

/*
��ӡ����
*/
void TaskPriorityQueue::PrintQueue(std::deque<T_PTR_TaskPriorityBase>& container)
{
	static const string ftag("TaskPriorityQueue::PrintQueue() ");

	std::string strDebug, strTrans;
	for (std::deque<T_PTR_TaskPriorityBase>::iterator it = container.begin();
		container.end() != it; ++it)
	{
		strDebug += "price=";
		strDebug += sof_string::itostr((*it)->GetPrice(), strTrans);
		strDebug += ",time=";
		strDebug += sof_string::itostr((*it)->GetOrderTime(), strTrans);
		strDebug += "\n";
	}

	EzLog::i(ftag, strDebug);
}