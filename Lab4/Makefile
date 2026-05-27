CC = g++
CFLAGS = -Wall -std=c++17 -I. -Ilib
LDFLAGS = -Wl,--subsystem,console

MAIN_SRC = main.cpp
TESTS_SRC = tests.cpp

COMMON_SRC = streams/FileLineReadOnlyStream.cpp

MAIN_TARGET = program
TESTS_TARGET = tests

.PHONY: all clean run run_tests

all: $(MAIN_TARGET) $(TESTS_TARGET)

$(MAIN_TARGET): $(MAIN_SRC) $(COMMON_SRC)
	$(CC) $(CFLAGS) $(MAIN_SRC) $(COMMON_SRC) -o $(MAIN_TARGET) $(LDFLAGS)

$(TESTS_TARGET): $(TESTS_SRC) $(COMMON_SRC)
	$(CC) $(CFLAGS) $(TESTS_SRC) $(COMMON_SRC) -o $(TESTS_TARGET) $(LDFLAGS)

run: $(MAIN_TARGET)
	./$(MAIN_TARGET)

run_tests: $(TESTS_TARGET)
	./$(TESTS_TARGET)

clean:
	rm -f $(MAIN_TARGET) $(TESTS_TARGET) *.o *.exe
