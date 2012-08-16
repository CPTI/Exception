#ifndef LOGGER_H
#define LOGGER_H


#include "Exception.h"
#include "BackTrace.h"
#include "TypeManip.h"
#include <exception>
#include <string>

#include <QMap>
#include <QSharedPointer>

#include <cstdio>

namespace Log {

	enum Level {
		LERROR = 0,
		LWARN,
		LINFO,
		LDEBUG0,
		LDEBUG1,
		LDEBUG2
	};

	enum ExceptOpts {
		LOG_WHAT = 0,
		LOG_ST,
		LOG_ST_DBG
	};

	class Logger;
}

namespace LogImpl {

	template<class T>
	struct Formatter {
		typedef const T& ret_type;
		static ret_type format(const T& t, const Log::Logger*) { return t; }
	};

	template<>
	struct Formatter<std::string> {
		typedef const char* ret_type;
		static ret_type format(const std::string& t, const Log::Logger*) { return t.c_str(); }
	};

	template<>
	struct Formatter<std::exception> {
		typedef QString ret_type;
		static ret_type format(const std::exception& t, const Log::Logger* l);
	};


	template<class T>
	struct F {

		typedef typename Loki::Select<
			INSTANCE_OF(T, std::exception),
			Formatter<std::exception>,
			Formatter<T>
		>::Result Frmttr;

		static typename Frmttr::ret_type doIt(const T& t, const Log::Logger* l) {
			return Frmttr::format(t, l);
		}

	};

}


namespace Log {

	class Output {
	public:
		virtual ~Output() {}
		// O unico requisito de write é que deve ser atomico
		virtual void write(Level level, const QString& str) = 0;
	};

	// Segundo o posix as operações de stream são sempre atômicas: http://www.gnu.org/software/libc/manual/html_node/Streams-and-Threads.html;
	class StreamOutput : public Output {
	public:
		StreamOutput(::std::FILE*  out);

		void write(Level level, const QString& str);

		static QSharedPointer<StreamOutput> StdErr();
		static QSharedPointer<StreamOutput> StdOut();
	private:
		::std::FILE* m_file;
	};

	class ColoredStreamOutput : public Output {
	public:
		ColoredStreamOutput(::std::FILE*  out);

		void write(Level level, const QString& str);

		static QSharedPointer<ColoredStreamOutput> StdErr();
		static QSharedPointer<ColoredStreamOutput> StdOut();

		static const char * error_attr;
		static const char * warn_attr;
		static const char * info_attr;
		static const char * debug0_attr;
		static const char * debug1_attr;
		static const char * debug2_attr;
		static const char * reset;

		static const char * attrs[];

	private:
		::std::FILE* m_file;
	};


	class Logger;

	class LoggerFactory {
	public:

		typedef QSharedPointer<Logger> LoggerPtr;
		typedef QMap<QString, LoggerPtr> LoggerMap;
		typedef QSharedPointer<Output> OutputPtr;

		static Logger& getLogger(const QString& name);

		static void changeDefaultOutput(const OutputPtr& o);

		static OutputPtr defaultOutput();

		static void changeDefaultLevel(Level l);

		static Level defaultLevel();

		static LoggerMap& loggers();

		static void changeDefaultExceptionLog(ExceptOpts opt);

		static ExceptOpts defaultExceptionLog();

	private:

		static OutputPtr& defaultOutputPriv();

		static Level& defaultLevelPriv();

		static ExceptOpts& defaultExceptionLogPriv();
	};

	class Logger
	{
	private:

	public:

		const QString& getName() { return m_name; }

		Level getLevel() const { return m_level; }

		void changeLevel(Level l) { m_level = l; }

		QSharedPointer<Output> getOuput() const { return m_output; }

		void changeOutput(QSharedPointer<Output> o) { m_output = o; }

		ExceptOpts getExceptionOpts() const { return m_exOpts; }

		void changeExceptionOpts(ExceptOpts o) { m_exOpts = o; }

		void log(Level l, const QString& str) { // esse overload é o mais lento se a sua mensagem não vai para a saída
			if (m_level >= l) {
				output(l, str);
			}
		}

		template <class T1>
		void log(Level l, const char* fmt, const T1& t1) {
			using namespace LogImpl;
			if (m_level >= l) {
				output(l, QString(fmt).arg(F<T1>::doIt(t1, this)));
			}
		}

		template <class T1, class T2>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2) {
			using namespace LogImpl;
			if (m_level >= l) {
				output(l, QString(fmt).arg(F<T1>::doIt(t1, this)).arg(F<T2>::doIt(t2, this)));
			}
		}

		template <class T1, class T2, class T3>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3) {
			using namespace LogImpl;
			if (m_level >= l) {
				output(l, QString(fmt).arg(F<T1>::doIt(t1, this)).arg(F<T2>::doIt(t2, this)).arg(F<T3>::doIt(t3, this)));
			}
		}

		template <class T1, class T2, class T3, class T4>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4) {
			using namespace LogImpl;
			if (m_level >= l) {
				output(l, QString(fmt).arg(F<T1>::doIt(t1, this)).arg(F<T2>::doIt(t2, this)).arg(F<T3>::doIt(t3, this)).arg(F<T4>::doIt(t4, this)));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5) {
			using namespace LogImpl;
			if (m_level >= l) {
				output(l, QString(fmt).arg(F<T1>::doIt(t1, this)).arg(F<T2>::doIt(t2, this)).arg(F<T3>::doIt(t3, this)).arg(F<T4>::doIt(t4, this)).arg(F<T5>::doIt(t5, this)));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6) {
			using namespace LogImpl;
			if (m_level >= l) {
				output(l, QString(fmt).arg(F<T1>::doIt(t1, this)).arg(F<T2>::doIt(t2, this)).arg(F<T3>::doIt(t3, this)).arg(F<T4>::doIt(t4, this)).arg(F<T5>::doIt(t5, this)).arg(F<T6>::doIt(t6, this)));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7) {
			using namespace LogImpl;
			if (m_level >= l) {
				output(l, QString(fmt).arg(F<T1>::doIt(t1, this)).arg(F<T2>::doIt(t2, this)).arg(F<T3>::doIt(t3, this)).arg(F<T4>::doIt(t4, this)).arg(F<T5>::doIt(t5, this)).arg(F<T6>::doIt(t6, this)).arg(F<T7>::doIt(t7, this)));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8) {
			using namespace LogImpl;
			if (m_level >= l) {
				output(l, QString(fmt).arg(F<T1>::doIt(t1, this)).arg(F<T2>::doIt(t2, this)).arg(F<T3>::doIt(t3, this)).arg(F<T4>::doIt(t4, this)).arg(F<T5>::doIt(t5, this)).arg(F<T6>::doIt(t6, this)).arg(F<T7>::doIt(t7, this)).arg(F<T8>::doIt(t8, this)));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9) {
			using namespace LogImpl;
			if (m_level >= l) {
				output(l, QString(fmt).arg(F<T1>::doIt(t1, this)).arg(F<T2>::doIt(t2, this)).arg(F<T3>::doIt(t3, this)).arg(F<T4>::doIt(t4, this)).arg(F<T5>::doIt(t5, this)).arg(F<T6>::doIt(t6, this)).arg(F<T7>::doIt(t7, this)).arg(F<T8>::doIt(t8, this)).arg(F<T9>::doIt(t9, this)));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10) {
			using namespace LogImpl;
			if (m_level >= l) {
				output(l, QString(fmt).arg(F<T1>::doIt(t1, this)).arg(F<T2>::doIt(t2, this)).arg(F<T3>::doIt(t3, this)).arg(F<T4>::doIt(t4, this)).arg(F<T5>::doIt(t5, this)).arg(F<T6>::doIt(t6, this)).arg(F<T7>::doIt(t7, this)).arg(F<T8>::doIt(t8, this)).arg(F<T9>::doIt(t9, this)).arg(F<T10>::doIt(t10, this)));
			}
		}

	private:
		Logger(const Logger&);
		Logger& operator=(const Logger&);

		Logger(QString name, QSharedPointer<Output> defaultOutput, Level defaultLevel, ExceptOpts);

		QString m_name;

		QSharedPointer<Output> m_output;

		Level m_level;

		ExceptOpts m_exOpts;

		friend class LoggerFactory;

		void output(Level level, const QString& str);
	};
}



#endif // LOGGER_H
