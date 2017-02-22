#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "config_parser.h"
#include "request_handler.h"
#include <boost/asio.hpp>
#include <unordered_map>

class Webserver {
public:
    bool load_configs(NginxConfig config);
    bool parse_config(const char* file_name);
    void run_server(boost::asio::io_service& io_service);
    void session(boost::asio::ip::tcp::socket sock);
    bool syntax_error(std::shared_ptr<NginxConfigStatement> parent_statement);
    bool add_handler(std::string attribute, NginxConfig child_config, std::string handler_name);
    virtual RequestHandler* get_config(std::string attribute);
    unsigned short get_port();
    std::string buffer_to_string(const boost::asio::streambuf &buffer);

private:
    NginxConfigParser config_parser;
    NginxConfig config_out;
    unsigned short port;
    std::unordered_map<std::string, RequestHandler*> handler_map;
};

#endif
