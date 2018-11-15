#include "TaskPriorityQueue.h"

#include "config/conf_msg.h"
#include "tool_string/sof_string.h"
#include "util/EzLog.h"

const uint64_t TaskPriorityQueue::MAX_QUEUE_SIZE = (uint64_t)(std::numeric_limits<uint64_t>::max)() - 10;

/*
插入任务

Param :
uint64_t& out_CurrentTaskNum : 当前此线程已有任务总数
const bool in_bIsMatchOrdered : false 需插入委托无序队列
true 需插入委托有序队列

Return :
0 -- 插入成功
2 -- 数量已满，不允许插入
-1 -- 插入失败
*/
int TaskPriorityQueue::AddTask(T_PTR_TaskPriorityBase& in_ptrTask, uint64_t& out_CurrentTaskNum, const bool in_bIsMatchOrdered)
{
	static const std::string ftag("TaskPriorityQueue::AddTask() ");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	// 检查当前任务数是否已超标
	if (MAX_QUEUE_SIZE <= m_uiTaskNums)
	{
		// 有相同的任务，但是数量已满，不允许插入
		// 避免数量太大，越界

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
		// 委托挂单		
		if (in_bIsMatchOrdered)
		{
			// true 需插入有序队列
			it_p3_order_que = m_p3_order_que.find(in_ptrTask->GetKey());
			if (m_p3_order_que.end() == it_p3_order_que)
			{
				// 无该队列
				std::pair<T_MAP_INT_DEQUE::iterator, bool> ret =
					m_p3_order_que.insert(
					pair<int, std::deque<T_PTR_TaskPriorityBase>>
					(in_ptrTask->GetKey(), std::deque<T_PTR_TaskPriorityBase>()));

				if (!ret.second)
				{
					// 创建队列失败
					std::string strDebug("insert deque failed, queueid=");
					std::string strTrans;
					strDebug += sof_string::itostr(m_uiTaskQueId, strTrans);
					EzLog::e(ftag, strDebug);
				}
				else
				{
					// 直接push_back插入
					ret.first->second.push_back(in_ptrTask);
				}

			}
			else
			{
				// 有队列
				// 折半插入
				Deque_Insert(it_p3_order_que->second, in_ptrTask);
			}

		}
		else
		{
			// false 需插入无序队列
			m_p1_que.push_back(in_ptrTask);
		}
		break;


	case QueueTask::Level_Check_Order:
		// 委托检查
		m_p2_que.push_back(in_ptrTask);

		break;

	case QueueTask::Level_Match_Order:
		// 委托处理
		if (in_bIsMatchOrdered)
		{
			// true 需插入有序队列
			it_p3_order_que = m_p3_order_que.find(in_ptrTask->GetKey());
			if (m_p3_order_que.end() == it_p3_order_que)
			{
				// 无该队列
				std::pair<T_MAP_INT_DEQUE::iterator, bool> ret =
					m_p3_order_que.insert(
					pair<int, std::deque<T_PTR_TaskPriorityBase>>
					(in_ptrTask->GetKey(), std::deque<T_PTR_TaskPriorityBase>()));

				if (!ret.second)
				{
					// 创建队列失败
					std::string strDebug("insert deque failed, queueid=");
					std::string strTrans;
					strDebug += sof_string::itostr(m_uiTaskQueId, strTrans);
					EzLog::e(ftag, strDebug);
				}
				else
				{
					// 直接push_back插入
					ret.first->second.push_back(in_ptrTask);
				}
			}
			else
			{
				// 有队列
				// 折半插入
				Deque_Insert(it_p3_order_que->second, in_ptrTask);
			}

		}
		else
		{
			// false 需插入无序队列
			m_p3_unorder_que.push_back(in_ptrTask);

		}
		break;

	case QueueTask::Level_Cancel_Order:
		// 撤单处理
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

	// 增加任务数量
	++m_uiTaskNums;

	out_CurrentTaskNum = m_uiTaskNums;

	return 0;
}

/*
获取任务

Return :
0 -- 获取成功
1 -- 无任务
-1 -- 失败
*/
int TaskPriorityQueue::GetTask(T_PTR_TaskPriorityBase& out_ptrTask)
{
	static const string ftag("TaskPriorityQueue::GetTask() ");

	boost::unique_lock<boost::mutex> Locker(m_mutexlock);

	// reset.
	out_ptrTask = nullptr;

	// 按优先级Get Queues
	if (0 != m_p4_que.size())
	{
		// 撤单处理
		out_ptrTask = m_p4_que.front();
		m_p4_que.pop_front();
	}
	else if (0 != m_p3_unorder_que.size())
	{
		// 委托处理
		// 无序queue
		out_ptrTask = m_p3_unorder_que.front();
		m_p3_unorder_que.pop_front();
	}
	else if (0 != m_p3_order_que.size())
	{
		// 委托处理
		// 有序（如价格）queue
		T_MAP_INT_DEQUE::iterator it = m_p3_order_que.begin();
		for (; m_p3_order_que.end() != it; ++it)
		{
			std::map<int, bool>::iterator it_unmatch = m_map_p3_order_queUnMatch.find(it->first);
			if (m_map_p3_order_queUnMatch.end() == it_unmatch)
			{
				// 没找到，非挂单
				if (0 != it->second.size())
				{
					// 队列有委托
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
				// 找到，是挂单（挂单跟踪map不存在非挂单任务）

				BOOST_LOG_SEV(m_scl, trivial::trace) << ftag << "order hanged, key=" << it->first;
				continue;
			}
		}

		if (m_p3_order_que.end() == it)
		{
			// 直到队列结束仍没有找到合适任务，已遍历整个p3_order_que，都是挂单，清理挂单跟踪表，让下次循环重新遍历
			m_map_p3_order_queUnMatch.clear();

			if (nullptr != out_ptrTask)
			{
				// 遍历完结果还拿出data，严重错误
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
			// 委托挂单，接受由委托无序queue的挂单
			out_ptrTask = m_p1_que.front();
			m_p1_que.pop_front();
		}
		else
		{
			// 还无任务
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
		// 有错误或问题
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
删除任务(撤单时使用)

0 -- 有该任务，且删除成功
1 -- 无该任务
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
		// 委托处理 有序（如价格）queue
		sFoundQuename = "p3_order_que";

		T_MAP_INT_DEQUE::iterator it_p3_order_que = m_p3_order_que.find(in_iKey);
		if (m_p3_order_que.end() != it_p3_order_que)
		{
			// 有该key
			iRes = DeleteOrderByClordId(it_p3_order_que->second, in_strClordid, out_ptrTask);
			if (0 == iRes)
			{
				// 删除
				bHave = true;
			}
		}
		else
		{
			/*
			暂时无撤单时key不完整的情况
			*/
		}
	}

	if (!bHave && 0 != m_p3_unorder_que.size())
	{
		// 委托处理 无序queue
		iRes = DeleteOrderByClordId(m_p3_unorder_que, in_strClordid, out_ptrTask);
		if (0 == iRes)
		{
			// 删除
			bHave = true;
		}
	}

	if (!bHave && 0 != m_p2_que.size())
	{
		// 委托检查
		iRes = DeleteOrderByClordId(m_p2_que, in_strClordid, out_ptrTask);
		if (0 == iRes)
		{
			// 删除
			bHave = true;
		}
	}

	if (!bHave && 0 != m_p1_que.size())
	{
		// 委托挂单，接受由委托无序queue的挂单
		iRes = DeleteOrderByClordId(m_p1_que, in_strClordid, out_ptrTask);
		if (0 == iRes)
		{
			// 删除
			bHave = true;
		}
	}

	if (!bHave)
	{
		// 所以队列都没有找到
		return 1;
	}

	// 当前队列任务总数
	--m_uiTaskNums;

	// 找到
	if (nullptr == out_ptrTask)
	{
		// 指针意外是空，严重问题
		string sdebug(sFoundQuename);
		sdebug += " found but nullptr clordid=";
		sdebug += in_strClordid;
		EzLog::e(ftag, sdebug);
		return 1;
	}

	return 0;
}

/*
获取当前任务情况

Return :
0 -- 成功
-1 -- 失败
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
插入到有序队列

@param deque& container : 含任务的容器
@param T_PTR_TaskPriorityBase& out_ptrTask : 要插入的任务

@return :
0 -- 插入成功
-1 -- 失败
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
按价格、时间折半插入有序队列
升序

@param deque& container : 含任务的容器
@param T_PTR_TaskPriorityBase& in_ptrTask : 要插入的任务

@return :
0 -- 插入成功
-1 -- 失败
*/
int TaskPriorityQueue::Deque_Price_Time_Binary_Insert_Inc(
	std::deque<T_PTR_TaskPriorityBase>& container,
	T_PTR_TaskPriorityBase& in_ptrTask)
{
	static const string ftag("TaskPriorityQueue::Deque_Price_Time_Binary_Insert_Inc() ");

	if (0 == container.size())
	{
		// 队列无元素
		container.push_back(in_ptrTask);

		return 0;
	}

	const int64_t ciLen = container.size();
	int64_t iLow = 0;
	int64_t iHigh = ciLen - 1;

	int64_t iMid = 0;
	uint64_t ui64Price = in_ptrTask->GetPrice();

	// 按价格折半查找位置
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
			// 找到的位置价格相同
			if (tTime <= container[iMid]->GetOrderTime())
			{
				// mid位置插入
				std::deque<T_PTR_TaskPriorityBase>::iterator it = container.begin() + iMid;
				container.insert(it, in_ptrTask);

				break;
			}
			else
			{
				// 大于当前时间，要向下移动				
				if (ciLen <= iMid + 1)
				{
					// 为了防止队列仅有一个元素的情况
					// 队尾
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
			// 大于当前值，升序，要在当前位置+1插入进序列
			if (iMid + 1 >= ciLen)
			{
				// 为了防止队列仅有一个元素的情况
				// 队尾
				container.push_back(in_ptrTask);
			}
			else
			{
				// mid+1位置插入
				std::deque<T_PTR_TaskPriorityBase>::iterator it = container.begin() + iMid + 1;
				container.insert(it, in_ptrTask);
			}

			break;
		}
		else
		{
			// 小于当前值，升序，要在当前位置插入进序列
			if (0 >= iMid)
			{
				// 为了防止队列仅有一个元素的情况
				// 队首
				container.push_front(in_ptrTask);
			}
			else
			{
				// mid位置插入
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
按价格、时间折半插入有序队列
降序

@param deque& container : 含任务的容器
@param T_PTR_TaskPriorityBase& in_ptrTask : 要插入的任务

@return :
0 -- 插入成功
-1 -- 失败
*/
int TaskPriorityQueue::Deque_Price_Time_Binary_Insert_Desc(
	std::deque<T_PTR_TaskPriorityBase>& container,
	T_PTR_TaskPriorityBase& in_ptrTask)
{
	static const string ftag("TaskPriorityQueue::Deque_Price_Time_Binary_Insert_Desc() ");

	if (0 == container.size())
	{
		// 队列无元素
		container.push_back(in_ptrTask);

		return 0;
	}

	const int64_t ciLen = container.size();
	int64_t iLow = 0;
	int64_t iHigh = ciLen - 1;

	int64_t iMid = 0;
	uint64_t ui64Price = in_ptrTask->GetPrice();

	// 按价格折半查找位置
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
			// 找到的位置价格相同
			if (tTime <= container[iMid]->GetOrderTime())
			{
				// 小于当前时间，在当前位置插入
				std::deque<T_PTR_TaskPriorityBase>::iterator it = container.begin() + iMid;
				container.insert(it, in_ptrTask);

				break;
			}
			else
			{
				// 大于当前时间，要向下移动				
				if (ciLen <= iMid + 1)
				{
					// 为了防止队列仅有一个元素的情况
					// 队尾
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
			// 大于当前值，降序，要在当前位置插入进序列
			if (0 >= iMid)
			{
				// 为了防止队列仅有一个元素的情况
				// 队首
				container.push_front(in_ptrTask);
			}
			else
			{
				// mid位置插入
				std::deque<T_PTR_TaskPriorityBase>::iterator it = container.begin() + iMid;
				container.insert(it, in_ptrTask);
			}
			break;
		}
		else
		{
			// (ui64Price < container[iMid]->GetPrice())
			// 小于当前值，降序，要在当前位置的下一位插入进序列
			if (ciLen <= iMid + 1)
			{
				// 为了防止队列仅有一个元素的情况
				// 队尾
				container.push_back(in_ptrTask);
			}
			else
			{
				// mid位置插入
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
打印队列
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