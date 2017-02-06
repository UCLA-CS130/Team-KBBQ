// Based on Boost library blocking_tcp_echo_server example.
// http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp11/echo/blocking_tcp_echo_server.cpp

#include "HttpConstants.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Webserver.h"
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>

using boost::asio::ip::tcp;

bool Webserver::load_configs(NginxConfig config, std::string parent_name, int inside_block) {
    std::string name = parent_name;
    int count = inside_block;

    for (size_t i = 0; i < config.statements_.size(); i++) {
        std::shared_ptr<NginxConfigStatement> parent_statement = config.statements_[i];

        if (parent_statement->child_block_.get()) {
            name = parent_statement->tokens_[0];
        }

        if (parent_statement->child_block_.get() && name != "" && count == 0) {
            // Recurse on child block
            bool success = load_configs(*(parent_statement->child_block_.get()), name, count + 1);
            if (!success) {
                return false;
            }
            name = "";
        }
        // Parse statements
        else if (parent_statement->tokens_.size() != 2) {
            std::cerr << "Error: Invalid config syntax.\n";
            std::cerr << parent_statement->ToString(1);
            return false;
        }
        else {
            if (name == "server") {
                server_attributes[parent_statement->tokens_[0]] = parent_statement->tokens_[1];
            }
            else if (name == "directories") {
                dir_attributes[parent_statement->tokens_[0]] = parent_statement->tokens_[1];
            }
            else {
                //ignore statements not in a child block
                ;
            }
        }
    }

    return true;
}

bool Webserver::parse_config(const char* file_name){
	// Try to parse the config file.
	if (config_parser.Parse(file_name, &config_out)) {
        // Put configs into map
        if (load_configs(config_out, "", 0)) {
            std::string port_str = get_server_config("port");
            
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

std::string Webserver::get_server_config(std::string attribute){
    std::unordered_map<std::string, std::string>::const_iterator found = server_attributes.find(attribute);
    // Get the attribute value
    if (found != server_attributes.end()) {
        return found->second;
    } else {
        return "";
    }
}

std::string Webserver::get_dir_config(std::string attribute){
    std::unordered_map<std::string, std::string>::const_iterator found = dir_attributes.find(attribute);
    // Get the attribute value
    if (found != dir_attributes.end()) {
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
            boost::asio::read_until(sock, request, "\r\n\r\n");

            printf("Connected to client.\n");

            HttpRequest processed_request;
            int status = processed_request.create_request(request);
            if (status == NOT_IMPLEMENTED) {
                boost::asio::write(sock, boost::asio::buffer(HttpResponse::not_implemented_response()));
                return;
            }
            else if (status == BAD_REQUEST) {
                boost::asio::write(sock, boost::asio::buffer(HttpResponse::bad_request_response()));
                return;
            }

            std::unique_ptr<HttpResponse> response_ptr;
            status = create_response(processed_request, response_ptr);

            if (status == NOT_FOUND) {
                boost::asio::write(sock, boost::asio::buffer(HttpResponse::not_found_response()));
                return;
            }

            response_ptr->send(sock);
            return;
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}

// TODO: REFACTOR to create_response(HttpRequest &request, std::unique_ptr<HttpResponse> resp)
// Then write resp->output() to socket
int Webserver::create_response(HttpRequest &request, std::unique_ptr<HttpResponse> &response_ptr) {
    std::string type = request.get_type();
    std::string file_name = request.get_file();

    std::string echo_url = get_server_config("echo");

    if (type == echo_url && file_name == "") {
        response_ptr = std::unique_ptr<HttpResponse>(new EchoResponse(request.to_string()));
        return 0;
    }
    else if ((dir_attributes.find(type) != dir_attributes.end()) && file_name != "") {
        std::string directory = get_dir_config(type);
        response_ptr = std::unique_ptr<HttpResponse>(new FileResponse(directory, file_name));
        return 0;
    }

    return NOT_FOUND;
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

