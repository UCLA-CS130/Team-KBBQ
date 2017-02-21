// Based on Boost library blocking_tcp_echo_server example.
// http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/echo/blocking_tcp_echo_server.cpp

#include "request_handler.h"
#include "echo_handler.h"
#include "static_file_handler.h"
#include "Webserver.h"
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>

using boost::asio::ip::tcp;

bool Webserver::load_configs(NginxConfig config) {
    if (config.statements_.size() == 0) {
        std::cerr << "Error: Empty config.\n";
        return false;
    }

    for (size_t i = 0; i < config.statements_.size(); i++) {
        std::shared_ptr<NginxConfigStatement> parent_statement = config.statements_[i];
        NginxConfig child_config;
        bool has_child = false;

        if (parent_statement->child_block_.get()) {
            has_child = true;
            child_config = *(parent_statement->child_block_.get());
        }

        size_t size = parent_statement->tokens_.size();
        std::string first_token = "";
        std::string second_token = "";
        std::string third_token = "";

        // Check statement size
        if (size > 1 && size < 4) {
            first_token = parent_statement->tokens_[0];
            second_token = parent_statement->tokens_[1];
            if (size == 3) {
                third_token = parent_statement->tokens_[2];
            }
        }
        else {
            return syntax_error(parent_statement);
        }

        // Parse statements
        if (first_token == "port" && third_token == "") {
            if (has_child) {
                return syntax_error(parent_statement);
            }

            bool is_number = (second_token.find_first_not_of("1234567890") == std::string::npos);
            if (is_number) {
                port = std::stoi(second_token);
            }
            else {
                return syntax_error(parent_statement);
            }
        }
        else if (first_token == "path" && second_token == "/echo" && third_token == "EchoHandler") {
            if (has_child && child_config.statements_.size() == 0) {
                if (!add_handler(second_token, child_config, "EchoHandler")) {
                    return false;
                }
            }
            else {
                return syntax_error(parent_statement);
            }
        }
        else if (first_token == "path" && third_token == "StaticHandler") {
            if (has_child && child_config.statements_.size() != 0) {
                if (!add_handler(second_token, child_config, "StaticFileHandler")) {
                    return false;
                }
            }
            else {
                return syntax_error(parent_statement);
            }
        }
        else if (first_token == "default" && second_token == "NotFoundHandler" && third_token == "") {
            if (has_child && child_config.statements_.size() == 0) {
                if (!add_handler(first_token, child_config, "NotFoundHandler")) {
                    return false;
                }
            }
            else {
                return syntax_error(parent_statement);
            }
        }
        else {
            return syntax_error(parent_statement);
        }
    }

    return true;
}

bool Webserver::syntax_error(std::shared_ptr<NginxConfigStatement> parent_statement) {
    std::cerr << "Error: Invalid config syntax.\n";
    std::cerr << parent_statement->ToString(1);
    return false;
}

bool Webserver::add_handler(std::string attribute, NginxConfig child_config, const char* handler_name) {
    RequestHandler* handler = RequestHandler::CreateByName(handler_name);
    handler->Init(attribute, child_config);

    std::unordered_map<std::string, RequestHandler*>::const_iterator found = handler_map.find(attribute);
    // If mapping already exists, return false, else add to map
    if (found != handler_map.end()) {
        std::cerr << "Error: " << attribute << " is already mapped.\n";
        return false;
    } else {
        handler_map[attribute] = handler;
    }

    return true;
}

bool Webserver::parse_config(const char* file_name) {
	// Try to parse the config file.
	if (config_parser.Parse(file_name, &config_out)) {
        // Put configs into map
        return load_configs(config_out);
	}
    else {
	    std::cerr << "Error: Could not parse config file.\n";
        return false;
	}
}

RequestHandler* Webserver::get_config(std::string attribute) {
    std::unordered_map<std::string, RequestHandler*>::const_iterator found = handler_map.find(attribute);
    // Get the attribute value
    if (found != handler_map.end()) {
        return found->second;
    } else {
        return NULL;
    }
}

unsigned short Webserver::get_port() {
    return port;
}

void Webserver::session(tcp::socket sock) {
    try {
        for (;;) {
            boost::asio::streambuf request;

            // Read the request.
            boost::asio::read_until(sock, request, "\r\n\r\n");

            printf("Connected to client.\n");

            /* TODO: 
             * Parse into new Request Class
             * Call handler
             * Write Response to socket
             */
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
