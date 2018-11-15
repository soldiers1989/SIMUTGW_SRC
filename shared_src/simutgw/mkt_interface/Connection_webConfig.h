#ifndef __CONNECTION_WEB_CONFIG_H__
#define __CONNECTION_WEB_CONFIG_H__

#include <stdint.h>
#include <string>
#include <vector>

/*
接口连接 与Web配置相关的 信息
*/
struct Connection_webConfig
{
	std::string strWebLinkid;
	// 清算池别名
	std::string strSettleGroupName;
	// 成交配置和通道的关系
	// Note:使用map是方便查找
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
	输入的WebLinkid是否和本地相同

	@return true : 相同
	@return false : 不相同
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