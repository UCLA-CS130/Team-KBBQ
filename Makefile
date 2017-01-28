CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
GTEST_DIR=googletest/googletest
SERVERCLASSES=config_parser.cc Webserver.h Webserver_main.cc

all: Webserver Webserver_test config_parser config_parser_test

Webserver: $(SERVERCLASSES)
	$(CXX) -o $@ $^ $@.cc $(CXXFLAGS) -lboost_system

Webserver_test:
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread Webserver_test.cc Webserver.cc config_parser.cc -lboost_system ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o Webserver_test

config_parser: config_parser.cc config_parser_main.cc
	$(CXX) -o $@ $^ $(CXXFLAGS)

config_parser_test:
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread config_parser_test.cc config_parser.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o config_parser_test

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM Webserver Webserver_test config_parser config_parser_test *.tar.gz
