#ifndef __AES_MSG_CRYPT_H__
#define __AES_MSG_CRYPT_H__

#include <string>
#include <stdint.h>

namespace EncryptAndDecrypt
{

	static const unsigned int kAesKeySize = 32;
	static const unsigned int kAesIVSize = 16;
	static const unsigned int kEncodingKeySize = 43;
	static const unsigned int kRandEncryptStrLen = 16;
	static const unsigned int kMsgLen = 4;
	static const unsigned int kMaxBase64Size = 1000000000;
	enum  AesMsgCryptErrorCode
	{
		MsgCrypt_OK = 0,
		MsgCrypt_ValidateSignature_Error = -40001,
		MsgCrypt_ParseXml_Error = -40002,
		MsgCrypt_ComputeSignature_Error = -40003,
		MsgCrypt_IllegalAesKey = -40004,
		MsgCrypt_ValidateAppid_Error = -40005,
		MsgCrypt_EncryptAES_Error = -40006,
		MsgCrypt_DecryptAES_Error = -40007,
		MsgCrypt_IllegalBuffer = -40008,
		MsgCrypt_EncodeBase64_Error = -40009,
		MsgCrypt_DecodeBase64_Error = -40010,
		MsgCrypt_GenReturnXml_Error = -40011,
		MsgCrypt_content_Error = -40012,
	};

	class AesMsgCrypt
	{
	public:
		/*
		���캯��
		@param sEncodingAESKey: �ӡ��������õ���ԿEncodingAESKey
		*/
		AesMsgCrypt(const std::string &sEncodingAESKey)
			:m_sEncodingAESKey(sEncodingAESKey)
		{
		}

		virtual ~AesMsgCrypt()
		{
		}

		/*
		��ȡ���ܺ������
		@param sPostData: ���ģ���ӦPOST���������
		@param sMsg: ���ܺ��ԭ�ģ���return����0ʱ��Ч
		@return: �ɹ�0��ʧ�ܷ��ض�Ӧ�Ĵ�����
		*/
		int DecryptMsg(const std::string &sPostData,
			std::string &sMsg);


		/*
		����Ϣ���ܴ��
		@param sReplyMsg:���ںŴ��ظ��û�����Ϣ��xml��ʽ���ַ���
		@param sTimeStamp: ʱ����������Լ����ɣ�Ҳ������URL������timestamp
		@param sNonce: ������������Լ����ɣ�Ҳ������URL������nonce
		@param sEncryptMsg: ���ܺ�Ŀ���ֱ�ӻظ��û������ģ�����msg_signature, timestamp, nonce, encrypt��xml��ʽ���ַ���,
		��return����0ʱ��Ч
		return���ɹ�0��ʧ�ܷ��ض�Ӧ�Ĵ�����
		*/
		int EncryptMsg(const std::string &sReplyMsg,
			std::string &sEncryptMsg);
	private:
		std::string m_sEncodingAESKey;

	private:
		// AES CBC
		int AES_CBCEncrypt(const char * sSource, const uint32_t iSize,
			const char * sKey, unsigned int iKeySize, std::string * poResult);

		int AES_CBCEncrypt(const std::string & objSource,
			const std::string & objKey, std::string * poResult);

		int AES_CBCDecrypt(const char * sSource, const uint32_t iSize,
			const char * sKey, uint32_t iKeySize, std::string * poResult);

		int AES_CBCDecrypt(const std::string & objSource,
			const std::string & objKey, std::string * poResult);

		//base64
		int EncodeBase64(const std::string sSrc, std::string & sTarget);

		int DecodeBase64(const std::string sSrc, std::string & sTarget);

		//genkey
		int GenAesKeyFromEncodingKey(const std::string & sEncodingKey, std::string & sAesKey);

		//signature
		int ComputeSignature(const std::string sToken, const std::string sTimeStamp, const std::string & sNonce,
			const std::string & sMessage, std::string & sSignature);

		int ValidateSignature(const std::string &sMsgSignature, const std::string &sTimeStamp,
			const std::string &sNonce, const std::string & sEncryptMsg);

		//get , set data
		void GenRandStr(std::string & sRandStr, uint32_t len);
	};

}

#endif