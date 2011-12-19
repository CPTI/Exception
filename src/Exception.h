/*
 * Exception.h
 *
 *  Created on: May 5, 2010
 *      Author: maxdebayser
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_
#include <exception>
#include <stdexcept>
#include <string>
#include <ostream>

#ifdef QT_CORE_LIB
#include <QtCore>
#include <QString>
#include <QCoreApplication>
#include <QList>
	typedef QtConcurrent::Exception BaseExceptionType;
#else
	typedef std::exception BaseExceptionType;
#endif

/* This file defines a hierachy of Exception classes that is meant to
 * unify the definition and treatment of exceptions. All exceptions
 * inherit from std::exception, thus making it easier to integrate with
 * existing code. Most of the classes defined in this file should be
 * used as base classes for more detailed exceptions. It is important
 * to remember that all exceptions should be caught by reference to
 * avoid slicing:
 * 
 * try {
 *     throw SomeException();
 * } catch (const Exception& ex) {
 *     // do something
 * }
 * 
 * This class hierarchy also defines a stacktrace facility to make the
 * localization of errors easier.
 * 
 */


namespace Backtrace {
	class StackTrace;
}

namespace ExceptionLib {

	/**
	 * Classes para forçar a implementação de alguns métodos na classe mais concreta.
	 *
	 *
	 */

	class Exception;
	class ExceptionBase;


	template<class Ex>
	class ExceptionFactory {
	public:
		typedef Ex ExceptionType;

		static void raise(const ExceptionBase* ex) {
			const ExceptionType* myException = dynamic_cast<const ExceptionType*>(ex);
			if (myException) {
				throw *myException;
			}
			throw std::logic_error("erro no uso de excecoes");
		}


		static ExceptionBase* clone(const ExceptionBase* ex) {
			const ExceptionType* myException = dynamic_cast<const ExceptionType*>(ex);
			if (myException) {
				// chama o construtor de copia
				return new ExceptionType(*myException);
			}
			return NULL;
		}
	};



	class ExceptionBase: public BaseExceptionType {
	public:

		typedef void (*raiser)(const ExceptionBase*);
		typedef ExceptionBase* (*cloner)(const ExceptionBase*);

		// Esse construtor é template para evitar o trabalho do programador
		template <class Ex>
		ExceptionBase(
				Ex*,
				bool enableTrace,
				const std::string& what = "",
				// se nested for diferente de null, uma copia é feita com o clone
				const ExceptionBase* nested = NULL
					  )
			: m_raiser(ExceptionFactory<Ex>::raise)
			, m_cloner(ExceptionFactory<Ex>::clone)
			, st(NULL)
			, m_what(what)
			, m_nested(NULL)
		{
			setup(enableTrace, nested);
		}

		~ExceptionBase() throw();

		ExceptionBase(const ExceptionBase&);

		virtual void raise() const;

		virtual ExceptionBase* clone() const;

		virtual const char* what() const throw();

		const ExceptionBase* nested() const;


		/* Returns the stacktrace of the context where the exception was thrown.
		 * The information available is totally platform and compiler dependant,
		 * and might not be implemented for your platform. In this case, the method
		 * returns "stacktrace not implemented". Stacktraces can also be disabled (see below)
		 * and in that case the return value is "stacktrace deactivated". As the
		 * construction of stacktraces may be expensive and unecessary for some classes
		 * the construction of the stacktrace can be disabled using the second argument
		 * to the constructor. In this case, the method returns "stacktrace not available"
		 */
		std::string stacktrace() const;

	private:
		ExceptionBase();
		ExceptionBase& operator=(const ExceptionBase&);

		raiser m_raiser;
		cloner m_cloner;

		mutable ::Backtrace::StackTrace * st;

		std::string m_what;
		ExceptionBase* m_nested;

		void setup(bool enableTrace, const ExceptionBase* nested);
	};


  /**
   * Classe base de exceções. Tem métodos para obter a mensagem de erro,
   * se houver, e a pilha de execução que resultou na exceção.
   */

  class Exception : public virtual ExceptionBase {
  private:
      Exception& operator=(const Exception&);
  public:
	  Exception(std::string errorMsg = "Exception", const Exception* nested = NULL, bool trace = true);
      /* Exceptions should always be caught by reference, but in case you forget,
       * the copy constructor is defined and does the right thing
       */
      Exception(const Exception&);
	  virtual ~Exception() throw () {}
  };



  class IOException : public Exception {
  public:
	  IOException(std::string errorMsg = "IOException", const Exception* nested = NULL, bool trace = true )
		  : ExceptionBase(this, trace, errorMsg, nested) {}
	  IOException(const IOException& that) : ExceptionBase(that), Exception(that) {}
      virtual ~IOException() throw () {}
  };

  class InvalidStateException : public Exception {
  public:
	  InvalidStateException(std::string errorMsg = "InvalidStateException", const Exception* nested = NULL, bool trace = true )
		  : ExceptionBase(this, trace, errorMsg, nested) {}
	  InvalidStateException(const InvalidStateException& that) : ExceptionBase(that), Exception(that) {}
	  virtual ~InvalidStateException() throw () {}
  };

  class AbortException : public Exception {
  public:
	  AbortException(std::string errorMsg = "AbortException", const Exception* nested = NULL, bool trace = true )
		  : ExceptionBase(this, trace, errorMsg, nested) {}
	  AbortException(const AbortException& that) : ExceptionBase(that), Exception(that) {}
	  virtual ~AbortException() throw () {}
  };


  /* This class is intended as a base class, don't throw it directly.
   * Take a look at the exceptions below or derive your on class.
   */
  class ProgrammingError : public Exception {
  public:
      /* Always enable stacktrace for this */
	  ProgrammingError(std::string errorMsg = "Programming Error", const Exception* nested = NULL)
		  : ExceptionBase(this, true, errorMsg, nested) {}
	  ProgrammingError(const ProgrammingError& that) : ExceptionBase(that), Exception(that) {}
      virtual ~ProgrammingError() throw () {}
  };


  class IllegalArgumentException : public ProgrammingError {
  public:
	  IllegalArgumentException(std::string errorMsg = "IllegalArgumentException", const Exception* nested = NULL)
		: ExceptionBase(this, true, errorMsg, nested) {}
	  IllegalArgumentException(const IllegalArgumentException& that) : ExceptionBase(that), ProgrammingError(that) {}
      virtual ~IllegalArgumentException() throw () {}
  };

  class SegmentationFault : public ProgrammingError {
  public:
	  SegmentationFault(std::string errorMsg = "SegmentationFault", const Exception* nested = NULL)
		  : ExceptionBase(this, true, errorMsg, nested) {}
	  SegmentationFault(const SegmentationFault& that) : ExceptionBase(that), ProgrammingError(that) {}
      virtual ~SegmentationFault() throw () {}
  };

  class IllegalInstruction : public ProgrammingError {
  public:
	  IllegalInstruction(std::string errorMsg = "IllegalInstruction", const Exception* nested = NULL)
		  : ExceptionBase(this, true, errorMsg, nested) {}
	  IllegalInstruction(const IllegalInstruction& that) : ExceptionBase(that), ProgrammingError(that) {}
      virtual ~IllegalInstruction() throw () {}
  };

  class FloatingPointException : public ProgrammingError {
  public:
	  FloatingPointException(std::string errorMsg = "FloatingPointException", const Exception* nested = NULL)
		  : ExceptionBase(this, true, errorMsg, nested) {}
	  FloatingPointException(const FloatingPointException& that) : ExceptionBase(that), ProgrammingError(that) {}
      virtual ~FloatingPointException() throw () {}
  };


  /* The construction of stacktraces might be expensive and useless because
    * you may not be able to see anything except binary addresses depending on
    * your compilation flags. In this case, you can do something like this:
    * #ifndef DEBUG
    *     exception::Exception::stacktraceEnabled(false);
    * #endif
    * 
    * with a stack depth of 10, the treatment of exceptions is about 25% faster
    * if you disable stacktraces
    */
  void stacktraceEnabled(bool enable);

  /* Enable global error handling. This function will overwite any handlers for
   * SIGSEGV, SIGFPE, SIGILL and SIGBUS. The C++ terminate handler will also
   * be overwritten. You don't need to call this method if Qt support is enabled
   */
  void init(char *argv0);
}

inline std::ostream& operator<< (std::ostream& o, const ExceptionLib::Exception& e)
{
   return o << e.what();
}

#endif /* EXCEPTION_H_ */
