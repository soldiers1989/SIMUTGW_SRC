#ifndef __ETF_HELPER_H__
#define __ETF_HELPER_H__

#include <memory>

#include "conf_etf_info.h"

/*
	����/�Ϻ�etf����Ϣhelper��
	*/
class ETFHelper
{
	/*
		function
		*/
public:
	ETFHelper();
	virtual ~ETFHelper();

	/*
		����ָ��Ŀ¼��etf�ļ�
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int LoadETF(const std::string& in_strDir);

	/*
		����ָ��Ŀ¼��pcf�ļ�
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int LoadSZPcf(const std::string& in_strDir);

	/*
		����ָ��Ŀ¼���Ϻ�etf�����ļ�
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int LoadSHETF(const std::string& in_strDir);

	/*
		��ѯĳ֧etf����Ϣ
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int Query(const std::string& in_strSecurityID,
		std::shared_ptr<struct simutgw::SzETF>& ptrEtf);

	/*
		��ӡ��ǰ�洢��ETF��Ϣ��֤ȯ����
		*/
	static int Overview();

private:
	/*
		��ȡָ��Ŀ¼���µ�pcf�ļ�����
		Return:
		0 -- �ɹ�
		-1 -- ʧ��
		*/
	static int GetPCFFileNameInDir(const std::string& in_strDir, std::vector<std::string>& out_vecFileName);

	/*
	���ShenZhen pcf�ļ����ƵĺϷ���

	�ļ���������Ϊ�� pcf_<ETF ����>_YYYYMMDD.xml ������ YYYYMMDD Ϊ T�����ڡ� ʾ�����£�
	pcf_159901_20150115.xml
	*/
	static bool Check_SZETF_FileName(const std::string& in_strFileName);

	/*
	��ȡָ��Ŀ¼���µ��Ϻ�ETF�ļ�����
	Return:
	0 -- �ɹ�
	-1 -- ʧ��
	*/
	static int Get_SHETF_FileNameInDir(const std::string& in_strDir, std::vector<std::string>& out_vecFileName);

	/*
		����Ϻ�ETF�ļ����ƵĺϷ���


		*/
	static bool Check_SHETF_FileName(const std::string& in_strFileName);
};

#endif