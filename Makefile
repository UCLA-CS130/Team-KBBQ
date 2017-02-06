CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
COVFLAGS=
SERVER_CLASSES=config_parser.cc Webserver.h Webserver_main.cc HttpResponse.h HttpResponse.cc HttpRequest.h HttpRequest.cc

GTEST_DIR=googletest/googletest
GMOCK_DIR=googletest/googlemock
GTEST_FLAGS=-std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread
GMOCK_FLAGS=-std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -isystem ${GMOCK_DIR}/include -I${GMOCK_DIR} -pthread
GTEST_CLASSES=${GTEST_DIR}/src/gtest_main.cc libgtest.a
GMOCK_CLASSES=${GMOCK_DIR}/src/gmock_main.cc libgmock.a

all: Webserver config_parser Webserver_test config_parser_test

Webserver: $(SERVER_CLASSES)
	$(CXX) -o $@ $^ $@.cc $(CXXFLAGS) -lboost_system

Webserver_test: Webserver.cc config_parser.cc HttpRequest.cc HttpResponse.cc $(GMOCK_CLASSES)
	$(CXX) -o $@ $^ $@.cc $(GMOCK_FLAGS) $(COVFLAGS) -lboost_system

config_parser: config_parser_main.cc
	$(CXX) -o $@ $^ $@.cc $(CXXFLAGS)

config_parser_test: config_parser.cc $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $@.cc $(GTEST_FLAGS) $(COVFLAGS)

HttpRequest_test: HttpRequest.cc $(GTEST_CLASSES) 
	$(CXX) -o $@ $^ $@.cc $(GTEST_FLAGS) $(COVFLAGS) -lboost_system

HttpResponse_test: HttpResponse.cc $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $@.cc $(GTEST_FLAGS) $(COVFLAGS) -lboost_system

libgtest.a: gtest-all.o
	ar -rv $@ $^

gtest-all.o: ${GTEST_DIR}/src/gtest-all.cc
	$(CXX) $(GTEST_FLAGS) -c ${GTEST_DIR}/src/gtest-all.cc

libgmock.a: gmock-all.o gtest-all2.o
	ar -rv $@ $^

gtest-all2.o: ${GTEST_DIR}/src/gtest-all.cc
	$(CXX) $(GMOCK_FLAGS) -c ${GTEST_DIR}/src/gtest-all.cc -o gtest-all2.o

gmock-all.o: ${GMOCK_DIR}/src/gmock-all.cc
	$(CXX) $(GMOCK_FLAGS) -c ${GMOCK_DIR}/src/gmock-all.cc

coverage: COVFLAGS += -fprofile-arcs -ftest-coverage
coverage: Webserver_test config_parser_test HttpRequest_test HttpResponse_test
	./Webserver_test && gcov -r Webserver.cc;
	./config_parser_test && gcov -r config_parser.cc;
	./HttpRequest_test && gcov -r HttpRequest.cc;
	./HttpResponse_test && gcov -r HttpResponse.cc;

test:
	python3 integration_test.py

clean:
	rm -rf *.o *.a *~ *.gch *.swp *.dSYM *.gcno *.gcda *.gcov Webserver config_parser *_test *.tar.gz

.PHONY: all clean test coverage
	
