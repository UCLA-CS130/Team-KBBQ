// Based on Boost library blocking_tcp_echo_server example.
// http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/echo/blocking_tcp_echo_server.cpp

#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>
#include "Webserver.h"

int main(int argc, char* argv[]) {
    
    try {
        // Need exactly one argument containing the config file.
        if (argc != 2) {
            std::cerr << "Usage: ./webserver <path to config file>\n";
            return 1;
        }

        Webserver server;
        server.parse_config(argv[1]);
        
        boost::asio::io_service io_service;
        // Start the server.
        server.run_server(io_service);
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}