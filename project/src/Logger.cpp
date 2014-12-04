#include "Logger.h"

#include "Exception.h"
#include "BackTrace.h"

#include <QSharedPointer>
#include <QStringBuilder>
#include <VectorIO.h>
#include <vector>
#include <memory>
#include <QDateTime>

static const char* levelNames[] = {
	"ERROR ",
	"WARN  ",
	"INFO  ",
	"DEBUG0",
	"DEBUG1",
	"DEBUG2"
};

namespace {
	qint64 currentTimeMs() {
#if QT_VERSION >= 0x040700
		return QDateTime::currentMSecsSinceEpoch();
#else
		QDateTime dateTime = QDateTime::currentDateTime();
		return dateTime.toTime_t() * 1000 + dateTime.time().msec();
#endif
	}
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

	QSharedPointer<ColoredStreamOutput> ColoredStreamOutput::StdErr() {
		return QSharedPointer<ColoredStreamOutput>(new ColoredStreamOutput(stderr));
	}

	QSharedPointer<ColoredStreamOutput> ColoredStreamOutput::StdOut() {
		return QSharedPointer<ColoredStreamOutput>(new ColoredStreamOutput(stdout));
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

		QMutexLocker lock(&m_mutex);
		line.setLineId(++m_lastLine);

		m_lines.push_back(line);

		while(m_lines.size() > m_maxSize) {
			m_lines.pop_front();
		}
	}


    Logger& LoggerFactory::getLogger(const QString& name, QString outputName, bool colored) {
		if (loggers().contains(name)) {
			return *loggers()[name].data();
		} else {
            LoggerPtr ptr(new Logger(name, namedOutput(outputName, colored), defaultLevel(), defaultExceptionLog()));
			loggers().insert(name, ptr);
			return *ptr.data();
		}
	}

	void LoggerFactory::changeDefaultOutput(OutputPtr o)
	{
		defaultOutputPriv() = o;
	}

    void LoggerFactory::changeNamedOutput(const QSharedPointer<Output>& o, QString outputName, bool colored)
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

    LoggerFactory::OutputPtr LoggerFactory::namedOutput(QString name, bool colored)
    {
        return namedOutputPriv(name, colored);
    }

    LoggerFactory::OutputPtr& LoggerFactory::namedOutputPriv(QString name, bool colored)
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

	Logger::Logger(QString name, QSharedPointer<Output> defaultOutput, Level defaultLevel, ExceptOpts exOpts)
		: m_name(name.toUtf8())
		, m_output(defaultOutput)
		, m_level(defaultLevel)
		, m_exOpts(exOpts)
    {}

	void Logger::output(Level level, const char* str, int len) {

		VectorIO::out_elem vec[] = {
			{ "Log: ", sizeof("Log: ") - 1 },
			{ levelNames[level], strlen(levelNames[level]) },
			{ " - ", sizeof(" - ") - 1 },
			{ m_name.constData(), static_cast<size_t>(m_name.size()) },
			{ ": ", sizeof(": ") -1 },
			{ str, static_cast<size_t>(len) },
			{ "\n", sizeof("\n") -1 },
		};

		m_output->write(*this, level, vec, 7);
	}
}

namespace {

#define MAX_NESTED 10

	QString formatException(int depth, const std::exception& t, const Log::Logger* l) {
		using namespace Backtrace;

		QString result;

		if (l->getExceptionOpts() >= Log::LOG_ST) {
			size_t depth = 0;
			const StackFrame* frames = ExceptionLib::getBT(t, &depth, (l->getExceptionOpts() >= Log::LOG_ST_DBG));
			result = QString("%1:\n%2").arg(t.what()).arg(Backtrace::StackTrace::asString(depth, frames).c_str());
		} else {
			result = QString("%1").arg(t.what());
		}

		const ExceptionLib::Exception* ex = dynamic_cast<const ExceptionLib::Exception*>(&t);

		if (ex && ex->nested() && depth < MAX_NESTED) {
			result += "\nNested exception: \n" % formatException(depth+1, *ex->nested(), l);
		}

		return result;
	}

}

namespace Log {

	Formatter<std::exception>::ret_type Formatter<std::exception>::format(const std::exception& t, const Log::Logger* l) {		
		return formatException(0, t, l);
	}

	BTPlaceHolder BT;

	Formatter<BTPlaceHolder>::ret_type Formatter<BTPlaceHolder>::format(const BTPlaceHolder& , const Log::Logger*)	{
		std::auto_ptr<Backtrace::StackTrace> trace(Backtrace::trace());
		return QString::fromStdString(trace->asString(true, 4 /* skip */));
	}

	TimeMS NowMS;

	Formatter<TimeMS>::ret_type Formatter<TimeMS>::format(const TimeMS& t, const Log::Logger*)	{
		return (currentTimeMs() - t.m_rel);
	}
}
