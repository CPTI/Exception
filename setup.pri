# exportacao e configuracoes de build

INCLUDEPATH += $$PWD/src

win32 {
    win32-g++ {
        EXCEPTION_LIBS += -lexception -limagehlp
	bfd {
            EXCEPTION_LIBS += -lbfd -liberty
        }
    } else {
        EXCEPTION_LIBS += exception.lib imagehlp.lib
    }
}


unix {
    EXCEPTION_LIBS += -lexception
    bfd {
        EXCEPTION_LIBS += -lbfd -ldl -lz -liberty
    }
}
