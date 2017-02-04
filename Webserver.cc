// Based on Boost library blocking_tcp_echo_server example.
// http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/echo/blocking_tcp_echo_server.cpp

#include "Webserver.h"
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>

using boost::asio::ip::tcp;

bool Webserver::load_configs(NginxConfig config) {
    for (size_t i = 0; i < config.statements_.size(); i++) {
        std::shared_ptr<NginxConfigStatement> parent_statement = config.statements_[i];

        if (parent_statement->child_block_.get()) {
            // Recurse on child block
            load_configs(*(parent_statement->child_block_.get()));
        }

        // Parse statements
        else if (parent_statement->tokens_.size() != 2) {
            std::cerr << "Error: Invalid config syntax.\n";
            std::cerr << parent_statement->ToString(1);
            return false;
        } else {
            config_attributes[parent_statement->tokens_[0]] = parent_statement->tokens_[1];
        }
    }

    return true;
}

bool Webserver::parse_config(const char* file_name){
	// Try to parse the config file.
	if (config_parser.Parse(file_name, &config_out)) {
        // Put configs into map
        if (load_configs(config_out)) {
            std::string port_str = get_config("port");
            
            if (port_str == "") {
                std::cerr << "Error: No port found.\n";
                return false;
            }

            port = std::stoi(port_str);
        } else {
            return false;
        }

        printf("Parsed config file.\n\n");
        return true;
	}
    else {
	    std::cerr << "Error: Could not parse config file.\n";
        return false;
	}
}
 
std::string Webserver::get_config(std::string attribute){
    std::unordered_map<std::string, std::string>::const_iterator found = config_attributes.find(attribute);
    // Get the attribute value
    if (found != config_attributes.end()) {
        return found->second;
    } else {
        return "";
    }
}

void Webserver::session(tcp::socket sock) {
    try {
        for (;;) {
            boost::asio::streambuf request;

            // Read the request.
            size_t request_length = boost::asio::read_until(sock, request, "\r\n\r\n");

            printf("Connected to client.\n\n");

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

std::vector<char> Webserver::create_response(boost::asio::streambuf& request, size_t request_length) {
    std::vector<char> response;
    std::string response_header = "HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\n";
    boost::asio::streambuf::const_buffers_type req_data = request.data();

    response.insert(response.end(), response_header.begin(), response_header.end());
    response.insert(response.end(), boost::asio::buffers_begin(req_data), boost::asio::buffers_begin(req_data) + request_length);
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

