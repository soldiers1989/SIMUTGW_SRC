#include "EzLog.h"

#include <string>
#include <fstream>

#ifdef _MSC_VER

#else
#include <execinfo.h>
#endif

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
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

using namespace std;

EzLog::EzLog(void)
{
}

EzLog::~EzLog(void)
{
}

boost::log::sources::severity_logger_mt< boost::log::trivial::severity_level > EzLog::m_lg;

// 初始化日志环境
int EzLog::Init_log_ern(std::string _cfg)
{
	namespace logging = boost::log;

	if (!boost::filesystem::exists("./log/"))
	{
		boost::filesystem::create_directory("./log/");
	}
	logging::add_common_attributes();

	logging::register_simple_formatter_factory<boost::log::trivial::severity_level, char>("Severity");
	logging::register_simple_filter_factory<boost::log::trivial::severity_level, char>("Severity");

	std::ifstream file(_cfg.c_str());
	try
	{
		boost::log::init_from_stream(file);
	}
	catch (const std::exception& e)
	{
		std::cout << "init_logger is fail, read log config file fail. curse: " << e.what() << std::endl;
		exit(-2);
	}
	
	return true;
}

// string
void EzLog::Out(const string& strTag, trivial::severity_level eType, const string& obj)
{
	BOOST_LOG_SEV(m_lg, eType) << strTag << obj;
}

// int
void EzLog::Out(const string& strTag, trivial::severity_level eType, int obj)
{
	string sObj;
	sof_string::itostr(obj, sObj);
	Out(strTag, eType, sObj);
}

// long
void EzLog::Out(const string& strTag, trivial::severity_level eType, long obj)
{
	string sObj;
	sof_string::itostr(obj, sObj);
	Out(strTag, eType, sObj);
}

// trace
void EzLog::t(const string& strTag, const string& strOutput)
{
	BOOST_LOG_SEV(m_lg, trivial::trace) << strTag << strOutput;
}

// debug
void EzLog::d(const string& strTag, const string& strOutput)
{
	BOOST_LOG_SEV(m_lg, trivial::debug) << strTag << strOutput;
}


// info
void EzLog::i(const string& strTag, const string& strOutput)
{
	BOOST_LOG_SEV(m_lg, trivial::info) << strTag << strOutput;
}

// warning
void EzLog::w(const string& strTag, const string& strOutput)
{
	BOOST_LOG_SEV(m_lg, trivial::warning) << strTag << strOutput;
}

// error
void EzLog::e(const string& strTag, const string& strOutput)
{
	BOOST_LOG_SEV(m_lg, trivial::error) << strTag << strOutput;
}

// printStackTrace
int EzLog::printStackTrace(string& strExce)
{
#ifdef _MSC_VER
	void *stack[TRACE_MAX_STACK_FRAMES];

	HANDLE process = GetCurrentProcess();

	SymInitialize(process, NULL, TRUE);
	WORD numberOfFrames = CaptureStackBackTrace(TRACE_FramesToSkip, TRACE_MAX_STACK_FRAMES, stack, NULL);

	SYMBOL_INFO *symbol = (SYMBOL_INFO *)malloc(sizeof(SYMBOL_INFO) + (TRACE_MAX_FUNCTION_NAME_LENGTH - 1) * sizeof(TCHAR));
	symbol->MaxNameLen = TRACE_MAX_FUNCTION_NAME_LENGTH;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	DWORD displacement;
	IMAGEHLP_LINE64 *line = (IMAGEHLP_LINE64 *)malloc(sizeof(IMAGEHLP_LINE64));
	line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	strExce = "\n======begin printStackTrace======\n";

	WORD numberOfFramesCutted = numberOfFrames;

	if (6 < numberOfFrames)
	{
		numberOfFramesCutted = numberOfFrames - 5;
	}

	stringstream ss;

	for (int i = 0; i < numberOfFramesCutted; i++)
	{
		DWORD64 address = (DWORD64)(stack[i]);
		SymFromAddr(process, address, NULL, symbol);
		if (SymGetLineFromAddr64(process, address, &displacement, line))
		{
			string strItoa;
			strExce += "\tat ";
			strExce += symbol->Name;
			strExce += " in ";
			strExce += line->FileName;
			strExce += ": line: ";
			strExce += sof_string::itostr((uint64_t)line->LineNumber, strItoa);
			strExce += ": address: 0x";

			ss.clear();
			ss.str("");
			ss << std::hex << symbol->Address;
			strExce += ss.str();
			//printf("\tat %s in %s: line: %lu: address: 0x%0X\n", symbol->Name, line->FileName, line->LineNumber, symbol->Address);
		}
		else
		{
			string strItoa;
			strExce += "\tSymGetLineFromAddr64 returned error code ";
			strExce += sof_string::itostr((uint64_t)GetLastError(), strItoa);
			strExce += ".\n\tat ";
			strExce += symbol->Name;
			strExce += ", address 0x";

			ss.clear();
			ss.str("");
			ss << std::hex << symbol->Address;
			strExce += ss.str();

			//printf("\tSymGetLineFromAddr64 returned error code %lu.\n", GetLastError());
			//printf("\tat %s, address 0x%0X.\n", symbol->Name, symbol->Address);
		}
	}
#else
	void *buffer[TRACE_MAX_STACK_FRAMES];
	char **symbols = NULL;

	int nptrs = backtrace(buffer, TRACE_MAX_STACK_FRAMES);

	symbols = backtrace_symbols(buffer, nptrs);
	if (NULL == symbols)
	{
		perror("backtrace_symbols");
		BOOST_LOG_SEV(m_lg, trivial::error) << "backtrace_symbols";
		exit(EXIT_FAILURE);
	}

	strExce = "\n======begin printStackTrace======\n";

	for (int j = 0; j < nptrs; ++j)
	{
		strExce += symbols[j];
	}

	free(symbols);
#endif
	strExce += "\n======end printStackTrace======\n";

	return 0;
}

// exception
void EzLog::ex(const string& strTag, const exception& e)
{
	string strExce;
	printStackTrace(strExce);
	string sout(strTag);
	sout += "Exception----";
	sout += e.what();
	sout += strExce;

	BOOST_LOG_SEV(m_lg, trivial::error) << strTag << sout;
}

// map
void EzLog::Out(const string& strTag, trivial::severity_level eType, const string& strComment,
	const map<string, vector<string> >& mapOutput)
{
	map<string, vector<string> >::const_iterator iterMap;

	string strMsg(strComment);
	strMsg += " mapAddr=[";

	stringstream ss;
	ss << std::hex << &mapOutput;
	strMsg += ss.str();

	strMsg += "] key      value\n";

	for (iterMap = mapOutput.begin(); iterMap != mapOutput.end(); ++iterMap)
	{
		strMsg += iterMap->first;
		strMsg += "    ";

		for (size_t i = 0; i < iterMap->second.size(); ++i)
		{
			strMsg += " [";
			strMsg += iterMap->second[i];
			strMsg += "]";
		}
	}

	BOOST_LOG_SEV(m_lg, eType) << strTag << strMsg;
}

// map
void EzLog::Out(const string& strTag, trivial::severity_level eType, const string& strComment,
	const map<string, int>& mapOutput)
{
	string strMsg(strComment);
	strMsg += " mapAddr=[";

	stringstream ss;
	ss << std::hex << &mapOutput;
	strMsg += ss.str();

	strMsg += "] key      value\n";

	string strItoa;

	map<string, int>::const_iterator iterMap;
	for (iterMap = mapOutput.begin(); iterMap != mapOutput.end(); ++iterMap)
	{
		strMsg += iterMap->first;
		strMsg += "    ";
		strMsg += sof_string::itostr(iterMap->second, strItoa);
		strMsg += " \n";
	}

	BOOST_LOG_SEV(m_lg, eType) << strTag << strMsg;
}

// map
void EzLog::Out(const string& strTag, trivial::severity_level eType, const string& strComment,
	const map<string, string>& mapOutput)
{
	string strMsg(strComment);
	strMsg += " mapAddr=[";

	stringstream ss;
	ss << std::hex << &mapOutput;
	strMsg += ss.str();

	strMsg += "] key      value\n";

	map<string, string>::const_iterator iterMap;
	for (iterMap = mapOutput.begin(); iterMap != mapOutput.end(); ++iterMap)
	{
		strMsg += iterMap->first;
		strMsg += "    ";
		strMsg += iterMap->second;
		strMsg += " \n";
	}

	BOOST_LOG_SEV(m_lg, eType) << strTag << strMsg;
}

// vector
void EzLog::Out(const string& strTag, trivial::severity_level eType, const string& strComment,
	const vector <pair<string, int> >& vecOutput)
{
	string strMsg(strComment);
	strMsg += " vctAddr=[";

	stringstream ss;
	ss << std::hex << &vecOutput;
	strMsg += ss.str();

	strMsg += "] key      value\n";

	string strItoa;

	for (size_t index = 0; index < vecOutput.size(); ++index)
	{
		strMsg += vecOutput[index].first;
		strMsg += "  ";
		strMsg += sof_string::itostr(vecOutput[index].second, strItoa);
		strMsg += "\n";
	}

	BOOST_LOG_SEV(m_lg, eType) << strTag << strMsg;
}
