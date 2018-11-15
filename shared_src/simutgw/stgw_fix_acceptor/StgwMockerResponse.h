#ifndef __STGW_MOCKER_RESPONSE_H__
#define __STGW_MOCKER_RESPONSE_H__

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4003)
#include "rapidjson/document.h"
#pragma warning (pop)
#else
#include "rapidjson/document.h"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "simutgw/stgw_fix_acceptor/StgwApplication.h"

/*
ģ��ر��࣬��������json�ļ����ͻر�
*/
class StgwMockerResponse
{
	//
	// member
	//
private:
	// �����еĻر�name
	static const std::string strResponseKeyName;

	//
	// function
	//
public:
	StgwMockerResponse();
	virtual ~StgwMockerResponse();

	/*
	������Ϣ����ر�
	@param message �յ���ί����Ϣ
	@return
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int Response(const FIX::Message& message);

private:
	/*
	����json���ò��ػر�
	@param msessage �յ���ί����Ϣ
	@param resValue json��ʽ�Ļر�����
	@return
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ParseConfigResponse(const FIX::Message& message, rapidjson::Value& resValue);

	/*
	�滻json���ûػر�
	@param msessage �յ���ί����Ϣ
	@param resValue json��ʽ�Ļر�����
	@return
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ReplaceConfig_SendResponse(const FIX::Message& message, rapidjson::Value& resValue);

	/*
	�滻json����
	@param msessage �յ���ί����Ϣ
	@param fixReport �ر���Ϣ
	@param resValue json��ʽ�Ļر�����
	@return
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int ReplaceConfig(const FIX::Message& message,
		FIX::FieldMap& fixReport,
		rapidjson::Value& resValue,
		const std::string& strSeat,
		const std::string& strAccount);

	// set field
	static int SetField(FIX::FieldMap& fixReport,
		int field, const std::string& strValue);

	/*
	ȡ��ϯλ���˺š�Ӫҵ������
	@param msessage �յ���ί����Ϣ
	@param strSeat ϯλ
	@param strAccount �˺�
	*/
	static int GetSeat_Account(const FIX::Message& message,
		std::string& strSeat,
		std::string& strAccount);
};

#endif