#ifndef __ETF_CONTAINER_H__
#define __ETF_CONTAINER_H__

#include <string>
#include <map>
#include <mutex>
#include <memory>

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

#include "util/EzLog.h"
#include "etf/conf_etf_info.h"
#include "tool_mysql/MysqlConnectionPool.h"

/*
订单统计，无锁，适用于单个物理连接
*/
class ETFContainer
{
	//
	//	members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// Mutex
	std::mutex m_mutex;

	typedef std::map<std::string, std::shared_ptr<struct simutgw::SzETF>> MAP_ETF_COMP;

	MAP_ETF_COMP m_mapEtfContents;

	//
	//	functions
	//
public:
	ETFContainer(void);

	virtual ~ETFContainer(void);

	/*
	插入ETF信息

	@return 0 : 成功
	@return 1 : 失败
	*/
	int InsertEtf_FromWebControl(rapidjson::Value& docValue, std::string& out_errmsg);

	/*
	从数据库重新载入 ETF信息

	@return 0 : 成功
	@return -1 : 失败
	*/
	int ReloadFromDb(void);

protected:
	/*
	将ETF内容中除成分股之外的内容存入结构体
	内容来自Web管理端

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int Set_ETF_Info_FromWebControl(const std::string& in_strKey, const std::string& in_strValue,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf, std::string& out_errmsg);

	/*
	Set 成分股信息
	内容来自Web管理端

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int Set_ETF_Component_FromWebControl(rapidjson::Value& docValue,
	struct simutgw::SzETFComponent& out_cpn, std::string& out_errmsg);

	/*
	执行更新sql语句

	@return 0 : 更新成功
	@return -1 : 更新失败
	*/
	int UpdateOneEtfPackageToDb(const std::string& in_strSecurityID,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf, std::string& out_errmsg);

	/*
	执行更新sql语句

	@return 0 : 更新成功
	@return -1 : 更新失败
	*/
	int Update_EtfInfo(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	执行更新sql语句

	@return 0 : 更新成功
	@return -1 : 更新失败
	*/
	int Update_EtfComponent(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	获取所有的 ETF SecurityId

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int GetFromDB_AllEtfStockIds(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
		std::vector<string>& out_vctEtfStockIds);

	/*
	从数据库读取一个ETF代码的全部数据

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int GetFromDB_OneEtfPackage(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSecurityID,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	从数据库读取一个ETF代码的info

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int GetFromDB_OneEtfInfo(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSecurityID,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	从数据库读取一个ETF代码的所有的Components

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int GetFromDB_OneEtfComponents(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSecurityID,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	将ETF内容中除成分股之外的内容存入结构体
	内容来自数据库

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int Set_ETF_Info_FromDb(const std::string& in_strKey,
		const map<string, struct MySqlCnnC602_DF::DataInRow>& in_mapRowData,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	Set 成分股信息
	内容来自数据库

	Return:
	0 -- 成功
	-1 -- 失败
	*/
	int Set_ETF_Component_FromDb(const std::string& in_strKey,
		const map<string, struct MySqlCnnC602_DF::DataInRow>& in_mapRowData,
	struct simutgw::SzETFComponent& out_cpn);
};

#endif