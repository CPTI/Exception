#include "BacktraceTest.h"

#include "TestSuite.h"

REGISTER_TEST_CLASS(BacktraceTest)

#include "BackTrace.h"
#include "StackAddressLoader.h"
#include "DebugSymbolLoader.h"
#include <iostream>
using namespace std;
static const int STACK_DEPTH = 20;
#ifdef __GNUC__
// O asm é bom para esse teste porque impede o inline e a reodenacao
#define GET_CURRENT_ADDR(var) __asm__ volatile ("1: mov $1b, %0" : "=r" (var));
#else
#define GET_CURRENT_ADDR(var) var = (void*)~0;
#endif

void level5(int* eff, Backtrace::StackFrame* stack, void** vstack)
{
	*eff = Backtrace::getPlatformStackLoader().getStack(STACK_DEPTH, stack);
	GET_CURRENT_ADDR(vstack[0]);
}

void level4(int* eff, Backtrace::StackFrame* stack, void** vstack)
{
	level5(eff, stack, vstack);
	GET_CURRENT_ADDR(vstack[1]);
}

void level3(int* eff, Backtrace::StackFrame* stack, void** vstack)
{
	level4(eff, stack, vstack);
	GET_CURRENT_ADDR(vstack[2]);
}

void level2(int* eff, Backtrace::StackFrame* stack, void** vstack)
{
	level3(eff, stack, vstack);
	GET_CURRENT_ADDR(vstack[3]);
}

void level1(int* eff, Backtrace::StackFrame* stack, void** vstack)
{
	level2(eff, stack, vstack);
	GET_CURRENT_ADDR(vstack[4]);
}

BacktraceTest::BacktraceTest() :
	QObject(NULL)
{
	Backtrace::initializeExecutablePath(qApp->applicationFilePath().toAscii().data());
}


void BacktraceTest::testBacktrace()
{
	void* start[5];
	Backtrace::StackFrame middle[STACK_DEPTH];
	void* end[5];
	int eff = 0;

	start[0] = (void*)level5;
	start[1] = (void*)level4;
	start[2] = (void*)level3;
	start[3] = (void*)level2;
	start[4] = (void*)level1;

	const char* names[] = {
		"level5(int*, Backtrace::StackFrame*, void**)",
		"level4(int*, Backtrace::StackFrame*, void**)",
		"level3(int*, Backtrace::StackFrame*, void**)",
		"level2(int*, Backtrace::StackFrame*, void**)",
		"level1(int*, Backtrace::StackFrame*, void**)"
	};

	level1(&eff, middle, end);

	for (int i = 0; i < eff; ++i) {
		QVERIFY(middle[i].imageFile.empty() || QFile::exists(QString::fromStdString(middle[i].imageFile)));
	}

	eff = std::min(eff, 5) ;

	for (int i = 0; i < eff; ++i) {
		QVERIFY( start[i] <= middle[i].addr );
		QVERIFY( middle[i].addr <= end[i] );
        if (!middle[i].function.empty()) {
            QCOMPARE(middle[i].function, names[i]);
        }
	}
}


void BacktraceTest::testBacktraceDebugInfo()
{
	Backtrace::StackFrame middle[STACK_DEPTH];
	void* end[5];
	const char* names[] = {
		"level5(int*, Backtrace::StackFrame*, void**)",
		"level4(int*, Backtrace::StackFrame*, void**)",
		"level3(int*, Backtrace::StackFrame*, void**)",
		"level2(int*, Backtrace::StackFrame*, void**)",
		"level1(int*, Backtrace::StackFrame*, void**)"
	};
#ifdef DEBUG
	int lines[][2] = {
		{20,24},
		{26,30},
		{32,36},
		{39,42},
		{44,48}
	};
#endif
	int eff = 0;

	level1(&eff, middle, end);

	eff = std::min(eff, 5);

	Backtrace::getPlatformDebugSymbolLoader().findDebugInfo(middle, eff);

	for (int i = 0; i < eff; ++i) {
		QCOMPARE(middle[i].function, names[i]);

		QString imageFile  = QString::fromStdString(middle[i].imageFile);

        QVERIFY(!imageFile.isEmpty() && QFile::exists(imageFile));
        QVERIFY(imageFile.contains("unit_test"));
	}

#ifdef DEBUG
	// Esse loop pode falhar se nao houver simbolos de debug
	for (int i = 0; i < eff; ++i) {
		QString sourceFile = QString::fromStdString(middle[i].sourceFile);
		QVERIFY(sourceFile.endsWith("unit_test/src/BacktraceTest.cpp"));
		QVERIFY(middle[i].line >= lines[i][0]);
		QVERIFY(middle[i].line <= lines[i][1]);
	}
#endif
}
