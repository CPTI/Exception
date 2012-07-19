#ifndef LOGGER_H
#define LOGGER_H

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

	private:

		static OutputPtr& defaultOutputPriv();

		static Level& defaultLevelPriv();
	};

	class Logger
	{
	public:

		const QString& getName() { return m_name; }

		Level getLevel() const { return m_level; }

		void changeLevel(Level l) { m_level = l; }

		QSharedPointer<Output> getOuput() const { return m_output; }

		void changeOutput(QSharedPointer<Output> o) { m_output = o; }

		void log(Level l, const QString& str) { // esse overload é o mais lento se a sua mensagem não vai para a saída
			if (m_level >= l) {
				output(l, str);
			}
		}

		inline void log(Level l, const char* fmt) {
			if (m_level >= l) {
				output(l, fmt);
			}
		}

		template <class T1>
		void log(Level l, const char* fmt, const T1& t1) {
			if (m_level >= l) {
				output(l, QString(fmt).arg(t1));
			}
		}

		template <class T1, class T2>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2) {
			if (m_level >= l) {
				output(l, QString(fmt).arg(t1).arg(t2));
			}
		}

		template <class T1, class T2, class T3>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3) {
			if (m_level >= l) {
				output(l, QString(fmt).arg(t1).arg(t2).arg(t3));
			}
		}

		template <class T1, class T2, class T3, class T4>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4) {
			if (m_level >= l) {
				output(l, QString(fmt).arg(t1).arg(t2).arg(t3).arg(t4));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5) {
			if (m_level >= l) {
				output(l, QString(fmt).arg(t1).arg(t2).arg(t3).arg(t4).arg(t5));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6) {
			if (m_level >= l) {
				output(l, QString(fmt).arg(t1).arg(t2).arg(t3).arg(t4).arg(t5).arg(t6));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7) {
			if (m_level >= l) {
				output(l, QString(fmt).arg(t1).arg(t2).arg(t3).arg(t4).arg(t5).arg(t6).arg(t7));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8) {
			if (m_level >= l) {
				output(l, QString(fmt).arg(t1).arg(t2).arg(t3).arg(t4).arg(t5).arg(t6).arg(t7).arg(t8));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9) {
			if (m_level >= l) {
				output(l, QString(fmt).arg(t1).arg(t2).arg(t3).arg(t4).arg(t5).arg(t6).arg(t7).arg(t8).arg(t9));
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10) {
			if (m_level >= l) {
				output(l, QString(fmt).arg(t1).arg(t2).arg(t3).arg(t4).arg(t5).arg(t6).arg(t7).arg(t8).arg(t9).arg(t10));
			}
		}

	private:
		Logger(const Logger&);
		Logger& operator=(const Logger&);

		Logger(QString name, QSharedPointer<Output> defaultOutput, Level defaultLevel);

		QString m_name;

		QSharedPointer<Output> m_output;

		Level m_level;

		friend class LoggerFactory;

		void output(Level level, const QString& str);
	};
}

#endif // LOGGER_H
