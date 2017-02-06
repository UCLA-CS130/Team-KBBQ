#ifndef REQUEST_H
#define REQUEST_H

#include <boost/asio.hpp>

class HttpRequest {
public:
    int create_request(boost::asio::streambuf& buffer);
    virtual std::string to_string();
    virtual std::string get_file();
    virtual std::string get_type();

private:
    std::string file;
    std::string buffer;
    std::string type;
};

#endif
