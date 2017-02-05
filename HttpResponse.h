#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <boost/asio.hpp>
#include <string>
#include <vector>

class HttpResponse {
public:
    std::vector<char> bad_request_response();
    std::vector<char> not_found_response();
    virtual int send(boost::asio::ip::tcp::socket &sock) = 0;
};

class EchoResponse : public HttpResponse {
public:
    EchoResponse(std::string req_string);
    virtual int send(boost::asio::ip::tcp::socket &sock);

private:
    std::string request;
};

class FileResponse : HttpResponse {
public:
    FileResponse(std::string serve_directory, std::string req_file);
    std::string get_content_type(const std::string &filename);
    virtual int send(boost::asio::ip::tcp::socket &sock);

private:
    std::string file_name;
    std::string directory;
};

#endif
