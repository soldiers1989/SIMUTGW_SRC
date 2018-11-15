#ifndef __MEMORY_STORE_CELL_H__
#define __MEMORY_STORE_CELL_H__

#include <list>
#include <deque>
#include <string>

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

/*
�����ڴ�洢����
*/
template <typename T>
class MemoryStoreCell
{
	//
	// Members
	//
protected:
	// ������
	boost::mutex m_mutexlock;

	std::deque<T> m_listStore;

	//
	// Functions
	//
public:
	MemoryStoreCell(void)
	{
	}

	virtual ~MemoryStoreCell(void)
	{
	}

	virtual int PushBack(T obj)
	{
		static const std::string ftag("MemoryStoreCell::PushBack()");

		boost::unique_lock<boost::mutex> Locker(m_mutexlock);

		m_listStore.push_back(obj);

		return 0;
	}

	virtual size_t GetSize(void)
	{
		static const std::string ftag("MemoryStoreCell::GetSize()");

		boost::unique_lock<boost::mutex> Locker(m_mutexlock);

		if( m_listStore.empty() )
		{
			return 0;
		}

		return m_listStore.size();
	}

	/*
	Return :
	0 -- �ɹ�ȡ��
	1 -- ������
	*/
	virtual int PopFront(T& obj)
	{
		static const std::string ftag("MemoryStoreCell::PushBack()");

		boost::unique_lock<boost::mutex> Locker(m_mutexlock);

		if( m_listStore.empty() )
		{
			return 1;
		}

		obj = m_listStore.front();
		m_listStore.pop_front();

		return 0;
	}
};

#endif