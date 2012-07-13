# exportacao e configuracoes de build

INCLUDEPATH += $$PWD/project/src

win32 {
	win32-g++ {
		EXCEPTION_LIBS += -lexception -limagehlp
		bfd {
			EXCEPTION_LIBS += -lbfd -liberty
		}
		POST_TARGETDEPS += $$BUILD_DIR/libexception.a
		EXCEPTION_TEST_LIBS = -lexception_tests
	} else {
		EXCEPTION_LIBS += exception.lib imagehlp.lib
		POST_TARGETDEPS += $$BUILD_DIR/libexception.lib
		EXCEPTION_TEST_LIBS = exception_tests.lib
	}
}


unix {
	EXCEPTION_LIBS += -lexception
	bfd {
		EXCEPTION_LIBS += -lbfd -ldl -lz -liberty
	}

	POST_TARGETDEPS += $$BUILD_DIR/libexception.a
	EXCEPTION_TEST_LIBS = -Wl,--whole-archive -lexception_tests -Wl,--no-whole-archive
}

EXCEPTION_TEST_LIBS += $$EXCEPTION_LIBS
