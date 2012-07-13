#ifndef EXCEPTION_TEST_H
#define EXCEPTION_TEST_H


#include <QObject>

// A classe devia se chamar ExceptionTest, mas por algum motivo isso
// dá conflito com o moc.

class MyExceptionTest
	: public QObject
{
	Q_OBJECT

public:
	MyExceptionTest();
	virtual ~MyExceptionTest() {}

private slots:
	void testThrow();
};


#endif
