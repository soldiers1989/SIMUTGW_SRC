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
����ͳ�ƣ������������ڵ�����������
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
	����ETF��Ϣ

	@return 0 : �ɹ�
	@return 1 : ʧ��
	*/
	int InsertEtf_FromWebControl(rapidjson::Value& docValue, std::string& out_errmsg);

	/*
	�����ݿ��������� ETF��Ϣ

	@return 0 : �ɹ�
	@return -1 : ʧ��
	*/
	int ReloadFromDb(void);

protected:
	/*
	��ETF�����г��ɷֹ�֮������ݴ���ṹ��
	��������Web�����

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int Set_ETF_Info_FromWebControl(const std::string& in_strKey, const std::string& in_strValue,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf, std::string& out_errmsg);

	/*
	Set �ɷֹ���Ϣ
	��������Web�����

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int Set_ETF_Component_FromWebControl(rapidjson::Value& docValue,
	struct simutgw::SzETFComponent& out_cpn, std::string& out_errmsg);

	/*
	ִ�и���sql���

	@return 0 : ���³ɹ�
	@return -1 : ����ʧ��
	*/
	int UpdateOneEtfPackageToDb(const std::string& in_strSecurityID,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf, std::string& out_errmsg);

	/*
	ִ�и���sql���

	@return 0 : ���³ɹ�
	@return -1 : ����ʧ��
	*/
	int Update_EtfInfo(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	ִ�и���sql���

	@return 0 : ���³ɹ�
	@return -1 : ����ʧ��
	*/
	int Update_EtfComponent(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	��ȡ���е� ETF SecurityId

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int GetFromDB_AllEtfStockIds(std::shared_ptr<MySqlCnnC602>& in_mysqlConn,
		std::vector<string>& out_vctEtfStockIds);

	/*
	�����ݿ��ȡһ��ETF�����ȫ������

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int GetFromDB_OneEtfPackage(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSecurityID,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	�����ݿ��ȡһ��ETF�����info

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int GetFromDB_OneEtfInfo(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSecurityID,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	�����ݿ��ȡһ��ETF��������е�Components

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int GetFromDB_OneEtfComponents(std::shared_ptr<MySqlCnnC602> &in_mysqlConn,
		const std::string& in_strSecurityID,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	��ETF�����г��ɷֹ�֮������ݴ���ṹ��
	�����������ݿ�

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int Set_ETF_Info_FromDb(const std::string& in_strKey,
		const map<string, struct MySqlCnnC602_DF::DataInRow>& in_mapRowData,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
	Set �ɷֹ���Ϣ
	�����������ݿ�

	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	int Set_ETF_Component_FromDb(const std::string& in_strKey,
		const map<string, struct MySqlCnnC602_DF::DataInRow>& in_mapRowData,
	struct simutgw::SzETFComponent& out_cpn);
};

#endif