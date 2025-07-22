PROGRAMS = test-suite.exe

.SUFFIXES: .pc .exe

all: ${PROGRAMS}

test-suite.exe : test-suite.pc varchar-macros.h
	./scripts/debug-compile.sh $<

clean:
	rm -f *.exe *.o *.c *.lis *.log
