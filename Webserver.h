#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "config_parser.h"
#include <boost/asio.hpp>

class Webserver {

public:
	bool parse_config(const char* file_name);
	void run_server(boost::asio::io_service& io_service);
	void session(boost::asio::ip::tcp::socket sock); 


private:
	// boost::asio::io_service io_service;
	NginxConfigParser config_parser;
    NginxConfig config_out;
    unsigned short port;
};

#endif