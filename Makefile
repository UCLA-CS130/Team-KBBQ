CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -std=c++0x $(CXXOPTIMIZE)
GTEST_DIR=googletest/googletest
$SERVERCLASSES=

all: webserver config_parser config_parser_test

webserver: $(SERVERCLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cc

config_parser: config_parser.cc config_parser_main.cc
	$(CXX) -o $@ $^ $(CXXFLAGS)

config_parser_test: config_parser_test.cc config_parser.cc $(GTEST_DIR)/src/gtest_main.cc libgtest.a
	g++ -std=c++0x -isystem $(GTEST_DIR)/include -I$(GTEST_DIR) -pthread -c $(GTEST_DIR)/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++0x -isystem $(GTEST_DIR)/include -pthread $^ -o config_parser_test

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM webserver config_parser config_parser_test *.tar.gz
