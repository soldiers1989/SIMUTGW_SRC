#ifndef __FROZE_VOLUME_H__
#define __FROZE_VOLUME_H__

#include <string>
#include <vector>
#include <stdint.h>

#include "boost/thread/mutex.hpp"

#include "tool_string/sof_string.h"
#include "util/EzLog.h"

/*
	�ɹ�����Ļ��������
	*/
class FrozeVolume
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// ��������ݿ�ͬ���ļ���
	uint64_t m_ui64LatestSynVolume;

	// ʣ��δ����
	uint64_t m_ui64UnFrozeVolume;
	
	// ������
	uint64_t m_ui64ForzeVolume;

	// access mutex
	boost::mutex m_mutex;

	//
	// Functions
	//
public:
	FrozeVolume() : m_scl(keywords::channel = "FrozeVolume"),
		m_ui64LatestSynVolume(0), m_ui64UnFrozeVolume(0), m_ui64ForzeVolume(0)
	{
	}

	FrozeVolume(const uint64_t max) : m_scl(keywords::channel = "FrozeVolume"),
		m_ui64LatestSynVolume(max), m_ui64UnFrozeVolume(max), m_ui64ForzeVolume(0)
	{
	}

	FrozeVolume(const FrozeVolume & src) : m_scl(keywords::channel = "FrozeVolume"),
		m_ui64LatestSynVolume(src.m_ui64LatestSynVolume), m_ui64UnFrozeVolume(src.m_ui64UnFrozeVolume),
		m_ui64ForzeVolume(src.m_ui64ForzeVolume)
	{
	}

	virtual ~FrozeVolume()
	{
	}

	/*
	��ѯʣ�ཻ�׶��Ƿ��㹻

	Param:
	out_maxAvl -- �����������������ʹ�õ�����

	Return:
	0 -- ʣ�ཻ�׶��㹻
	-1 -- ʣ�ཻ�׶��
	*/
	int Query(const uint64_t cnt, uint64_t *out_maxAvl = NULL)
	{
		static const std::string ftag("FrozeVolume::Query() ");

		boost::unique_lock<boost::mutex> Locker(m_mutex);

		// �ж��Ƿ�����
		if (m_ui64UnFrozeVolume < cnt)
		{
			// �������ƶ��
			if (NULL != out_maxAvl)
			{
				// ����������
				*out_maxAvl = m_ui64UnFrozeVolume;
			}

			return -1;
		}
		else
		{
			if (NULL != out_maxAvl)
			{
				// ����������
				*out_maxAvl = cnt;
			}
		}

		return 0;
	}

	/*
	���ύ�׶�
	Param:
	out_maxfrozev -- ���frozev����ʧ�ܣ��ٴζ���ʣ�����������ص�ǰ���������

	Return:
	0 -- δ����������ƶ�ȣ�����ɹ�
	-1 -- ����ʧ��
	*/
	int Froze(const uint64_t frozev, uint64_t *out_maxfrozev = NULL)
	{
		static const std::string ftag("FrozeVolume::Froze() ");

		boost::unique_lock<boost::mutex> Locker(m_mutex);

		// ������Ԥ������
		uint64_t ui64_ForzeVolume_After = m_ui64ForzeVolume + frozev;

		uint64_t ui64ActualFroze = 0;
		// �����Ƿ�������
		if (ui64_ForzeVolume_After < m_ui64ForzeVolume)
		{
			// ���������
			std::string sItoa;
			std::string sDebug("overflow, ForzeVolume=");
			sDebug += sof_string::itostr(m_ui64ForzeVolume, sItoa);
			sDebug += "; forze=";
			sDebug += sof_string::itostr(frozev, sItoa);

			BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << sDebug;
		}

		// �ж��Ƿ�����
		if (m_ui64UnFrozeVolume < frozev)
		{
			// �������ƶ��
			if (NULL != out_maxfrozev)
			{
				// ����������
				*out_maxfrozev = m_ui64UnFrozeVolume;
			}
			ui64ActualFroze = m_ui64UnFrozeVolume;
		}
		else
		{
			if (NULL != out_maxfrozev)
			{
				// ����������
				*out_maxfrozev = frozev;
			}
			ui64ActualFroze = frozev;
		}

		m_ui64ForzeVolume += ui64ActualFroze;
		m_ui64UnFrozeVolume -= ui64ActualFroze;

		return 0;
	}

	/*
	������ύ�׶�

	Return:
	0 -- �������ɹ�
	-1 -- �������ʧ��
	*/
	int Defroze(const uint64_t frozev)
	{
		static const std::string ftag("FrozeVolume::Defroze() ");

		boost::unique_lock<boost::mutex> Locker(m_mutex);

		if (m_ui64ForzeVolume < frozev)
		{
			// ���������ȣ�������
			std::string sItoa;
			std::string sDebug("not enough to defroze, ForzeVolume=");
			sDebug += sof_string::itostr(m_ui64ForzeVolume, sItoa);
			sDebug += "; deforze=";
			sDebug += sof_string::itostr(frozev, sItoa);

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;
		}

		m_ui64ForzeVolume -= frozev;
		m_ui64UnFrozeVolume += frozev;

		return 0;
	}

	/*
	ȷ�Ͻ��׶���Ӷ���Ĳ��ֿ۳���Ӧ���

	Return:
	0 -- ȷ�ϳɹ�
	-1 -- ȷ��ʧ��
	*/
	int Confirm(const uint64_t confirm)
	{
		static const std::string ftag("FrozeVolume::Confirm() ");

		boost::unique_lock<boost::mutex> Locker(m_mutex);

		if (m_ui64ForzeVolume < confirm)
		{
			// ���������ȣ�������
			std::string sItoa;
			std::string sDebug("not enough to confirm, ForzeVolume=");
			sDebug += sof_string::itostr(m_ui64ForzeVolume, sItoa);
			sDebug += "; confirm=";
			sDebug += sof_string::itostr(confirm, sItoa);

			BOOST_LOG_SEV(m_scl, trivial::error) << ftag << sDebug;

			return -1;
		}

		m_ui64ForzeVolume -= confirm;

		return 0;
	}

	/*
		�����ݿ���Ϣͬ�����������ͬ��������δ�������
		*/
	int DBSyn(uint64_t& out_origin, uint64_t& out_update)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutex);

		out_origin = m_ui64LatestSynVolume;
		out_update = m_ui64ForzeVolume + m_ui64UnFrozeVolume;

		m_ui64LatestSynVolume = m_ui64ForzeVolume + m_ui64UnFrozeVolume;

		return 0;
	}

	/*
		��ʼ��
		����ͬ��������δ����������һ�δ����ݿ��ȡʱ����
		*/
	int Init(uint64_t volume)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutex);

		m_ui64LatestSynVolume = volume;
		m_ui64UnFrozeVolume = volume;

		return 0;
	}

	/*
		������������ݿ�ͬ���ļ���
		*/
	int ResetSyn(uint64_t volume, uint64_t* out_volumeOrigin = NULL)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutex);

		if (out_volumeOrigin)
		{
			*out_volumeOrigin = m_ui64LatestSynVolume;
		}

		m_ui64LatestSynVolume = volume;

		return 0;
	}

	/*
		����δ����
		*/
	int Add(uint64_t inc)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutex);

		m_ui64UnFrozeVolume += inc;

		return 0;
	}

	/*
		��ӡ��ǰ��Ϣ
		*/
	int GetVolumeInfo(std::string &strInfo)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutex);

		std::string strTrans;
		strInfo = "LatestSynVolume:";
		strInfo += sof_string::itostr(m_ui64LatestSynVolume, strTrans);
		strInfo = ",UnFrozeVolume:";
		strInfo += sof_string::itostr(m_ui64UnFrozeVolume, strTrans);
		strInfo = ",ForzeVolume:";
		strInfo += sof_string::itostr(m_ui64ForzeVolume, strTrans);
		strInfo = ".";

		return 0;
	}
};

#endif