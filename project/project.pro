TARGET = exception

!include(../setup.pri) { includeErrorMessage(../setup.pri) }
!include(../../pri/setup.pri) { error("Erro incluindo o subrepo pri -- $$TARGET") }

!include(../../unpackers/setup.pri) { subrepoErrorMessage(unpackers) }
!include(../../exception/setup.pri) { subrepoErrorMessage(exception) }
!include(../../type_utils/setup.pri) { subrepoErrorMessage(type_utils) }
!include(../../util/setup.pri) { subrepoErrorMessage(util) }


TEMPLATE = lib
CONFIG += static

CONFIG += exception rtti


SRC  = ./src


HEADERS += \
	$$SRC/BackTrace.h \
        $$SRC/DebugSymbolLoader.h \
        $$SRC/Demangling.h \
        $$SRC/Error.h \
        $$SRC/Exception.h \
        $$SRC/Logger.h \
        $$SRC/Software.h \
        $$SRC/StackAddressLoader.h \
        $$SRC/svector.h \
        $$SRC/VectorIO.h \


SOURCES += \
        $$SRC/BackTracePlatIndep.cpp \
        $$SRC/Demangling.cpp \
        $$SRC/Error.cpp \
        $$SRC/Exception.cpp \
        $$SRC/Logger.cpp \
        $$SRC/VectorIO.cpp \


win32 {
	DEFINES -= UNICODE


        SOURCES += \
		$$SRC/windows/BackTrace.cpp \
                $$SRC/windows/StackLoader.cpp \

        bfd {
            SOURCES += $$SRC/bfd/DebugSymbolLoader.cpp
        } else {
            SOURCES += $$SRC/default/DebugSymbolLoader.cpp
        }

}

unix {
	macx {
		SOURCES += \
                        $$SRC/default/StackLoader.cpp \
                        $$SRC/default/DebugSymbolLoader.cpp \

	} else {
		SOURCES += \
			$$SRC/linux/BackTrace.cpp \
                        $$SRC/linux/StackLoader.cpp \

                bfd {
                        SOURCES += \
                            $$SRC/bfd/DebugSymbolLoader.cpp
                } else {
                        SOURCES += \
                            $$SRC/linux/DebugSymbolLoader.cpp
                }
	}
}

QMAKE_CFLAGS -= -O2

QMAKE_CXXFLAGS -= -O2

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

                #QMAKE_CFLAGS += -O2
		QMAKE_CFLAGS -= -O0
                #QMAKE_CXXFLAGS += -O2
		QMAKE_CXXFLAGS -= -O0
	}
}










