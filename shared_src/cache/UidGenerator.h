#ifndef __UID_GENERATOR_H__
#define __UID_GENERATOR_H__

#include <stdint.h>

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

/*
UId ������
*/
template<typename T>
class UidGenerator
{
	//
	// member
	//
private:
	// ������
	boost::mutex m_mutexlock;

	T m_Id;

	//
	// function
	//
public:
	UidGenerator( T id = 0 )
		:m_Id( id )
	{
	}

	virtual ~UidGenerator( void )
	{
	}

	// work counter
	T GetId( void )
	{
		boost::unique_lock<boost::mutex> Locker( m_mutexlock );
		return m_Id++;
	}
};

#endif