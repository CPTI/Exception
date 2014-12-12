#ifndef LOGGER_H
#define LOGGER_H


#include "ArrayPtr.h"
#include "BackTrace.h"
#include "Exception.h"
#include "svector.h"
#include "TypeManip.h"
#include "VectorIO.h"
#include "string_format.h"

#include <algorithm>
#include <cstdio>
#include <deque>
#include <exception>
#include <string>
#include <string.h>
#include <list>
#include <stdint.h>

#if __cplusplus >= 201103L
#include <memory>
#include <unordered_map>
#include <mutex>
#endif

#if QT_CORE_LIB
#include <QFile>
#include <QList>
#include <QMap>
#include <QSharedPointer>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#endif

#include "LoggerFwd.h"
#include <stdio.h>

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
}



namespace Log {

	// placeholder para backtrace
	struct BTPlaceHolder {};

	extern BTPlaceHolder BT;

	struct TimeMS {
        TimeMS(int64_t relativeTo = 0) : m_rel(relativeTo) {}
        int64_t m_rel;
	};

	extern TimeMS NowMS;

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
        typedef std::string ret_type;
		static ret_type format(const std::exception& t, const Log::Logger* l);
	};

	template<>
	struct Formatter<BTPlaceHolder> {
        typedef std::string ret_type;
		static ret_type format(const BTPlaceHolder& , const Log::Logger* l);
	};

	template<>
	struct Formatter<TimeMS> {
        typedef int64_t ret_type;
		static ret_type format(const TimeMS& , const Log::Logger* l);
	};

	template<class V>
	struct IterableFormatter {
        typedef std::string ret_type;
		static ret_type format(const V& v, const Log::Logger*) {
            std::stringstream ss;
            ss << "[";
			bool first = true;
			for (typename V::const_iterator it = v.begin(); it != v.end(); ++it) {
                if (!first) { ss << ", "; }
                ss << *it;
				first = false;
			}
            ss << "]";
            return ss.str();
		}
	};


	template<class T>
	struct Formatter<std::vector<T> > : public IterableFormatter<std::vector<T> > {	};


	template<class T>
	struct Formatter<std::list<T> > : public IterableFormatter<std::list<T> > {	};

#ifdef  QT_CORE_LIB
	template<class T>
	struct Formatter<QVector<T> > : public IterableFormatter<QVector<T> > {	};


	template<class T>
	struct Formatter<QList<T> > : public IterableFormatter<QList<T> > {	};
#endif

#ifdef DEBUG
	template<class T>
	struct Formatter<svector<T> > : public IterableFormatter<svector<T> > {	};
#endif

}

namespace LogImpl {


	template<class T>
	struct F {

		typedef typename Loki::Select<
			INSTANCE_OF(T, std::exception),
			Log::Formatter<std::exception>,
			Log::Formatter<T>
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
		virtual void write(const Logger& l, Level level, VectorIO::out_elem* data, int len) = 0;

            virtual void flush() = 0;
	};

	class SplitOutput: public Output {
	public:
		virtual ~SplitOutput() {}

#if __cplusplus >= 201103L
        typedef std::shared_ptr<Output> output_ptr;
#elif defined QT_CORE_LIB
        typedef QSharedPointer<Output> output_ptr;
#endif

        virtual void write(const Logger& l, Level level, VectorIO::out_elem* data, int len) {
            foreach(const output_ptr output, m_outputs) {
                output->write(l, level, data, len);
            }
        }

        virtual void flush() {
            foreach(const output_ptr output, m_outputs) {
                output->flush();
            }
        }

        void addOutput(const output_ptr out) {
			m_outputs.push_back(out);
		}

	private:
        svector<output_ptr > m_outputs;
	};

	class LevelFilter: public Output {
    public:
#if __cplusplus >= 201103L
        typedef std::shared_ptr<Output> output_ptr;
#elif defined QT_CORE_LIB
        typedef QSharedPointer<Output> output_ptr;
#endif

        LevelFilter(output_ptr out = output_ptr(), Level l = LINFO) : m_output(out), m_level(l) {}

        void setOutput(const output_ptr out) {
            m_output = out;
        }


		virtual ~LevelFilter() {}

		virtual void write(const Logger& l, Level level, VectorIO::out_elem* data, int len) {
            if (m_output.operator->() != NULL && m_level >= level) {
				m_output->write(l, level, data, len);
			}
		}

        virtual void flush();

		void setLevel(Level l) { m_level = l; }
		Level getLevel() const { return m_level; }

	private:
        output_ptr m_output;
		Level m_level;
	};

	// Segundo o posix as operações de stream são sempre atômicas: http://www.gnu.org/software/libc/manual/html_node/Streams-and-Threads.html;
	class StreamOutput : public Output {
	public:
		StreamOutput(::std::FILE*  out);
        ~StreamOutput();

		void write(const Logger& l, Level level, VectorIO::out_elem* data, int len);

        void flush() {}


#if __cplusplus >= 201103L
        typedef std::shared_ptr<StreamOutput> stream_output_ptr;
#elif defined QT_CORE_LIB
        typedef QSharedPointer<StreamOutput> stream_output_ptr;
#endif

        static stream_output_ptr StdErr();
        static stream_output_ptr StdOut();
	private:
        ::std::FILE* m_file;
	};

	class ColoredStreamOutput : public Output {
	public:
		ColoredStreamOutput(::std::FILE*  out);
		virtual ~ColoredStreamOutput();

		void write(const Logger& l, Level level, VectorIO::out_elem* data, int len);

        void flush() {}

#if __cplusplus >= 201103L
        typedef std::shared_ptr<ColoredStreamOutput> stream_output_ptr;
#elif defined QT_CORE_LIB
        typedef QSharedPointer<ColoredStreamOutput> stream_output_ptr;
#endif

        static stream_output_ptr StdErr();
        static stream_output_ptr StdOut();

	private:
        ::std::FILE* m_file;
	};

	/**
	 * @brief The LineBufferOutput class
	 *
	 * Esta classe representa um buffer de tamanho fixo para a saída dos
	 * logs. A cada linha ela vai enchendo até que o limite máximo, à partir
	 * daí, a primeira linha é descartada a cada escrita.
	 *
	 * Cada linha tem um identificador único crescente, assim é possível
	 * verificar se há uma saída nova.
	 *
	 * Para garantir apenas uma cópia de memória as linhas são armazenadas
	 * no struct Line. Assim conseguimos minimizar o custo da instrumentação.
	 *
	 * Essa classe é thread-safe
	 *
	 */
	class LineBufferOutput: public Output {
#if __cplusplus >= 201103L
        typedef std::mutex mutex_t;
        typedef std::lock_guard<std::mutex> mutex_locker_t;
        inline mutex_t& locker_arg(mutex_t& m) {return m; }
#elif defined QT_CORE_LIB
        typedef QMutex mutex_t;
        typedef QMutexLocker mutex_locker_t;
        inline mutex_t* locker_arg(mutex_t& m) {return &m; }
#endif
	public:

		class Line {
		public:
			Line(int dataSize, Level level) : m_data(dataSize), m_line(-1), m_level(level) {}
			Line() : m_data(0), m_line(-1), m_level(LDEBUG0)  {}
			Line(const Line& that) : m_data(that.m_data), m_line(that.m_line), m_level(that.m_level) {}

			const char* data() const { return m_data.get(); }
			int64_t lineId() const { return m_line; }
			Level level() const { return m_level; }

		private:
			friend class LineBufferOutput;
			char* wdata() { return m_data.get(); }
			void setLineId(int64_t id) { m_line = id; }

			SharedArray<char> m_data;
			int64_t m_line;
			Level m_level;
		};

		/** Retorna o número de linhas que este buffer contém */
		int numLines() const { return m_lines.size(); }

		/** Retorna o máximo de linhas */
		int capacity() const { return m_maxSize; }

		int64_t lastLineId() const { return m_lastLine; }

		/** Usar o método pop garante o minimo de contencao com os escritores
		 * Como várias threads podem escrever no log, tem um mutex interno que
		 * precisa ser adquirido
		 */
		Line pop();

		/** Copia as N primeiras entradas
		 *
		 * Exemplo:
		 *
		 * std::vector<Line> lines(buffer.numLines());
		 * buffer.readN(lines.size(), lines.begin())
		 *
		 * Funciona com a maioria dos conteineres
		 */
		template<class Iterator>
		int readN(size_t numLines, Iterator out) {
            mutex_locker_t lock(locker_arg(m_mutex));
			const int linesToRead = std::min(m_lines.size(), numLines);
			std::copy(m_lines.begin(), m_lines.begin()+linesToRead, out);
			return numLines;
		}

		/** Extrai as N primeiras entradas*/
		template<class Iterator>
		int popN(size_t numLines, Iterator out) {
            mutex_locker_t lock(locker_arg(m_mutex));
			const int linesToRead = std::min(m_lines.size(), numLines);
			std::copy(m_lines.begin(), m_lines.begin()+linesToRead, out);
			return numLines;
		}

		LineBufferOutput(int size);
		~LineBufferOutput();

		void write(const Logger& l, Level level, VectorIO::out_elem* data, int len);

        void flush() {}

	private:

        mutex_t m_mutex;
		volatile int64_t m_lastLine;
		const size_t m_maxSize;
		std::deque<Line> m_lines;
	};


	class Logger;

	class LoggerFactory {
	public:

#if __cplusplus >= 201103L
        typedef std::shared_ptr<Logger> LoggerPtr;
        typedef std::unordered_map<std::string, LoggerPtr> LoggerMap;
        typedef std::shared_ptr<Output> OutputPtr;
#elif defined QT_CORE_LIB
        typedef QSharedPointer<Logger> LoggerPtr;
        typedef std::map<std::string, LoggerPtr> LoggerMap;
        typedef QSharedPointer<Output> OutputPtr;
#endif

        static Logger& getLogger(const std::string& name, std::string outputName = "stderr", bool colored = true);

        static Logger& getLogger(const char* name, const char* outputName = "stderr", bool colored = true);

#ifdef QT_CORE_LIB
        static Logger& getLogger(const QString& name, QString outputName = "stderr", bool colored = true);
#endif

		static void changeDefaultOutput(const OutputPtr o);

        static void changeNamedOutput(const OutputPtr& o, std::string outputName, bool colored);

		static OutputPtr defaultOutput();

        static OutputPtr namedOutput(std::string name, bool colored);

		static void changeDefaultLevel(Level l);

		static Level defaultLevel();

		static LoggerMap& loggers();

		static void changeDefaultExceptionLog(ExceptOpts opt);

		static ExceptOpts defaultExceptionLog();

	private:

		static OutputPtr defaultOutputPriv();

        static OutputPtr& namedOutputPriv(std::string name, bool colored);

		static Level& defaultLevelPriv();

		static ExceptOpts& defaultExceptionLogPriv();
	};

	class Logger
	{
	private:

	public:
#if __cplusplus >= 201103L
        typedef std::shared_ptr<Output> output_ptr;
#elif defined QT_CORE_LIB
        typedef QSharedPointer<Output> output_ptr;
#endif

        std::string getName() const { return m_name; }

		Level getLevel() const { return m_level; }

        void flush() { m_output->flush(); }

		void changeLevel(Level l) { m_level = l; }

        output_ptr getOuput() const { return m_output; }

        void changeNamedOutput(output_ptr o) { m_output = o; }

		ExceptOpts getExceptionOpts() const { return m_exOpts; }

		void changeExceptionOpts(ExceptOpts o) { m_exOpts = o; }

		void log(Level l, const char* fmt) {
			using namespace LogImpl;
			if (m_level >= l) {
				output(l, fmt, strlen(fmt));
			}
		}

		template <class T1>
		void log(Level l, const char* fmt, const T1& t1) {
			using namespace LogImpl;
            if (m_level >= l) {
                std::string result = string_format::format(fmt, F<T1>::doIt(t1, this));
                output(l, result.c_str(), result.size());
			}
		}

		template <class T1, class T2>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2) {
			using namespace LogImpl;
            if (m_level >= l) {
                std::string result = string_format::format(fmt, F<T1>::doIt(t1, this), F<T2>::doIt(t2, this));
                output(l, result.c_str(), result.size());
			}
		}

		template <class T1, class T2, class T3>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3) {
			using namespace LogImpl;
            if (m_level >= l) {
                std::string result = string_format::format(fmt, F<T1>::doIt(t1, this), F<T2>::doIt(t2, this), F<T3>::doIt(t3, this));
                output(l, result.c_str(), result.size());
			}
		}

		template <class T1, class T2, class T3, class T4>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4) {
			using namespace LogImpl;
            if (m_level >= l) {
                std::string result = string_format::format(fmt, F<T1>::doIt(t1, this), F<T2>::doIt(t2, this), F<T3>::doIt(t3, this), F<T4>::doIt(t4, this));
                output(l, result.c_str(), result.size());
			}
		}

		template <class T1, class T2, class T3, class T4, class T5>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5) {
			using namespace LogImpl;
            if (m_level >= l) {
                std::string result = string_format::format(fmt, F<T1>::doIt(t1, this), F<T2>::doIt(t2, this), F<T3>::doIt(t3, this), F<T4>::doIt(t4, this), F<T5>::doIt(t5, this));
                output(l, result.c_str(), result.size());
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6) {
			using namespace LogImpl;
            if (m_level >= l) {
                std::string result = string_format::format(fmt, F<T1>::doIt(t1, this), F<T2>::doIt(t2, this), F<T3>::doIt(t3, this), F<T4>::doIt(t4, this), F<T5>::doIt(t5, this), F<T6>::doIt(t6, this));
                output(l, result.c_str(), result.size());
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7) {
			using namespace LogImpl;
            if (m_level >= l) {
                std::string result = string_format::format(fmt, F<T1>::doIt(t1, this), F<T2>::doIt(t2, this), F<T3>::doIt(t3, this), F<T4>::doIt(t4, this), F<T5>::doIt(t5, this), F<T6>::doIt(t6, this), F<T7>::doIt(t7, this));
                output(l, result.c_str(), result.size());
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8) {
			using namespace LogImpl;
            if (m_level >= l) {
                std::string result = string_format::format(fmt, F<T1>::doIt(t1, this), F<T2>::doIt(t2, this), F<T3>::doIt(t3, this), F<T4>::doIt(t4, this), F<T5>::doIt(t5, this), F<T6>::doIt(t6, this), F<T7>::doIt(t7, this), F<T8>::doIt(t8, this));
                output(l, result.c_str(), result.size());
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9) {
			using namespace LogImpl;
            if (m_level >= l) {
                std::string result = string_format::format(fmt, F<T1>::doIt(t1, this), F<T2>::doIt(t2, this), F<T3>::doIt(t3, this), F<T4>::doIt(t4, this), F<T5>::doIt(t5, this), F<T6>::doIt(t6, this), F<T7>::doIt(t7, this), F<T8>::doIt(t8, this), F<T9>::doIt(t9, this));
                output(l, result.c_str(), result.size());
			}
		}

		template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
		void log(Level l, const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10) {
			using namespace LogImpl;
            if (m_level >= l) {
                std::string result = string_format::format(fmt, F<T1>::doIt(t1, this), F<T2>::doIt(t2, this), F<T3>::doIt(t3, this), F<T4>::doIt(t4, this), F<T5>::doIt(t5, this), F<T6>::doIt(t6, this), F<T7>::doIt(t7, this), F<T8>::doIt(t8, this), F<T9>::doIt(t9, this), F<T10>::doIt(t10, this));
                output(l, result.c_str(), result.size());
			}
		}

	private:
		Logger(const Logger&);
		Logger& operator=(const Logger&);

        Logger(std::string name, output_ptr defaultOutput, Level defaultLevel, ExceptOpts);

        std::string m_name;

        output_ptr m_output;

		Level m_level;

		ExceptOpts m_exOpts;

		friend class LoggerFactory;

		void output(Level level, const char* str, int len);
	};
}



#endif // LOGGER_H
