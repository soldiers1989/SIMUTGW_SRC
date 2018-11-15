#ifndef __OUT_MEMORY_MANAGE_H__
#define __OUT_MEMORY_MANAGE_H__

#include <map>
#include <string>
#include <memory>

#include "order/define_order_msg.h"
#include "cache/MemoryStoreCell.h"

#include "util/TimeDuration.h"

/*
������Ϣ�Ĵ�����
*/
class OutMemoryManage
{
	//
	// Members
	//
protected:
	// ������
	boost::mutex m_mutexlock;

	// ���ڷ�����Ϣ
	std::map<std::string, MemoryStoreCell<std::shared_ptr<struct simutgw::OrderMessage>>>	m_sz_Buffer;

	// �Ϻ�������Ϣ
	MemoryStoreCell<std::shared_ptr<struct simutgw::OrderMessage>> m_sh_Buffer;
	
	//
	// Functions
	//
public:
	OutMemoryManage( void );

	virtual ~OutMemoryManage( void );

	/*
	���뷵����Ϣ

	Param :

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int PushBack( std::shared_ptr<struct simutgw::OrderMessage>& in_msg );

	/*
	��ȡ������Ϣ
	����

	Param :

	Return :
	0 -- ����Ϣ
	1 -- ����Ϣ
	-1 -- ʧ��
	*/
	int PopFront_sz( const std::string& in_strConnKey, std::shared_ptr<struct simutgw::OrderMessage>& in_msg );

	/*
	��ȡ������Ϣ
	�Ϻ�

	Param :

	Return :
	0 -- ����Ϣ
	1 -- ����Ϣ
	-1 -- ʧ��
	*/
	int PopFront_sh( std::shared_ptr<struct simutgw::OrderMessage>& in_msg );

	/*
	��ȡ���������Ϣ
	����
	Param :
	Return :
	0 -- ����Ϣ
	1 -- ����Ϣ
	-1 -- ʧ��
	*/
	size_t GetSize_sz( const std::string& in_strConnKey );

	/*
	��ȡ���������Ϣ
	�Ϻ�

	Param :


	Return :
	size_t
	*/
	size_t GetSize_sh();

protected:
	/*
	���뷵����Ϣ
	����

	Param :

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int PushBack_sz( const string& in_key, std::shared_ptr<struct simutgw::OrderMessage>& in_msg );

	/*
	���뷵����Ϣ
	�Ϻ�

	Param :

	Return :
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int PushBack_sh( std::shared_ptr<struct simutgw::OrderMessage>& in_msg );

};

#endif