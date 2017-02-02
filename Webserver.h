#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "config_parser.h"
#include <boost/asio.hpp>
#include <unordered_map>

class Webserver {

public:
    bool load_configs(NginxConfig config);
	bool parse_config(const char* file_name);
	void run_server(boost::asio::io_service& io_service);
	void session(boost::asio::ip::tcp::socket sock);
    std::vector<char> create_response(char* request, size_t request_length);
    std::string get_config(std::string attribute);

private:
	NginxConfigParser config_parser;
    NginxConfig config_out;
    unsigned short port;
    std::unordered_map<std::string, std::string> config_attributes;
};

#endif
