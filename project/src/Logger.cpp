#include "Logger.h"

#include "Exception.h"
#include "BackTrace.h"

#include <QSharedPointer>
#include <VectorIO.h>
#include <vector>

static const char* levelNames[] = {
	"ERROR ",
	"WARN  ",
	"INFO  ",
	"DEBUG0",
	"DEBUG1",
	"DEBUG2"
};


namespace Log {



	using namespace std;
	using namespace ExceptionLib;

	StreamOutput::StreamOutput(::std::FILE*  out) : m_file() {
		if (out == NULL) {
			throw Exception("Stream output received a null pointer");
		}
		m_file.open(out, QIODevice::WriteOnly);
	}

	void StreamOutput::write(Level, VectorIO::out_elem* data, int len) {
		VectorIO::write_vec(&m_file, data, len);
	}

	QSharedPointer<StreamOutput> StreamOutput::StdErr() {
		return QSharedPointer<StreamOutput>(new StreamOutput(stderr));
	}

	QSharedPointer<StreamOutput> StreamOutput::StdOut() {
		return QSharedPointer<StreamOutput>(new StreamOutput(stdout));
	}

	ColoredStreamOutput::ColoredStreamOutput(::std::FILE*  out) : m_file() {
		if (out == NULL) {
			throw Exception("Stream output received a null pointer");
		}
		m_file.open(out, QIODevice::WriteOnly);
	}

	const char * ColoredStreamOutput::error_attr = "\x1b[0;31m";
	const char * ColoredStreamOutput::warn_attr = "\x1b[0;33m";
	const char * ColoredStreamOutput::info_attr =  "\x1b[1;37m";
	const char * ColoredStreamOutput::debug0_attr = "\x1b[0;34m";
	const char * ColoredStreamOutput::debug1_attr = "\x1b[1;34m";
	const char * ColoredStreamOutput::debug2_attr = "\x1b[0;37m";
	const char * ColoredStreamOutput::reset = "\x1b[0m";

	const char * ColoredStreamOutput::attrs[] = {
		error_attr,
		warn_attr,
		info_attr,
		debug0_attr,
		debug1_attr,
		debug2_attr
	};

	void ColoredStreamOutput::write(Level level, VectorIO::out_elem* data, int len) {

		VectorIO::out_elem prologue = {
			reinterpret_cast<const void*>(attrs[level]),
			strlen(attrs[level])
		};

		VectorIO::out_elem epilogue = {
			reinterpret_cast<const void*>(reset), strlen(reset)
		};

		std::vector<VectorIO::out_elem> out;
		out.reserve(10);
		out.push_back(prologue);
		out.insert(out.end(), data, data+len);
		out.push_back(epilogue);

		VectorIO::write_vec(&m_file, &out[0], out.size());
	}

	QSharedPointer<ColoredStreamOutput> ColoredStreamOutput::StdErr() {
		return QSharedPointer<ColoredStreamOutput>(new ColoredStreamOutput(stderr));
	}

	QSharedPointer<ColoredStreamOutput> ColoredStreamOutput::StdOut() {
		return QSharedPointer<ColoredStreamOutput>(new ColoredStreamOutput(stdout));
	}


	Logger& LoggerFactory::getLogger(const QString& name) {
		if (loggers().contains(name)) {
			return *loggers()[name].data();
		} else {
			LoggerPtr ptr(new Logger(name, defaultOutput(), defaultLevel(), defaultExceptionLog()));
			loggers().insert(name, ptr);
			return *ptr.data();
		}
	}

	void LoggerFactory::changeDefaultOutput(const QSharedPointer<Output>& o)
	{
		defaultOutput() = o;
	}

	LoggerFactory::OutputPtr LoggerFactory::defaultOutput()
	{
		return defaultOutputPriv();
	}

	void LoggerFactory::changeDefaultLevel(Level l)
	{
		defaultLevelPriv() = l;
	}

	Level LoggerFactory::defaultLevel()
	{
		return defaultLevelPriv();
	}

	LoggerFactory::OutputPtr& LoggerFactory::defaultOutputPriv()
	{
#ifdef LINUX
		static OutputPtr ptr(ColoredStreamOutput::StdErr());
#else
		static OutputPtr ptr(StreamOutput::StdErr());
#endif
		return ptr;
	}

	Level& LoggerFactory::defaultLevelPriv()
	{
		static Level l = LINFO;
		return l;
	}

	LoggerFactory::LoggerMap& LoggerFactory::loggers()
	{
		static LoggerMap map;
		return map;
	}

	void LoggerFactory::changeDefaultExceptionLog(ExceptOpts opt)
	{
		defaultExceptionLogPriv() = opt;
	}

	ExceptOpts LoggerFactory::defaultExceptionLog()
	{
		return defaultExceptionLogPriv();
	}

	ExceptOpts& LoggerFactory::defaultExceptionLogPriv()
	{
		static ExceptOpts opts = LOG_ST_DBG;
		return opts;
	}

	Logger::Logger(QString name, QSharedPointer<Output> defaultOutput, Level defaultLevel, ExceptOpts exOpts)
		: m_name(name.toUtf8())
		, m_output(defaultOutput)
		, m_level(defaultLevel)
		, m_exOpts(exOpts)
	{}

	void Logger::output(Level level, const char* str, int len) {

		VectorIO::out_elem vec[] = {
			{ "Log: ", strlen("Log: ") },
			{ levelNames[level], strlen(levelNames[level]) },
			{ " - ", strlen(" - ") },
			{ m_name.constData(), m_name.size() },
			{ ": ", strlen(": ") },
			{ str, len },
			{ "\n", strlen("\n") },
		};


		m_output->write(level, vec, 7);
	}
}

namespace LogImpl {

	Formatter<std::exception>::ret_type Formatter<std::exception>::format(const std::exception& t, const Log::Logger* l) {
		using namespace Backtrace;
		if (l->getExceptionOpts() >= Log::LOG_ST) {
			size_t depth = 0;
			const StackFrame* frames = ExceptionLib::getBT(t, &depth, (l->getExceptionOpts() >= Log::LOG_ST_DBG));
			return QString("%1:\n%2").arg(t.what()).arg(Backtrace::StackTrace::asString(depth, frames).c_str());
		} else {
			return QString("%1").arg(t.what());
		}
	}


}
