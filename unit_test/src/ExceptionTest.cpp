
#include "ExceptionTest.h"


#include "TestSuite.h"
#include "Exception.h"
#include "BackTrace.h"
#include <stdexcept>
#include <exception>
#include <cxxabi.h>

#include <QTime>
#include <QtTest/QtTest>

#include <iostream>
using namespace std;


REGISTER_TEST_CLASS(MyExceptionTest)


MyExceptionTest::MyExceptionTest()
: QObject()
{

}


void do_throw_1()
{
	throw std::logic_error("lalala");
}

void MyExceptionTest::testThrowStdExcept()
{
	try {
		do_throw_1();
		QFAIL("expected exception throw");
	} catch(const std::exception& ex) {
		size_t depth = 0;
		const Backtrace::StackFrame* frames = ExceptionLib::getBT(ex, &depth, true);

		QCOMPARE(ex.what(), "lalala");

		if (depth < 2) {
			std::cout << "warning: no stack" << std::endl;
		}

		if (depth > 0) {
			QCOMPARE(frames[0].function, "do_throw_1()");
		}
		if (depth > 1) {
			QCOMPARE(frames[1].function, "MyExceptionTest::testThrowStdExcept()");
		}
	} catch(...) {
		QFAIL("expected std::exception");
	}
}

void do_throw_2()
{
	throw ExceptionLib::IOException("lalala");
}

void MyExceptionTest::testThrowExcept()
{
	try {
		do_throw_2();
		QFAIL("expected exception throw");
	} catch(const ExceptionLib::Exception& ex) {
		QCOMPARE(ex.what(), "lalala");
		/* caso generico */ {
			size_t depth = 0;
			const Backtrace::StackFrame* frames = ExceptionLib::getBT(ex, &depth, true);


			if (depth < 2) {
				std::cout << "warning: no stack" << std::endl;
			}

			if (depth > 0) {
				QCOMPARE(frames[0].function, "do_throw_2()");
			}
			if (depth > 1) {
				QCOMPARE(frames[1].function, "MyExceptionTest::testThrowExcept()");
			}
		}
		/* caso especifico de ExceptionLib::Exception */ {
			Backtrace::StackTrace* trace = ex.stacktrace();
			QVERIFY(trace != NULL);
			std::vector<Backtrace::StackFrame>& frames = trace->getFrames();

			if (frames.size() < 2) {
				std::cout << "warning: no stack" << std::endl;
			}

			if (frames.size() > 0) {
				QCOMPARE(frames[0].function, "do_throw_2()");
			}
			if (frames.size() > 1) {
				QCOMPARE(frames[1].function, "MyExceptionTest::testThrowExcept()");
			}
		}
	} catch(...) {
		QFAIL("ExceptionLib::Exception");
	}
}
