#include "Logger.h"

#include "Exception.h"
#include "BackTrace.h"

#include <QSharedPointer>


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

	StreamOutput::StreamOutput(::std::FILE*  out) : m_file(out) {
		if (out == NULL) {
			throw Exception("Stream output received a null pointer");
		}
	}

	void StreamOutput::write(Level, const QString& str) {
		// Utf-8 não é null terminated
		QByteArray bytes = str.toUtf8();
		::std::fwrite(bytes.data(), sizeof(char), bytes.size(), m_file);
	}

	QSharedPointer<StreamOutput> StreamOutput::StdErr() {
		return QSharedPointer<StreamOutput>(new StreamOutput(stderr));
	}

	QSharedPointer<StreamOutput> StreamOutput::StdOut() {
		return QSharedPointer<StreamOutput>(new StreamOutput(stdout));
	}

	ColoredStreamOutput::ColoredStreamOutput(::std::FILE*  out) : m_file(out) {
		if (out == NULL) {
			throw Exception("Stream output received a null pointer");
		}
	}

	const char * ColoredStreamOutput::error_attr = "0;31";
	const char * ColoredStreamOutput::warn_attr = "0;33";
	const char * ColoredStreamOutput::info_attr =  "1;37";
	const char * ColoredStreamOutput::debug0_attr = "0;34";
	const char * ColoredStreamOutput::debug1_attr = "1;34";
	const char * ColoredStreamOutput::debug2_attr = "0;37";
	const char * ColoredStreamOutput::reset = "\x1b[0m";

	const char * ColoredStreamOutput::attrs[] = {
		error_attr,
		warn_attr,
		info_attr,
		debug0_attr,
		debug1_attr,
		debug2_attr
	};

	void ColoredStreamOutput::write(Level level, const QString& str) {
		// Utf-8 não é null terminated

		QByteArray bytes("\x1b[");
		bytes.append(attrs[level]).append("m");
		bytes.append(str.toUtf8());
		bytes.append(reset);

		::std::fwrite(bytes.data(), sizeof(char), bytes.size(), m_file);
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
		static ExceptOpts opts = LOG_ST;
		return opts;
	}

	Logger::Logger(QString name, QSharedPointer<Output> defaultOutput, Level defaultLevel, ExceptOpts exOpts)
		: m_name(name)
		, m_output(defaultOutput)
		, m_level(defaultLevel)
		, m_exOpts(exOpts)
	{}

	void Logger::output(Level level, const QString& str) {
		m_output->write(level, QString("Log: %1 - %2: %3\n").arg(levelNames[level]).arg(m_name).arg(str));
	}
}

namespace LogImpl {

}
