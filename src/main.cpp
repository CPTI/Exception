#include "Exception.h"

using namespace ExceptionLib;
using namespace std;

void faulty_1(int depth) {
	if (depth <= 0)
		throw IOException("lalala");
	return faulty_1(depth-1);
}

void faulty_2(int depth) {
	int * f = 0;
	if (depth <= 0)
		*f = 666;
	return faulty_2(depth-1);
}


int main(int argc, char *argv[])
{
	init(argv[0]);
	//stacktraceEnabled(false);
	try { 
		faulty_1(10);
	} catch (const Exception& ex) {
		cout << ex.what() << endl;
		cout << ex.stacktrace() << endl;
	}
	return 0;
}
