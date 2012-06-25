#include "Exception.h"

#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <vector>

#include "BackTrace.h"


using namespace std;


static bool initialized = false;


namespace ExceptionLib {

	static bool stackEnabled = true;


	void terminate_handler()
	{
#ifdef __GNUC__
		static bool tried_throw = false;

		try {
			if (!tried_throw) {
				tried_throw = true;
				throw;
			}
			cerr << "no active exception" << endl;
		}
		catch (const ExceptionLib::Exception& ex) {
			cerr << "Caught an Exception: what: " << ex.what() << endl;
			string trace = ex.stacktrace();
			cerr << "stacktrace: " << endl << trace << endl;

			const ExceptionBase * eptr = &ex;
			while(eptr->nested()) {
				eptr = eptr->nested();
				cerr << "Nested exception: what: " << eptr->what() << endl;
				trace = eptr->stacktrace();
				cerr << "stacktrace: " << endl << trace << endl;
			}

		}
		catch (const std::exception &ex) {
			cout << "Caught an std::exception. what(): " << ex.what() << endl;
		}
		catch (...) {
			std::cout << "Caught unknown exception" << std::endl;
		}
#else
		cerr << "Caught unhandled exception" << endl;
#endif
	}

	void init(char *argv0)
	{
		::Backtrace::initialize(argv0);
		set_terminate(terminate_handler);
		initialized = true;
	}

	ExceptionBase::ExceptionBase(const ExceptionBase& that)
		: BaseExceptionType()
		, m_raiser(that.m_raiser)
		, m_cloner(that.m_cloner)
		, st(NULL)
		, m_what(that.m_what)
		, m_nested(NULL)
	{
		if (that.m_nested) {
			m_nested = that.m_nested->clone();
		}
		if (that.st) {
			st = that.st;
			st->increaseCount();
		}
	}

	ExceptionBase::~ExceptionBase() throw ()
	{
		try {
			if (m_nested) {
				delete m_nested;
			}
			if (st) st->decreaseCount();
			st = NULL;
		} catch (...) {}
	}


	void ExceptionBase::raise() const
	{
		m_raiser(this);
	}

	ExceptionBase* ExceptionBase::clone() const
	{
		return m_cloner(this);
	}

	const char* ExceptionBase::what() const throw()
	{
		return m_what.c_str();
	}

	const ExceptionBase* ExceptionBase::nested() const
	{
		return m_nested;
	}

	std::string ExceptionBase::stacktrace() const
	{
		if (st) {
			return st->getTrace();
		} else if (!stackEnabled) {
			return "stacktrace deactivated";
		} else {
			return "stacktrace not available";
		}
	}

	void ExceptionBase::setup(bool enableTrace, const ExceptionBase* nested)
	{
		if (nested) {
			m_nested = nested->clone();
		}
		if (stackEnabled && enableTrace) {
			st = ::Backtrace::trace();
		}
	}

	Exception::Exception(const Exception& that) :
		ExceptionBase(that)
	{	}



	void stacktraceEnabled(bool enable) {
		stackEnabled = enable;
	}

}
