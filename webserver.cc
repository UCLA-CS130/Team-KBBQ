// Based on Boost library blocking_tcp_echo_server example.
// http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/echo/blocking_tcp_echo_server.cpp

#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>
#include "config_parser.h"

using boost::asio::ip::tcp;

const int max_length = 4096;

void session(tcp::socket sock) {
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

void server(boost::asio::io_service& io_service, unsigned short port) {
    // Listen on the given port
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));
    for (;;) {
        // Create a socket to accept connections on the port
        tcp::socket sock(io_service);
        acceptor.accept(sock);
        std::thread(session, std::move(sock)).detach();
    }
}

int main(int argc, char* argv[]) {
    try {
        // Need exactly one argument containing the config file.
        if (argc != 2) {
            std::cerr << "Usage: ./webserver <path to config file>\n";
            return 1;
        }

        NginxConfigParser config_parser;
        NginxConfig config_out;
        int port;

        // Try to parse the config file.
        if (config_parser.Parse(argv[1], &config_out)) {
            // Get the port number.
            port = std::stoi(config_out.statements_[0]->tokens_[1]);
        } else {
            std::cerr << "Error: Could not parse config file.\n";
            return 1;
        }

        boost::asio::io_service io_service;

        // Start the server.
        server(io_service, port);
    }
        catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}