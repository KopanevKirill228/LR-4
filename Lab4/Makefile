CC = g++
CFLAGS = -Wall -std=c++17 -I. -Ilib
LDFLAGS = -Wl,--subsystem,console

MAIN_SRC = main.cpp
MAIN_TARGET = program

TESTS_SRC = tests.cpp
SOLVERS_TEST_SRC = SolversTest.cpp

TESTS_TARGET = tests
SOLVERS_TEST_TARGET = solvers_tests

.PHONY: all clean run run_tests run_solvers

all: $(MAIN_TARGET) $(TESTS_TARGET) $(SOLVERS_TEST_TARGET)

$(MAIN_TARGET): $(MAIN_SRC)
	$(CC) $(CFLAGS) $(MAIN_SRC) -o $(MAIN_TARGET) $(LDFLAGS)

$(TESTS_TARGET): $(TESTS_SRC)
	$(CC) $(CFLAGS) $(TESTS_SRC) -o $(TESTS_TARGET) $(LDFLAGS)

$(SOLVERS_TEST_TARGET): $(SOLVERS_TEST_SRC)
	$(CC) $(CFLAGS) $(SOLVERS_TEST_SRC) -o $(SOLVERS_TEST_TARGET) $(LDFLAGS)

run: $(MAIN_TARGET)
	./$(MAIN_TARGET)

run_tests: $(TESTS_TARGET)
	./$(TESTS_TARGET)

run_solvers: $(SOLVERS_TEST_TARGET)
	./$(SOLVERS_TEST_TARGET)


clean:
	rm -f $(MAIN_TARGET) $(TESTS_TARGET) $(SOLVERS_TEST_TARGET) *.o *.exe