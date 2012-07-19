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

		static Logger& getLogger(const QString& name);

		static void changeDefaultOutput(const QSharedPointer<Output>& o);

		static QSharedPointer<Output> defaultOutput();

		static void changeDefaultLevel(Level l);

		static Level defaultLevel();

	private:

		typedef QSharedPointer<Logger> LoggerPtr;
		typedef QSharedPointer<Output> OutputPtr;
		typedef QMap<QString, LoggerPtr> LoggerMap;

		static OutputPtr& defaultOutputPriv();

		static Level& defaultLevelPriv();

		static LoggerMap& loggers();
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
