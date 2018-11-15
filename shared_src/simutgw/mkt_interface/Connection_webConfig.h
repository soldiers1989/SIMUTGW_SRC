#ifndef __CONNECTION_WEB_CONFIG_H__
#define __CONNECTION_WEB_CONFIG_H__

#include <stdint.h>
#include <string>
#include <vector>

/*
�ӿ����� ��Web������ص� ��Ϣ
*/
struct Connection_webConfig
{
	std::string strWebLinkid;
	// ����ر���
	std::string strSettleGroupName;
	// �ɽ����ú�ͨ���Ĺ�ϵ
	// Note:ʹ��map�Ƿ������
	std::map<uint64_t, uint64_t> mapLinkRules;

	Connection_webConfig()
		:strWebLinkid(""), strSettleGroupName("")
	{}

	Connection_webConfig(const std::string& cstrWebLinkId, const std::string& cstrSettleGroupName)
		:strWebLinkid(cstrWebLinkId), strSettleGroupName(cstrSettleGroupName)
	{}

	Connection_webConfig(const Connection_webConfig& orig)
	{
		Copy(orig);
	}

	Connection_webConfig& operator =(const Connection_webConfig& orig)
	{
		Copy(orig);
		return *this;
	}

	void Copy(const Connection_webConfig& orig)
	{
		strWebLinkid = orig.strWebLinkid;
		strSettleGroupName = orig.strSettleGroupName;
		mapLinkRules.clear();
		mapLinkRules.insert( orig.mapLinkRules.begin(), orig.mapLinkRules.end());
	}

	/*
	�����WebLinkid�Ƿ�ͱ�����ͬ

	@return true : ��ͬ
	@return false : ����ͬ
	*/
	bool HasWebLinkid(const std::string& cstrWebLinkId)
	{
		int iRes = strWebLinkid.compare(cstrWebLinkId);
		if (0 == iRes)
		{
			return true;
		}
		else
		{
			return false;
		}		
	}
};

#endif