TARGET = exception


TEMPLATE = lib
CONFIG += static


SRC  = ./src

HEADERS += \
	$$SRC/Exception.h \
	$$SRC/Backtrace.h \

SOURCES += \
	$$SRC/Exception.h \
	$$SRC/Backtrace.h \

win32 {
	HEADERS += \
		$$SRC/windows/StackWalker.h \

	SOURCES += \
		$$SRC/windows/StackWalker.cpp \
		$$SRC/windows/BackTrace.cpp \

}

unix {
	macx {
		SOURCES += \
			$$SRC/default/BackTrace.cpp \

	} else {
		SOURCES += \
			$$SRC/linux/BackTrace.cpp \

	}
}

