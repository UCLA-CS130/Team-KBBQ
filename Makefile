CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
SERVER_CLASSES=config_parser.cc Webserver.h Webserver_main.cc

GTEST_DIR=googletest/googletest
GTEST_FLAGS=-std=c++0x -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread
GTEST_CLASSES=${GTEST_DIR}/src/gtest_main.cc libgtest.a

all: Webserver config_parser Webserver_test config_parser_test

Webserver: $(SERVER_CLASSES)
	$(CXX) -o $@ $^ $@.cc $(CXXFLAGS) -lboost_system

Webserver_test: Webserver.cc config_parser.cc $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $@.cc $(GTEST_FLAGS) -lboost_system 
 
config_parser: config_parser.cc config_parser_main.cc
	$(CXX) -o $@ $^ $(CXXFLAGS)

config_parser_test: config_parser.cc $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $@.cc $(GTEST_FLAGS)

libgtest.a: gtest-all.o
	ar -rv $@ $^

gtest-all.o: ${GTEST_DIR}/src/gtest-all.cc
	$(CXX) $(GTEST_FLAGS) -c ${GTEST_DIR}/src/gtest-all.cc

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM Webserver Webserver_test config_parser config_parser_test *.tar.gz

.PHONY: all clean