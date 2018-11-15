#ifndef __OUT_MEMORY_MANAGE_H__
#define __OUT_MEMORY_MANAGE_H__

#include <map>
#include <string>
#include <memory>

#include "order/define_order_msg.h"
#include "cache/MemoryStoreCell.h"

#include "util/TimeDuration.h"

/*
返回消息的储存类
*/
class OutMemoryManage
{
	//
	// Members
	//
protected:
	// 锁对象
	boost::mutex m_mutexlock;

	// 深圳返回消息
	std::map<std::string, MemoryStoreCell<std::shared_ptr<struct simutgw::OrderMessage>>>	m_sz_Buffer;

	// 上海返回消息
	MemoryStoreCell<std::shared_ptr<struct simutgw::OrderMessage>> m_sh_Buffer;
	
	//
	// Functions
	//
public:
	OutMemoryManage( void );

	virtual ~OutMemoryManage( void );

	/*
	加入返回消息

	Param :

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int PushBack( std::shared_ptr<struct simutgw::OrderMessage>& in_msg );

	/*
	获取返回消息
	深圳

	Param :

	Return :
	0 -- 有消息
	1 -- 无消息
	-1 -- 失败
	*/
	int PopFront_sz( const std::string& in_strConnKey, std::shared_ptr<struct simutgw::OrderMessage>& in_msg );

	/*
	获取返回消息
	上海

	Param :

	Return :
	0 -- 有消息
	1 -- 无消息
	-1 -- 失败
	*/
	int PopFront_sh( std::shared_ptr<struct simutgw::OrderMessage>& in_msg );

	/*
	获取队列深度消息
	深圳
	Param :
	Return :
	0 -- 有消息
	1 -- 无消息
	-1 -- 失败
	*/
	size_t GetSize_sz( const std::string& in_strConnKey );

	/*
	获取队列深度消息
	上海

	Param :


	Return :
	size_t
	*/
	size_t GetSize_sh();

protected:
	/*
	加入返回消息
	深圳

	Param :

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int PushBack_sz( const string& in_key, std::shared_ptr<struct simutgw::OrderMessage>& in_msg );

	/*
	加入返回消息
	上海

	Param :

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int PushBack_sh( std::shared_ptr<struct simutgw::OrderMessage>& in_msg );

};

#endif