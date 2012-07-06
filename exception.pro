
# inclui o config.pri se existir
!include(../config.pri) {
    message("A inclusao do arquivo config.pri eh opcional")
}

TARGET = exception




TEMPLATE = lib
CONFIG += static

CONFIG += exception rtti


SRC  = ./src

INCLUDEPATH += \
        $$SRC \


HEADERS += \
	$$SRC/BackTrace.h \
        $$SRC/Error.h \
        $$SRC/Exception.h \
        $$SRC/IStackAddressLoader.h \
        $$SRC/Software.h \

SOURCES += \
        $$SRC/Error.cpp \
        $$SRC/Exception.cpp \


win32 {
	DEFINES -= UNICODE
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
                        $$SRC/linux/StackLoader.cpp \

	}
}

debug {
	DEFINES -= NDEBUG
	DEFINES += DEBUG
	unix {
		QMAKE_CFLAGS += -g
		QMAKE_CXXFLAGS += -g

		QMAKE_CFLAGS -= -O2
		QMAKE_CFLAGS += -O0
		QMAKE_CXXFLAGS -= -O2
		QMAKE_CXXFLAGS += -O0
	}
}

release {
	DEFINES += NDEBUG
	DEFINES -= DEBUG
	unix {
		QMAKE_CFLAGS -= -g
		QMAKE_CXXFLAGS -= -g

		QMAKE_CFLAGS += -O2
		QMAKE_CFLAGS -= -O0
		QMAKE_CXXFLAGS += -O2
		QMAKE_CXXFLAGS -= -O0
	}
}





