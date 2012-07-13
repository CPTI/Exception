# exportacao e configuracoes de build

INCLUDEPATH += $$PWD/src

win32 {
	win32-g++ {
		EXCEPTION_LIBS += -lexception -limagehlp
		bfd {
			EXCEPTION_LIBS += -lbfd -liberty
		}
		POST_TARGETDEPS += $$BUILD_DIR/libexception.a
	} else {
		EXCEPTION_LIBS += exception.lib imagehlp.lib
		POST_TARGETDEPS += $$BUILD_DIR/libexception.lib
	}
}


unix {
	EXCEPTION_LIBS += -lexception
	bfd {
		EXCEPTION_LIBS += -lbfd -ldl -lz -liberty
	}

	POST_TARGETDEPS += $$BUILD_DIR/libexception.a
}
