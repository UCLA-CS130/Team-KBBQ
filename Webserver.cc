// Based on Boost library blocking_tcp_echo_server example.
// http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/echo/blocking_tcp_echo_server.cpp

#include "Webserver.h"
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>

const int max_length = 4096;

using boost::asio::ip::tcp;

bool Webserver::parse_config(const char* file_name){
	
	bool result = config_parser.Parse(file_name, &config_out);
	// Try to parse the config file.
	if (result) {
	    // Get the port number.
	    port = std::stoi(config_out.statements_[0]->tokens_[1]);
	} else {
	    std::cerr << "Error: Could not parse config file.\n";
	}

	return result;
}


void Webserver::session(tcp::socket sock) {
    try {
        for (;;) {
            char data[max_length];

            boost::system::error_code error;

            // Read the request.
            size_t length = sock.read_some(boost::asio::buffer(data), error);
            if (error == boost::asio::error::eof) {
                break; // Connection closed cleanly by peer. 
            }
            else if (error) {
                throw boost::system::system_error(error); // Some other error.
            }

            // Create the response header.
            const char* header = "HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\n";
            int header_length = std::strlen(header);

            // Send response header and echo received request.
            boost::asio::write(sock, boost::asio::buffer(header, header_length));
            boost::asio::write(sock, boost::asio::buffer(data, length));
            return;
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}

void Webserver::run_server(boost::asio::io_service& io_service) {
    // Listen on the given port
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));
    
    for (;;) {
        // Create a socket to accept connections on the port
        tcp::socket sock(io_service);
        acceptor.accept(sock);
        std::thread(&Webserver::session, this, std::move(sock)).detach();
    }
}

