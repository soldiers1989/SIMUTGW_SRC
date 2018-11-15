#ifndef __SH_CONNECTION_DEFINE_H__
#define __SH_CONNECTION_DEFINE_H__

#include <stdint.h>
#include <string>

/*
�Ϻ�odbc�ӿ�������Ϣ
*/
class ShConnection
{
	/*
	member
	*/
private:
	//��¼�Ϻ�ί�ж�ȡ��rec_num
	uint64_t m_ui64Rec_Num;

	//��¼�Ϻ�ȷ�ϵ�����������
	uint64_t m_ui64Teordernum;

	//��¼�Ϻ��ر��ĳɽ����
	uint64_t m_ui64Cjbh;

	// �������
	std::string m_strName;

	// odbc���Ӵ�
	std::string m_strConnection;

	boost::mutex m_mutex_recnum;

	boost::mutex m_mutex_cjhb;

	/*
	functions
	*/
public:
	ShConnection() :m_ui64Rec_Num(0), m_ui64Teordernum(0), m_ui64Cjbh(0){};
	~ShConnection()
	{
		m_ui64Rec_Num = 0;
		m_ui64Teordernum = 0;
		m_ui64Cjbh = 0;
	}

	void SetRecNum(uint64_t ui64Num)
	{ 
		boost::unique_lock<boost::mutex> Locker(m_mutex_recnum);
		m_ui64Rec_Num = ui64Num; 
	}

	uint64_t GetRecNum(){
		boost::unique_lock<boost::mutex> Locker(m_mutex_recnum);
		return m_ui64Rec_Num;
	}

	uint64_t IncRecNum(){ 
		boost::unique_lock<boost::mutex> Locker(m_mutex_recnum);
		return ++m_ui64Rec_Num; 
	}

	void SetTeordNum(uint64_t ui64Num){ m_ui64Teordernum = ui64Num; }
	uint64_t GetTeordNum(){ return m_ui64Teordernum++; }

	void SetCjbh(uint64_t ui64Num){ 
		boost::unique_lock<boost::mutex> Locker(m_mutex_cjhb);
		m_ui64Cjbh = ui64Num;
	}
	uint64_t GetCjbh(){ 
		boost::unique_lock<boost::mutex> Locker(m_mutex_cjhb); 
		return m_ui64Cjbh;
	}
	uint64_t IncCjbh(){
		boost::unique_lock<boost::mutex> Locker(m_mutex_cjhb);
		return ++m_ui64Cjbh;
	}

	std::string GetName(){ return m_strName; }
	void SetName(const std::string& strID){ m_strName = strID; }

	std::string GetConnection(){ return m_strConnection; }
	void SetConnection(const std::string& strConn){ m_strConnection = strConn; }
};

#endif