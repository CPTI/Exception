TARGET = exception

!include(../../pri/setup.pri) { error("Erro incluindo o subrepo pri -- $$TARGET") }

TEMPLATE = lib
CONFIG += static

CONFIG += exception rtti

SRC  = ./src

INCLUDEPATH += ./src

HEADERS += \
	$$SRC/BackTrace.h \
        $$SRC/DebugSymbolLoader.h \
        $$SRC/Demangling.h \
        $$SRC/Error.h \
        $$SRC/Exception.h \
        $$SRC/Logger.h \
        $$SRC/LoggerFwd.h \
        $$SRC/Software.h \
        $$SRC/StackAddressLoader.h \
        $$SRC/str_conversion.h \
        $$SRC/string_format.h \
        $$SRC/svector.h \
        $$SRC/SymbolCache.h \
        $$SRC/VectorIO.h \
        $$SRC/MapUtils.h \
        $$SRC/VectorOf.h \
        $$SRC/ArrayPtr.h \
        $$SRC/NullType.h \
        $$SRC/ScopeGuard.h \
        $$SRC/Typelist.h \
        $$SRC/TypeManip.h \
        $$SRC/TypelistMacros.h \
        $$SRC/NotNull.h \


SOURCES += \
        $$SRC/BackTracePlatIndep.cpp \
        $$SRC/Demangling.cpp \
        $$SRC/Error.cpp \
        $$SRC/Exception.cpp \
        $$SRC/Logger.cpp \
        $$SRC/string_format.cpp \
        $$SRC/SymbolCache.cpp \
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



