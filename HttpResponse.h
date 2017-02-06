#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <boost/asio.hpp>
#include <string>
#include <vector>

class HttpResponse {
public:
    static std::vector<char> bad_request_response();
    static std::vector<char> not_found_response();
    static std::vector<char> not_implemented_response();
    virtual int generate_response(std::vector<char> &response) = 0;
    virtual int send(boost::asio::ip::tcp::socket &sock) = 0;
};

class EchoResponse : public HttpResponse {
public:
    EchoResponse(std::string req_string);
    virtual int generate_response(std::vector<char> &response);
    virtual int send(boost::asio::ip::tcp::socket &sock);

private:
    std::string request;
};

class FileResponse : public HttpResponse {
public:
    FileResponse(std::string serve_directory, std::string req_file);
    std::string get_content_type(const std::string &filename);
    virtual int generate_response(std::vector<char> &response);
    virtual int send(boost::asio::ip::tcp::socket &sock);

private:
    std::string file_path;
};

#endif
