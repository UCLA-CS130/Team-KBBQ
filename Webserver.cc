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
    printf("Parsed config file.\n\n");

	return result;
}


void Webserver::session(tcp::socket sock) {
    try {
        for (;;) {
            char request[max_length];

            boost::system::error_code error;

            // Read the request.
            size_t request_length = sock.read_some(boost::asio::buffer(request), error);
            if (error == boost::asio::error::eof) {
                break; // Connection closed cleanly by peer. 
            }
            else if (error) {
                throw boost::system::system_error(error); // Some other error.
            }

            printf("Connected to client.\n\n");
            printf("Received GET Request:\n%s\n\n", request);

            // Create response
            std::vector<char> response = create_response(request, request_length);

            // Send response header and echo received request.
            boost::asio::write(sock, boost::asio::buffer(response));
            return;
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}

std::vector<char> Webserver::create_response(char* request, size_t request_length) {
    std::vector<char> response;
    std::string response_header = "HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\n";

    response.insert(response.end(), response_header.begin(), response_header.end());
    response.insert(response.end(), request, request + request_length);
    return response;
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

