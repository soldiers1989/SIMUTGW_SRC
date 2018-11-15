#ifndef __EZ_LOG_H__
#define __EZ_LOG_H__

#ifdef _MSC_VER
#include <Windows.h>
#include <DbgHelp.h>
#else

#endif

#include <exception>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include <map>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/log/common.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/from_stream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

#include "tool_string/sof_string.h"

using namespace std;

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace trivial = boost::log::trivial;

class EzLog
{

	//
	// Members
	//
private:
	static const int TRACE_MAX_STACK_FRAMES = 200;
	static const int TRACE_MAX_FUNCTION_NAME_LENGTH = 1024;
	static const int TRACE_FramesToSkip = 1;

public:
	//定义一个日志输出通道对象
	//static boost::log::sources::severity_channel_logger<severity_level, std::string> m_scl;
	static boost::log::sources::severity_logger_mt< boost::log::trivial::severity_level > m_lg;

	//
	// Functions
	//
public:
	EzLog(void);
	virtual ~EzLog(void);

	// 初始化日志环境
	static int Init_log_ern(std::string _cfg);

	// trace
	static void t(const string& strTag, const string& strOutput);

	// debug
	static void d(const string& strTag, const string& strOutput);

	// info
	static void i(const string& strTag, const string& strOutput);

	// warning
	static void w(const string& strTag, const string& strOutput);

	// error
	static void e(const string& strTag, const string& strOutput);

	// printStackTrace
	static int printStackTrace(string& strExce);

	// exception
	static void ex(const string& strTag, const exception& e);

	// string
	static void Out(const string& strTag, trivial::severity_level eType, const string& obj);
	
	// int
	static void Out(const string& strTag, trivial::severity_level eType, int obj);

	// long
	static void Out(const string& strTag, trivial::severity_level eType, long obj);
	
	// map
	static void Out(const string& strTag, trivial::severity_level eType, const string& strComment,
		const map<string, int> &mapOutput);

	// map
	static void Out(const string& strTag, trivial::severity_level eType, const string& strComment,
		const map<string, string>& mapOutput);

	// map
	static void Out(const string& strTag, trivial::severity_level eType, const string& strComment,
		const map<string, vector<string> > &mapOutput);

	// vector
	static void Out(const string& strTag, trivial::severity_level eType, const string& strComment,
		const vector <pair<string, int> >& vecOutput);
};

#endif