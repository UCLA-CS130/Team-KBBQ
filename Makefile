CXX=g++
CXXOPTIMIZE= -O2
MYSQL_LD= -lmysqlcppconn-static -lmysqlclient -ldl -lz -lc
LDFLAGS= $(MYSQL_LD) -static-libgcc -static-libstdc++ -Wl,-Bstatic -lboost_regex
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
COVFLAGS=
SERVER_CLASSES= $(wildcard src/*.cc)

SRC_DIR=src
TEST_DIR=test
GTEST_DIR=googletest/googletest
GMOCK_DIR=googletest/googlemock
MD_DIR=cpp-markdown
MD_INCL=-I$(MD_DIR)
MD_CLASSES=$(MD_DIR)/*.cpp

GTEST_FLAGS=-std=c++11 -isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include -pthread
GTEST_INCL=-I$(GTEST_DIR)
GMOCK_INCL=-I$(GTEST_DIR) -I$(GMOCK_DIR)

GTEST_CLASSES=libgtest.a
GMOCK_CLASSES=libgmock.a


all: Webserver Webserver_test config_parser_test \
	 server_status_tracker_test \
	 request_handler_test echo_handler_test static_file_handler_test not_found_handler_test reverse_proxy_handler_test

build: Dockerfile
	sudo docker build -t webserver.build .
	sudo docker run --rm webserver.build > Webserver.tar

deploy: Webserver.tar build Dockerfile.run
	rm -rf deploy
	mkdir deploy
	tar -xf Webserver.tar -C deploy
	cp Dockerfile.run deploy
	cp config deploy
	cp -r static_files deploy
	cp -r private_files deploy
	cp database_init.sql deploy
	cp actors.csv deploy
	cd deploy; \
	sudo docker build -f Dockerfile.run -t webserver.deploy .
	sudo docker save webserver.deploy | bzip2 | ssh -i "team-kbbq.pem" ec2-user@ec2-54-202-60-252.us-west-2.compute.amazonaws.com 'bunzip2 | docker load'
	ssh -i "team-kbbq.pem" ec2-user@ec2-54-202-60-252.us-west-2.compute.amazonaws.com -t 'docker stop $$(docker ps -a -q); docker run -d -t -p 80:2020 webserver.deploy; exit'
	
Webserver: $(SERVER_CLASSES) $(MD_CLASSES)
	$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS) $(MD_INCL) -lboost_system

Webserver_test: $(filter-out $(SRC_DIR)/Webserver_main.cc, $(SERVER_CLASSES)) $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $(TEST_DIR)/$@.cc -I$(SRC_DIR) $(GTEST_FLAGS) $(COVFLAGS) $(LDFLAGS) -lboost_system

config_parser_test: $(SRC_DIR)/config_parser.cc $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $(TEST_DIR)/$@.cc -I$(SRC_DIR) $(GTEST_FLAGS) $(COVFLAGS)

request_handler_test: $(SRC_DIR)/request_handler.cc $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $(TEST_DIR)/$@.cc -I$(SRC_DIR) $(GTEST_FLAGS) $(COVFLAGS)

echo_handler_test: $(SRC_DIR)/request_handler.cc $(SRC_DIR)/echo_handler.cc $(GMOCK_CLASSES)
	$(CXX) -o $@ $^ $(TEST_DIR)/$@.cc -I$(SRC_DIR) $(GTEST_FLAGS) $(COVFLAGS)

static_file_handler_test: $(SRC_DIR)/request_handler.cc $(SRC_DIR)/static_file_handler.cc $(SRC_DIR)/config_parser.cc $(GMOCK_CLASSES)
	$(CXX) -o $@ $^ $(TEST_DIR)/$@.cc -I$(SRC_DIR) $(GTEST_FLAGS) $(COVFLAGS)

not_found_handler_test: $(SRC_DIR)/request_handler.cc $(SRC_DIR)/not_found_handler.cc $(GMOCK_CLASSES)
	$(CXX) -o $@ $^ $(TEST_DIR)/$@.cc -I$(SRC_DIR) $(GTEST_FLAGS) $(COVFLAGS)

status_handler_test: $(SRC_DIR)/request_handler.cc $(SRC_DIR)/status_handler.cc $(SRC_DIR)/server_status_tracker.cc $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $(TEST_DIR)/$@.cc -I$(SRC_DIR) $(GTEST_FLAGS) $(COVFLAGS)

server_status_tracker_test: $(SRC_DIR)/request_handler.cc $(SRC_DIR)/server_status_tracker.cc $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $(TEST_DIR)/$@.cc -I$(SRC_DIR) $(GTEST_FLAGS) $(COVFLAGS)

reverse_proxy_handler_test: $(SRC_DIR)/request_handler.cc $(SRC_DIR)/reverse_proxy_handler.cc $(GMOCK_CLASSES)
	$(CXX) -o $@ $^ $(TEST_DIR)/$@.cc -I$(SRC_DIR) $(GTEST_FLAGS) $(COVFLAGS) -lboost_system

database_handler_test: $(SRC_DIR)/request_handler.cc $(SRC_DIR)/database_handler.cc $(SRC_DIR)/config_parser.cc $(GTEST_CLASSES)
	$(CXX) -o $@ $^ $(TEST_DIR)/$@.cc -I$(SRC_DIR) $(GTEST_FLAGS) $(COVFLAGS) $(MYSQL_LD)

gtest-all.o: $(GTEST_DIR)/src/gtest-all.cc
	$(CXX) $(GTEST_FLAGS) $(GTEST_INCL) -c $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o: $(GTEST_DIR)/src/gtest_main.cc
	$(CXX) $(GTEST_FLAGS) $(GTEST_INCL) -c $(GTEST_DIR)/src/gtest_main.cc

libgtest.a: gtest-all.o gtest_main.o
	ar -rv $@ $^

gmock-all.o : $(GMOCK_DIR)/src/gmock-all.cc
	$(CXX) $(GTEST_FLAGS) $(GMOCK_INCL) -c $(GMOCK_DIR)/src/gmock-all.cc

gmock_main.o : $(GMOCK_DIR)/src/gmock_main.cc
	$(CXX) $(GTEST_FLAGS) $(GMOCK_INCL) -c $(GMOCK_DIR)/src/gmock_main.cc

libgmock.a : gmock-all.o gtest-all.o gmock_main.o
	ar -rv $@ $^

coverage: COVFLAGS += -fprofile-arcs -ftest-coverage
coverage: Webserver_test status_handler_test server_status_tracker_test \
		  echo_handler_test static_file_handler_test not_found_handler_test \
		  reverse_proxy_handler_test database_handler_test \
		  request_handler_test config_parser_test
	./Webserver_test && gcov -s src -r Webserver.cc;
	./config_parser_test && gcov -s src -r config_parser.cc;
	./request_handler_test && gcov -s src -r request_handler.cc;
	./echo_handler_test && gcov -s src -r echo_handler.cc;
	./static_file_handler_test && gcov -s src -r static_file_handler.cc;
	./not_found_handler_test && gcov -s src -r not_found_handler.cc;
	./server_status_tracker_test && gcov -s src -r server_status_tracker.cc;
	./reverse_proxy_handler_test && gcov -s src -r reverse_proxy_handler.cc;
	./database_handler_test && gcov -s src -r database_handler.cc;

test:
	python3 $(TEST_DIR)/integration_test.py;
	python3 $(TEST_DIR)/integration_test_proxy.py;

clean:
	rm -rf *.o *.a *~ *.gch *.swp *.dSYM *.gcno *.gcda *.gcov Webserver config_parser *_test *.tar.gz

.PHONY: all clean test coverage
