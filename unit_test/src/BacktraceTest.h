#ifndef BACKTRACETEST_H
#define BACKTRACETEST_H

#include <QObject>

class BacktraceTest : public QObject
{
    Q_OBJECT
public:
	BacktraceTest();


private slots:
	void testBacktrace();
	void testBacktraceDebugInfo();
};

#endif // BACKTRACETEST_H
