#ifndef __FROZE_VOLUME_H__
#define __FROZE_VOLUME_H__

#include <string>
#include <vector>
#include <stdint.h>

#include "boost/thread/mutex.hpp"

#include "tool_string/sof_string.h"
#include "util/EzLog.h"

/*
	可供冻结的缓存计数类
	*/
class FrozeVolume
{
	//
	// Members
	//
protected:
	src::severity_channel_logger<trivial::severity_level, std::string> m_scl;

	// 最近与数据库同步的计数
	uint64_t m_ui64LatestSynVolume;

	// 剩余未冻结
	uint64_t m_ui64UnFrozeVolume;
	
	// 冻结数
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
	查询剩余交易额是否足够

	Param:
	out_maxAvl -- 如果不够，返回最大可使用的数量

	Return:
	0 -- 剩余交易额足够
	-1 -- 剩余交易额不足
	*/
	int Query(const uint64_t cnt, uint64_t *out_maxAvl = NULL)
	{
		static const std::string ftag("FrozeVolume::Query() ");

		boost::unique_lock<boost::mutex> Locker(m_mutex);

		// 判断是否超限制
		if (m_ui64UnFrozeVolume < cnt)
		{
			// 超过限制额度
			if (NULL != out_maxAvl)
			{
				// 最大可用数量
				*out_maxAvl = m_ui64UnFrozeVolume;
			}

			return -1;
		}
		else
		{
			if (NULL != out_maxAvl)
			{
				// 最大可用数量
				*out_maxAvl = cnt;
			}
		}

		return 0;
	}

	/*
	冻结交易额
	Param:
	out_maxfrozev -- 如果frozev冻结失败，再次冻结剩余数量，返回当前冻结的数量

	Return:
	0 -- 未超过最大限制额度，冻结成功
	-1 -- 冻结失败
	*/
	int Froze(const uint64_t frozev, uint64_t *out_maxfrozev = NULL)
	{
		static const std::string ftag("FrozeVolume::Froze() ");

		boost::unique_lock<boost::mutex> Locker(m_mutex);

		// 冻结后的预计算额度
		uint64_t ui64_ForzeVolume_After = m_ui64ForzeVolume + frozev;

		uint64_t ui64ActualFroze = 0;
		// 看看是否计数溢出
		if (ui64_ForzeVolume_After < m_ui64ForzeVolume)
		{
			// 发生了溢出
			std::string sItoa;
			std::string sDebug("overflow, ForzeVolume=");
			sDebug += sof_string::itostr(m_ui64ForzeVolume, sItoa);
			sDebug += "; forze=";
			sDebug += sof_string::itostr(frozev, sItoa);

			BOOST_LOG_SEV(m_scl, trivial::warning) << ftag << sDebug;
		}

		// 判断是否超限制
		if (m_ui64UnFrozeVolume < frozev)
		{
			// 超过限制额度
			if (NULL != out_maxfrozev)
			{
				// 最大可用数量
				*out_maxfrozev = m_ui64UnFrozeVolume;
			}
			ui64ActualFroze = m_ui64UnFrozeVolume;
		}
		else
		{
			if (NULL != out_maxfrozev)
			{
				// 最大可用数量
				*out_maxfrozev = frozev;
			}
			ui64ActualFroze = frozev;
		}

		m_ui64ForzeVolume += ui64ActualFroze;
		m_ui64UnFrozeVolume -= ui64ActualFroze;

		return 0;
	}

	/*
	解除冻结交易额

	Return:
	0 -- 解除冻结成功
	-1 -- 解除冻结失败
	*/
	int Defroze(const uint64_t frozev)
	{
		static const std::string ftag("FrozeVolume::Defroze() ");

		boost::unique_lock<boost::mutex> Locker(m_mutex);

		if (m_ui64ForzeVolume < frozev)
		{
			// 超过冻结额度，有问题
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
	确认交易额，将从冻结的部分扣除相应额度

	Return:
	0 -- 确认成功
	-1 -- 确认失败
	*/
	int Confirm(const uint64_t confirm)
	{
		static const std::string ftag("FrozeVolume::Confirm() ");

		boost::unique_lock<boost::mutex> Locker(m_mutex);

		if (m_ui64ForzeVolume < confirm)
		{
			// 超过冻结额度，有问题
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
		与数据库信息同步，更新最近同步计数和未冻结计数
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
		初始化
		设置同步计数和未冻结数，第一次从数据库读取时调用
		*/
	int Init(uint64_t volume)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutex);

		m_ui64LatestSynVolume = volume;
		m_ui64UnFrozeVolume = volume;

		return 0;
	}

	/*
		重设最近与数据库同步的计数
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
		增加未冻结
		*/
	int Add(uint64_t inc)
	{
		boost::unique_lock<boost::mutex> Locker(m_mutex);

		m_ui64UnFrozeVolume += inc;

		return 0;
	}

	/*
		打印当前信息
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