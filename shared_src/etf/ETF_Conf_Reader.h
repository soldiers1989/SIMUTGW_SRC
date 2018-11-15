#ifndef __ETF_CONF_READER_H__
#define __ETF_CONF_READER_H__

#include <string>
#include <vector>
#include <memory>

#include "conf_etf_info.h"

/*
	etf���ö�ȡ
*/
class ETF_Conf_Reader
{
public:
	ETF_Conf_Reader();
	virtual ~ETF_Conf_Reader();

	/*
		��ȡ����pcf�ļ�
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
	*/
	static int Read_SZ_ETF_Pcf_Conf(const std::string& in_strFileName);

	/*
		��ȡ�Ϻ�pcf�ļ�
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
	*/
	static int Read_SH_ETF_Conf(const std::string& in_strFileName);

private:
	/*
		��������pcf�ļ�
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
	*/
	static int Parse_Pcf_XML_And_Store(const std::string& in_strXMLContent);

	/*
		��pcf�����г��ɷֹ�֮������ݴ���ṹ��
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
	*/
	static int Set_ETF_Info(std::shared_ptr<struct simutgw::SzETF>& ptrEtf,
		const std::string& in_strKey, const std::string& in_strValue);

	/*
		Set�ɷֹ���Ϣ
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
	*/
	static int Set_ETFComponent_Info(struct simutgw::SzETFComponent& cpn,
		const std::string& in_strKey, const std::string& in_strValue);

	/*
	�����Ϻ�ETF�ļ�
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int Parse_SH_ETF_And_Store(const std::string& in_strETFContent);

	/*
	��SH ETF�����г��ɷֹ�֮������ݴ���ṹ��
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int Set_SH_ETF_Info(std::shared_ptr<struct simutgw::SzETF>& ptrEtf,
		const std::string& in_strKey, const std::string& in_strValue);

	/*
	Set SH�ɷֹ���Ϣ
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int Set_SHETFComponent_Info(struct simutgw::SzETFComponent& cpn,
		const std::vector<std::string>& vecComponent);

	/*
		���ETF��Ϣ
	*/
	static void Add_ETF_Info(std::shared_ptr<struct simutgw::SzETF>& ptrEtf);
};

#endif