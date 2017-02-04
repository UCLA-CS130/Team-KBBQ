CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
COVFLAGS=
SERVER_CLASSES=config_parser.cc Webserver.h Webserver_main.cc

GTEST_DIR=googletest/googletest
GTEST_FLAGS=-std=c++0x -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread
GTEST_CLASSES=${GTEST_DIR}/src/gtest_main.cc libgtest.a

all: Webserver config_parser Webserver_test config_parser_test

Webserver: $(SERVER_CLASSES)
	$(CXX) -o $@ $^ $@.cc $(CXXFLAGS) -lboost_system

Webserver_test: Webserver.cc config_parser.cc $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $@.cc $(GTEST_FLAGS) $(COVFLAGS) -lboost_system
 
config_parser: config_parser_main.cc
	$(CXX) -o $@ $^ $@.cc $(CXXFLAGS)

config_parser_test: config_parser.cc $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $@.cc $(GTEST_FLAGS) $(COVFLAGS)

HttpRequest_test: HttpRequest.cc $(GTEST_CLASSES) 
	$(CXX) -o $@ $^ $@.cc $(GTEST_FLAGS) $(COVFLAGS) -lboost_system

libgtest.a: gtest-all.o
	ar -rv $@ $^

gtest-all.o: ${GTEST_DIR}/src/gtest-all.cc
	$(CXX) $(GTEST_FLAGS) -c ${GTEST_DIR}/src/gtest-all.cc

coverage: COVFLAGS += -fprofile-arcs -ftest-coverage
coverage: Webserver_test config_parser_test
	./Webserver_test && gcov -r Webserver.cc; ./config_parser_test && gcov -r config_parser.cc

test:
	python3 integration_test.py

clean:
	rm -rf *.o *.a *~ *.gch *.swp *.dSYM *.gcno *.gcda *.gcov Webserver Webserver_test HttpRequest_test config_parser config_parser_test *.tar.gz

.PHONY: all clean test coverage