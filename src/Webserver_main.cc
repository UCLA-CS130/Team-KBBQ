#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include "Webserver.h"

int main(int argc, char* argv[]) {
    
    try {
        // Need exactly one argument containing the config file.
        if (argc != 2) {
            std::cerr << "Usage: ./webserver <path to config file>\n";
            return 1;
        }

        //Create server object and parse config file
        Webserver server;
        if (!server.parse_config(argv[1])) {
            std::cerr << "Error: Failed to parse config file.\n";
            return 1;
        }

        boost::asio::io_service io_service;
        // Start the server.
        server.run_server(io_service);
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}