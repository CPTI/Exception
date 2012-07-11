# exportacao e configuracoes de build

INCLUDEPATH += $$PWD/src

win32 {
    win32-g++ {
	LIBS += -lexception -limagehlp
	bfd {
	    LIBS += -lbfd -liberty
        }
    } else {
	LIBS += exception.lib imagehlp.lib
    }
    POST_TARGETDEPS += $$BUILD_DIR/libexception.lib
}


unix {
    LIBS += -lexception
    bfd {
        LIBS += -lbfd -ldl -lz -liberty
    }

    POST_TARGETDEPS += $$BUILD_DIR/libexception.a
}
