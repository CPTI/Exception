################################################################################
#
#   Configurações do projeto
#

TARGET = exception_tests

!include(../../pri/setup.pri) { error("Erro incluindo o subrepo pri -- $$TARGET") }
!include(../../test_lib/setup.pri) { subrepoErrorMessage(test_lib) }

TEMPLATE = lib

CONFIG += qtestlib static
QT += network
unix:LIBS += -ldl

SRC = ./src
SRC_EXCEPTION = ../project/src

INCLUDEPATH += \
	$$SRC \
	$$SRC_EXCEPTION \


DEPENDPATH += $$INCLUDEPATH

################################################################################
#
# Includes
#
HEADERS += \
        $$SRC/BacktraceTest.h \
	$$SRC/ExceptionTest.h \


SOURCES += \
        $$SRC/BacktraceTest.cpp \
	$$SRC/ExceptionTest.cpp \


################################################################################
#
#   Exibe o ambiente configurado
#
message(**************** EXCEPTION UNIT_TEST ****************)
message(QMAKESPEC: $$(QMAKESPEC))
message(CONFIG: $$CONFIG)
message(DEFINES: $$DEFINES)
message(QMAKE_CFLAGS: $$QMAKE_CFLAGS)
message(QMAKE_CXXFLAGS: $$QMAKE_CXXFLAGS)
message(LIBS: $$LIBS)
message()


