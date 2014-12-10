#include "Logger.h"

#include "Exception.h"
#include "BackTrace.h"

#include <VectorIO.h>
#include <vector>
#include <memory>

#include <stdint.h>
#include <sstream>

#if __cplusplus >= 201103L
#include <chrono>
#elif defined QT_CORE_LIB
#include <QDateTime>
#endif


static const char* levelNames[] = {
	"ERROR ",
	"WARN  ",
	"INFO  ",
	"DEBUG0",
	"DEBUG1",
	"DEBUG2"
};

namespace {

#if __cplusplus >= 201103L

int64_t currentTimeMs() {
    auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

#elif defined QT_CORE_LIB

    int64_t currentTimeMs() {
#if QT_VERSION >= 0x040700
		return QDateTime::currentMSecsSinceEpoch();
#else
		QDateTime dateTime = QDateTime::currentDateTime();
		return dateTime.toTime_t() * 1000 + dateTime.time().msec();
#endif
	}
#endif
}



namespace Log {

	using namespace std;
	using namespace ExceptionLib;

	StreamOutput::StreamOutput(::std::FILE*  out) : m_file() {
		if (out == NULL) {
			throw Exception("Stream output received a null pointer");
		}
        m_file = out;
	}

    StreamOutput::~StreamOutput() {
        fclose(m_file);
    }

	void StreamOutput::write(const Logger&, Level, VectorIO::out_elem* data, int len) {
        VectorIO::write_vec(m_file, data, len);
	}

    StreamOutput::stream_output_ptr StreamOutput::StdErr() {
        return StreamOutput::stream_output_ptr(new StreamOutput(stderr));
	}

    StreamOutput::stream_output_ptr StreamOutput::StdOut() {
        return StreamOutput::stream_output_ptr(new StreamOutput(stdout));
	}

	ColoredStreamOutput::ColoredStreamOutput(::std::FILE*  out) : m_file() {
		if (out == NULL) {
			throw Exception("Stream output received a null pointer");
        }
        m_file = out;
	}

	static const char reset[] = "\x1b[0m";

	static char attrs[][8] = {
		"\x1b[0;31m",
		"\x1b[0;33m",
		"\x1b[1;37m",
		"\x1b[0;34m",
		"\x1b[1;34m",
		"\x1b[0;37m"
	};

	ColoredStreamOutput::~ColoredStreamOutput() {
		VectorIO::out_elem epilogue = {
			reinterpret_cast<const void*>(reset), sizeof(reset)-1
		};

        VectorIO::write_vec(m_file, &epilogue, 1);
        fclose(m_file);
	}

	void ColoredStreamOutput::write(const Logger&, Level level, VectorIO::out_elem* data, int len) {

		VectorIO::out_elem prologue = {
			reinterpret_cast<const void*>(attrs[level]),
			sizeof(attrs[level]) - 1
		};

		VectorIO::out_elem epilogue = {
			reinterpret_cast<const void*>(reset), sizeof(reset)-1
		};

		std::vector<VectorIO::out_elem> out;
		out.reserve(10);
		out.push_back(prologue);
		out.insert(out.end(), data, data+len);
		out.push_back(epilogue);

        VectorIO::write_vec(m_file, &out[0], out.size());
	}

    ColoredStreamOutput::stream_output_ptr ColoredStreamOutput::StdErr() {
        return ColoredStreamOutput::stream_output_ptr(new ColoredStreamOutput(stderr));
	}

    ColoredStreamOutput::stream_output_ptr ColoredStreamOutput::StdOut() {
        return ColoredStreamOutput::stream_output_ptr(new ColoredStreamOutput(stdout));
	}

	LineBufferOutput::LineBufferOutput(int size)
		: m_lastLine(-1)
		, m_maxSize(size)
	{}

	LineBufferOutput::~LineBufferOutput() {}

	void LineBufferOutput::write(const Logger&, Level level, VectorIO::out_elem* data, int len)
	{
		// Como temos que copiar o conteúdo de data mesmo, é mais eficiente
		// concatenar tudo logo

		int totalSize = 1; // para o \0 no final

		for (int i = 0; i < len; ++i) {
			totalSize += data[i].size;
		}

		Line line(totalSize, level);

		char* out = line.wdata();

		for (int i = 0; i < len; ++i) {
			memcpy(out, data[i].begin, data[i].size);
			out += data[i].size;
		}
		*out = '\0';

        mutex_locker_t lock(locker_arg(m_mutex));
		line.setLineId(++m_lastLine);

		m_lines.push_back(line);

		while(m_lines.size() > m_maxSize) {
			m_lines.pop_front();
		}
	}


    Logger& LoggerFactory::getLogger(const std::string& name, std::string outputName, bool colored) {
        if (loggers().find(name) != loggers().end()) {
            return *loggers()[name];
		} else {
            LoggerPtr ptr(new Logger(name, namedOutput(outputName, colored), defaultLevel(), defaultExceptionLog()));
            loggers().insert(LoggerMap::value_type(name, ptr));
            return *ptr;
		}
	}

    Logger& LoggerFactory::getLogger(const char* name, const char* outputName, bool colored) {
        return getLogger(std::string(name), std::string(outputName), colored);
    }

#ifdef QT_CORE_LIB
    Logger& LoggerFactory::getLogger(const QString& name, QString outputName, bool colored) {
        return getLogger(name.toStdString(), outputName.toStdString(), colored);
    }

#endif

	void LoggerFactory::changeDefaultOutput(OutputPtr o)
	{
		defaultOutputPriv() = o;
	}

    void LoggerFactory::changeNamedOutput(const OutputPtr& o, std::string outputName, bool colored)
    {
        if (outputName == "stdout") {
            namedOutputPriv("stdout", colored) = o;
        } else {
            namedOutputPriv("stderror", colored) = o;
        }
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

	LoggerFactory::OutputPtr LoggerFactory::defaultOutputPriv()
	{
#ifdef LINUX
		static OutputPtr ptr(ColoredStreamOutput::StdErr());
#else
		static OutputPtr ptr(StreamOutput::StdErr());
#endif
		return ptr;
	}

    LoggerFactory::OutputPtr LoggerFactory::namedOutput(std::string name, bool colored)
    {
        return namedOutputPriv(name, colored);
    }

    LoggerFactory::OutputPtr& LoggerFactory::namedOutputPriv(std::string name, bool colored)
    {
#ifdef LINUX
        if (name == "stdout") {
            if ( colored ) {
                static OutputPtr ptr(ColoredStreamOutput::StdOut());
                return ptr;
            } else {
                static OutputPtr ptr(StreamOutput::StdOut());
                return ptr;
            }
        } else {
            if ( colored ) {
                static OutputPtr ptr(ColoredStreamOutput::StdErr());
                return ptr;
            } else {
                static OutputPtr ptr(StreamOutput::StdErr());
                return ptr;
            }
        }
#else
        if (name == "stdout") {
            static OutputPtr ptr(StreamOutput::StdOut());
            return ptr;
        } else {
            static OutputPtr ptr(StreamOutput::StdErr());
            return ptr;
        }
#endif
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

    Logger::Logger(std::string name, Logger::output_ptr defaultOutput, Level defaultLevel, ExceptOpts exOpts)
        : m_name(name)
		, m_output(defaultOutput)
		, m_level(defaultLevel)
		, m_exOpts(exOpts)
    {}

	void Logger::output(Level level, const char* str, int len) {

		VectorIO::out_elem vec[] = {
			{ "Log: ", sizeof("Log: ") - 1 },
			{ levelNames[level], strlen(levelNames[level]) },
			{ " - ", sizeof(" - ") - 1 },
            { m_name.c_str(), static_cast<size_t>(m_name.size()) },
			{ ": ", sizeof(": ") -1 },
			{ str, static_cast<size_t>(len) },
			{ "\n", sizeof("\n") -1 },
		};

		m_output->write(*this, level, vec, 7);
	}
}

namespace {

#define MAX_NESTED 10

    std::string formatException(int depth, const std::exception& t, const Log::Logger* l) {
		using namespace Backtrace;

        std::stringstream result;

		if (l->getExceptionOpts() >= Log::LOG_ST) {
			size_t depth = 0;
			const StackFrame* frames = ExceptionLib::getBT(t, &depth, (l->getExceptionOpts() >= Log::LOG_ST_DBG));
            result << t.what() << ":\n" << Backtrace::StackTrace::asString(depth, frames);
		} else {
            result << t.what();
		}

		const ExceptionLib::Exception* ex = dynamic_cast<const ExceptionLib::Exception*>(&t);

		if (ex && ex->nested() && depth < MAX_NESTED) {
            result << "\nNested exception: \n" << formatException(depth+1, *ex->nested(), l);
		}

        return result.str();
	}

}

namespace Log {

	Formatter<std::exception>::ret_type Formatter<std::exception>::format(const std::exception& t, const Log::Logger* l) {		
		return formatException(0, t, l);
	}

	BTPlaceHolder BT;

	Formatter<BTPlaceHolder>::ret_type Formatter<BTPlaceHolder>::format(const BTPlaceHolder& , const Log::Logger*)	{
		std::auto_ptr<Backtrace::StackTrace> trace(Backtrace::trace());
        return trace->asString(true, 4 /* skip */);
	}

	TimeMS NowMS;

	Formatter<TimeMS>::ret_type Formatter<TimeMS>::format(const TimeMS& t, const Log::Logger*)	{
		return (currentTimeMs() - t.m_rel);
	}
}
