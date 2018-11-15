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
		构造函数
		@param sEncodingAESKey: 加、解密所用的秘钥EncodingAESKey
		*/
		AesMsgCrypt(const std::string &sEncodingAESKey)
			:m_sEncodingAESKey(sEncodingAESKey)
		{
		}

		virtual ~AesMsgCrypt()
		{
		}

		/*
		获取解密后的明文
		@param sPostData: 密文，对应POST请求的数据
		@param sMsg: 解密后的原文，当return返回0时有效
		@return: 成功0，失败返回对应的错误码
		*/
		int DecryptMsg(const std::string &sPostData,
			std::string &sMsg);


		/*
		将消息加密打包
		@param sReplyMsg:公众号待回复用户的消息，xml格式的字符串
		@param sTimeStamp: 时间戳，可以自己生成，也可以用URL参数的timestamp
		@param sNonce: 随机串，可以自己生成，也可以用URL参数的nonce
		@param sEncryptMsg: 加密后的可以直接回复用户的密文，包括msg_signature, timestamp, nonce, encrypt的xml格式的字符串,
		当return返回0时有效
		return：成功0，失败返回对应的错误码
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