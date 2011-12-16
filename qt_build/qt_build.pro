MAKEFILE = Makefile.foo


Makefile.target = Makefile
Makefile.commands = cmake .. -DUSE_QT=ON #-DCMAKE_VERBOSE_MAKEFILE=ON

all.commands = make
all.depends = Makefile
all.CONFIG = phony

TARGET = ///
QMAKE_EXTRA_TARGETS += Makefile all
