# exportacao e configuracoes de build

INCLUDEPATH += $$PWD/project/src

win32 {
	win32-g++ {
		unit_tests {
		    LIBS += -Wl,--whole-archive -lexception_tests -Wl,--no-whole-archive
		    EXE_DEPS += $$BUILD_DIR/libexception_tests.a
		}
		LIBS += -lexception -limagehlp
		bfd {
			LIBS += -lbfd -liberty
		}
		EXE_DEPS += $$BUILD_DIR/libexception.a
	} else {
		unit_tests {
		    LIBS += exception_tests.lib
		    EXE_DEPS += $$BUILD_DIR/exception_tests.lib
		}
		LIBS += exception.lib imagehlp.lib
		EXE_DEPS += $$BUILD_DIR/exception.lib
	}
}


unix {
	unit_tests {	
    	    EXE_DEPS += $$BUILD_DIR/libexception_tests.a
	    LIBS += -Wl,--whole-archive -lexception_tests -Wl,--no-whole-archive
	}
	LIBS += -lexception
	bfd {
		LIBS += -lbfd -ldl -lz -liberty
	}

	EXE_DEPS += $$BUILD_DIR/libexception.a
}
