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
#include <string>
#include <vector>

#include "BackTrace.h"


using namespace std;

namespace ExceptionLib {

	static bool stackEnabled = true;


	void terminate_handler()
	{
		cerr << "Caught unhandled exception" << endl;

		if (stackEnabled && ::Backtrace::backtraceSupported()) {
			::Backtrace::StackTrace* trace = ::Backtrace::trace();
			cerr << trace->getTrace() << endl;
			delete trace;
		}
	}

	void init(char *argv0)
	{
		::Backtrace::initialize(argv0);
		set_terminate(terminate_handler);
	}

	ExceptionBase::ExceptionBase(const ExceptionBase& that)
		: m_raiser(that.m_raiser)
		, m_cloner(that.m_cloner)
		, st(that.st)
		, m_what(that.m_what)
		, m_nested(that.m_nested->clone())
	{
		if (st) {
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

	Exception* ExceptionBase::clone() const
	{
		return m_cloner(this);
	}

	const char* ExceptionBase::what() const throw()
	{
		return m_what.c_str();
	}

	const Exception* ExceptionBase::nested() const
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

	void ExceptionBase::setup(bool enableTrace)
	{
		if (m_nested) {
			m_nested = m_nested->clone();
		}
		if (stackEnabled && enableTrace) {
			st = ::Backtrace::trace();
		}
	}


	Exception::Exception(std::string errorMsg, bool trace) :
		ExceptionBase(this, trace, errorMsg)
	{}

	Exception::Exception(const Exception& that) :
		ExceptionBase(that)
	{	}



	void stacktraceEnabled(bool enable) {
		stackEnabled = enable;
	}

}
