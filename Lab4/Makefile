CC = g++
CFLAGS = -Wall -std=c++17 -I. -Ilib
LDFLAGS = -Wl,--subsystem,console

MAIN_SRC = main.cpp
TESTS_SRC = tests.cpp

COMMON_SRC = streams/FileLineReadOnlyStream.cpp streams/FileLineWriteOnlyStream.cpp

MAIN_TARGET = program.exe
TESTS_TARGET = tests.exe

.PHONY: all main tests build_main build_tests run_main run_tests clean

all: build_main build_tests

build_main: $(MAIN_TARGET)

build_tests: $(TESTS_TARGET)

$(MAIN_TARGET): $(MAIN_SRC) $(COMMON_SRC)
	$(CC) $(CFLAGS) $(MAIN_SRC) $(COMMON_SRC) -o $(MAIN_TARGET) $(LDFLAGS)

$(TESTS_TARGET): $(TESTS_SRC) $(COMMON_SRC)
	$(CC) $(CFLAGS) $(TESTS_SRC) $(COMMON_SRC) -o $(TESTS_TARGET) $(LDFLAGS)

main: $(MAIN_TARGET)
	.\$(MAIN_TARGET)

tests: $(TESTS_TARGET)
	.\$(TESTS_TARGET)

run_main: main

run_tests: tests

clean:
	rm -f $(MAIN_TARGET) $(TESTS_TARGET) *.o *.exe
